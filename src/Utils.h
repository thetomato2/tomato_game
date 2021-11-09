#pragma once

namespace tomato::util
{

template<typename T>
consteval T to_kilobytes(T val)
{
	return T * val;
}

template<typename T>
consteval T to_megabytes(T val)
{
	return T * kilobytes(val);
}

template<typename T>
consteval T to_gigagbytes(T val)
{
	return T * megabytes(val);
}

template<typename T>
consteval T to_teragbytes(T val)
{
	return T * megabytes(val);
}

// custom std::format print
template<typename... Args>
std::string fmt(std::string_view rt_fmt_str, Args&&... args)
{
	return std::vformat(rt_fmt_str, std::make_format_args(args...));
}
template<typename... Args>
void Print(std::string_view rt_fmt_str, Args&&... args)
{
	std::cout << std::vformat(rt_fmt_str, std::make_format_args(args...)) + '\n';
}
namespace win32
{
static std::wstring TranslateHRESULT(HRESULT hr) noexcept
{
	wchar_t* msgBuf	   = nullptr;
	const DWORD msgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msgBuf), 0, nullptr);
	if (msgLen == 0) {
		return L"Unidentified error code";
	}
	std::wstring errorString = msgBuf;
	LocalFree(msgBuf);
	return errorString;
}

// Convert a wide Unicode string to an UTF8 string
inline std::string ws2s(const std::wstring& wstr) noexcept
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
inline std::wstring s2ws(const std::string& str) noexcept
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
}  // namespace win32
// Returns min or max if input is not in between
template<typename T>
T bounds(T in, T min, T max) noexcept
{
	if (in < min)
		in = min;
	else if (in > max)
		in = max;

	return in;
}

// Push fuctions to this class that will be ran in reverse order when flush() is called
class DeletionQueue
{
public:
	void pushFunction(std::function<void()>&& voidFunc);
	void flush();

private:
	std::vector<std::function<void()>> deletors_;
};

// Helper class for COM exceptions

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		throw std::runtime_error("Hr failed!");
	}
}

// Helper for output debug tracing
inline void DebugTrace(_In_z_ _Printf_format_string_ const char* format, ...) noexcept
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);

	char buff[1024] = {};
	vsprintf_s(buff, format, args);
	OutputDebugStringA(buff);
	va_end(args);
#else
	UNREFERENCED_PARAMETER(format);
#endif
}

// Helper smart-pointers
struct virtual_deleter
{
	void operator()(void* p) noexcept
	{
		if (p) VirtualFree(p, 0, MEM_RELEASE);
	}
};

struct handle_closer
{
	void operator()(HANDLE h) noexcept
	{
		if (h) CloseHandle(h);
	}
};

using ScopedHandle = std::unique_ptr<void, handle_closer>;

inline HANDLE safe_handle(HANDLE h) noexcept
{
	return (h == INVALID_HANDLE_VALUE) ? nullptr : h;
}

}  // namespace tomato::util
