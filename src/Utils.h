#pragma once
#include "TomatoFramework.h"

namespace tomato::util
{
inline szt
GetWStrLen(const wchar* wstr)
{
	szt len {};
	while (*wstr++) {
		++len;
	}
	return len;
}

inline szt
GetStrLen(const char* str)
{
	szt len {};
	while (*str++) {
		++len;
	}
	return len;
}

// Takes strings lenght inputs
inline void
CatStrings(const char* left, szt leftLen, const char* right, szt rightLen, char* out)
{
	for (szt i {}; i < leftLen; ++i) {
		*out++ = *left++;
	}
	for (szt i {}; i < rightLen; ++i) {
		*out++ = *right++;
	}
	*out = '\0';
}

// calcs the string lenghts also
inline void
CatStrings(const char* left, const char* right, char* out)
{
	auto leftLen  = GetStrLen(left);
	auto rightLen = GetStrLen(right);

	if (!leftLen || !rightLen) {
		printf("Empty strings passed in!");
		return;
	}

	for (szt i {}; i < leftLen; ++i) {
		*out++ = *left++;
	}
	for (szt i {}; i < rightLen; ++i) {
		*out++ = *right++;
	}
	*out = '\0';
}

namespace win32
{

// Convert a wide Unicode string to an UTF8 string
inline szt
GetWStrSz(const wchar* wstr)
{
	auto len  = GetWStrLen(wstr);
	auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (i32)len, NULL, 0, NULL, NULL);
	return size;
}

inline szt
GetStrSz(const char* str)
{
	auto len  = GetStrLen(str);
	auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (i32)len, NULL, 0);
	return size;
}

// FIXME: idk if this is working
inline void
WStr2Str(const wchar* wstr, char* buf)
{
	auto len  = GetWStrLen(wstr);
	auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (i32)len, buf, 0, NULL, NULL);
}

inline void
Str2WStr(const char* str, wchar* buf)
{
	auto len  = GetStrLen(str);
	auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (i32)len, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, (i32)len, buf, (i32)size);
}

}  // namespace win32
// Returns min or max if input is not in between
template<typename T>
T
bounds(T in, T min, T max) noexcept
{
	if (in < min)
		in = min;
	else if (in > max)
		in = max;

	return in;
}

}  // namespace tomato::util
