#ifndef TOMATO_UTILS_HPP_
#define TOMATO_UTILS_HPP_

#include "tomato_platform.h"

inline szt
get_wstr_len(const wchar *wstr_)
{
    szt len;
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

// #ifdef TOM_WIN32
#if 0
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
#else
inline szt
get_wstr_sz(const wchar *wstr_)
{
    // TODO: stub
    return -1;
}

inline szt
get_wstr_sz(const char *str_)
{
    // TODO: stub
    return -1;
}

// FIXME: idk if this is working
inline void
wstr_to_str(const wchar *wstr_, char *buf_)
{
    // TODO: stub
}

inline void
str_to_wstr(const char *str_, wchar *buf_)
{
    // TODO: stub
}
#endif

#endif  //  TOMATO_UTILS_HPP_
