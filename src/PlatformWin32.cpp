#include "platformWin32.hpp"

// ===============================================================================================
// #GLOBALS
// ===============================================================================================
namespace
{
constexpr u32 gGameUpdateHertz       = 60;
constexpr f32 gTargetFramesPerSecond = 1.f / scast(f32, gGameUpdateHertz);

bool running;
bool gPause;

WINDOWPLACEMENT gWinPos   = { sizeof(gWinPos) };
const TCHAR *gGameDllName = T("TomatoGame.Dll");
bool gDebugShowCursor;

OffscreenBuffer gBackBuffer;
WindowDims gWinDim;
s64 gPerformanceCounterFrequency;

// TODO: the sleep precision issue is keeping this above 1 frame... I think
constexpr f32 gFramesOfAudioLatency = (1.1f / 30.f) * scast(f32, gGameUpdateHertz);
IAudioClient *gAudioClient;
IAudioRenderClient *gAudioRenderClient;
IAudioClock *gAudioClock;

//! this is a roundabout way of extracting a method out of a header...
#define XINPUTGetState(Name) DWORD WINAPI name(DWORD dwUserIndex, XINPUTState *pState)
typedef XINPUTGetState(XinputGetState);
XINPUTGetState(XinputGetState)
{
    return (ERRORDeviceNotConnected);
}
xinputGetState *XInputGetState { _xinputGetState };

#define XINPUTSetState(Name) DWORD WINAPI name(DWORD dwUserIndex, XINPUTVibration *pVibration)
typedef XINPUTSetState(XinputSetState);
XINPUTSetState(XinputSetState)
{
    return (ERRORDeviceNotConnected);
}
xinputSetState *XInputSetState { _xinputSetState };

#define XInputGetState XInputGetState
#define XInputSetState XInputSetState

#ifdef TOMInternal
DEBUGPlatformFreeFileMemory(DebugPlatformFreeFileMemory)
{
    if (memory) {
        VirtualFree(memory, 0, MEMRelease);
    }
}

DEBUGPlatformReadEntireFile(DebugPlatformReadEntireFile)
{
    DebugReadFileResult file = {};

    HANDLE fileHandle { CreateFileA(fileName, GENERICRead, 0, 0, OPENExisting, FILEAttributeNormal,
                                    0) };
    if (fileHandle != INVALIDHandleValue) {
        LARGEInteger fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize)) {
            u32 fileSize32 = safeTruncateU32ToU64(Filesize.Quadpart);
            file.contents  = VirtualAlloc(0, fileSize32, MEMReserve | MEMCommit, PAGEReadwrite);
            if (file.contents) {
                DWORD bytesRead;
                if (ReadFile(fileHandle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
                    fileSize32 == bytesRead) {
                    file.contentSize = fileSize32;
                } else {
                    DebugPlatformFreeFileMemory(Thread, file.contents);
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

DEBUGPlatformWriteEntireFile(DebugPlatformWriteEntireFile)
{
    b32 success {};

    HANDLE fileHandle = CreateFileA(fileName, GENERICWrite, 0, 0, CREATEAlways, 0, 0);
    if (fileHandle != INVALIDHandleValue) {
        DWORD bytesWritten;
        if (WriteFile(fileHandle, memory, (DWORD)memorySize, &bytesWritten, 0)) {
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

void
toggleFullscreen(Hwnd hwnd)
{
    DWORD dwStyle = scast(DWORD, GetWindowLong(hwnd, GWLStyle));
    if (dwStyle & WSOverlappedwindow) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &gWinPos) &&
            GetMonitorInfo(MonitorFromWindow(hwnd, MONITORDefaulttoprimary), &mi)) {
            SetWindowLong(hwnd, GWLStyle, dwStyle & ~WSOverlappedwindow);
            SetWindowPos(hwnd, HWNDTop, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWPNoownerzorder | SWPFramechanged);
        }
    } else {
        SetWindowLong(hwnd, GWLStyle, dwStyle | WSOverlappedwindow);
        SetWindowPlacement(hwnd, &gWinPos);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWPNomove | SWPNosize | SWPNozorder | SWPNoownerzorder | SWPFramechanged);
    }
}

FILETIME
getLastWriteTime(Const TCHAR *fileName)
{
    FILETIME lastWriteTime = {};

#if 0
    // NOTE: old way with a handle
    WIN32FindData findData;
    HANDLE findHandle = FindFirstFile(fileName, &findData);
    if (findHandle != INVALIDHandleValue) {
        lastWriteTime = findData.ftLastWriteTime;
        FindClose(findHandle);
    }
#endif

    // NOTE: this has no handle
    WIN32FileAttributeData data;
    if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &data)) {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

GameCode
loadGameCode(Const TCHAR *dllName)
{
    GameCode code        = {};
    const TCHAR *dllCopy = T("LoadedGamecodeCopy.Dll");

    code.lastWriteTime_DLL = get_lastWriteTime(dll_name);

    CopyFile(dllName, dllCopy, FALSE);

    code.gameCodeDll = LoadLibrary(dllCopy);
    if (code.gameCodeDll) {
        code.updateAndRender =
            (gameUpdateAndRenderStub *)GetProcAddress(code.gameCodeDll, "gameUpdateAndRender");
        code.getSoundSamples =
            (gameGetSoundSamplesStub *)GetProcAddress(code.gameCodeDll, "gameGetSoundSamples");
        code.isValid = (code.updateAndRender && code.getSoundSamples);
    }

    if (!code.isValid) {
        printf("Failed to load game code!\n");
        code.updateAndRender = 0;
        code.getSoundSamples = 0;
    } else {
        printf("Game code successfully loaded.\n");
    }

    return code;
}

void
unload_game_code(GameCode &gameCode)
{
    if (game_code.gameCodeDll) {
        FreeLibrary(game_code.gameCodeDll);
        game_code.updateAndRender = 0;
        game_code.getSoundSamples = 0;
    }
    printf("Game code unloaded.");
}

void
loadXinput()
{
    // TODO: test this on other windows version
    HMODULE XInputLibrary = LoadLibraryA("xinput14.Dll");
    if (!XInputLibrary) {
        HMODULE XInputLibrary = LoadLibraryA("xinput13.Dll");
    }
    if (XInputLibrary) {
        XInputGetState = (xinputGetState *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinputSetState *)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        printf("ERROR->failed to load XInput!\n");
    }
}

void
initWasapi(S32 samplesPerSecond, s32 bufferSizeInSamples)
{
    if (FAILED(CoInitializeEx(0, COINITSpeedOverMemory))) {
        INVALIDCodePath;
    }

    IMMDeviceEnumerator *enumerator = {};
    if (FAILED(CoCreateInstance(Uuidof(Mmdeviceenumerator), nullptr, CLSCTXAll,
                                IIDPpvArgs(&Enumerator)))) {
        INVALIDCodePath;
    }

    IMMDevice *device = {};
    if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        INVALIDCodePath;
    }

    if (FAILED(device->Activate(Uuidof(Iaudioclient), CLSCTXAll, NULL, (LPVOID *)&gAudioClient))) {
        INVALIDCodePath;
    }

    WAVEFORMATEXTENSIBLE waveFormat  = {};
    waveFormat.Format.cbSize         = sizeof(waveFormat);
    waveFormat.Format.wFormatTag     = WAVEFormatExtensible;
    waveFormat.Format.wBitsPerSample = 16;
    waveFormat.Format.nChannels      = 2;
    waveFormat.Format.nSamplesPerSec = scast(DWORD, samplesPerSecond);
    waveFormat.Format.nBlockAlign =
        (WORD)(waveFormat.Format.nChannels * waveFormat.Format.wBitsPerSample / 8);
    waveFormat.Format.nAvgBytesPerSec =
        waveFormat.Format.nSamplesPerSec * waveFormat.Format.nBlockAlign;
    waveFormat.Samples.wValidBitsPerSample = 16;
    waveFormat.dwChannelMask               = KSAUDIOSpeakerStereo;
    waveFormat.SubFormat                   = KSDATAFORMATSubtypePcm;

    // buffer size in 100 nanoseconds
    REFERENCETime bufferDuration =
        scast(REFERENCETime, 10000000ULL * bufferSizeInSamples / samplesPerSecond);

    if (FAILED(gAudioClient->Initialize(AudclntSharemodeShared, AUDCLNTStreamflagsNopersist,
                                        bufferDuration, 0, &waveFormat.Format, nullptr))) {
        TomAssert(false);
    }

    if (FAILED(gAudioClient->Getservice(IidPpvArgs(&GAudioRenderClient)))) {
        TomAssert(false);
    }

    UINT32 soundFrameCnt;
    if (FAILED(gAudioClient->Getbuffersize(&SoundFrameCnt))) {
        TomAssert(false);
    }

    if (FAILED(gAudioClient->Getservice(IidPpvArgs(&GAudioClock)))) {
        TomAssert(false);
    }

    // Check if we got what we requested (better would to pass this value back
    // as real buffer size)
    TomAssert(bufferSizeInSamples <= scast(s32, soundFrameCnt));
}

void
fillSoundBuffer(SoundOutput &soundOutput, s32 samplesToWrite, GameSoundOutputBuffer &sourceBuffer)
{
    {
        BYTE *soundBufDat;
        if (SUCCEEDED(gAudioRenderClient->Getbuffer(Scast(Uint32, samplesToWrite), &soundBufDat))) {
            s16 *sourceSample = sourceBuffer.Samples;
            s16 *destSample   = rcast(s16 *, soundBufDat);
            for (szt i = 0; i < samplesToWrite; ++i) {
                *destSample++ = *sourceSample++;
                *destSample++ = *sourceSample++;
                ++soundOutput.RunningSampleIndex;
            }

            gAudioRenderClient->Releasebuffer(Scast(Uint32, samplesToWrite), 0);
        }
    }
}

WindowDims
GetWindowDimensions(Hwnd hwnd)
{
    RECT clientRect;
    WindowDims winDim;
    GetClientRect(hwnd, &clientRect);
    winDim.Width  = clientRect.Right - clientRect.Left;
    winDim.Height = clientRect.Bottom - clientRect.Top;
    return winDim;
}

void
resize_DIB_section(OffscreenBuffer &buffer, s32 width, s32 height)
{
    // TODO: bulletproof this
    // maybe don't free first, free after, then free first if that fails

    if (buffer.memory) {
        VirtualFree(buffer.memory, 0, MEMRelease);
    }
    buffer.width         = width;
    buffer.height        = height;
    buffer.bytesPerPixel = 4;

    buffer.info.bmiHeader.biSize        = sizeof(buffer.info.bmiHeader);
    buffer.info.bmiHeader.biWidth       = width;
    buffer.info.bmiHeader.biHeight      = -height;
    buffer.info.bmiHeader.biPlanes      = 1;
    buffer.info.bmiHeader.biBitCount    = 32;
    buffer.info.bmiHeader.biCompression = BIRgb;

    s32 bytesPerPixel    = 4;
    s32 bitmapMemorySize = (width * height) * bytesPerPixel;
    buffer.memory        = VirtualAlloc(0, bitmapMemorySize, MEMReserve | MEMCommit, PAGEReadwrite);

    buffer.pitch = width * buffer.bytesPerPixel;
}

void
displayBufferInWindow(Hdc hdc, OffscreenBuffer &buffer, s32 x, s32 y, s32 width, s32 height)
{
    if (width == buffer.width * 2 && height == buffer.height * 2) {
        ::StretchDIBits(hdc, 0, 0, width, height, 0, 0, buffer.width, buffer.height, buffer.memory,
                        &buffer.info, DIBRgbColors, SRCCOPY);
    } else {
        s32 offsetX = 0, offsetY = 0;

#if 0
        // NOTE: this causes screen flickering - out of sync with screen refresh rate?
        ::PatBlt(hdc, 0, 0, width, offsetY, BLACKNESS);
        ::PatBlt(hdc, 0, offsetY + buffer.height, width, height, BLACKNESS);
        ::PatBlt(hdc, 0, 0, offsetX, height, BLACKNESS);
        ::PatBlt(hdc, offsetX + buffer.width, 0, width, height, BLACKNESS);

#endif

        // NOTE: this is matches the windows dimensions
        ::StretchDIBits(hdc, offsetX, offsetY, buffer.width, buffer.height, 0, 0, buffer.width,
                        buffer.height, buffer.memory, &buffer.info, DIBRgbColors, SRCCOPY);
    }
}

void
processKeyboardMessage(GameButtonState &newState, const b32 isDown)
{
    if (newState.EndedDown != (isDown != 0)) {
        newState.EndedDown = isDown;
        ++newState.HalfTransitionCount;
    }
}

void
processXinputDigitalButton(Dword XinputButtonState, GameButtonState &oldState, DWORD buttonBit,
                           GameButtonState &newState)
{
    newState.EndedDown = ((XinputButtonState & buttonBit) == buttonBit);
    if (newState.EndedDown && oldState.EndedDown)
        newState.HalfTransitionCount = ++oldState.HalfTransitionCount;
}

void
doControllerInput(GameInput &oldInput, GameInput &newInput, HWND hwnd)
{
    // mouse cursor
    POINT mousePoint;
    GetCursorPos(&mousePoint);
    ScreenToClient(hwnd, &mousePoint);
    newInput.MouseX = mousePoint.X;
    newInput.MouseY = mousePoint.Y;
    newInput.MouseZ = 0;

    // mouse buttons
    processKeyboardMessage(NewInput.MouseButtons[0], ::GetKeyState(VKLbutton) & (1 << 15));
    processKeyboardMessage(NewInput.MouseButtons[1], ::GetKeyState(VKRbutton) & (1 << 15));
    processKeyboardMessage(NewInput.MouseButtons[2], ::GetKeyState(VKMbutton) & (1 << 15));

    for (szt key = 0; key < ArrayCount(oldInput.Keyboard.Keys); ++key) {
        if (oldInput.Keyboard.Keys[Key].HalfTransitionCount > 0 &&
            oldInput.Keyboard.Keys[Key].EndedDown == 0)
            oldInput.Keyboard.Keys[Key].HalfTransitionCount = 0;
    }

    // keyboard
    processKeyboardMessage(NewInput.Keyboard.Enter, ::GetKeyState(Keys::enter) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.W, ::GetKeyState(Keys::w) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.A, ::GetKeyState(Keys::a) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.S, ::GetKeyState(Keys::s) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.D, ::GetKeyState(Keys::d) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.P, ::GetKeyState(Keys::p) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.T, ::GetKeyState(Keys::t) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.D1, ::GetKeyState(Keys::d1) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.D2, ::GetKeyState(Keys::d2) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.D3, ::GetKeyState(Keys::d3) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.D4, ::GetKeyState(Keys::d4) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.D5, ::GetKeyState(Keys::d5) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.Space, ::GetKeyState(Keys::space) & (1 << 15));
    processKeyboardMessage(NewInput.Keyboard.LeftShift, ::GetKeyState(Keys::leftShift) & (1 << 15));

    // controller
    // poll the input device
    s32 maxControllerCount = XUSERMaxCount;
    if (maxControllerCount > 4) {
        maxControllerCount = 4;
    }

    for (DWORD controllerIndex = 0; controllerIndex < XUSERMaxCount; controllerIndex++) {
        GameControllerInput &oldController = oldInput.controllers[controllerIndex];
        GameControllerInput &newController = newInput.controllers[controllerIndex];

        XINPUTState controllerState;
        if (XInputGetState(controllerIndex, &controllerState) == ERRORSuccess) {
            //! the controller is plugged in
            XINPUTGamepad &pad = controllerState.Gamepad;

            newController.IsConnected = true;

            // NOTE: this is hardcoded for convenience
            // newController.isAnalog = oldController->isAnalog;
            newController.IsAnalog = true;

            //  no rmal stick input
            auto normalize = [](SHORT val) -> f32 {
                if (val < 0)
                    return scast(f32, val) / 32768.0f;
                else
                    return scast(f32, val) / 32767.0f;
            };

            f32 stickLeftX  = normalize(pad.sThumbLX);
            f32 stickLeftY  = normalize(pad.sThumbLY);
            f32 stickRightX = normalize(pad.sThumbRX);
            f32 stickRightY = normalize(pad.sThumbRY);

            newController.MinX = newController.MaxX = newController.EndLeftStickX = stickLeftX;
            newController.MinY = newController.MaxY = newController.EndLeftStickY = stickLeftY;

            for (szt button = 0; button < ArrayCount(oldController.Buttons); ++button) {
                if (!oldController.Buttons[Button].EndedDown)
                    oldController.Buttons[Button].HalfTransitionCount = 0;
            }

            processXinputDigitalButton(Pad.Wbuttons, oldController.DpadUp, XINPUTGamepad_DPAD_UP,
                                       newController.DpadUp);
            processXinputDigitalButton(Pad.Wbuttons, oldController.DpadRight,
                                       XINPUTGamepad_DPAD_RIGHT, newController.DpadRight);
            processXinputDigitalButton(Pad.Wbuttons, oldController.DpadDown,
                                       XINPUTGamepad_DPAD_DOWN, newController.DpadDown);
            processXinputDigitalButton(Pad.Wbuttons, oldController.DpadLeft,
                                       XINPUTGamepad_DPAD_LEFT, newController.DpadLeft);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonA, XINPUTGamepad_A,
                                       newController.ButtonA);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonB, XINPUTGamepad_B,
                                       newController.ButtonB);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonX, XINPUTGamepad_X,
                                       newController.ButtonX);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonY, XINPUTGamepad_Y,
                                       newController.ButtonY);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonRb,
                                       XINPUTGamepad_RIGHT_SHOULDER, newController.ButtonRb);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonLb,
                                       XINPUTGamepad_LEFT_SHOULDER, newController.ButtonLb);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonBack, XINPUTGamepad_BACK,
                                       newController.ButtonBack);
            processXinputDigitalButton(Pad.Wbuttons, oldController.ButtonStart, XINPUTGamepad_START,
                                       newController.ButtonStart);

            // NOTE: Not currently used
            // float dpadStickRX = pad.sThumbRX;
            // float dpadStickRY = pad.sThumbRY;
            // bool dPadStart = (pad->wButtons & XINPUTGamepad_START);
            // bool dPadBack = (pad->wButtons & XINPUTGamepad_BACK);
            // bool dPadRB = (pad->wButtons & XINPUTGamepad_RIGHT_SHOULDER);
            // bool dPadLB = (pad->wButtons & XINPUTGamepad_LEFT_SHOULDER);
            // bool dPadR3 = (pad->wButtons & XINPUTGamepad_RIGHT_THUMB);
            // bool dPadL3 = (pad->wButtons & XINPUTGamepad_LEFT_THUMB);
            // unsigned TCHAR dPadRT = pad->bRightTrigger;
            // unsigned TCHAR dPadLT = pad->bLeftTrigger;
        }
    }
}

