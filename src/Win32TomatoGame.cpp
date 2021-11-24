#include "Win32TomatoGame.h"

namespace tomato
{
#ifdef TOM_INTERNAL

DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_PlatformFreeFileMemory)
{
	if (mem) {
		VirtualFree(mem, 0, MEM_RELEASE);
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_PlatformReadEntireFile)
{
	debug_ReadFileResult file = {};

	HANDLE fileHandle =
		CreateFileA(fileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			u32 fileSize32 = SafeTruncateUint64(fileSize.QuadPart);
			file.contents  = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (file.contents) {
				DWORD bytesRead;
				if (ReadFile(fileHandle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
					fileSize32 == bytesRead) {
					// NOTE: file read successfully
					file.contentSize = fileSize32;
				} else {
					debug_PlatformFreeFileMemory(file.contents);
					file.contents = 0;
				}
			} else {
				printf("ERROR-> Failed to read file contents!\n");
			}
		} else {
			printf("ERROR-> Failed to open file handle!\n");
		}
		CloseHandle(fileHandle);
	}
	return file;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_PlatformWriteEntireFile)
{
	bool32 success = false;

	HANDLE fileHandle = CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		if (WriteFile(fileHandle, mem, (DWORD)memorySize, &bytesWritten, 0)) {
			// NOTE: file wrote successfully
			success = (bytesWritten == memorySize);
		} else {
			printf("ERROR-> Failed to write file contents!\n");
		}
		CloseHandle(fileHandle);
	} else {
		printf("ERROR-> Failed to oepn file handle!\n");
	}
	return success;
}

#endif

namespace win32
{
//! this is a roundabout way of extracting a method out of a header...
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state* XInputGetState_ = XInputGetStateStub;

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_set_state* XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

namespace
{
// #Globals
constexpr u32 gameUpdateHz			= 60;
constexpr f32 targetSecondsPerFrame = 1.0f / (f32)gameUpdateHz;

bool g_isRunning;
bool g_pause;
const _TCHAR* g_gameDLLName = _T("TomatoGame.dll");

OffScreenBuffer g_backBuffer;
WindowDimensions g_winDims;
i64 g_perfCountFrequency;

// TODO: the sleep precision issue is keeping this above 1 frame... I think
constexpr f32 framesOfAudioLatency = (1.1f / 30) * gameUpdateHz;
IAudioClient* g_audioClient;
IAudioRenderClient* g_audioRenderClient;
IAudioClock* g_audioClock;

struct GameCode
{
	HMODULE gameCodeDLL;
	FILETIME lastWriteTimeDLL;
	Game_Update_And_Render* updateAndRender;
	Game_Get_Sound_Samples* getSoundSamples;
	bool isValid;
};

inline FILETIME
GetLastWritTime(const _TCHAR* fileName)
{
	FILETIME lastWriteTime {};

#if 0
	// NOTE: old way with a handle
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(fileName, &findData);
	if (findHandle != INVALID_HANDLE_VALUE) {
		lastWriteTime = findData.ftLastWriteTime;
		FindClose(findHandle);
	}
#endif

	// NOTE: this has no handle
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &data)) {
		lastWriteTime = data.ftLastWriteTime;
	}

	return lastWriteTime;
}

GameCode
LoadGameCode(const _TCHAR* dllName)
{
	GameCode gameCode {};
	const _TCHAR* dll_copy = _T("loaded_gamecode_copy.dll");

	gameCode.lastWriteTimeDLL = GetLastWritTime(dllName);

	CopyFile(dllName, dll_copy, FALSE);

	gameCode.gameCodeDLL = LoadLibrary(dll_copy);
	if (gameCode.gameCodeDLL) {
		gameCode.updateAndRender =
			(Game_Update_And_Render*)GetProcAddress(gameCode.gameCodeDLL, "GameUpdateAndRender");
		gameCode.getSoundSamples =
			(Game_Get_Sound_Samples*)GetProcAddress(gameCode.gameCodeDLL, "GameGetSoundSamples");
		gameCode.isValid = (gameCode.updateAndRender && gameCode.getSoundSamples);
	}

	if (!gameCode.isValid) {
		printf("Failed to load game code!\n");
		gameCode.updateAndRender = 0;
		gameCode.getSoundSamples = 0;
	} else {
		printf("Game code successfully loaded.\n");
	}

	return gameCode;
}

