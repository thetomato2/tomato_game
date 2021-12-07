#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "tomato_framework.h"

namespace tomato
{
class Console
{
public:
	Console()
	{
		isInitialized_ = AllocConsole();

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
	Console(Console&&)		= delete;
	Console&
	operator=(Console&&) = delete;
	Console&
	operator=(const Console&) = delete;
#endif

	explicit operator bool() const { return isInitialized_; }

	// FIXME: this should set the console window icon
	void
	setIcon(HICON& icon)
	{
		if (icon != NULL) icon_ = icon;
		HMODULE hKernel32 = ::LoadLibrary(_T("kernel32.dll"));
		typedef BOOL(_stdcall * SetConsoleIconFunc)(HICON);
		SetConsoleIconFunc setConsoleIcon =
			(SetConsoleIconFunc)::GetProcAddress(hKernel32, "SetConsoleIcon");
		if (setConsoleIcon != NULL) setConsoleIcon(icon_);
		::FreeLibrary(hKernel32);
	}

	~Console() {}

private:
	bool isInitialized_ = false;
	HICON icon_			= nullptr;
};

}  // namespace tomato
#endif
