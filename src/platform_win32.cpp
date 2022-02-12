#include "platform_win32.hpp"

// ===============================================================================================
// #GLOBALS
// ===============================================================================================

static constexpr u32 g_game_update_hertz        = 60;
static constexpr f32 g_target_frames_per_second = 1.f / (f32)g_game_update_hertz;

static bool running;
static bool g_pause;

static WINDOWPLACEMENT g_win_pos    = { sizeof(g_win_pos) };
static const TCHAR *g_game_DLL_name = _T("tomato_game.dll");
static bool g_debug_show_cursor;

static offscreen_buffer g_back_buffer;
static window_dims g_win_dim;
static s64 g_performance_counter_frequency;

// TODO: the sleep precision issue is keeping this above 1 frame... I think
static constexpr f32 g_frames_of_audio_latency = (1.1f / 30) * g_game_update_hertz;
static IAudioClient *g_audio_client;
static IAudioRenderClient *g_audio_render_client;
static IAudioClock *g_audio_clock;

#ifdef TOM_INTERNAL
DEBUG_PLATFORM_FREE_FILE_MEMORY(_debug_platform_free_file_memory)
{
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(_debug_platform_read_entire_file)
{
    debug_read_file_result file = {};

    HANDLE file_handle =
        CreateFileA(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(file_handle, &fileSize)) {
            u32 fileSize32 = safe_truncate_u32_to_u64(fileSize.QuadPart);
            file.contents  = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (file.contents) {
                DWORD bytesRead;
                if (ReadFile(file_handle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
                    fileSize32 == bytesRead) {
                    file.content_size = fileSize32;
                } else {
                    _debug_platform_free_file_memory(thread, file.contents);
                    file.contents = 0;
                }
            } else {
                printf("ERROR-> Failed to read file contents!\n");
            }
        } else {
            printf("ERROR-> Failed to open file handle!\n");
        }
        CloseHandle(file_handle);
    }
    return file;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(_debug_platform_write_entire_file)
{
    b32 success = false;

    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if (WriteFile(file_handle, memory, (DWORD)memory_size, &bytes_written, 0)) {
            success = (bytes_written == memory_size);
        } else {
            printf("ERROR-> Failed to write file contents!\n");
        }
        CloseHandle(file_handle);
    } else {
        printf("ERROR-> Failed to oepn file handle!\n");
    }
    return success;
}

#endif

//! this is a roundabout way of extracting a method out of a header...
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

static void
toggle_fullscreen(HWND hwnd)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &g_win_pos) &&
            GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &g_win_pos);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static FILETIME