void
UnloadGameCode(GameCode& gameCode)
{
	if (gameCode.gameCodeDLL) {
		FreeLibrary(gameCode.gameCodeDLL);
		gameCode.updateAndRender = 0;
		gameCode.getSoundSamples = 0;
	}
	printf("Game code unloaded.");
}

void
LoadXinput()
{
	// TODO: test this on other windows version
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!XInputLibrary) {
		HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if (XInputLibrary) {
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
	} else {
		printf("ERROR->failed to load XInput!\n");
	}
}

void
InitWASAPI(i32 samplesPerSec, i32 bufSzInSamples)
{
	if (FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) {
		assert(false);
	}

	IMMDeviceEnumerator* enumerator;
	if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
								IID_PPV_ARGS(&enumerator)))) {
		assert(false);
	}

	IMMDevice* device;
	if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
		assert(false);
	}

	if (FAILED(
			device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (LPVOID*)&g_audioClient))) {
		assert(false);
	}

	WAVEFORMATEXTENSIBLE waveFmt;

	waveFmt.Format.cbSize		  = sizeof(waveFmt);
	waveFmt.Format.wFormatTag	  = WAVE_FORMAT_EXTENSIBLE;
	waveFmt.Format.wBitsPerSample = 16;
	waveFmt.Format.nChannels	  = 2;
	waveFmt.Format.nSamplesPerSec = (DWORD)samplesPerSec;
	waveFmt.Format.nBlockAlign =
		(WORD)(waveFmt.Format.nChannels * waveFmt.Format.wBitsPerSample / 8);
	waveFmt.Format.nAvgBytesPerSec = waveFmt.Format.nSamplesPerSec * waveFmt.Format.nBlockAlign;
	waveFmt.Samples.wValidBitsPerSample = 16;
	waveFmt.dwChannelMask				= KSAUDIO_SPEAKER_STEREO;
	waveFmt.SubFormat					= KSDATAFORMAT_SUBTYPE_PCM;

	REFERENCE_TIME bufDur =
		10000000ULL * bufSzInSamples / samplesPerSec;  // buffer size in 100 nanoseconds
	if (FAILED(g_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST,
										 bufDur, 0, &waveFmt.Format, nullptr))) {
		assert(false);
	}

	if (FAILED(g_audioClient->GetService(IID_PPV_ARGS(&g_audioRenderClient)))) {
		assert(false);
	}

	UINT32 soundFrmCnt;
	if (FAILED(g_audioClient->GetBufferSize(&soundFrmCnt))) {
		assert(false);
	}

	if (FAILED(g_audioClient->GetService(IID_PPV_ARGS(&g_audioClock)))) {
		assert(false);
	}

	// Check if we got what we requested (better would to pass this value back
	// as real buffer size)
	assert(bufSzInSamples <= (i32)soundFrmCnt);
}
void
FillSoundBuffer(SoundOutput& soundOutput, i32 samplesToWrite, GameSoundOutputBuffer& sourceBuffer)
{
	{
		BYTE* soundBufDat;
		if (SUCCEEDED(g_audioRenderClient->GetBuffer((UINT32)samplesToWrite, &soundBufDat))) {
			i16* sourceSample = sourceBuffer.samples;
			i16* destSample	  = (i16*)soundBufDat;
			for (szt i = 0; i < samplesToWrite; ++i) {
				*destSample++ = *sourceSample++;
				*destSample++ = *sourceSample++;
				++soundOutput.runningSampleInd;
			}

			g_audioRenderClient->ReleaseBuffer((UINT32)samplesToWrite, 0);
		}
	}
}

WindowDimensions
GetWindowDimension(HWND hWnd)
{
	RECT clientRect;
	WindowDimensions winDim;
	GetClientRect(hWnd, &clientRect);
	winDim.width  = clientRect.right - clientRect.left;
	winDim.height = clientRect.bottom - clientRect.top;
	return winDim;
}

