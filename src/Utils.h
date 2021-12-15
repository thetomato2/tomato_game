#ifndef UTILS_H_
#define UTILS_H_

#include "tomato_framework.h"

namespace tomato::util
{
inline szt
get_wstr_len(const wchar* wstr)
{
	szt len {};
	while (*wstr++) {
		++len;
	}
	return len;
}

inline szt
get_str_len(const char* str)
{
	szt len {};
	while (*str++) {
		++len;
	}
	return len;
}

// Takes strings length inputs
inline void
cat_str(const char* left, szt left_len, const char* right, szt right_len, char* out)
{
	for (szt i {}; i < left_len; ++i) {
		*out++ = *left++;
	}
	for (szt i {}; i < right_len; ++i) {
		*out++ = *right++;
	}
	*out = '\0';
}

// calcs the string lengths also
inline void
cat_str(const char* left, const char* right, char* out)
{
	auto left_len  = get_str_len(left);
	auto right_len = get_str_len(right);

	if (!left_len || !right_len) {
		printf("Empty strings passed in!");
		return;
	}

	for (szt i {}; i < left_len; ++i) {
		*out++ = *left++;
	}
	for (szt i {}; i < right_len; ++i) {
		*out++ = *right++;
	}
	*out = '\0';
}

namespace win32
{

// Convert a wide Unicode string to an UTF8 string
inline szt
get_wstr_sz(const wchar* wstr)
{
	auto len  = get_wstr_len(wstr);
	auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (i32)len, NULL, 0, NULL, NULL);
	return size;
}

inline szt
get_wstr_sz(const char* str)
{
	auto len  = get_str_len(str);
	auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (i32)len, NULL, 0);
	return size;
}

// FIXME: idk if this is working
inline void
wstr_to_str(const wchar* wstr, char* buf)
{
	auto len  = get_wstr_len(wstr);
	auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (i32)len, buf, 0, NULL, NULL);
}

inline void
str_to_wstr(const char* str, wchar* buf)
{
	auto len  = get_str_len(str);
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

#endif