LARGEInteger
getWallClock()
{
    LARGEInteger time;
    QueryPerformanceCounter(&time);
    return time;
}

f32
get_seconds_elapsed(LARGEInteger start, LARGEInteger end)
{
    return scast(f32, end.QuadPart - start.QuadPart) / scast(f32, gPerformanceCounterFrequency);
}

// ===============================================================================================
// #PLAYBACK
// ===============================================================================================

#if REPLAYBuffers == 1
void
getInputFilePath(Win32State &state, b32 isInputStream)
{
    int x;
}

ReplayBuffer &
getReplayBuffer(Win32State &state, szt index)
{
    TomAssert(index < ArrayCount(state.replayBuffers));
    return state.replayBuffers[Index];
}

void
beginRecordingInput(Win32State &state, s32 input_recording_index)
{
    auto &replayBuffer = getReplayBuffer(State, input_recording_index);
    if (replayBuffer.MemoryBlock) {
        printf("Recording...\n");
        state.inputRecordingIndex = inputRecordingIndex;

        TCHAR fileName[512];
        StprintfS(FileName, 512, T("Replay%DInput.Ti"), input_recording_index);
        // const TCHAR* fileName = T("Replay1Input.Ti");
        state.recordingHandle = CreateFile(fileName, GENERICWrite | GENERICRead, NULL, NULL,
                                           CREATEAlways, FILEAttributeNormal, 0);

        CopyMemory(replayBuffer.MemoryBlock, state.gameMemoryBlock, state.totalSize);
    }
}