void
ResizeDIBSection(OffScreenBuffer& buffer, i32 width, i32 height)
{
	// TODO: bulletproof this
	// maybe don't free first, free after, then free first if that fails

	if (buffer.mem) {
		VirtualFree(buffer.mem, 0, MEM_RELEASE);
	}
	buffer.width	 = width;
	buffer.height	 = height;
	buffer.bytPerPix = 4;

	buffer.info.bmiHeader.biSize		= sizeof(buffer.info.bmiHeader);
	buffer.info.bmiHeader.biWidth		= width;
	buffer.info.bmiHeader.biHeight		= -height;
	buffer.info.bmiHeader.biPlanes		= 1;
	buffer.info.bmiHeader.biBitCount	= 32;
	buffer.info.bmiHeader.biCompression = BI_RGB;

	i32 bytesPerPixel	 = 4;
	i32 bitmapMemorySize = (width * height) * bytesPerPixel;
	buffer.mem = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	buffer.pitch = width * buffer.bytPerPix;
}

void
DisplayBufferInWindow(HDC hdc, OffScreenBuffer& buffer, i32 x, i32 y, i32 width, i32 height)
{
	// NOTE: this is matches the windows dimensions
	StretchDIBits(hdc, 0, 0, width, height, 0, 0, g_winDims.width, g_winDims.height, buffer.mem,
				  &buffer.info, DIB_RGB_COLORS, SRCCOPY);
}

void
ProcessKbdMsg(GameButtonState& newState, bool32 isDown)
{
	if (newState.endedDown != (isDown != 0)) {
		newState.endedDown = isDown;
		++newState.halfTransitionCount;
	}
}

void
ProcessXInputDigitalButton(DWORD XInputButtonState, GameButtonState& oldState, DWORD buttonBit,
						   GameButtonState& newState)
{
	newState.endedDown			 = ((XInputButtonState & buttonBit) == buttonBit);
	newState.halfTransitionCount = (oldState.endedDown != newState.endedDown) ? 1 : 0;
}

void
DoControllerInput(GameInput& oldInput, GameInput& newInput, HWND hWnd)
{
	// mouse cursor
	POINT mouseP;
	GetCursorPos(&mouseP);
	ScreenToClient(hWnd, &mouseP);
	newInput.mouseX = mouseP.x;
	newInput.mouseY = mouseP.y;
	newInput.mouseZ = 0;

	// mouse buttons
	ProcessKbdMsg(newInput.mouseButtons[0], ::GetKeyState(VK_LBUTTON) & (1 << 15));
	ProcessKbdMsg(newInput.mouseButtons[1], ::GetKeyState(VK_RBUTTON) & (1 << 15));
	ProcessKbdMsg(newInput.mouseButtons[2], ::GetKeyState(VK_MBUTTON) & (1 << 15));

	// Controller
	// poll the input device
	i32 maxControllerCount = XUSER_MAX_COUNT;
	if (maxControllerCount > 4) {
		maxControllerCount = 4;
	}

	for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {
		GameControllerInput& oldController = oldInput.controllers[controllerIndex];
		GameControllerInput& newController = newInput.controllers[controllerIndex];

		XINPUT_STATE controllerState;
		if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
			//! the controller is plugged in
			XINPUT_GAMEPAD& pad = controllerState.Gamepad;

			newController.isConnnected = true;

			// NOTE: this is hardcoded for convenience
			// newController.isAnalog		= oldController->isAnalog;
			newController.isAnalog = true;

			//  no rmal stick input
			auto normalize = [](SHORT val) {
				if (val < 0)
					return (f32)val / 32768.0f;
				else
					return (f32)val / 32767.0f;
			};

			f32 sLX			   = normalize(pad.sThumbLX);
			f32 sLY			   = normalize(pad.sThumbLY) * -1.0f;
			f32 sRX			   = normalize(pad.sThumbRX);
			f32 sRY			   = normalize(pad.sThumbRY) * -1.0f;
			newController.minX = newController.maxX = newController.endLX = sLX;
			newController.minY = newController.maxY = newController.endLY = sLY;

			ProcessXInputDigitalButton(pad.wButtons, oldController.dpad_up, XINPUT_GAMEPAD_DPAD_UP,
									   newController.dpad_up);
			ProcessXInputDigitalButton(pad.wButtons, oldController.dpad_right,
									   XINPUT_GAMEPAD_DPAD_RIGHT, newController.dpad_right);
			ProcessXInputDigitalButton(pad.wButtons, oldController.dpad_down,
									   XINPUT_GAMEPAD_DPAD_DOWN, newController.dpad_down);
			ProcessXInputDigitalButton(pad.wButtons, oldController.dpad_left,
									   XINPUT_GAMEPAD_DPAD_LEFT, newController.dpad_left);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_A, XINPUT_GAMEPAD_A,
									   newController.button_A);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_B, XINPUT_GAMEPAD_B,
									   newController.button_B);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_X, XINPUT_GAMEPAD_X,
									   newController.button_X);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_Y, XINPUT_GAMEPAD_Y,
									   newController.button_Y);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_RB,
									   XINPUT_GAMEPAD_RIGHT_SHOULDER, newController.button_RB);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_LB,
									   XINPUT_GAMEPAD_LEFT_SHOULDER, newController.button_LB);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_back, XINPUT_GAMEPAD_BACK,
									   newController.button_back);
			ProcessXInputDigitalButton(pad.wButtons, oldController.button_start,
									   XINPUT_GAMEPAD_START, newController.button_start);

			// NOTE: Not currently used
			// float dpadStickRX = pad.sThumbRX;
			// float dpadStickRY = pad.sThumbRY;
			// bool dPadStart = (pad->wButtons & XINPUT_GAMEPAD_START);
			// bool dPadBack = (pad->wButtons & XINPUT_GAMEPAD_BACK);
			// bool dPadRB = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			// bool dPadLB = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			// bool dPadR3 = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
			// bool dPadL3 = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
			// unsigned _TCHAR dPadRT = pad->bRightTrigger;
			// unsigned _TCHAR dPadLT = pad->bLeftTrigger;
		}
	}
}

