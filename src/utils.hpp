#ifndef TOMATO_UTILS_HPP_
#define TOMATO_UTILS_HPP_

#include "Platform.h"
namespace tom
{

inline szt
getWstrLen(const wchar *wstr)
{
    szt len;
    while (*wstr++) {
        ++len;
    }
    return len;
}

inline szt
getStrLen(const char *str)
{
    szt len {};
    while (*str++) {
        ++len;
    }
    return len;
}

// Takes strings length inputs
inline void
catStr(const char *left, szt leftLen, const char *right, szt rightLen, char *out)
{
    for (szt i {}; i < leftLen; ++i) {
        *out++ = *left++;
    }
    for (szt i {}; i < rightLen; ++i) {
        *out++ = *right++;
    }
    *out = '\0';
}

// calcs the string lengths also
inline void
catStr(const char *left, const char *right, char *out)
{
    auto leftLen  = getStrLen(left);
    auto rightLen = getStrLen(right);

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

// calcs the string lengths also
inline void
catStr(const char *left, const char *right, char *out, szt *lenOut)
{
    auto leftLen  = getStrLen(left);
    auto rightLen = getStrLen(right);

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
    *lenOut = leftLen + rightLen;
    *out    = '\0';
}

// #ifdef TOM_WIN32
#if 0
// Convert a wide Unicode string to an UTF8 string
inline szt
getWstrSz(const wchar *wstr)
{
    auto len  = get_wstr_len(wstr);
    auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (s32)len, NULL, 0, NULL, NULL);
    return size;
}

inline szt
getWstrSz(const char *str)
{
    auto len  = getStrLen(str);
    auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (s32)len, NULL, 0);
    return size;
}

// FIXME: idk if this is working
inline void
wstrToStr(const wchar *wstr, char *buf)
{
    auto len  = get_wstr_len(wstr);
    auto size = (szt)WideCharToMultiByte(CP_UTF8, 0, wstr, (s32)len, buf, 0, NULL, NULL);
}

inline void
strToWstr(const char *str, wchar *buf)
{
    auto len  = getStrLen(str);
    auto size = (szt)MultiByteToWideChar(CP_UTF8, 0, str, (s32)len, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, (s32)len, buf, (s32)size);
}
#else
inline szt
getWstrSz(const wchar *wstr)
{
    // TODO: stub
    return -1;
}

inline szt
getWstrSz(const char *str)
{
    // TODO: stub
    return -1;
}

// FIXME: idk if this is working
inline void
wstrToStr(const wchar *wstr, char *buf)
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