void
endRecordingInput(Win32State &state)
{
    printf("Recording ended.\n");
    CloseHandle(state.recordingHandle);
    state.inputRecordingIndex = 0;
}

void
beginInputPlayback(Win32State &state, s32 input_playbackIndex)
{
    auto &replayBuffer = getReplayBuffer(State, input_playbackIndex);
    if (replayBuffer.MemoryBlock) {
        printf("Input Playback started...\n");
        state.inputPlaybackIndex = inputPlaybackIndex;

        TCHAR fileName[512];
        StprintfS(FileName, 512, T("Replay%DInput.Ti"), input_playbackIndex);
        state.playbackHandle =
            CreateFile(fileName, GENERICRead, NULL, NULL, OPENExisting, FILEAttributeNormal, 0);

        CopyMemory(state.gameMemoryBlock, replayBuffer.MemoryBlock, state.totalSize);
    }
}

void
endInputPlayback(Win32State &state)
{
    printf("Input playback ended.\n");
    CloseHandle(state.playbackHandle);
    state.inputPlaybackIndex = 0;
}

void
recordInput(Win32State &state, GameInput &newInput)
{
    DWORD bytesWritten;
    WriteFile(state.recordingHandle, &newInput, sizeof(newInput), &bytesWritten, 0);
}