inline LARGE_INTEGER
GetWallClock()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time;
}

inline f32
GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
	f32 seconds = f32(end.QuadPart - start.QuadPart) / f32(g_perfCountFrequency);
	return seconds;
}

#if 0
void
debug_DrawVerticalLine(OffScreenBuffer& backBuffer, i32 x, i32 top, i32 bot, u32 color)
{
	u8* pixel = (u8*)backBuffer.mem + x * backBuffer.bytPerPix + top * backBuffer.pitch;
	for (i32 y = top; y < bot; ++y) {
		*(u32*)pixel = color;
		pixel += backBuffer.pitch;
	}
}

void
debug_SyncDisplay(OffScreenBuffer& backBuffer, SoundOutput& soundOutput,
				  debug_SoundTimeMarker* debug_markerArr, szt debug_markerArrSz,
				  szt debug_markerInd, f32 targetSecondsPerFrame)

{
	i32 padX = 16;
	i32 padY = 16;

	i32 topPlay	 = padY;
	i32 botPlay	 = backBuffer.height - (backBuffer.height - 50 - topPlay);
	i32 topWrite = botPlay;
	i32 botWrite = backBuffer.height - (backBuffer.height - 50 - botPlay);
	f32 c		 = f32(backBuffer.width) / f32(soundOutput.secondaryBufSz);

	auto drawSoundBufferMarker = [&](DWORD cursor, i32 top, i32 bot, u32 color) {
		// assert(cursor < soundOutput.secondaryBufSz);
		i32 x = padX + i32(c * (f32)cursor);
		debug_DrawVerticalLine(backBuffer, x, top, bot, color);
	};

	for (szt i {}; i < debug_markerArrSz; ++i) {
		drawSoundBufferMarker(debug_markerArr[i].playCursor, topPlay, botPlay, 0xFFFFFFFF);
		drawSoundBufferMarker(debug_markerArr[i].writeCursor, topWrite, botWrite, 0xFFFF0000);
	}
}

#endif

// ===============================================================================================
// #PLAYBACK
// ===============================================================================================

void
GetInputFilePath(Win32State& state, bool32 isInputStream)
{
	int x = 0;
}

ReplayBuffer&
GetReplayBuffer(Win32State& state, szt index)
{
	assert(index < ArrayCount(state.replayBuffers));
	return state.replayBuffers[index];
}