get_last_write_time(const TCHAR *file_name)
{
    FILETIME last_write_time {};

#if 0
    // NOTE: old way with a handle
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFile(fileName, &findData);
    if (find_handle != INVALID_HANDLE_VALUE) {
        last_write_time = findData.ftLastWriteTime;
        FindClose(find_handle);
    }
#endif

    // NOTE: this has no handle
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(file_name, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

static game_code
load_game_code(const TCHAR *dll_name)
{
    game_code code {};
    const TCHAR *dll_copy = _T("loaded_gamecode_copy.dll");

    code.last_write_time_DLL = get_last_write_time(dll_name);

    CopyFile(dll_name, dll_copy, FALSE);

    code.game_code_DLL = LoadLibrary(dll_copy);
    if (code.game_code_DLL) {
        code.update_and_render = (game_update_and_render_stub *)GetProcAddress(
            code.game_code_DLL, "game_update_and_render");
        code.get_sound_samples = (game_get_sound_samples_stub *)GetProcAddress(
            code.game_code_DLL, "game_get_sound_samples");
        code.is_valid = (code.update_and_render && code.get_sound_samples);
    }

    if (!code.is_valid) {
        printf("Failed to load game code!\n");
        code.update_and_render = 0;
        code.get_sound_samples = 0;
    } else {
        printf("Game code successfully loaded.\n");
    }

    return code;
}

static void
unload_game_code(game_code &game_code)
{
    if (game_code.game_code_DLL) {
        FreeLibrary(game_code.game_code_DLL);
        game_code.update_and_render = 0;
        game_code.get_sound_samples = 0;
    }
    printf("Game code unloaded.");
}

static void
load_Xinput()
{
    // TODO: test this on other windows version
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary) {
        HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if (XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        printf("ERROR->failed to load XInput!\n");
    }
}

static void
init_WASAPI(s32 samples_per_second, s32 buffer_size_in_samples)
{
    if (FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) {
        INVALID_CODE_PATH;
    }

    IMMDeviceEnumerator *enumerator;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&enumerator)))) {
        INVALID_CODE_PATH;
    }

    IMMDevice *device;
    if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        INVALID_CODE_PATH;
    }

    if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL,
                                (LPVOID *)&g_audio_client))) {
        INVALID_CODE_PATH;
    }

    WAVEFORMATEXTENSIBLE wave_format;

    wave_format.Format.cbSize         = sizeof(wave_format);
    wave_format.Format.wFormatTag     = WAVE_FORMAT_EXTENSIBLE;
    wave_format.Format.wBitsPerSample = 16;
    wave_format.Format.nChannels      = 2;
    wave_format.Format.nSamplesPerSec = (DWORD)samples_per_second;
    wave_format.Format.nBlockAlign =
        (WORD)(wave_format.Format.nChannels * wave_format.Format.wBitsPerSample / 8);
    wave_format.Format.nAvgBytesPerSec =
        wave_format.Format.nSamplesPerSec * wave_format.Format.nBlockAlign;
    wave_format.Samples.wValidBitsPerSample = 16;
    wave_format.dwChannelMask               = KSAUDIO_SPEAKER_STEREO;
    wave_format.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

    REFERENCE_TIME buffer_duration = 10000000ULL * buffer_size_in_samples /
                                     samples_per_second;  // buffer size in 100 nanoseconds
    if (FAILED(g_audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST,
                                          buffer_duration, 0, &wave_format.Format, nullptr))) {
        TOM_ASSERT(false);
    }

    if (FAILED(g_audio_client->GetService(IID_PPV_ARGS(&g_audio_render_client)))) {
        TOM_ASSERT(false);
    }

    UINT32 soundFrmCnt;
    if (FAILED(g_audio_client->GetBufferSize(&soundFrmCnt))) {
        TOM_ASSERT(false);
    }

    if (FAILED(g_audio_client->GetService(IID_PPV_ARGS(&g_audio_clock)))) {
        TOM_ASSERT(false);
    }

    // Check if we got what we requested (better would to pass this value back
    // as real buffer size)
    TOM_ASSERT(buffer_size_in_samples <= (s32)soundFrmCnt);
}

static void
fill_sound_buffer(sound_output &sound_output, s32 samples_to_write,
                  game_sound_output_buffer &source_buffer)
{
    {
        BYTE *soundBufDat;
        if (SUCCEEDED(g_audio_render_client->GetBuffer((UINT32)samples_to_write, &soundBufDat))) {
            s16 *sourceSample = source_buffer.samples;
            s16 *destSample   = (s16 *)soundBufDat;
            for (szt i = 0; i < samples_to_write; ++i) {
                *destSample++ = *sourceSample++;
                *destSample++ = *sourceSample++;
                ++sound_output.running_sample_index;
            }

            g_audio_render_client->ReleaseBuffer((UINT32)samples_to_write, 0);
        }
    }
}

static window_dims
Get_window_dimensions(HWND hwnd)
{
    RECT client_rect;
    window_dims WinDim;
    GetClientRect(hwnd, &client_rect);
    WinDim.width  = client_rect.right - client_rect.left;
    WinDim.height = client_rect.bottom - client_rect.top;
    return WinDim;
}

static void
resize_DIB_section(offscreen_buffer &buffer, s32 width, s32 height)
{
    // TODO: bulletproof this
    // maybe don't free first, free after, then free first if that fails

    if (buffer.memory) {
        VirtualFree(buffer.memory, 0, MEM_RELEASE);
    }
    buffer.width           = width;
    buffer.height          = height;
    buffer.bytes_per_pixel = 4;

    buffer.info.bmiHeader.biSize        = sizeof(buffer.info.bmiHeader);
    buffer.info.bmiHeader.biWidth       = width;
    buffer.info.bmiHeader.biHeight      = -height;
    buffer.info.bmiHeader.biPlanes      = 1;
    buffer.info.bmiHeader.biBitCount    = 32;
    buffer.info.bmiHeader.biCompression = BI_RGB;

    s32 bytes_per_pixel    = 4;
    s32 bitmap_memory_size = (width * height) * bytes_per_pixel;
    buffer.memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    buffer.pitch = width * buffer.bytes_per_pixel;
}

