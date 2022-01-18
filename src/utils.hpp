#pragma once
#include "framework.hpp"

namespace tomato::util
{
#if 0
void
generate_rainbow(Color_u32& color_, f32 frequency_, f32 time_)
{
	f32 f = (f32)sin(time * frequency_) / 2.0f + 0.5f;

	f32 a = (1.0f - f) / 0.2f;
	f32 x = floorf(a);
	f32 y = floorf(255.0f * (a - x));
	f32 red, green, blue;

	if (x == 0.0f) {
		red	  = 255.0f;
		green = y;
		blue  = 0;
	} else if (x == 1.0f) {
		red	  = 255.0f - y;
		green = 255.0f;
		blue  = 0;
	} else if (x == 2.0f) {
		red	  = 0.0f;
		green = 255.0f;
		blue  = y;
	} else if (x == 3.0f) {
		red	  = 0.0f;
		green = 255.0f - y;
		blue  = 255.0f;
	} else if (x == 4.0f) {
		red	  = y;
		green = 0.0f;
		blue  = 255.0f;
	} else if (x == 5.0f) {
		red	  = 255.0f;
		green = 0.0f;
		blue  = 255.0f;
	}

	color_.argb = (0xFF << 24) | ((u8)red << 16) | ((u8)green << 8) | ((u8)blue);
}
#endif

inline szt
get_wstr_len(const wchar *wstr_)
{
    szt len {};
    while (*wstr_++) {
        ++len;
    }
    return len;
}

inline szt
get_str_len(const char *str_)
{
    szt len {};
    while (*str_++) {
        ++len;
    }
    return len;
}

// Takes strings length inputs
inline void
cat_str(const char *left_, szt left_len_, const char *right_, szt right_len_, char *out_)
{
    for (szt i {}; i < left_len_; ++i) {
        *out_++ = *left_++;
    }
    for (szt i {}; i < right_len_; ++i) {
        *out_++ = *right_++;
    }
    *out_ = '\0';
}

// calcs the string lengths also
inline void
cat_str(const char *left_, const char *right_, char *out_)
{
    auto left_len  = get_str_len(left_);
    auto right_len = get_str_len(right_);

    if (!left_len || !right_len) {
        printf("Empty strings passed in!");
        return;
    }

    for (szt i {}; i < left_len; ++i) {
        *out_++ = *left_++;
    }
    for (szt i {}; i < right_len; ++i) {
        *out_++ = *right_++;
    }
    *out_ = '\0';
}

// calcs the string lengths also
inline void
cat_str(const char *left_, const char *right_, char *out_, szt *len_out_)
{
    auto left_len  = get_str_len(left_);
    auto right_len = get_str_len(right_);

    if (!left_len || !right_len) {
        printf("Empty strings passed in!");
        return;
    }

    for (szt i {}; i < left_len; ++i) {
        *out_++ = *left_++;
    }
    for (szt i {}; i < right_len; ++i) {
        *out_++ = *right_++;
    }
    *len_out_ = left_len + right_len;
    *out_     = '\0';
}

namespace win32
{

// Convert a wide Unicode string to an UTF8 string
inline szt
get_wstr_sz(const wchar *wstr_)
{
    auto len  = get_wstr_len(wstr_);
    auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr_, (i32)len, NULL, 0, NULL, NULL);
    return size;
}

inline szt
get_wstr_sz(const char *str_)
{
    auto len  = get_str_len(str_);
    auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str_, (i32)len, NULL, 0);
    return size;
}

// FIXME: idk if this is working
inline void
wstr_to_str(const wchar *wstr_, char *buf_)
{
    auto len  = get_wstr_len(wstr_);
    auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr_, (i32)len, buf_, 0, NULL, NULL);
}

inline void
str_to_wstr(const char *str_, wchar *buf_)
{
    auto len  = get_str_len(str_);
    auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str_, (i32)len, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, str_, (i32)len, buf_, (i32)size);
}

}  // namespace win32
// Returns min or max if input is not in between
template<typename T>
T
bounds(T in_, T min_, T max_) noexcept
{
    if (in_ < min_)
        in_ = min_;
    else if (in_ > max_)
        in_ = max_;

    return in_;
}
}  // namespace tomato::util