void
playbackInput(Win32State &state, GameInput &newInput)
{
    DWORD bytesRead;
    if (ReadFile(state.playbackHandle, &newInput, sizeof(newInput), &bytesRead, 0)) {
        if (bytesRead == 0) {
            // NOTE: hit end of stream, go back to begining;
            s32 playbackIndex = state.inputPlaybackIndex;
            endInputPlayback(State);
            beginInputPlayback(State, playbackIndex);
            ReadFile(state.playbackHandle, &newInput, sizeof(newInput), &bytesRead, 0);
        }
    }
}
#endif

void
initConsole()
{
    bool isInitialized = AllocConsole();
    TomAssert(isInitialized);

    if (isInitialized) {
        FILE *fDummy;
        freopenS(&Fdummy, "CONOUT$", "w", stdout);
        freopenS(&Fdummy, "CONOUT$", "w", stderr);
        freopenS(&Fdummy, "CONIN$", "r", stdin);

        HANDLE hConOut =
            CreateFile(T("Conout$"), GENERICRead | GENERICWrite, FILEShareRead | FILEShareWrite,
                       NULL, OPENExisting, FILEAttributeNormal, NULL);
        HANDLE hConIn =
            CreateFile(T("Conin$"), GENERICRead | GENERICWrite, FILEShareRead | FILEShareWrite,
                       NULL, OPENExisting, FILEAttributeNormal, NULL);
        SetStdHandle(STDOutputHandle, hConOut);
        SetStdHandle(STDErrorHandle, hConOut);
        SetStdHandle(STDInputHandle, hConIn);
    }
}