static void
display_buffer_in_window(HDC hdc, offscreen_buffer &buffer, s32 x, s32 y, s32 width, s32 height)
{
    if (width == buffer.width * 2 && height == buffer.height * 2) {
        ::StretchDIBits(hdc, 0, 0, width, height, 0, 0, buffer.width, buffer.height, buffer.memory,
                        &buffer.info, DIB_RGB_COLORS, SRCCOPY);
    } else {
        s32 offset_x = 0;
        s32 offset_y = 0;

#if 0
    // NOTE: this causes screen flickering - out of sync with screen refresh rate?
    ::PatBlt(hdc, 0, 0, width, offset_y, BLACKNESS);
    ::PatBlt(hdc, 0, offset_y + buffer.height, width, height, BLACKNESS);
    ::PatBlt(hdc, 0, 0, offset_x, height, BLACKNESS);
    ::PatBlt(hdc, offset_x + buffer.width, 0, width, height, BLACKNESS);

#endif
        // s32 x_offset = 0;
        // s32 y_offset = 0;

        // NOTE: this is matches the windows dimensions
        ::StretchDIBits(hdc, offset_x, offset_y, buffer.width, buffer.height, 0, 0, buffer.width,
                        buffer.height, buffer.memory, &buffer.info, DIB_RGB_COLORS, SRCCOPY);
    }
}

static void
process_keyboard_message(game_button_state &new_state, const b32 is_down)
{
    if (new_state.ended_down != (is_down != 0)) {
        new_state.ended_down = is_down;
        ++new_state.half_transition_count;
    }
}

static void
process_Xinput_digital_button(DWORD Xinput_button_state_, game_button_state &old_state_,
                              DWORD button_bit_, game_button_state &new_state)
{
    new_state.ended_down = ((Xinput_button_state_ & button_bit_) == button_bit_);
    if (new_state.ended_down && old_state_.ended_down)
        new_state.half_transition_count = ++old_state_.half_transition_count;
}

