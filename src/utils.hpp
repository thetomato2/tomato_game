#ifndef TOMATO_UTILS_HPP_
#define TOMATO_UTILS_HPP_

#include "platform.h"

namespace tom
{

inline szt
get_wstr_len(const wchar *wstr)
{
    szt len;
    while (*wstr++) {
        ++len;
    }
    return len;
}

inline szt
get_str_len(const char *str)
{
    szt len {};
    while (*str++) {
        ++len;
    }
    return len;
}

// Takes strings length inputs
inline void
cat_str(const char *left, szt left_len, const char *right, szt right_len, char *out)
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
cat_str(const char *left, const char *right, char *out)
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

// calcs the string lengths also
inline void
cat_str(const char *left, const char *right, char *out, szt *len_out)
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
    *len_out = left_len + right_len;
    *out     = '\0';
}

// #ifdef TOM_WIN32
#if 0
// Convert a wide Unicode string to an UTF8 string
inline szt
get_wstr_sz(const wchar *wstr)
{
    auto len  = get_wstr_len(wstr);
    auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (s32)len, NULL, 0, NULL, NULL);
    return size;
}

inline szt
get_wstr_sz(const char *str)
{
    auto len  = get_str_len(str);
    auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (s32)len, NULL, 0);
    return size;
}

// FIXME: idk if this is working
inline void
wstr_to_str(const wchar *wstr, char *buf)
{
    auto len  = get_wstr_len(wstr);
    auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (s32)len, buf, 0, NULL, NULL);
}

inline void
str_to_wstr(const char *str, wchar *buf)
{
    auto len  = get_str_len(str);
    auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (s32)len, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, (s32)len, buf, (s32)size);
}
#else
inline szt
get_wstr_sz(const wchar *wstr)
{
    // TODO: stub
    return -1;
}

inline szt
get_wstr_sz(const char *str)
{
    // TODO: stub
    return -1;
}

inline void
wstr_to_str(const wchar *wstr, char *buf)
{
    // TODO: stub
}

inline void
str_to_wstr(const char *str, wchar *buf)
{
    // TODO: stub
}
#endif

}  // namespace tom
#endif  //  TOMATO_UTILS_HPP_
