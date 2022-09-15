#ifndef TOM_WIN32_HH
#define TOM_WIN32_HH

#include "tom_core.hh"

namespace tom
{

struct Win32State
{
    bool running;
    bool pause;
    bool resize;
    bool device_change;
    bool focus;
    DWORD dw_style;
    DWORD ex_style;
    v2i win_dims;
    i32 ms_scroll;
    WINDOWPLACEMENT win_pos = { sizeof(win_pos) };
    HWND hwnd;
    HINSTANCE hinst;
    HDC hdc;
    HDEVNOTIFY notify;
    const char *cls_name;
    HICON icon;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// #DECLARES
void toggle_fullscreen(Win32State *win32);
void get_cwd(char *buf);
bool dir_exists(const wchar *dir);
bool dir_exists(const char *dir);
v2i get_window_dimensions(HWND hwnd);
void create_dir(const char *dir_name);
void create_console();
void prevent_windows_DPI_scaling();
void create_window(Win32State *win32);
void process_pending_messages(Win32State *win32);

}  // namespace tom

#endif