static void
do_controller_input(game_input &old_input, game_input &new_input, HWND hwnd)
{
    // mouse cursor
    POINT mouse_point;
    GetCursorPos(&mouse_point);
    ScreenToClient(hwnd, &mouse_point);
    new_input.mouse_x = mouse_point.x;
    new_input.mouse_y = mouse_point.y;
    new_input.mouse_z = 0;

    // mouse buttons
    process_keyboard_message(new_input.mouse_buttons[0], ::GetKeyState(VK_LBUTTON) & (1 << 15));
    process_keyboard_message(new_input.mouse_buttons[1], ::GetKeyState(VK_RBUTTON) & (1 << 15));
    process_keyboard_message(new_input.mouse_buttons[2], ::GetKeyState(VK_MBUTTON) & (1 << 15));

    for (szt key {}; key < game_keyboard_input::s_key_cnt; ++key) {
        if (old_input.keyboard.keys[key].half_transition_count > 0 &&
            old_input.keyboard.keys[key].ended_down == 0)
            old_input.keyboard.keys[key].half_transition_count = 0;
    }

    // keyboard
    process_keyboard_message(new_input.keyboard.enter, ::GetKeyState(keys::enter) & (1 << 15));
    process_keyboard_message(new_input.keyboard.w, ::GetKeyState(keys::w) & (1 << 15));
    process_keyboard_message(new_input.keyboard.a, ::GetKeyState(keys::a) & (1 << 15));
    process_keyboard_message(new_input.keyboard.s, ::GetKeyState(keys::s) & (1 << 15));
    process_keyboard_message(new_input.keyboard.d, ::GetKeyState(keys::d) & (1 << 15));
    process_keyboard_message(new_input.keyboard.p, ::GetKeyState(keys::p) & (1 << 15));
    process_keyboard_message(new_input.keyboard.t, ::GetKeyState(keys::t) & (1 << 15));
    process_keyboard_message(new_input.keyboard.d1, ::GetKeyState(keys::d1) & (1 << 15));
    process_keyboard_message(new_input.keyboard.d2, ::GetKeyState(keys::d2) & (1 << 15));
    process_keyboard_message(new_input.keyboard.d3, ::GetKeyState(keys::d3) & (1 << 15));
    process_keyboard_message(new_input.keyboard.d4, ::GetKeyState(keys::d4) & (1 << 15));
    process_keyboard_message(new_input.keyboard.d5, ::GetKeyState(keys::d5) & (1 << 15));
    process_keyboard_message(new_input.keyboard.space, ::GetKeyState(keys::space) & (1 << 15));
    process_keyboard_message(new_input.keyboard.left_shift,
                             ::GetKeyState(keys::left_shift) & (1 << 15));

    // controller
    // poll the input device
    s32 max_controller_count = XUSER_MAX_COUNT;
    if (max_controller_count > 4) {
        max_controller_count = 4;
    }

    for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
        game_controller_input &old_controller = old_input.controllers[controller_index];
        game_controller_input &new_controller = new_input.controllers[controller_index];

        XINPUT_STATE controller_state;
        if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS) {
            //! the controller is plugged in
            XINPUT_GAMEPAD &pad = controller_state.Gamepad;

            new_controller.is_connected = true;

            // NOTE: this is hardcoded for convenience
            // newController.isAnalog = oldController->isAnalog;
            new_controller.is_analog = true;

            //  no rmal stick input
            auto normalize = [](SHORT val) {
                if (val < 0)
                    return (f32)val / 32768.0f;
                else
                    return (f32)val / 32767.0f;
            };

            f32 stick_left_x  = normalize(pad.sThumbLX);
            f32 stick_left_y  = normalize(pad.sThumbLY);
            f32 stick_right_x = normalize(pad.sThumbRX);
            f32 stick_right_y = normalize(pad.sThumbRY);

            new_controller.min_x = new_controller.max_x = new_controller.end_left_stick_x =
                stick_left_x;
            new_controller.min_y = new_controller.max_y = new_controller.end_left_stick_y =
                stick_left_y;

            for (szt button {}; button < game_controller_input::s_button_cnt; ++button) {
                if (!old_controller.buttons[button].ended_down)
                    old_controller.buttons[button].half_transition_count = 0;
            }

            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_up,
                                          XINPUT_GAMEPAD_DPAD_UP, new_controller.dpad_up);
            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_right,
                                          XINPUT_GAMEPAD_DPAD_RIGHT, new_controller.dpad_right);
            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_down,
                                          XINPUT_GAMEPAD_DPAD_DOWN, new_controller.dpad_down);
            process_Xinput_digital_button(pad.wButtons, old_controller.dpad_left,
                                          XINPUT_GAMEPAD_DPAD_LEFT, new_controller.dpad_left);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_A, XINPUT_GAMEPAD_A,
                                          new_controller.button_A);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_B, XINPUT_GAMEPAD_B,
                                          new_controller.button_B);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_X, XINPUT_GAMEPAD_X,
                                          new_controller.button_X);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_Y, XINPUT_GAMEPAD_Y,
                                          new_controller.button_Y);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_RB,
                                          XINPUT_GAMEPAD_RIGHT_SHOULDER, new_controller.button_RB);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_LB,
                                          XINPUT_GAMEPAD_LEFT_SHOULDER, new_controller.button_LB);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_back,
                                          XINPUT_GAMEPAD_BACK, new_controller.button_back);
            process_Xinput_digital_button(pad.wButtons, old_controller.button_start,
                                          XINPUT_GAMEPAD_START, new_controller.button_start);

            // NOTE: Not currently used
            // float dpadStickRX = pad.sThumbRX;
            // float dpadStickRY = pad.sThumbRY;
            // bool dPadStart = (pad->wButtons & XINPUT_GAMEPAD_START);
            // bool dPadBack = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            // bool dPadRB = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            // bool dPadLB = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            // bool dPadR3 = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            // bool dPadL3 = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            // unsigned TCHAR dPadRT = pad->bRightTrigger;
            // unsigned TCHAR dPadLT = pad->bLeftTrigger;
        }
    }
}

static LARGE_INTEGER
get_wall_clock()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time;
}

static f32
get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 seconds = f32(end.QuadPart - start.QuadPart) / f32(g_performance_counter_frequency);
    return seconds;
}

