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

        FILE* fDummy;
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

// NOTE: you shouldn't try to make more than one console window
#ifndef MULTI_CONSOLE
    Console(const Console&) = delete;
    Console(Console&&)      = delete;
    Console&
    operator=(Console&&) = delete;
    Console&
    operator=(const Console&) = delete;
#endif

    explicit operator bool() const { return _is_initialized; }

    // FIXME: this should set the console window icon, but it doesn't
    void
    setIcon(HICON& icon_)
    {
        if (icon_ != NULL) _icon = icon_;
        HMODULE hKernel32 = ::LoadLibrary(_T("kernel32.dll"));
        typedef BOOL(_stdcall * SetConsoleIconFunc)(HICON);
        SetConsoleIconFunc setConsoleIcon =
            (SetConsoleIconFunc)::GetProcAddress(hKernel32, "SetConsoleIcon");
        if (setConsoleIcon != NULL) setConsoleIcon(_icon);
        ::FreeLibrary(hKernel32);
    }

    ~Console() {}

private:
    bool _is_initialized = false;
    HICON _icon          = nullptr;
};

}  // namespace tomato