void
processPendingMessages(Win32State &state, GameInput &input)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PMRemove)) {
        switch (msg.message) {
            case WMQuit: running = false; break;
            case WMSyskeydown:
            case WMSyskeyup:
            case WMKeydown:
            case WMKeyup: {
                u32 VKCode      = scast(u32, msg.wParam);
                bool wasDown    = ((msg.lParam & (1 << 30)) != 0);
                bool isDown     = ((msg.lParam & (1 << 29)) == 0);
                bool altKeyDown = (msg.lParam & (1 << 29));
                if (wasDown != isDown) {
                    switch (VKCode) {
                        case VKEscape: running = false; break;
                        case 'P': {
                            if (isDown) {
                                gPause = !gPause;
                            }
                        } break;

                        case (VKReturn): {
                            if (altKeyDown) {
                                toggleFullscreen(Msg.Hwnd);
                            }
                        } break;
                        case (VKF4): {
                            if (altKeyDown) {
                                running = false;
                            }
                        } break;
#if REPLAYBuffers
                        case 'L': {
                            if (isDown) {
                                if (state.inputPlaybackIndex == 0) {
                                    if (state.inputRecordingIndex == 0) {
                                        beginRecordingInput(State, 1);
                                    } else {
                                        endRecordingInput(State);
                                        beginInputPlayback(State, 1);
                                    }
                                } else {
                                    endInputPlayback(State);
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

//==================================================================================

LRESULT CALLBACK
WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch (msg) {
        case WMSetcursor: {
            if (gDebugShowCursor) {
                result = DefWindowProcA(hwnd, msg, wparam, lparam);
            } else {
                SetCursor(0);
            }
        } break;
        case WMSize: {
            gWinDim = GetWindowDimensions(Hwnd);
            // ResizeDIBSection(gBackbuffer, gWindims.Width, gWindims.Height);
        } break;
        case WMDestroy: {
            running = false;
        } break;
        case WMClose: {
            running = false;
            PostQuitMessage(0);
        } break;
        case WMActivateapp: break;
        case WMPaint: {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(hwnd, &paint);
            s32 x             = paint.rcPaint.left;
            s32 y             = paint.rcPaint.right;
            s32 height        = paint.rcPaint.bottom - paint.rcPaint.top;
            s32 width         = paint.rcPaint.right - paint.rcPaint.left;
            display_buffer_in_window(deviceContext, gBackBuffer, x, y, width, height);
            EndPaint(hwnd, &paint);
        } break;
        default:
            //            OutPutDebugStringA("default\n");
            result = DefWindowProc(hwnd, msg, wparam, lparam);
            break;
    }
    return result;
}

// ===============================================================================================
// #START
// ===============================================================================================

s32
win32Main(Hinstance hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, s32 nShowCmd)
{
    // Init

    // WARN
    const TCHAR *iconPath = T("T:\\Data\\Tomato.Ico");
    auto icon             = (HICON)(LoadImage(NULL, iconPath, IMAGEIcon, 0, 0,
                                              LRLoadfromfile | LRDefaultsize | LRShared));

    initConsole();
    auto consHwnd = GetConsoleWindow();
    TomAssert(consHwnd);
    SendMessage(consHwnd, WMSeticon, NULL, (LPARAM)icon);

    printf("Starting...\n");

    Win32State state = {};

    DWORD exePathLen = GetModuleFileNameA(NULL, state.exePath, sizeof(state.exePath));
    printf("exe path %s\n", state.exePath);

#if Cppuwind
    printf("Exceptions are enabled!\n");
#endif

    LARGEInteger performanceQueryResult;
    QueryPerformanceFrequency(&performanceQueryResult);
    gPerformanceCounterFrequency = performanceQueryResult.Quadpart;

    GameCode code = loadGameCode(GGameDllName);
    loadXinput();

#ifdef TOMInternal
    gDebugShowCursor = true;
#else
    debugShowCursor    = false;
#endif

    constexpr s32 winWidth  = 960;
    constexpr s32 winHeight = 540;

    resize_DIB_section(gBackBuffer, winWidth, winHeight);

    WNDCLASS windowClass = {

        .style         = CSHredraw | CSVredraw | CSOwndc,
        .lpfnWndProc   = WndProc,
        .hInstance     = hInstance,
        .hIcon         = icon,
        .hCursor       = LoadCursor(NULL, IDCArrow),
        .lpszClassName = T("Tomatowincls")
    };

    if (!RegisterClass(&windowClass)) {
        printf("ERROR--> Failed to register window class!\n");
        TomAssert(false);
        return 0;
    }

    DWORD dwStyle = WSOverlappedwindow | WSVisible;
    // DWORD dwStyle = WSOverlapped | WSMinimizebox | WSSysmenu;

    RECT wr = { .left = 0, .top = 0, .right = winWidth + wr.left, .bottom = winHeight + wr.top };

    if (AdjustWindowRect(&wr, dwStyle, false) == 0) {
        printf("ERROR--> Failed to adjust window rect");
        TomAssert(false);
    }

    HWND hwnd = CreateWindowEx(0, windowClass.lpszClassName, T("Tomatogame"), dwStyle, CWUsedefault,
                               CWUsedefault, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL,
                               hInstance, NULL);

    if (!hwnd) {
        printf("Failed to create window!\n");
        TomAssert(hwnd);
        return 0;
    }

    ::ShowWindow(hwnd, SWShownormal);

    HRESULT hr;
    hr = GetLastError();

    // BOOL fOK;
    // TCHAR msgBuf[128];
    // fOK = FormatMessage(
    //     FORMATMessageAllocateBuffer | FORMATMessageFromSystem |
    //     FORMATMessageIgnoreInserts, NULL, hr, 0, (PTSTR)&msgBuf, 0, NULL);
    // // if (!fOK) msgBuf = T("Failed to format Message!");
    // Tprintf(Text("%D\T%S\N"), hr, msgBuf);
    // LocalFree(msgBuf);

    // NOTE: Set the windows schedule granularity to 1ms
    // so sleep will be more granular
    UINT desiredSchedulerMs = 1;
    b32 isSleepGranular     = (timeBeginPeriod(desiredSchedulerMs) == TIMERRNoerror);
    isSleepGranular         = false;

    HDC deviceContext           = GetDC(hwnd);
    running                     = true;
    gPause                      = false;
    g_back_buffer.bytesPerPixel = 4;

    s32 monitorRefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
    printf("Monitor Refresh Rate: %d\n", monitorRefreshRate);

    SoundOutput soundOutput = {
        .samplesPerSec    = 48000,
        .bytesPerSample   = sizeof(s16) * 2,
        .secondaryBufSize = scast(DWORD, soundOutput.samplesPerSec),
        .latencySampleCount =
            scast(s32, gFramesOfAudioLatency *
                           scast(f32, (soundOutput.samplesPerSec / scast(f32, gGameUpdateHertz))))
    };

    init_WASAPI(soundOutput.samplesPerSec, soundOutput.secondaryBufSize);
    gAudioClient->Start();

    // TODO: Pool with bitmap VirtualAlloc
    s16 *samples = scast(s16 *, VirtualAlloc(0, soundOutput.secondaryBufSize,
                                             MEMReserve | MEMCommit, PAGEReadwrite));

#ifdef TOMInternal
    LPVOID baseAddress = rcast(LPVOID, TERABYTES(scast(u64, 2)));
#else
    LPVOID baseAddress = 0;
#endif

    GameMemory memory = { .permanentStorageSize    = MEGABYTES(256),
                          .transientStorageSize    = GIGABYTES(1),
                          .platformFreeFileMemory  = DebugPlatformFreeFileMemory,
                          .platfromReadEntireFile  = DebugPlatformReadEntireFile,
                          .platformWriteEntireFile = DebugPlatformWriteEntireFile };

    state.totalSize = memory.permanentStorageSize + memory.transientStorageSize;
    // TODO: use large pages
    state.gameMemoryBlock =
        VirtualAlloc(baseAddress, state.totalSize, MEMReserve | MEMCommit, PAGEReadwrite);
    memory.permanentStorage = state.gameMemoryBlock;

    memory.transientStorage = (scast(u8 *, memory.permanentStorage) + memory.permanentStorageSize);

#if REPLAYBuffers
    // mapping memory to file
    for (s32 replayIndex = 0; replayIndex < ArrayCount(state.replayBuffers); ++replayIndex) {
        replayBuffer &replayBuffer = state.replayBuffers[replayIndex];
        _stprintf_s(replayBuffer.file_name, sizeof(TCHAR) * 512, T("Replay%DState.Ti"),
                    replayIndex);

        replayBuffer.fileHandle = CreateFile(replayBuffer.file_name, GENERICWrite | GENERICRead,
                                             NULL, NULL, CREATEAlways, FILEAttributeNormal, 0);

        LARGEInteger maxSize;
        maxSize.Quadpart        = state.totalSize;
        replayBuffer.memory_map = CreateFileMapping(replayBuffer.fileHandle, NULL, PAGEReadwrite,
                                                    maxSize.Highpart, maxSize.Lowpart, NULL);
        DWORD error             = GetLastError();
        replayBuffer.memory_block =
            MapViewOfFile(replayBuffer.memory_map, FILEMapAllAccess, 0, 0, state.totalSize);
        TomAssert(replayBuffer.MemoryBlock);
    }
#endif

    GameInput input[2]  = {};
    GameInput *newInput = &input[0];
    GameInput *oldInput = &input[1];

    LARGEInteger lastCounter = getWallClock();
    u64 lastCycleCount       = Rdtsc();

    bool isSoundValid     = true;
    bool isGameCodeLoaded = true;
    DWORD lastPlayCursor  = {};
    DWORD lastWriteCursor = {};

    // =============================================================================================
    // #MAIN LOOP
    // =============================================================================================
    while (running) {
        do_controller_input(*oldInput, *newInput, hwnd);
        processPendingMessages(State, *newInput);
        // NOTE: this isn't calculated and needs to be for a varaible framerate
        newInput->DeltaTime = gTargetFramesPerSecond;

        FILETIME dllWriteTime = get_lastWriteTime(g_game_DLL_name);
        if (CompareFileTime(&dllWriteTime, &code.lastWriteTime_DLL)) {
            unloadGameCode(Code);
            code = loadGameCode(GGameDllName);
            printf("New Game Code loaded!\n");
        }

        // NOTE: temp program exit from controller
        if (newInput->Controllers->ButtonBack.EndedDown) {
            running = false;
        }

        // #Sound

        REFERENCETime latency;
        if (SUCCEEDED(gAudioClient->Getstreamlatency(&Latency))) {
        } else {
            printf("ERROR--> Failed to get audio latency\n");
        }

        s32 samplesToWrite;
        UINT32 soundPadSize;
        if (SUCCEEDED(gAudioClient->Getcurrentpadding(&SoundPadSize))) {
            s32 maxSampleCnt = scast(s32, soundOutput.secondaryBufSize - soundPadSize);
            samplesToWrite   = scast(s32, soundOutput.latencySampleCount - soundPadSize);
            if (samplesToWrite < 0) samplesToWrite = 0;
            // TomAssert(samplesToWrite < maxSampleCnt);
        }

        GameSoundOutputBuffer soundBuffer = { .samplesPerSecond = soundOutput.samplesPerSec,
                                              .sampleCount      = samplesToWrite,
                                              .samples          = samples };

        Game_OffscreenBuffer buffer = { .memory        = gBackBuffer.Memory,
                                        .width         = gBackBuffer.Width,
                                        .height        = gBackBuffer.Height,
                                        .pitch         = gBackBuffer.Pitch,
                                        .bytesPerPixel = 4 };

#if REPLAYBuffers
        if (state.inputRecordingIndex) {
            recordInput(State, *newInput);
        }
        if (state.inputPlaybackIndex) {
            playbackInput(State, *newInput);
        }
#endif
        // null check for stub sections
        isGameCodeLoaded = code.updateAndRender && code.getSoundSamples;
        // isGameCodeLoaded = false;

        // NOTE: dummy thread context, for now
        ThreadContext thread {};

        if (isGameCodeLoaded) {
            code.updateAndRender(&thread, memory, *input, buffer, soundBuffer);
            code.getSoundSamples(&thread, memory, soundBuffer);
        }

        fill_soundBuffer(soundOutput, samplesToWrite, soundBuffer);

        // clock stuffs
        auto workCounter       = getWallClock();
        f32 workSecondsElapsed = get_seconds_elapsed(lastCounter, workCounter);

        f32 secondsElapsedForFrame = workSecondsElapsed;
        if (secondsElapsedForFrame < gTargetFramesPerSecond) {
            if (isSleepGranular) {
                auto sleepMs =
                    scast(DWORD, 1000.f * (gTargetFramesPerSecond - secondsElapsedForFrame));
                if (sleepMs > 0) {
                    ::Sleep(sleepMs);
                }
            }
            f32 test_secondsElapsedForFrame = get_seconds_elapsed(lastCounter, getWallClock());
            while (secondsElapsedForFrame < gTargetFramesPerSecond) {
                secondsElapsedForFrame = get_seconds_elapsed(lastCounter, getWallClock());
            }
        } else {
            printf("WARNING--> missed frame timing!!!\n");
        }

        auto endCounter = getWallClock();
        f32 msPerFrame  = 1000.f * get_seconds_elapsed(lastCounter, endCounter);
        // printf("%f\n", msPerFrame);

        lastCounter = endCounter;

        // NOTE: this is debug code
        display_buffer_in_window(deviceContext, gBackBuffer, 0, 0, g_winDim.Width, g_winDim.Height);

        DWORD playCursor;
        DWORD writeCursor;

        UINT64 frequencyPosition;
        UINT64 unitsPosisition;

        gAudioClock->Getfrequency(&FrequencyPosition);
        gAudioClock->Getposition(&UnitsPosisition, 0);

        playCursor = scast(DWORD, soundOutput.samplesPerSec * unitsPosisition / frequencyPosition) %
                     soundOutput.samplesPerSec;
        writeCursor =
            scast(DWORD, soundOutput.samplesPerSec * unitsPosisition / frequencyPosition) %
                soundOutput.samplesPerSec +
            soundPadSize;
        if (writeCursor > soundOutput.secondaryBufSize) {
            writeCursor -= soundOutput.secondaryBufSize;
        }

        GameInput *tempInput = newInput;
        newInput             = oldInput;
        oldInput             = tempInput;

        u64 endCycleCount = Rdtsc();
        u64 cyclesElapsed = endCycleCount - lastCycleCount;
        lastCycleCount    = endCycleCount;
    }

    return 0;
}
}  // namespace
//========================================================================================
// ENTRY POINT
//========================================================================================

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    Declspec(Dllexport) DWORD NvOptimusEnablement                = 0x00000001;
    Declspec(Dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Entry point
int WINAPI
wWinMain(In HINSTANCE hInstance, InOpt HINSTANCE hPrevInstance, In LPWSTR lpCmdLine,
         In int nCmdShow)
{
    UNREFERENCEDParameter(Hprevinstance);
    UNREFERENCEDParameter(Lpcmdline);

    HRESULT hr = CoInitializeEx(nullptr, COINITBASEMultithreaded);
    if (FAILED(hr)) return 1;

    s32 ecode = win32Main(Hinstance, hPrevInstance, lpCmdLine, nCmdShow);

    CoUninitialize();

    return ecode;
}