// ===============================================================================================
// #PLAYBACK
// ===============================================================================================

#if REPLAY_BUFFERS == 1
static void
get_input_file_path(win32_state &state, b32 is_input_stream)
{
    int x = 0;
}

static replay_buffer &
get_replay_buffer(win32_state &state, szt index_)
{
    TOM_ASSERT(index_ < ArrayCount(state.replay_buffers));
    return state.replay_buffers[index_];
}

static void
begin_recording_input(win32_state &state, s32 input_recording_index_)
{
    auto &replay_buffer = get_replay_buffer(state, input_recording_index_);
    if (replay_buffer.memory_block) {
        printf("Recording...\n");
        state.input_recording_index = input_recording_index_;

        TCHAR file_name[512];
        _stprintf_s(file_name, 512, _T("replay_%d_input.ti"), input_recording_index_);
        // const TCHAR* fileName = _T("replay_1_input.ti");
        state.recording_handle = CreateFile(file_name, GENERIC_WRITE | GENERIC_READ, NULL, NULL,
                                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

        CopyMemory(replay_buffer.memory_block, state.game_memory_block, state.total_size);
    }
}

static void
end_recording_input(win32_state &state)
{
    printf("Recording ended.\n");
    CloseHandle(state.recording_handle);
    state.input_recording_index = 0;
}

static void
begin_input_playback(win32_state &state, s32 input_playback_index_)
{
    auto &replay_buffer = get_replay_buffer(state, input_playback_index_);
    if (replay_buffer.memory_block) {
        printf("Input Playback started...\n");
        state.input_playback_index = input_playback_index_;

        TCHAR file_name[512];
        _stprintf_s(file_name, 512, _T("replay_%d_input.ti"), input_playback_index_);
        state.playback_handle = CreateFile(file_name, GENERIC_READ, NULL, NULL, OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL, 0);

        CopyMemory(state.game_memory_block, replay_buffer.memory_block, state.total_size);
    }
}

static void
end_input_playback(win32_state &state)
{
    printf("Input playback ended.\n");
    CloseHandle(state.playback_handle);
    state.input_playback_index = 0;
}

static void
record_input(win32_state &state, game_input &new_input)
{
    DWORD bytes_written;
    WriteFile(state.recording_handle, &new_input, sizeof(new_input), &bytes_written, 0);
}

static void
playback_input(win32_state &state, game_input &new_input)
{
    DWORD bytes_read;
    if (ReadFile(state.playback_handle, &new_input, sizeof(new_input), &bytes_read, 0)) {
        if (bytes_read == 0) {
            // NOTE: hit end of stream, go back to begining;
            s32 playback_index = state.input_playback_index;
            end_input_playback(state);
            begin_input_playback(state, playback_index);
            ReadFile(state.playback_handle, &new_input, sizeof(new_input), &bytes_read, 0);
        }
    }
}
#endif

static void
init_console()
{
    bool is_initialized = AllocConsole();
    TOM_ASSERT(is_initialized);

    if (is_initialized) {
        FILE *fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);

        HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE hConIn  = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, NULL);
        SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
        SetStdHandle(STD_ERROR_HANDLE, hConOut);
        SetStdHandle(STD_INPUT_HANDLE, hConIn);
    }
}