void
BeginRecordingInput(Win32State& state, i32 inputRecordingInd)
{
	auto& replayBuf = GetReplayBuffer(state, inputRecordingInd);
	if (replayBuf.memBlock) {
		printf("Recording...\n");
		state.inputRecordingInd = inputRecordingInd;

		_TCHAR fileName[512];
		_stprintf_s(fileName, 512, _T("replay_%d_input.ti"), inputRecordingInd);
		// const _TCHAR* fileName = _T("replay_1_input.ti");
		state.recordingHandle = CreateFile(fileName, GENERIC_WRITE | GENERIC_READ, NULL, NULL,
										   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#if 0
		// dump the whole game state to a file
		LARGE_INTEGER filePos;
		filePos.QuadPart = state.totalSz;
		SetFilePointerEx(state.recordingHandle, filePos, 0, FILE_BEGIN);
#endif

		CopyMemory(replayBuf.memBlock, state.gameMemoryBlock, state.totalSz);
	}
}

void
EndRecordingInput(Win32State& state)
{
	printf("Recording ended.\n");
	CloseHandle(state.recordingHandle);
	state.inputRecordingInd = 0;
}

void
BeginInputPlayBack(Win32State& state, i32 inputPlaybackIndex)
{
	auto& replayBuf = GetReplayBuffer(state, inputPlaybackIndex);
	if (replayBuf.memBlock) {
		printf("Input Playback started...\n");
		state.inputPlayBackInd = inputPlaybackIndex;

		_TCHAR fileName[512];
		_stprintf_s(fileName, 512, _T("replay_%d_input.ti"), inputPlaybackIndex);
		state.playBackHandle =
			CreateFile(fileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#if 0
		// get the whole game state from file
		LARGE_INTEGER filePos;
		filePos.QuadPart = state.totalSz;
		SetFilePointerEx(state.playBackHandle, filePos, 0, FILE_BEGIN);
#endif

		CopyMemory(state.gameMemoryBlock, replayBuf.memBlock, state.totalSz);
	}
}

void
EndInputPlayback(Win32State& state)
{
	printf("Input playback ended.\n");
	CloseHandle(state.playBackHandle);
	state.inputPlayBackInd = 0;
}

void
RecordInput(Win32State& state, GameInput& newInput)
{
	DWORD bytesWritten;
	WriteFile(state.recordingHandle, &newInput, sizeof(newInput), &bytesWritten, 0);
}

void
PlayBackInput(Win32State& state, GameInput& newInput)
{
	DWORD bytesRead;
	if (ReadFile(state.playBackHandle, &newInput, sizeof(newInput), &bytesRead, 0)) {
		if (bytesRead == 0) {
			// NOTE: hit end of stream, go back to begining;
			i32 playInd = state.inputPlayBackInd;
			EndInputPlayback(state);
			BeginInputPlayBack(state, playInd);
			ReadFile(state.playBackHandle, &newInput, sizeof(newInput), &bytesRead, 0);
		}
	}
}

void
ProcessPendingMSG(Win32State& state, GameInput& input)
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
			case WM_QUIT: g_isRunning = false; break;
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP: {
				u32 VKCode	 = (u32)msg.wParam;
				bool wasDown = ((msg.lParam & (1 << 30)) != 0);
				bool isDown	 = ((msg.lParam & (1 << 29)) == 0);
				if (wasDown != isDown) {
					switch (VKCode) {
						case VK_ESCAPE: g_isRunning = false; break;
#ifdef TOM_INTERNAL
						case 'P': {
							if (isDown) {
								g_pause = !g_pause;
							}
						} break;
						case 'L': {
							if (isDown) {
								if (state.inputPlayBackInd == 0) {
									if (state.inputRecordingInd == 0) {
										BeginRecordingInput(state, 1);
									} else {
										EndRecordingInput(state);
										BeginInputPlayBack(state, 1);
									}
								} else {
									EndInputPlayback(state);
								}
							}
						} break;
#endif
						default: break;
					}
				}
			} break;
			default: {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} break;
		}
	}
}

}  // namespace
//==================================================================================

LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (msg) {
		case WM_SIZE: {
			g_winDims = GetWindowDimension(hWnd);
			ResizeDIBSection(g_backBuffer, g_winDims.width, g_winDims.height);
		} break;
		case WM_DESTROY: {
			g_isRunning = false;
		} break;
		case WM_CLOSE: {
			g_isRunning = false;
			PostQuitMessage(0);
		} break;
		case WM_ACTIVATEAPP: break;
		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(hWnd, &paint);
			i32 x			  = paint.rcPaint.left;
			i32 y			  = paint.rcPaint.right;
			i32 height		  = paint.rcPaint.bottom - paint.rcPaint.top;
			i32 width		  = paint.rcPaint.right - paint.rcPaint.left;
			DisplayBufferInWindow(deviceContext, g_backBuffer, x, y, width, height);
			PatBlt(deviceContext, x, y, width, height, WHITENESS);
			EndPaint(hWnd, &paint);
		} break;
		default:
			//            OutPutDebugStringA("default\n");
			result = DefWindowProc(hWnd, msg, wParam, lParam);
			break;
	}
	return result;
}

// ===============================================================================================
// #START
// ===============================================================================================

i32
Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, i32 nShowCmd)
{
	// Init
	auto console = new Console();
	assert(console);
	printf("Starting...\n");

	Win32State state {};

	DWORD exePathLen = GetModuleFileNameA(NULL, state.exePath, sizeof(state.exePath));
	printf("exe path %s\n", state.exePath);

	LARGE_INTEGER perfCountFrequencyResult;
	QueryPerformanceFrequency(&perfCountFrequencyResult);
	g_perfCountFrequency = perfCountFrequencyResult.QuadPart;

	auto gameCode = LoadGameCode(g_gameDLLName);
	LoadXinput();

	WNDCLASS wndCls = {};  // should init to 0n

	ResizeDIBSection(g_backBuffer, 1280, 720);

	// TODO: install assets eventuallly
	const _TCHAR* iconPath = _T("C:\\dev\\TomatoGame\\assets\\icon\\tomato.ico");
	auto iconBg			   = (HICON)(LoadImage(NULL, iconPath, IMAGE_ICON, 0, 0,
											   LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));

	// console->setIcon(iconBg);

	wndCls.style		 = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndCls.lpfnWndProc	 = WndProc;
	wndCls.hInstance	 = hInstance;
	wndCls.hIcon		 = iconBg;
	wndCls.lpszClassName = _T("TomatoWinCls");

	if (!RegisterClass(&wndCls)) {
		printf("ERROR--> Failed to register window class!\n");
		assert(false);
		return 0;
	}

	HWND hWnd = CreateWindowEx(0, wndCls.lpszClassName, _T("TomatoGame"),
							   WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
							   CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		printf("Failed to create window!\n");
		assert(hWnd);
		return 0;
	}

	// NOTE: Set the windows schedule granularity to 1ms
	// so sleep will be more granular
	UINT desiredSchedulerMS = 1;
	bool32 sleepIsGranular	= (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);

	HDC deviceContext	   = GetDC(hWnd);
	g_isRunning			   = true;
	g_pause				   = false;
	g_backBuffer.bytPerPix = 4;

	i32 monitorRefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
	printf("Monitor Refresh Rate: %d\n", monitorRefreshRate);

	SoundOutput soundOutput	   = {};
	soundOutput.samplesPerSec  = 48000;
	soundOutput.bytPerSample   = sizeof(i16) * 2;
	soundOutput.secondaryBufSz = soundOutput.samplesPerSec;
	soundOutput.latencySampleCnt =
		i32(framesOfAudioLatency * f32(soundOutput.samplesPerSec / (f32)gameUpdateHz));

	InitWASAPI(soundOutput.samplesPerSec, soundOutput.secondaryBufSz);
	g_audioClient->Start();

	// TODO: Pool with bitmap VirtualAlloc
	i16* samples =
		(i16*)VirtualAlloc(0, soundOutput.secondaryBufSz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#ifdef TOM_INTERNAL
	LPVOID baseAddress = (LPVOID)Terabytes((u64)2);
#else
	LPVOID baseAddress = 0;
#endif

	GameMemory gameMemory					 = {};
	gameMemory.permanentStorageSize			 = Megabytes(64);
	gameMemory.transientStorageSize			 = Gigabytes(1);
	gameMemory.debug_platformFreeFileMem	 = debug_PlatformFreeFileMemory;
	gameMemory.debug_platfromReadEntireFile	 = debug_PlatformReadEntireFile;
	gameMemory.debug_platformWriteEntireFile = debug_PlatformWriteEntireFile;

	state.totalSz = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
	// TODO: use large pages
	state.gameMemoryBlock =
		VirtualAlloc(baseAddress, state.totalSz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory.permanentStorage = state.gameMemoryBlock;

	gameMemory.transientStorage =
		((u8*)gameMemory.permanentStorage + gameMemory.permanentStorageSize);

	// mapping memory to file
	for (i32 replayInd {}; replayInd < ArrayCount(state.replayBuffers); ++replayInd) {
		ReplayBuffer& replayBuf = state.replayBuffers[replayInd];
		_stprintf_s(replayBuf.fileName, sizeof(_TCHAR) * 512, _T("replay_%d_state.ti"), replayInd);

		replayBuf.fileHandle = CreateFile(replayBuf.fileName, GENERIC_WRITE | GENERIC_READ, NULL,
										  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		LARGE_INTEGER maxSz;
		maxSz.QuadPart	 = state.totalSz;
		replayBuf.memMap = CreateFileMapping(replayBuf.fileHandle, NULL, PAGE_READWRITE,
											 maxSz.HighPart, maxSz.LowPart, NULL);
		DWORD error		 = GetLastError();
		replayBuf.memBlock =
			MapViewOfFile(replayBuf.memMap, FILE_MAP_ALL_ACCESS, 0, 0, state.totalSz),
		assert(replayBuf.memBlock);
	}

	GameInput input[2]	= {};
	GameInput& newInput = input[0];
	GameInput& oldInput = input[1];

#ifdef TOM_INTERNAL
	debug_SoundTimeMarker debug_markerArr[gameUpdateHz / 2] {};
	szt debug_curTimeMarkerInd {};
#endif

	LARGE_INTEGER lastCounter = GetWallClock();
	u64 lastCycleCount		  = __rdtsc();

	bool isSoundValid	  = true;
	bool isGameCodeLoaded = true;
	DWORD lastPlayCursor {};
	DWORD lastWriteCursor {};

	// =============================================================================================
	// #MAIN LOOP
	// =============================================================================================
	while (g_isRunning) {
		DoControllerInput(oldInput, newInput, hWnd);
		ProcessPendingMSG(state, newInput);

		auto dllWriteTime = GetLastWritTime(g_gameDLLName);
		if (CompareFileTime(&dllWriteTime, &gameCode.lastWriteTimeDLL)) {
			UnloadGameCode(gameCode);
			gameCode = LoadGameCode(g_gameDLLName);
			printf("New Game Code loaded!\n");
		}

		// NOTE: temp program exit from controller
		if (newInput.controllers->button_back.endedDown) {
			g_isRunning = false;
		}

		// #Sound

		REFERENCE_TIME latency {};
		if (SUCCEEDED(g_audioClient->GetStreamLatency(&latency))) {
		} else {
			printf("ERROR--> Failed to get audio latency\n");
		}

		i32 samplesToWrite = 0;
		UINT32 soundPadSz;
		if (SUCCEEDED(g_audioClient->GetCurrentPadding(&soundPadSz))) {
			i32 maxSampleCnt = i32(soundOutput.secondaryBufSz - soundPadSz);
			samplesToWrite	 = i32(soundOutput.latencySampleCnt - soundPadSz);
			if (samplesToWrite < 0) samplesToWrite = 0;
			// assert(samplesToWrite < maxSampleCnt);
		}

		GameSoundOutputBuffer soundBuf {};
		soundBuf.samplesPerSecond = soundOutput.samplesPerSec;
		soundBuf.sampleCount	  = samplesToWrite;
		soundBuf.samples		  = samples;

		// video
		GameOffscreenBuffer buffer = {};
		buffer.mem				   = g_backBuffer.mem;
		buffer.width			   = g_backBuffer.width;
		buffer.height			   = g_backBuffer.height;
		buffer.bytesPerPix		   = 4;
		buffer.pitch			   = g_backBuffer.pitch;

		if (state.inputRecordingInd) {
			RecordInput(state, newInput);
		}
		if (state.inputPlayBackInd) {
			PlayBackInput(state, newInput);
		}

		// null check for stub sections
		isGameCodeLoaded = gameCode.updateAndRender && gameCode.getSoundSamples;
		// isGameCodeLoaded = false;

		// NOTE: dummy thread context, for now
		ThreadContext thread {};

		if (isGameCodeLoaded) {
			gameCode.updateAndRender(thread, gameMemory, *input, buffer, soundBuf);
			gameCode.getSoundSamples(thread, gameMemory, soundBuf);
		}

		FillSoundBuffer(soundOutput, samplesToWrite, soundBuf);

		// clock stuffs
		auto workCounter	   = GetWallClock();
		f32 workSecondsElapsed = GetSecondsElapsed(lastCounter, workCounter);

		f32 secondsElapsedForFrame = workSecondsElapsed;
		if (secondsElapsedForFrame < targetSecondsPerFrame) {
			if (sleepIsGranular) {
				auto sleepMs = DWORD(1000.f * (targetSecondsPerFrame - secondsElapsedForFrame));
				if (sleepMs > 0) {
					Sleep(sleepMs);
				}
			}
			f32 testSecondsElapsedForFrame = GetSecondsElapsed(lastCounter, GetWallClock());
			while (secondsElapsedForFrame < targetSecondsPerFrame) {
				secondsElapsedForFrame = GetSecondsElapsed(lastCounter, GetWallClock());
			}
		} else {
			printf("WARNING--> missed frame timing!!!\n");
		}

		auto endCounter = GetWallClock();
		f32 msPerFrame	= 1000.f * GetSecondsElapsed(lastCounter, endCounter);
#if 0

		// this draws a a line based on the curretn fps
		static i32 frmCnt {};
		static i32 x;

		++frmCnt;
		if (!g_pause) x = i32((f32(frmCnt % 30) / 30.f) * f32(g_backBuffer.width));
		debug_DrawVerticalLine(g_backBuffer, x, 10, 50, 0xFFFFFFFF);
#endif

		lastCounter = endCounter;

		// NOTE: this is debug code
		DisplayBufferInWindow(deviceContext, g_backBuffer, 0, 0, g_winDims.width, g_winDims.height);

		DWORD playCursor;
		DWORD writeCursor;

		UINT64 posFreq;
		UINT64 posUnits;

		g_audioClock->GetFrequency(&posFreq);
		g_audioClock->GetPosition(&posUnits, 0);

		playCursor =
			(DWORD)(soundOutput.samplesPerSec * posUnits / posFreq) % soundOutput.samplesPerSec;
		writeCursor =
			(DWORD)(soundOutput.samplesPerSec * posUnits / posFreq) % soundOutput.samplesPerSec +
			soundPadSz;
		if (writeCursor > soundOutput.secondaryBufSz) {
			writeCursor -= soundOutput.secondaryBufSz;
		}

#ifdef TOM_INTERNAL

		if (!g_pause) {
			debug_markerArr[debug_curTimeMarkerInd].playCursor	= playCursor;
			debug_markerArr[debug_curTimeMarkerInd].writeCursor = writeCursor;

			++debug_curTimeMarkerInd;
			if (debug_curTimeMarkerInd > ArrayCount(debug_markerArr)) debug_curTimeMarkerInd = 0;
		}
#endif

		// f64 FPS	 = (f64)gs_perfCountFrequency / (f64)counterElapsed;
		// f64 MCPF = (f64)(cyclesElapsed / (1000.f * 1000.f));

		GameInput& temp = newInput;
		newInput		= oldInput;
		oldInput		= temp;

		u64 endCycleCount = __rdtsc();
		u64 cyclesElapsed = endCycleCount - lastCycleCount;
		lastCycleCount	  = endCycleCount;
	}

	return 0;
}

}  // namespace win32
}  // namespace tomato

//========================================================================================
// ENTRY POINT
//========================================================================================

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement				   = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Entry point
int WINAPI
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
		 _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr)) return 1;

	i32 ecode = tomato::win32::Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	CoUninitialize();

	return ecode;
}
