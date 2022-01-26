#include "win32_layer.hpp"

namespace tomato
{
#ifdef TOM_INTERNAL

DEBUG_PLATFORM_FREE_FILE_MEMORY(_debug_platform_free_file_memory)
{
    if (memory_) {
        VirtualFree(memory_, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(_debug_platform_read_entire_file)
{
    debug_ReadFileResult file = {};

    HANDLE file_handle =
        CreateFileA(file_name_, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(file_handle, &fileSize)) {
            u32 fileSize32 = math::safe_truncate_u32_to_u64(fileSize.QuadPart);
            file.contents  = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (file.contents) {
                DWORD bytesRead;
                if (ReadFile(file_handle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
                    fileSize32 == bytesRead) {
                    file.content_size = fileSize32;
                } else {
                    _debug_platform_free_file_memory(thread_, file.contents);
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
    bool32 success = false;

    HANDLE file_handle = CreateFileA(file_name_, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if (WriteFile(file_handle, memory_, (DWORD)memory_size_, &bytes_written, 0)) {
            success = (bytes_written == memory_size_);
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

namespace win32
{
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

namespace
{
// windows keymap
enum Keys : byt
{
    none           = 0,
    back           = 0x8,
    tab            = 0x9,
    enter          = 0xd,
    pause          = 0x13,
    caps_lock      = 0x14,
    kana           = 0x15,
    kanji          = 0x19,
    escape         = 0x1b,
    ime_convert    = 0x1c,
    ime_no_convert = 0x1d,
    space          = 0x20,
    pageUp         = 0x21,
    pageDown       = 0x22,
    end            = 0x23,
    home           = 0x24,
    left           = 0x25,
    up             = 0x26,
    right          = 0x27,
    down           = 0x28,
    select         = 0x29,
    print          = 0x2a,
    execute        = 0x2b,
    print_screen   = 0x2c,
    insert         = 0x2d,
    delete_key     = 0x2e,
    help           = 0x2f,
    d0             = 0x30,
    d1             = 0x31,
    d2             = 0x32,
    d3             = 0x33,
    d4             = 0x34,
    d5             = 0x35,
    d6             = 0x36,
    d7             = 0x37,
    d8             = 0x38,
    d9             = 0x39,
    a              = 0x41,
    b              = 0x42,
    c              = 0x43,
    d              = 0x44,
    e              = 0x45,
    f              = 0x46,
    g              = 0x47,
    h              = 0x48,
    i              = 0x49,
    j              = 0x4a,
    k              = 0x4b,
    l              = 0x4c,
    m              = 0x4d,
    n              = 0x4e,
    o              = 0x4f,
    p              = 0x50,
    q              = 0x51,
    r              = 0x52,
    s              = 0x53,
    t              = 0x54,
    u              = 0x55,
    v              = 0x56,
    w              = 0x57,
    x              = 0x58,
    y              = 0x59,
    z              = 0x5a,
    left_windows   = 0x5b,
    right_windows  = 0x5c,
    apps           = 0x5d,
    sleep          = 0x5f,
    num_pad_0      = 0x60,
    num_pad_1      = 0x61,
    num_pad_2      = 0x62,
    num_pad_3      = 0x63,
    num_pad_4      = 0x64,
    num_pad_5      = 0x65,
    num_pad_6      = 0x66,
    num_pad_7      = 0x67,
    num_pad_8      = 0x68,
    num_pad_9      = 0x69,
    multiply       = 0x6a,
    add            = 0x6b,
    separator      = 0x6c,
    subtract       = 0x6d,
    decimal        = 0x6e,
    divide         = 0x6f,
    f1             = 0x70,
    f2             = 0x71,
    f3             = 0x72,
    f4             = 0x73,
    f5             = 0x74,
    f6             = 0x75,
    f7             = 0x76,
    f8             = 0x77,
    f9             = 0x78,
    f10            = 0x79,
    f11            = 0x7a,
    f12            = 0x7b,
    f13            = 0x7c,
    f14            = 0x7d,
    f15            = 0x7e,
    f16            = 0x7f,
    f17            = 0x80,
    f18            = 0x81,
    f19            = 0x82,
    f20            = 0x83,
    f21            = 0x84,
    f22            = 0x85,
    f23            = 0x86,
    f24            = 0x87,
    num_lock       = 0x90,
    scroll         = 0x91,
    left_shift     = 0xa0,
    right_shift    = 0xa1,
    left_control   = 0xa2,
    right_control  = 0xa3,
    left_alt       = 0xa4,
    right_alt      = 0xa5,
    semicolon      = 0xba,
    plus           = 0xbb,
    comma          = 0xbc,
    minus          = 0xbd,
    period         = 0xbe,
    question       = 0xbf,
    tilde          = 0xc0,
    open_brackets  = 0xdb,
    pipe           = 0xdc,
    close_brackets = 0xdd,
    quotes         = 0xde,
    oem8           = 0xdf,
    backslash      = 0xe2,
    process_key    = 0xe5,
};

namespace global
{
constexpr u32 game_update_hertz        = 60;
constexpr f32 target_frames_per_second = 1.f / (f32)game_update_hertz;

bool running;
bool pause;

WINDOWPLACEMENT win_pos    = { sizeof(win_pos) };
const TCHAR *game_DLL_name = _T("tomato_game.dll");
bool debug_show_cursor;

Offscreen_Buffer back_buffer;
Win_Dim win_dim;
s64 performance_counter_frequency;

// TODO: the sleep precision issue is keeping this above 1 frame... I think
constexpr f32 frames_of_audio_latency = (1.1f / 30) * game_update_hertz;
IAudioClient *audio_client;
IAudioRenderClient *audio_render_client;
IAudioClock *audio_clock;
}  // namespace global

void
toggle_fullscreen(HWND hWnd_)
{
    DWORD dwStyle = GetWindowLong(hWnd_, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hWnd_, &global::win_pos) &&
            GetMonitorInfo(MonitorFromWindow(hWnd_, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hWnd_, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hWnd_, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hWnd_, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hWnd_, &global::win_pos);
        SetWindowPos(hWnd_, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

inline FILETIME
get_last_write_time(const TCHAR *file_name_)
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
    if (GetFileAttributesEx(file_name_, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

Game_Code
load_game_code(const TCHAR *DLL_name_)
{
    Game_Code code {};
    const TCHAR *DLL_copy = _T("loaded_gamecode_copy.dll");

    code.last_write_time_DLL = get_last_write_time(DLL_name_);

    CopyFile(DLL_name_, DLL_copy, FALSE);

    code.game_code_DLL = LoadLibrary(DLL_copy);
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

void
unload_game_code(Game_Code &game_code_)
{
    if (game_code_.game_code_DLL) {
        FreeLibrary(game_code_.game_code_DLL);
        game_code_.update_and_render = 0;
        game_code_.get_sound_samples = 0;
    }
    printf("Game code unloaded.");
}

void
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

void
init_WASAPI(s32 samples_per_second_, s32 buffer_size_in_samples_)
{
    if (FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) {
        assert(false);
    }

    IMMDeviceEnumerator *enumerator;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&enumerator)))) {
        assert(false);
    }

    IMMDevice *device;
    if (FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        assert(false);
    }

    if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL,
                                (LPVOID *)&global::audio_client))) {
        assert(false);
    }

    WAVEFORMATEXTENSIBLE wave_format;

    wave_format.Format.cbSize         = sizeof(wave_format);
    wave_format.Format.wFormatTag     = WAVE_FORMAT_EXTENSIBLE;
    wave_format.Format.wBitsPerSample = 16;
    wave_format.Format.nChannels      = 2;
    wave_format.Format.nSamplesPerSec = (DWORD)samples_per_second_;
    wave_format.Format.nBlockAlign =
        (WORD)(wave_format.Format.nChannels * wave_format.Format.wBitsPerSample / 8);
    wave_format.Format.nAvgBytesPerSec =
        wave_format.Format.nSamplesPerSec * wave_format.Format.nBlockAlign;
    wave_format.Samples.wValidBitsPerSample = 16;
    wave_format.dwChannelMask               = KSAUDIO_SPEAKER_STEREO;
    wave_format.SubFormat                   = KSDATAFORMAT_SUBTYPE_PCM;

    REFERENCE_TIME buffer_duration = 10000000ULL * buffer_size_in_samples_ /
                                     samples_per_second_;  // buffer size in 100 nanoseconds
    if (FAILED(global::audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                AUDCLNT_STREAMFLAGS_NOPERSIST, buffer_duration, 0,
                                                &wave_format.Format, nullptr))) {
        assert(false);
    }

    if (FAILED(global::audio_client->GetService(IID_PPV_ARGS(&global::audio_render_client)))) {
        assert(false);
    }

    UINT32 soundFrmCnt;
    if (FAILED(global::audio_client->GetBufferSize(&soundFrmCnt))) {
        assert(false);
    }

    if (FAILED(global::audio_client->GetService(IID_PPV_ARGS(&global::audio_clock)))) {
        assert(false);
    }

    // Check if we got what we requested (better would to pass this value back
    // as real buffer size)
    assert(buffer_size_in_samples_ <= (s32)soundFrmCnt);
}

void
fill_sound_buffer(Sound_Output &sound_output_, s32 samples_to_write_,
                  Game_Sound_Output_Buffer &source_buffer)
{
    {
        BYTE *soundBufDat;
        if (SUCCEEDED(
                global::audio_render_client->GetBuffer((UINT32)samples_to_write_, &soundBufDat))) {
            s16 *sourceSample = source_buffer.samples;
            s16 *destSample   = (s16 *)soundBufDat;
            for (szt i = 0; i < samples_to_write_; ++i) {
                *destSample++ = *sourceSample++;
                *destSample++ = *sourceSample++;
                ++sound_output_.running_sample_index;
            }

            global::audio_render_client->ReleaseBuffer((UINT32)samples_to_write_, 0);
        }
    }
}

Win_Dim
Get_window_dimensions(HWND hWnd_)
{
    RECT client_rect;
    Win_Dim WinDim;
    GetClientRect(hWnd_, &client_rect);
    WinDim.width  = client_rect.right - client_rect.left;
    WinDim.height = client_rect.bottom - client_rect.top;
    return WinDim;
}

void
resize_DIB_section(Offscreen_Buffer &buffer_, s32 width_, s32 height_)
{
    // TODO: bulletproof this
    // maybe don't free first, free after, then free first if that fails

    if (buffer_.memory) {
        VirtualFree(buffer_.memory, 0, MEM_RELEASE);
    }
    buffer_.width           = width_;
    buffer_.height          = height_;
    buffer_.bytes_per_pixel = 4;

    buffer_.info.bmiHeader.biSize        = sizeof(buffer_.info.bmiHeader);
    buffer_.info.bmiHeader.biWidth       = width_;
    buffer_.info.bmiHeader.biHeight      = -height_;
    buffer_.info.bmiHeader.biPlanes      = 1;
    buffer_.info.bmiHeader.biBitCount    = 32;
    buffer_.info.bmiHeader.biCompression = BI_RGB;

    s32 bytes_per_pixel    = 4;
    s32 bitmap_memory_size = (width_ * height_) * bytes_per_pixel;
    buffer_.memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    buffer_.pitch = width_ * buffer_.bytes_per_pixel;
}

void
display_buffer_in_window(HDC hdc_, Offscreen_Buffer &buffer_, s32 x_, s32 y_, s32 width_,
                         s32 height_)
{
    if (width_ == buffer_.width * 2 && height_ == buffer_.height * 2) {
        ::StretchDIBits(hdc_, 0, 0, width_, height_, 0, 0, buffer_.width, buffer_.height,
                        buffer_.memory, &buffer_.info, DIB_RGB_COLORS, SRCCOPY);
    } else {
        s32 offset_x = 0;
        s32 offset_y = 0;

#if 0
    // NOTE: this causes screen flickering - out of sync with screen refresh rate?
    ::PatBlt(hdc_, 0, 0, width_, offset_y, BLACKNESS);
    ::PatBlt(hdc_, 0, offset_y + buffer_.height, width_, height_, BLACKNESS);
    ::PatBlt(hdc_, 0, 0, offset_x, height_, BLACKNESS);
    ::PatBlt(hdc_, offset_x + buffer_.width, 0, width_, height_, BLACKNESS);

#endif
        // s32 x_offset = 0;
        // s32 y_offset = 0;

        // NOTE: this is matches the windows dimensions
        ::StretchDIBits(hdc_, offset_x, offset_y, buffer_.width, buffer_.height, 0, 0,
                        buffer_.width, buffer_.height, buffer_.memory, &buffer_.info,
                        DIB_RGB_COLORS, SRCCOPY);
    }
}

void
process_keyboard_message(Game_Button_State &new_state_, const bool32 is_down_)
{
    if (new_state_.ended_down != (is_down_ != 0)) {
        new_state_.ended_down = is_down_;
        ++new_state_.half_transition_count;
    }
}

void
process_Xinput_digital_button(DWORD Xinput_button_state_, Game_Button_State &old_state_,
                              DWORD button_bit_, Game_Button_State &new_state_)
{
    new_state_.ended_down = ((Xinput_button_state_ & button_bit_) == button_bit_);
    if (new_state_.ended_down && old_state_.ended_down)
        new_state_.half_transition_count = ++old_state_.half_transition_count;
}

void
do_controller_input(Game_Input &old_input_, Game_Input &new_input_, HWND hWnd_)
{
    // mouse cursor
    POINT mouse_point;
    GetCursorPos(&mouse_point);
    ScreenToClient(hWnd_, &mouse_point);
    new_input_.mouse_x = mouse_point.x;
    new_input_.mouse_y = mouse_point.y;
    new_input_.mouse_z = 0;

    // mouse buttons
    process_keyboard_message(new_input_.mouse_buttons[0], ::GetKeyState(VK_LBUTTON) & (1 << 15));
    process_keyboard_message(new_input_.mouse_buttons[1], ::GetKeyState(VK_RBUTTON) & (1 << 15));
    process_keyboard_message(new_input_.mouse_buttons[2], ::GetKeyState(VK_MBUTTON) & (1 << 15));

    for (szt key {}; key < Game_Keyboard_Input::s_key_cnt; ++key) {
        if (old_input_.keyboard.keys[key].half_transition_count > 0 &&
            old_input_.keyboard.keys[key].ended_down == 0)
            old_input_.keyboard.keys[key].half_transition_count = 0;
    }

    // keyboard
    process_keyboard_message(new_input_.keyboard.enter, ::GetKeyState(Keys::enter) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.w, ::GetKeyState(Keys::w) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.a, ::GetKeyState(Keys::a) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.s, ::GetKeyState(Keys::s) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.d, ::GetKeyState(Keys::d) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.p, ::GetKeyState(Keys::p) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.t, ::GetKeyState(Keys::t) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.d1, ::GetKeyState(Keys::d1) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.d2, ::GetKeyState(Keys::d2) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.d3, ::GetKeyState(Keys::d3) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.d4, ::GetKeyState(Keys::d4) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.d5, ::GetKeyState(Keys::d5) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.space, ::GetKeyState(Keys::space) & (1 << 15));
    process_keyboard_message(new_input_.keyboard.left_shift,
                             ::GetKeyState(Keys::left_shift) & (1 << 15));

    // controller
    // poll the input device
    s32 max_controller_count = XUSER_MAX_COUNT;
    if (max_controller_count > 4) {
        max_controller_count = 4;
    }

    for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
        Game_Controller_Input &old_controller = old_input_.controllers[controller_index];
        Game_Controller_Input &new_controller = new_input_.controllers[controller_index];

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

            v2 stick_left;
            v2 stick_right;
            stick_left.x  = normalize(pad.sThumbLX);
            stick_left.y  = normalize(pad.sThumbLY);
            stick_right.x = normalize(pad.sThumbRX);
            stick_right.y = normalize(pad.sThumbRY);

            new_controller.min.x = new_controller.max.x = new_controller.end_left_stick.x =
                stick_left.x;
            new_controller.min.y = new_controller.max.y = new_controller.end_left_stick.y =
                stick_left.y;

            for (szt button {}; button < Game_Controller_Input::s_button_cnt; ++button) {
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

inline LARGE_INTEGER
get_wall_clock()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time;
}

inline f32
get_seconds_elapsed(LARGE_INTEGER start_, LARGE_INTEGER end_)
{
    f32 seconds = f32(end_.QuadPart - start_.QuadPart) / f32(global::performance_counter_frequency);
    return seconds;
}

// ===============================================================================================
// #PLAYBACK
// ===============================================================================================

#if REPLAY_BUFFERS == 1
void
get_input_file_path(Win32_State &state_, bool32 is_input_stream_)
{
    int x = 0;
}

Replay_Buffer &
get_replay_buffer(Win32_State &state_, szt index_)
{
    assert(index_ < ArrayCount(state_.replay_buffers));
    return state_.replay_buffers[index_];
}

void
begin_recording_input(Win32_State &state_, s32 input_recording_index_)
{
    auto &replay_buffer = get_replay_buffer(state_, input_recording_index_);
    if (replay_buffer.memory_block) {
        printf("Recording...\n");
        state_.input_recording_index = input_recording_index_;

        TCHAR file_name[512];
        _stprintf_s(file_name, 512, _T("replay_%d_input.ti"), input_recording_index_);
        // const TCHAR* fileName = _T("replay_1_input.ti");
        state_.recording_handle = CreateFile(file_name, GENERIC_WRITE | GENERIC_READ, NULL, NULL,
                                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

        CopyMemory(replay_buffer.memory_block, state_.game_memory_block, state_.total_size);
    }
}

void
end_recording_input(Win32_State &state_)
{
    printf("Recording ended.\n");
    CloseHandle(state_.recording_handle);
    state_.input_recording_index = 0;
}

void
begin_input_playback(Win32_State &state_, s32 input_playback_index_)
{
    auto &replay_buffer = get_replay_buffer(state_, input_playback_index_);
    if (replay_buffer.memory_block) {
        printf("Input Playback started...\n");
        state_.input_playback_index = input_playback_index_;

        TCHAR file_name[512];
        _stprintf_s(file_name, 512, _T("replay_%d_input.ti"), input_playback_index_);
        state_.playback_handle = CreateFile(file_name, GENERIC_READ, NULL, NULL, OPEN_EXISTING,
                                            FILE_ATTRIBUTE_NORMAL, 0);

        CopyMemory(state_.game_memory_block, replay_buffer.memory_block, state_.total_size);
    }
}

void
end_input_playback(Win32_State &state_)
{
    printf("Input playback ended.\n");
    CloseHandle(state_.playback_handle);
    state_.input_playback_index = 0;
}

void
record_input(Win32_State &state_, Game_Input &new_input_)
{
    DWORD bytes_written;
    WriteFile(state_.recording_handle, &new_input_, sizeof(new_input_), &bytes_written, 0);
}

void
playback_input(Win32_State &state_, Game_Input &new_input_)
{
    DWORD bytes_read;
    if (ReadFile(state_.playback_handle, &new_input_, sizeof(new_input_), &bytes_read, 0)) {
        if (bytes_read == 0) {
            // NOTE: hit end of stream, go back to begining;
            s32 playback_index = state_.input_playback_index;
            end_input_playback(state_);
            begin_input_playback(state_, playback_index);
            ReadFile(state_.playback_handle, &new_input_, sizeof(new_input_), &bytes_read, 0);
        }
    }
}
#endif

void
process_pending_messages(Win32_State &state_, Game_Input &input_)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            case WM_QUIT: global::running = false; break;
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
                        case VK_ESCAPE: global::running = false; break;
                        case 'P': {
                            if (is_down) {
                                global::pause = !global::pause;
                            }
                        } break;

                        case (VK_RETURN): {
                            if (alt_key_down) {
                                toggle_fullscreen(msg.hwnd);
                            }
                        } break;
                        case (VK_F4): {
                            if (alt_key_down) {
                                global::running = false;
                            }
                        } break;
#if REPLAY_BUFFERS
                        case 'L': {
                            if (is_down) {
                                if (state_.input_playback_index == 0) {
                                    if (state_.input_recording_index == 0) {
                                        begin_recording_input(state_, 1);
                                    } else {
                                        end_recording_input(state_);
                                        begin_input_playback(state_, 1);
                                    }
                                } else {
                                    end_input_playback(state_);
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
WndProc(HWND hWnd, UINT msg_, WPARAM wParam_, LPARAM lParam_)
{
    LRESULT result = 0;
    switch (msg_) {
        case WM_SETCURSOR: {
            if (global::debug_show_cursor) {
                result = DefWindowProcA(hWnd, msg_, wParam_, lParam_);
            } else {
                SetCursor(0);
            }
        } break;
        case WM_SIZE: {
            global::win_dim = Get_window_dimensions(hWnd);
            // ResizeDIBSection(g_backBuffer, g_winDims.width, g_winDims.height);
        } break;
        case WM_DESTROY: {
            global::running = false;
        } break;
        case WM_CLOSE: {
            global::running = false;
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP: break;
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(hWnd, &paint);
            s32 x              = paint.rcPaint.left;
            s32 y              = paint.rcPaint.right;
            s32 height         = paint.rcPaint.bottom - paint.rcPaint.top;
            s32 width          = paint.rcPaint.right - paint.rcPaint.left;
            display_buffer_in_window(device_context, global::back_buffer, x, y, width, height);
            EndPaint(hWnd, &paint);
        } break;
        default:
            //            OutPutDebugStringA("default\n");
            result = DefWindowProc(hWnd, msg_, wParam_, lParam_);
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
    auto console = new Console();
    assert(console);
    printf("Starting...\n");

    Win32_State state {};

    DWORD exe_path_len = GetModuleFileNameA(NULL, state.exe_path, sizeof(state.exe_path));
    printf("exe path %s\n", state.exe_path);

    LARGE_INTEGER performance_query_result;
    QueryPerformanceFrequency(&performance_query_result);
    global::performance_counter_frequency = performance_query_result.QuadPart;

    auto code = load_game_code(global::game_DLL_name);
    load_Xinput();

#ifdef TOM_INTERNAL
    global::debug_show_cursor = true;
#else
    global::debug_show_cursor = false;
#endif

    WNDCLASS window_class = {};  // should init to 0n

    static constexpr s32 win_width  = 960;
    static constexpr s32 win_height = 540;

    resize_DIB_section(global::back_buffer, win_width, win_height);

    // TODO: install assets eventuallly
    const TCHAR *icon_path = _T("T:\\data\\tomato.ico");
    auto icon              = (HICON)(LoadImage(NULL, icon_path, IMAGE_ICON, 0, 0,
                                               LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));
    console->set_icon(icon);

    window_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc   = WndProc;
    window_class.hInstance     = hInstance;
    window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window_class.hIcon         = icon;
    window_class.lpszClassName = _T("TomatoWinCls");

    if (!RegisterClass(&window_class)) {
        printf("ERROR--> Failed to register window class!\n");
        assert(false);
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
        assert(false);
    }

    HWND hWnd = CreateWindowEx(0, window_class.lpszClassName, _T("TomatoGame"), dw_style,
                               CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
                               NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        printf("Failed to create window!\n");
        assert(hWnd);
        return 0;
    }

    ::ShowWindow(hWnd, SW_SHOWNORMAL);

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
    bool32 is_sleep_granular  = (timeBeginPeriod(desired_scheduler_MS) == TIMERR_NOERROR);
    is_sleep_granular         = false;

    HDC device_context                  = GetDC(hWnd);
    global::running                     = true;
    global::pause                       = false;
    global::back_buffer.bytes_per_pixel = 4;

    s32 monitor_refresh_rate = GetDeviceCaps(device_context, VREFRESH);
    printf("Monitor Refresh Rate: %d\n", monitor_refresh_rate);

    Sound_Output sound_output       = {};
    sound_output.samples_per_sec    = 48000;
    sound_output.bytes_per_sample   = sizeof(s16) * 2;
    sound_output.secondary_buf_size = sound_output.samples_per_sec;
    sound_output.latency_sample_count =
        s32(global::frames_of_audio_latency *
            f32(sound_output.samples_per_sec / (f32)global::game_update_hertz));

    init_WASAPI(sound_output.samples_per_sec, sound_output.secondary_buf_size);
    global::audio_client->Start();

    // TODO: Pool with bitmap VirtualAlloc
    s16 *samples = (s16 *)VirtualAlloc(0, sound_output.secondary_buf_size, MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE);

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)TERABYTES((u64)2);
#else
    LPVOID base_address       = 0;
#endif

    Game_Mem memory                   = {};
    memory.permanent_storage_size     = MEGABYTES(64);
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
        Replay_Buffer &replay_buffer = state.replay_buffers[replay_index];
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
            MapViewOfFile(replay_buffer.memory_map, FILE_MAP_ALL_ACCESS, 0, 0, state.total_size),
        assert(replay_buffer.memory_block);
    }
#endif

    Game_Input input[2]   = {};
    Game_Input *new_input = &input[0];
    Game_Input *old_input = &input[1];

    LARGE_INTEGER last_counter = get_wall_clock();
    u64 last_cycle_count       = __rdtsc();

    bool is_sound_valid      = true;
    bool is_game_code_loaded = true;
    DWORD last_play_cursor {};
    DWORD last_write_cursor {};

    // =============================================================================================
    // #MAIN LOOP
    // =============================================================================================
    while (global::running) {
        do_controller_input(*old_input, *new_input, hWnd);
        process_pending_messages(state, *new_input);
        // NOTE: this isn't calculated and needs to be for a varaible framerate
        new_input->delta_time = global::target_frames_per_second;

        auto dllWriteTime = get_last_write_time(global::game_DLL_name);
        if (CompareFileTime(&dllWriteTime, &code.last_write_time_DLL)) {
            unload_game_code(code);
            code = load_game_code(global::game_DLL_name);
            printf("New Game Code loaded!\n");
        }

        // NOTE: temp program exit from controller
        if (new_input->controllers->button_back.ended_down) {
            global::running = false;
        }

        // #Sound

        REFERENCE_TIME latency {};
        if (SUCCEEDED(global::audio_client->GetStreamLatency(&latency))) {
        } else {
            printf("ERROR--> Failed to get audio latency\n");
        }

        s32 samples_to_write = 0;
        UINT32 sound_pad_size;
        if (SUCCEEDED(global::audio_client->GetCurrentPadding(&sound_pad_size))) {
            s32 maxSampleCnt = s32(sound_output.secondary_buf_size - sound_pad_size);
            samples_to_write = s32(sound_output.latency_sample_count - sound_pad_size);
            if (samples_to_write < 0) samples_to_write = 0;
            // assert(samplesToWrite < maxSampleCnt);
        }

        Game_Sound_Output_Buffer sound_buffer {};
        sound_buffer.samples_per_second = sound_output.samples_per_sec;
        sound_buffer.sample_count       = samples_to_write;
        sound_buffer.samples            = samples;

        // video
        Game_Offscreen_Buffer buffer = {};
        buffer.memory                = global::back_buffer.memory;
        buffer.width                 = global::back_buffer.width;
        buffer.height                = global::back_buffer.height;
        buffer.bytes_per_pixel       = 4;
        buffer.pitch                 = global::back_buffer.pitch;

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
        Thread_Context thread {};

        if (is_game_code_loaded) {
            code.update_and_render(&thread, memory, *input, buffer, sound_buffer);
            code.get_sound_samples(&thread, memory, sound_buffer);
        }

        fill_sound_buffer(sound_output, samples_to_write, sound_buffer);

        // clock stuffs
        auto work_counter        = get_wall_clock();
        f32 work_seconds_elapsed = get_seconds_elapsed(last_counter, work_counter);

        f32 seconds_elapsed_for_frame = work_seconds_elapsed;
        if (seconds_elapsed_for_frame < global::target_frames_per_second) {
            if (is_sleep_granular) {
                auto sleepMs =
                    DWORD(1000.f * (global::target_frames_per_second - seconds_elapsed_for_frame));
                if (sleepMs > 0) {
                    ::Sleep(sleepMs);
                }
            }
            f32 test_seconds_elapsed_for_frame =
                get_seconds_elapsed(last_counter, get_wall_clock());
            while (seconds_elapsed_for_frame < global::target_frames_per_second) {
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
        display_buffer_in_window(device_context, global::back_buffer, 0, 0, global::win_dim.width,
                                 global::win_dim.height);

        DWORD play_cursor;
        DWORD write_cursor;

        UINT64 frequency_position;
        UINT64 units_posisition;

        global::audio_clock->GetFrequency(&frequency_position);
        global::audio_clock->GetPosition(&units_posisition, 0);

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

        Game_Input *temp_input = new_input;
        new_input              = old_input;
        old_input              = temp_input;

        u64 end_cycle_count = __rdtsc();
        u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
        last_cycle_count    = end_cycle_count;
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

    s32 ecode = tomato::win32::Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    CoUninitialize();

    return ecode;
}
