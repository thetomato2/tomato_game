// ============================================================================================
// Very lazy C++ template way to handle strings
// TODO: float to string converstion -> https://github.com/ulfjack/ryu/tree/master/ryu
// not a trivial thing to implement, but my Handmade spirit wills me so
// ============================================================================================

namespace tom
{

fn char itoc(u64 d)
{
    Assert(d < 10);

    switch (d) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        default: break;
    }
    return '\0';
}

fn char itoc(i32 d)
{
    Assert(d < 10);

    switch (d) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        default: break;
    }
    return '\0';
}

fn char *itos(u64 n)
{
    szt len = 0;
    i32 x   = n;
    while (x > 0) {
        x /= 10;
        ++len;
    }

    char *result = (char *)plat_malloc(sizeof(char) * (len + 1));
    char *ptr    = result + len;
    *ptr--       = '\0';
    for (i32 i = 0; i < len; ++i) {
        *ptr-- = itoc(n % 10);
        n /= 10;
    }

    return result;
}

fn char *itos(i32 n)
{
    szt len = 0;
    i32 x   = n;
    while (x > 0) {
        x /= 10;
        ++len;
    }

    char *result = (char *)plat_malloc(sizeof(char) * (len + 1));
    char *ptr    = result + len;
    *ptr--       = '\0';
    for (i32 i = 0; i < len; ++i) {
        *ptr-- = itoc(n % 10);
        n /= 10;
    }

    return result;
}

// TODO: this is pretty barebones
fn i32 stoi(const char *str)
{
    i32 i = 0;
    while (*str >= '0' && *str <= '9') {
        i = i * 10 + (*str - '0');
        str++;
    }
    return i;
}

fn i32 stoi(const char c)
{
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;

        default: break;
    }

    return -1;
}

template<typename CharT>
fn void str_copy(const CharT *str, CharT *buf)
{
    for (; *str; ++str) *buf++ = *str;
    *buf = (CharT)'\0';
}

template<typename CharT>
fn bool str_equal(const CharT *a, const CharT *b)
{
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    if (*a != *b) return false;
    return true;
}

template<typename CharT>
fn CharT *rev_str(const CharT *str)
{
    szt len       = str_len(str);
    CharT *result = (CharT *)plat_malloc(sizeof(CharT) * (len + 1));
    for (szt i = 0; i < len; ++i) {
        result[i] = *(str + (len - i - 1));
    }
    result[len] = '\0';

    return result;
}

template<typename CharT>
fn szt str_len(const CharT *str)
{
    const CharT *s;
    for (s = str; *s; ++s)
        ;
    szt res = s - str;

    return res;
}

template<typename... Args>
fn szt str_len(Args... args)
{
    szt result = 0;
    for (auto s : { args... }) {
        result += str_len(s);
    }

    return result;
}

template<typename CharT, typename... Args>
fn CharT *str_cat(const CharT *first, Args... args)
{
    szt buf_len = str_len(first) + str_len(std::forward<Args>(args)...);
    // CharT* result  = (CharT*)plat_malloc(sizeof(CharT) * (buf_len + 1));
    CharT *result  = plat_malloc<CharT>(buf_len + 1);
    CharT *buf_ptr = result;
    while (*first) {
        *buf_ptr++ = *first++;
    }

    for (auto str : { args... }) {
        while (*str) {
            *buf_ptr++ = *str++;
        }
    }

    *buf_ptr = (CharT)'\0';

    return result;
}

template<typename CharT>
fn CharT *str_cat(const CharT *str)
{
    CharT *result  = (CharT *)plat_malloc(sizeof(CharT) * (str_len(str) + 1));
    CharT *buf_ptr = result;
    while (*str) {
        *buf_ptr++ = *str++;
    }
    *buf_ptr = (CharT)'\0';

    return result;
}

template<typename... Args>
fn char *str_fmt(const char *format, Args... args)
{
    szt buf_sz   = snprintf(nullptr, 0, format, args...) + 1;
    char *result = plat_malloc<char>(buf_sz);
    snprintf(result, buf_sz, format, args...);

    return result;
}

fn char *convert_wstring_to_string(const wchar *wstr)
{
    i32 cnt     = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    auto result = plat_malloc<char>(cnt);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, result, cnt, NULL, NULL);

    return result;
}

fn wchar *convert_string_to_wstring(const char *str)
{
    i32 cnt     = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    auto result = plat_malloc<wchar>(cnt);
    MultiByteToWideChar(CP_ACP, 0, str, -1, result, cnt);

    return result;
}

fn char *convert_wstring_to_string_utf8(const wchar *wstr)
{
    i32 cnt     = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    auto result = plat_malloc<char>(cnt);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result, cnt, NULL, NULL);

    return result;
}

fn wchar *convert_string_to_wstring_utf8(const char *str)
{
    i32 cnt     = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    auto result = plat_malloc<wchar>(cnt);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, result, cnt);

    return result;
}

}  // namespace tom