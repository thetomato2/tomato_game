#pragma once
#include "framework.hpp"

namespace tomato
{
class Console
{
public:
    Console()
    {
        _is_initialized = AllocConsole();

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

        _hWnd = GetConsoleWindow();
    }

// NOTE: you shouldn't try to make more than one console window (unless you want to)
#ifndef MULTI_CONSOLE
    Console(const Console &) = delete;
    Console(Console &&)      = delete;
    Console &
    operator=(Console &&) = delete;
    Console &
    operator=(const Console &) = delete;
#endif

    explicit operator bool() const { return _is_initialized; }

    void
    set_icon(const HICON &icon_)
    {
        if (_hWnd && icon_) SendMessage(_hWnd, WM_SETICON, NULL, (LPARAM)icon_);
    }

    ~Console() {}

private:
    bool _is_initialized = false;
    HWND _hWnd           = nullptr;
    HICON _icon          = nullptr;
};

}  // namespace tomato