static void
process_pending_messages(win32_state &state, game_input &input)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            case WM_QUIT: running = false; break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                u32 VKCode        = (u32)msg.wParam;
                bool was_down     = ((msg.lParam & (1 << 30)) != 0);
                bool is_down      = ((msg.lParam & (1 << 29)) == 0);
                bool alt_key_down = (msg.lParam & (1 << 29));
                if (was_down != is_down) {
                    switch (VKCode) {
                        case VK_ESCAPE: running = false; break;
                        case 'P': {
                            if (is_down) {
                                g_pause = !g_pause;
                            }
                        } break;

                        case (VK_RETURN): {
                            if (alt_key_down) {
                                toggle_fullscreen(msg.hwnd);
                            }
                        } break;
                        case (VK_F4): {
                            if (alt_key_down) {
                                running = false;
                            }
                        } break;
#if REPLAY_BUFFERS
                        case 'L': {
                            if (is_down) {
                                if (state.input_playback_index == 0) {
                                    if (state.input_recording_index == 0) {
                                        begin_recording_input(state, 1);
                                    } else {
                                        end_recording_input(state);
                                        begin_input_playback(state, 1);
                                    }
                                } else {
                                    end_input_playback(state);
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
        case WM_SETCURSOR: {
            if (g_debug_show_cursor) {
                result = DefWindowProcA(hwnd, msg, wparam, lparam);
            } else {
                SetCursor(0);
            }
        } break;
        case WM_SIZE: {
            g_win_dim = Get_window_dimensions(hwnd);
            // ResizeDIBSection(g_backBuffer, g_winDims.width, g_winDims.height);
        } break;
        case WM_DESTROY: {
            running = false;
        } break;
        case WM_CLOSE: {
            running = false;
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP: break;
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(hwnd, &paint);
            s32 x              = paint.rcPaint.left;
            s32 y              = paint.rcPaint.right;
            s32 height         = paint.rcPaint.bottom - paint.rcPaint.top;
            s32 width          = paint.rcPaint.right - paint.rcPaint.left;
            display_buffer_in_window(device_context, g_back_buffer, x, y, width, height);
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
Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, s32 nShowCmd)
{
    // Init

    // WARN
    const TCHAR *icon_path = _T("T:\\data\\tomato.ico");
    auto icon              = (HICON)(LoadImage(NULL, icon_path, IMAGE_ICON, 0, 0,
                                               LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));

    init_console();
    HWND cons_hwnd = GetConsoleWindow();
    TOM_ASSERT(cons_hwnd);
    SendMessage(cons_hwnd, WM_SETICON, NULL, (LPARAM)icon);

    printf("Starting...\n");

    win32_state state {};

    DWORD exe_path_len = GetModuleFileNameA(NULL, state.exe_path, sizeof(state.exe_path));
    printf("exe path %s\n", state.exe_path);

#if _CPPUWIND
    printf("Exceptions are enabled!\n");
#endif

    LARGE_INTEGER performance_query_result;
    QueryPerformanceFrequency(&performance_query_result);
    g_performance_counter_frequency = performance_query_result.QuadPart;

    auto code = load_game_code(g_game_DLL_name);
    load_Xinput();

#ifdef TOM_INTERNAL
    g_debug_show_cursor = true;
#else
    debug_show_cursor   = false;
#endif

    WNDCLASS window_class = {};  // should init to 0n

    static constexpr s32 win_width  = 960;
    static constexpr s32 win_height = 540;

    resize_DIB_section(g_back_buffer, win_width, win_height);

    window_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc   = WndProc;
    window_class.hInstance     = hInstance;
    window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window_class.hIcon         = icon;
    window_class.lpszClassName = _T("TomatoWinCls");

    if (!RegisterClass(&window_class)) {
        printf("ERROR--> Failed to register window class!\n");
        TOM_ASSERT(false);
        return 0;
    }

    DWORD dw_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    // DWORD dw_style = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU;

    RECT wr;
    wr.left   = 0;
    wr.top    = 0;
    wr.right  = win_width + wr.left;
    wr.bottom = win_height + wr.top;

    if (AdjustWindowRect(&wr, dw_style, false) == 0) {
        printf("ERROR--> Failed to adjust window rect");
        TOM_ASSERT(false);
    }

    HWND hwnd = CreateWindowEx(0, window_class.lpszClassName, _T("TomatoGame"), dw_style,
                               CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
                               NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        printf("Failed to create window!\n");
        TOM_ASSERT(hwnd);
        return 0;
    }

    ::ShowWindow(hwnd, SW_SHOWNORMAL);

    HRESULT hr;
    hr = GetLastError();

    // BOOL fOK;
    // TCHAR msgBuf[128];
    // fOK = FormatMessage(
    //     FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
    //     FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, 0, (PTSTR)&msgBuf, 0, NULL);
    // // if (!fOK) msgBuf = _T("Failed to format Message!");
    // _tprintf(TEXT("%d\t%s\n"), hr, msgBuf);
    // LocalFree(msgBuf);

    // NOTE: Set the windows schedule granularity to 1ms
    // so sleep will be more granular
    UINT desired_scheduler_MS = 1;
    b32 is_sleep_granular     = (timeBeginPeriod(desired_scheduler_MS) == TIMERR_NOERROR);
    is_sleep_granular         = false;

    HDC device_context            = GetDC(hwnd);
    running                       = true;
    g_pause                       = false;
    g_back_buffer.bytes_per_pixel = 4;

    s32 monitor_refresh_rate = GetDeviceCaps(device_context, VREFRESH);
    printf("Monitor Refresh Rate: %d\n", monitor_refresh_rate);

    sound_output sound_output         = {};
    sound_output.samples_per_sec      = 48000;
    sound_output.bytes_per_sample     = sizeof(s16) * 2;
    sound_output.secondary_buf_size   = sound_output.samples_per_sec;
    sound_output.latency_sample_count = s32(
        g_frames_of_audio_latency * f32(sound_output.samples_per_sec / (f32)g_game_update_hertz));

    init_WASAPI(sound_output.samples_per_sec, sound_output.secondary_buf_size);
    g_audio_client->Start();

    // TODO: Pool with bitmap VirtualAlloc
    s16 *samples = (s16 *)VirtualAlloc(0, sound_output.secondary_buf_size, MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)TERABYTES((u64)2);
#else
    LPVOID base_address = 0;
#endif

    game_memory memory                = {};
    memory.permanent_storage_size     = MEGABYTES(256);
    memory.transient_storage_size     = GIGABYTES(1);
    memory.platform_free_file_memory  = _debug_platform_free_file_memory;
    memory.platfrom_read_entire_file  = _debug_platform_read_entire_file;
    memory.platform_write_entire_file = _debug_platform_write_entire_file;

    state.total_size = memory.permanent_storage_size + memory.transient_storage_size;
    // TODO: use large pages
    state.game_memory_block =
        VirtualAlloc(base_address, state.total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    memory.permanent_storage = state.game_memory_block;

    memory.transient_storage = ((u8 *)memory.permanent_storage + memory.permanent_storage_size);

#if REPLAY_BUFFERS
    // mapping memory to file
    for (s32 replay_index {}; replay_index < ArrayCount(state.replay_buffers); ++replay_index) {
        replay_buffer &replay_buffer = state.replay_buffers[replay_index];
        _stprintf_s(replay_buffer.file_name, sizeof(TCHAR) * 512, _T("replay_%d_state.ti"),
                    replay_index);

        replay_buffer.file_handle =
            CreateFile(replay_buffer.file_name, GENERIC_WRITE | GENERIC_READ, NULL, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

        LARGE_INTEGER max_size;
        max_size.QuadPart = state.total_size;
        replay_buffer.memory_map =
            CreateFileMapping(replay_buffer.file_handle, NULL, PAGE_READWRITE, max_size.HighPart,
                              max_size.LowPart, NULL);
        DWORD error = GetLastError();
        replay_buffer.memory_block =
            MapViewOfFile(replay_buffer.memory_map, FILE_MAP_ALL_ACCESS, 0, 0, state.total_size);
        TOM_ASSERT(replay_buffer.memory_block);
    }
#endif

    game_input input[2]   = {};
    game_input *new_input = &input[0];
    game_input *old_input = &input[1];

    LARGE_INTEGER last_counter = get_wall_clock();
    u64 last_cycle_count       = __rdtsc();

    bool is_sound_valid      = true;
    bool is_game_code_loaded = true;
    DWORD last_play_cursor {};
    DWORD last_write_cursor {};

    // =============================================================================================
    // #MAIN LOOP
    // =============================================================================================
    while (running) {
        do_controller_input(*old_input, *new_input, hwnd);
        process_pending_messages(state, *new_input);
        // NOTE: this isn't calculated and needs to be for a varaible framerate
        new_input->delta_time = g_target_frames_per_second;

        auto dllWriteTime = get_last_write_time(g_game_DLL_name);
        if (CompareFileTime(&dllWriteTime, &code.last_write_time_DLL)) {
            unload_game_code(code);
            code = load_game_code(g_game_DLL_name);
            printf("New Game Code loaded!\n");
        }

        // NOTE: temp program exit from controller
        if (new_input->controllers->button_back.ended_down) {
            running = false;
        }

        // #Sound

        REFERENCE_TIME latency {};
        if (SUCCEEDED(g_audio_client->GetStreamLatency(&latency))) {
        } else {
            printf("ERROR--> Failed to get audio latency\n");
        }

        s32 samples_to_write = 0;
        UINT32 sound_pad_size;
        if (SUCCEEDED(g_audio_client->GetCurrentPadding(&sound_pad_size))) {
            s32 maxSampleCnt = s32(sound_output.secondary_buf_size - sound_pad_size);
            samples_to_write = s32(sound_output.latency_sample_count - sound_pad_size);
            if (samples_to_write < 0) samples_to_write = 0;
            // TOM_ASSERT(samplesToWrite < maxSampleCnt);
        }

        game_sound_output_buffer sound_buffer {};
        sound_buffer.samples_per_second = sound_output.samples_per_sec;
        sound_buffer.sample_count       = samples_to_write;
        sound_buffer.samples            = samples;

        // video
        game_offscreen_buffer buffer = {};
        buffer.memory                = g_back_buffer.memory;
        buffer.width                 = g_back_buffer.width;
        buffer.height                = g_back_buffer.height;
        buffer.bytes_per_pixel       = 4;
        buffer.pitch                 = g_back_buffer.pitch;

#if REPLAY_BUFFERS
        if (state.input_recording_index) {
            record_input(state, *new_input);
        }
        if (state.input_playback_index) {
            playback_input(state, *new_input);
        }
#endif
        // null check for stub sections
        is_game_code_loaded = code.update_and_render && code.get_sound_samples;
        // isGameCodeLoaded = false;

        // NOTE: dummy thread context, for now
        thread_context thread = {};

        if (is_game_code_loaded) {
            code.update_and_render(&thread, memory, *input, buffer, sound_buffer);
            code.get_sound_samples(&thread, memory, sound_buffer);
        }

        fill_sound_buffer(sound_output, samples_to_write, sound_buffer);

        // clock stuffs
        auto work_counter        = get_wall_clock();
        f32 work_seconds_elapsed = get_seconds_elapsed(last_counter, work_counter);

        f32 seconds_elapsed_for_frame = work_seconds_elapsed;
        if (seconds_elapsed_for_frame < g_target_frames_per_second) {
            if (is_sleep_granular) {
                auto sleepMs =
                    DWORD(1000.f * (g_target_frames_per_second - seconds_elapsed_for_frame));
                if (sleepMs > 0) {
                    ::Sleep(sleepMs);
                }
            }
            f32 test_seconds_elapsed_for_frame =
                get_seconds_elapsed(last_counter, get_wall_clock());
            while (seconds_elapsed_for_frame < g_target_frames_per_second) {
                seconds_elapsed_for_frame = get_seconds_elapsed(last_counter, get_wall_clock());
            }
        } else {
            printf("WARNING--> missed frame timing!!!\n");
        }

        auto end_counter = get_wall_clock();
        f32 ms_per_frame = 1000.f * get_seconds_elapsed(last_counter, end_counter);
        // printf("%f\n", ms_per_frame);

        last_counter = end_counter;

        // NOTE: this is debug code
        display_buffer_in_window(device_context, g_back_buffer, 0, 0, g_win_dim.width,
                                 g_win_dim.height);

        DWORD play_cursor;
        DWORD write_cursor;

        UINT64 frequency_position;
        UINT64 units_posisition;

        g_audio_clock->GetFrequency(&frequency_position);
        g_audio_clock->GetPosition(&units_posisition, 0);

        play_cursor =
            (DWORD)(sound_output.samples_per_sec * units_posisition / frequency_position) %
            sound_output.samples_per_sec;
        write_cursor =
            (DWORD)(sound_output.samples_per_sec * units_posisition / frequency_position) %
                sound_output.samples_per_sec +
            sound_pad_size;
        if (write_cursor > sound_output.secondary_buf_size) {
            write_cursor -= sound_output.secondary_buf_size;
        }

        game_input *temp_input = new_input;
        new_input              = old_input;
        old_input              = temp_input;

        u64 end_cycle_count = __rdtsc();
        u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
        last_cycle_count    = end_cycle_count;
    }

    return 0;
}

//========================================================================================
// ENTRY POINT
//========================================================================================

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement                = 0x00000001;
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

    s32 ecode = Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    CoUninitialize();

    return ecode;
}
