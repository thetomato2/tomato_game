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
    const char* cls_name;
    HICON icon;
};

}  // namespace tom