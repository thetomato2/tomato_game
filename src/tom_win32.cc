#include "tom_win32.hh"

namespace tom
{

// TODO: get rid of these??
global bool g_running       = true;
global bool g_pause         = false;
global bool g_resize        = false;
global bool g_device_change = false;
global bool g_focus         = false;
v2i g_win_dim               = {};
i32 g_ms_scroll             = {};

void toggle_fullscreen(Win32State *win32)
{
    DWORD dwStyle = (DWORD)GetWindowLong(win32->hwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(win32->hwnd, &win32->win_pos) &&
            GetMonitorInfo(MonitorFromWindow(win32->hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(win32->hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(win32->hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(win32->hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(win32->hwnd, &win32->win_pos);
        SetWindowPos(win32->hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

v2i get_window_dimensions(HWND hwnd)
{
    v2i result = {};

    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    // GetWindowRect(hwnd, &client_rect);
    result.w = client_rect.right - client_rect.left;
    result.h = client_rect.bottom - client_rect.top;

    return result;
}

void get_cwd(char *buf)
{
    GetCurrentDirectoryA(MAX_PATH, buf);
}

bool dir_exists(const wchar *dir)
{
    DWORD ftyp = GetFileAttributesW(dir);
    if (ftyp == INVALID_FILE_ATTRIBUTES) return false;

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;

    return false;
}

bool dir_exists(const char *dir)
{
    DWORD ftyp = GetFileAttributesA(dir);
    if (ftyp == INVALID_FILE_ATTRIBUTES) return false;

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;

    return false;
}

void create_dir(const char *dir_name)
{
    if (!dir_exists(dir_name)) {
        CreateDirectoryA(dir_name, NULL);
    }
}
#if 0
internal void create_dir(wstring dir_name)
{
    if (!dir_exists(dir_name.c_str())) {
        CreateDirectoryW(dir_name.c_str(), NULL);
    }
}


// recursively deletes the specified directory and all its contents
// absolute path of the directory that will be deleted
// NOTE: the path must not be terminated with a path separator.
internal void rm_rf_dir(const wstring& path)
{
    if (path.back() == L'\\') {
        INVALID_CODE_PATH;
        return;
    }

    wstring all_files_mask = L"\\*";
    WIN32_FIND_DATAW find_data;

    // First, delete the contents of the directory, recursively for subdirectories
    wstring search_mask  = path + all_files_mask;
    HANDLE search_handle = ::FindFirstFileExW(search_mask.c_str(), FindExInfoBasic, &find_data,
                                              FindExSearchNameMatch, nullptr, 0);
    if (search_handle == INVALID_HANDLE_VALUE) {
        DWORD last_err = ::GetLastError();
        if (last_err != ERROR_FILE_NOT_FOUND) {  // or ERROR_NO_MORE_FILES, ERROR_NOT_FOUND?
            INVALID_CODE_PATH;
        }
    }

    // Did this directory have any contents? If so, delete them first
    if (search_handle != INVALID_HANDLE_VALUE) {
        scoped_search_handle scope(search_handle);
        for (;;) {
            // Do not process the obligatory '.' and '..' directories
            if (find_data.cFileName[0] != '.') {
                bool isDirectory =
                ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
                ((find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0);

                // Subdirectories need to be handled by deleting their contents first
                wstring file_path = path + L"\\" + find_data.cFileName;
                if (isDirectory) {
                    rm_rf_dir(file_path);
                } else {
                    BOOL result = ::DeleteFileW(file_path.c_str());
                    if (result == FALSE) {
                        INVALID_CODE_PATH;
                    }
                }
            }

            // Advance to the next file in the directory
            BOOL result = ::FindNextFileW(search_handle, &find_data);
            if (result == FALSE) {
                DWORD last_err = ::GetLastError();
                if (last_err != ERROR_NO_MORE_FILES) {
                    INVALID_CODE_PATH;
                }
                break;  // All directory contents enumerated and deleted
            }

        }  // for
    }

    // The directory is empty, we can now safely remove it
    BOOL result = ::RemoveDirectoryW(path.c_str());
    if (result == FALSE) {
        INVALID_CODE_PATH;
    }
}

#endif

void create_console()
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

void process_pending_messages(Win32State *win32)
{
    win32->running       = g_running;
    win32->pause         = g_pause;
    win32->resize        = g_resize;
    win32->device_change = g_device_change;
    win32->focus         = g_focus;
    win32->win_dims      = g_win_dim;
    win32->ms_scroll     = g_ms_scroll;

    g_resize        = false;
    g_device_change = false;
    g_ms_scroll     = 0;

    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            case WM_QUIT: {
                g_running      = false;
                win32->running = false;
            } break;
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
                        case VK_ESCAPE: {
                            g_running      = false;
                            win32->running = false;
                        } break;
                        case 'P': {
                            if (is_down) {
                                g_pause      = !g_pause;
                                win32->pause = g_pause;
                            }
                        } break;

                        case (VK_RETURN): {
                            if (alt_key_down) {
                                toggle_fullscreen(win32);
                            }
                        } break;
                        case (VK_F4): {
                            if (alt_key_down) {
                                g_running      = false;
                                win32->running = false;
                            }
                        } break;
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch (msg) {
        case WM_SETCURSOR: {
            if (true) {
                result = DefWindowProcA(hwnd, msg, wparam, lparam);
            } else {
                SetCursor(0);
            }
        } break;
        case WM_SIZE: {
            g_win_dim = get_window_dimensions(hwnd);
            g_resize  = true;
        } break;
        case WM_DESTROY: {
            g_running = false;
        } break;
        case WM_CLOSE: {
            g_running = false;
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP: break;
        case WM_MOUSEWHEEL: {
            g_ms_scroll = GET_WHEEL_DELTA_WPARAM(wparam);
        } break;
        case WM_DEVICECHANGE: {
            g_device_change = true;
        } break;
        case WM_KILLFOCUS: {
            g_focus = false;
        } break;
        case WM_SETFOCUS: {
            g_focus = true;
        } break;
        default:
            //            OutPutDebugStringA("default\n");
            result = DefWindowProc(hwnd, msg, wparam, lparam);
            break;
    }
    return result;
}

typedef BOOL WINAPI SetProcessDpiAware(void);
typedef BOOL WINAPI SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT);
void prevent_windows_DPI_scaling()
{
    HMODULE WinUser = LoadLibraryW(L"user32.dll");
    SetProcessDpiAwarenessContext *dpi_context =
        (SetProcessDpiAwarenessContext *)GetProcAddress(WinUser, "SetProcessDPIAwarenessContext");
    if (dpi_context) {
        dpi_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    } else {
        SetProcessDpiAware *dpi_aware =
            (SetProcessDpiAware *)GetProcAddress(WinUser, "SetProcessDPIAware");
        if (dpi_aware) {
            dpi_aware();
        }
    }
}

void create_window(Win32State *win32)
{
    win32->cls_name = "TomatoWinCls";

    WNDCLASS cls { .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                   .lpfnWndProc   = WndProc,
                   .hInstance     = win32->hinst,
                   .hIcon         = win32->icon,
                   .hCursor       = LoadCursor(NULL, IDC_ARROW),
                   .lpszClassName = win32->cls_name };

    if (!RegisterClass(&cls)) {
        printf("ERROR--> Failed to register window class!\n");
        TOM_ASSERT(false);
        return;
    }

    // NOTE(casey): Martins says WS_EX_NOREDIRECTIONBITMAP is necessary to make
    // DXGI_SWAP_EFFECT_FLIP_DISCARD "not glitch on window resizing", and since
    // I don't normally program DirectX and have no idea, we're just going to
    // leave it here :)
    DWORD ex_style = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
    // DWORD ex_style = WS_EX_APPWINDOW;

    DWORD dw_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    // DWORD dw_style = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU;

    RECT wr = { .left   = 0,
                .top    = 0,
                .right  = win32->win_dims.w + wr.left,
                .bottom = win32->win_dims.h + wr.top };

    // if (AdjustWindowRect(&wr, dw_style, false) == 0) {
    //     printf("ERROR--> Failed to adjust window rect");
    //     INVALID_CODE_PATH;
    // }

    if (AdjustWindowRectEx(&wr, dw_style, false, ex_style) == 0) {
        printf("ERROR--> Failed to adjust window rect");
        INVALID_CODE_PATH;
    }

    win32->hwnd = CreateWindowEx(ex_style, cls.lpszClassName, _T("Tomato Game"), dw_style,
                                 CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left,
                                 wr.bottom - wr.top, NULL, NULL, win32->hinst, NULL);

    if (!win32->hwnd) {
        printf("Failed to create window!\n");
        TOM_ASSERT(win32->hwnd);
        return;
    }

    DEV_BROADCAST_DEVICEINTERFACE dev_broadcast = { .dbcc_size =
                                                        sizeof(DEV_BROADCAST_DEVICEINTERFACE),
                                                    .dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE,
                                                    .dbcc_classguid =
                                                        GUID_DEVINTERFACE_USB_DEVICE };

    win32->notify = RegisterDeviceNotification(
        win32->hwnd, &dev_broadcast,
        DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
    TOM_ASSERT(win32->notify);

    ShowWindow(win32->hwnd, SW_SHOWNORMAL);
    UpdateWindow(win32->hwnd);

    HDC hdc = GetDC(win32->hwnd);

    i32 monitor_refresh_rate = GetDeviceCaps(hdc, VREFRESH);
    printf("Monitor Refresh Rate: %d\n", monitor_refresh_rate);
}

}  // namespace tom
