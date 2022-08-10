#include <cassert>
#include <cmath>

#include <cstdio>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <type_traits>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
// #define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbt.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <tchar.h>

#include <d3d11_1.h>
#include <d3dcompiler.h>

#if 1
    #include <dxgidebug.h>
typedef HRESULT(WINAPI* LPDXGIGETDEBUGINTERFACE)(REFIID, void**);
#endif

#include <xinput.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../extern/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../extern/stb/stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../extern/stb/stb_truetype.h"

#ifdef _MSVC
    #define MSVC 1
#endif

#ifdef _LLVM
    #define LLVM 1
#endif

#if MSVC
    /* #include <intrin.h> */
    #pragma intrinsic(_BitScanForward)
#endif

#include "tom_types.hh"

// this is a unity build so all functions are static by default to speed up linking time
#define fn static
#define extern_fn
#define internal static
#define global   static
#define local    static

#define Z_UP 1

#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

enum CONSOLE_FG_COLORS
{
    FG_BLACK        = 0,
    FG_BLUE         = 1,
    FG_GREEN        = 2,
    FG_CYAN         = 3,
    FG_RED          = 4,
    FG_MAGENTA      = 5,
    FG_BROWN        = 6,
    FG_LIGHTGRAY    = 7,
    FG_GRAY         = 8,
    FG_LIGHTBLUE    = 9,
    FG_LIGHTGREEN   = 10,
    FG_LIGHTCYAN    = 11,
    FG_LIGHTRED     = 12,
    FG_LIGHTMAGENTA = 13,
    FG_YELLOW       = 14,
    FG_WHITE        = 15
};

#define SetConsoleColor(x)                                 \
    {                                                      \
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
        SetConsoleTextAttribute(hConsole, x);              \
    }

#define PrintRed(str)        \
    SetConsoleColor(FG_RED); \
    printf(str);             \
    SetConsoleColor(FG_WHITE);

#define PrintGreen(str)        \
    SetConsoleColor(FG_GREEN); \
    printf(str);               \
    SetConsoleColor(FG_WHITE);

#define PrintBlue(str)        \
    SetConsoleColor(FG_BLUE); \
    printf(str);              \
    SetConsoleColor(FG_WHITE);

#define PrintYellow(str)        \
    SetConsoleColor(FG_YELLOW); \
    printf(str);                \
    SetConsoleColor(FG_WHITE);

#define PrintInfo(str)    \
    PrintGreen("INFO: "); \
    printf("%s\n", str)

#define PrintMessage(str)   \
    PrintBlue("MESSAGE: "); \
    printf("%s\n", str);

#define PrintWarning(str)     \
    PrintYellow("WARNING: "); \
    printf("%s\n", str)

#define PrintCorruption(str)  \
    PrintRed("CORRUPTION: "); \
    printf("%s\n", str);

#define PrintError(str)  \
    PrintRed("ERROR: "); \
    printf("%s\n", str)

#ifdef TOM_INTERNAL
    #define Assert(x)                                                                  \
        do {                                                                           \
            if (!(x)) {                                                                \
                PrintRed("FAILED ASSERT:") printf(" %s at :%d\n", __FILE__, __LINE__); \
                __debugbreak();                                                        \
            }                                                                          \
            assert(x);                                                                  \
        } while (0)
    #define DebugBreak(x)   \
        if (x) {            \
            __debugbreak(); \
        }
    #define InternalOnlyExecute(args) args
#else
    #define Assert(x)
    #define DebugBreak(x)
    #define InternalOnlyExecute(args)
#endif

#define InvalidCodePath Assert(!"Invalid code path!")
#define InvalidDefaultCase \
    default: {             \
        InvalidCodePath;   \
    } break
#define NotImplemented Assert(!"Not implemented!")

#define CTAssert(Expr) static_assert(Expr, "Assertion failed: " #Expr)

#ifdef __cplusplus
extern "C++"
{
    template<typename _CountofType, size_t _SizeOfArray>
    char (*__countof_helper(_UNALIGNED _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];

    #define CountOf(_Array) (sizeof(*__countof_helper(_Array)) + 0)
}
#else
    #define CountOf(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#define OffsetOf(type, member) (umm)&(((type *)0)->member

namespace tom
{
struct ThreadContext
{
    i32 place_holder;
};

// Generic flag stuff

inline bool is_flag_set(i32 flags, i32 flag)
{
    return flags & flag;
}

inline void set_flags(i32& flags, i32 flag)
{
    flags |= flag;
}

inline void clear_flags(i32& flags, i32 flag)
{
    flags &= ~flag;
}

inline u32 safe_truncate_u32_to_u64(u64 value)
{
    u32 result = (u32)value;
    return result;
}

}  // namespace tom
