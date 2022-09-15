#ifndef TOM_CORE_HH
#define TOM_CORE_HH

TOM_CORE_HH
// #ifdef _MSVC
//     #define MSVC 1
// #endif

#define MSVC    1
#define USE_DS5 1

// #ifdef _LLVM
//     #define LLVM 1
// #endif

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
typedef HRESULT(WINAPI *LPDXGIGETDEBUGINTERFACE)(REFIID, void **);
#endif

#include <xinput.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "../extern/stb/stb_image.h"
#include "../extern/stb/stb_image_write.h"
#include "../extern/stb/stb_truetype.h"

#if MSVC
    #include <intrin.h>
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

#define SET_CONSOLE_COLOR(x)                               \
    {                                                      \
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
        SetConsoleTextAttribute(hConsole, x);              \
    }

#define PRINT_RED(str)         \
    SET_CONSOLE_COLOR(FG_RED); \
    printf(str);               \
    SET_CONSOLE_COLOR(FG_WHITE);

#define PRINT_GREEN(str)         \
    SET_CONSOLE_COLOR(FG_GREEN); \
    printf(str);                 \
    SET_CONSOLE_COLOR(FG_WHITE);

#define PRINT_BLUE(str)         \
    SET_CONSOLE_COLOR(FG_BLUE); \
    printf(str);                \
    SET_CONSOLE_COLOR(FG_WHITE);

#define PRINT_YELLOW(str)         \
    SET_CONSOLE_COLOR(FG_YELLOW); \
    printf(str);                  \
    SET_CONSOLE_COLOR(FG_WHITE);

#define PRINT_INFO(str)    \
    PRINT_GREEN("INFO: "); \
    printf("%s\n", str)

#define PRINT_MSG(str)       \
    PRINT_BLUE("MESSAGE: "); \
    printf("%s\n", str);

#define PRINT_WARN(str)        \
    PRINT_YELLOW("WARNING: "); \
    printf("%s\n", str)

#define PRINT_CORRUPTION(str)  \
    PRINT_RED("CORRUPTION: "); \
    printf("%s\n", str);

#define PRINT_ERR(str)    \
    PRINT_RED("ERROR: "); \
    printf("%s\n", str)

#ifdef TOM_INTERNAL
    #define TOM_ASSERT(x)                                                               \
        do {                                                                            \
            if (!(x)) {                                                                 \
                PRINT_RED("FAILED ASSERT:") printf(" %s at :%d\n", __FILE__, __LINE__); \
                __debugbreak();                                                         \
            }                                                                           \
            assert(x);                                                                  \
        } while (0)
    #define DebugBreak(x)   \
        if (x) {            \
            __debugbreak(); \
        }
    #define InternalOnlyExecute(args) args
#else
    #define TOM_ASSERT(x)
    #define DebugBreak(x)
    #define InternalOnlyExecute(args)
#endif

#define INVALID_CODE_PATH TOM_ASSERT(!"Invalid code path!")
#define INVALID_DEFAULT_CASE \
    default: {             \
        INVALID_CODE_PATH; \
    } break
#define NOT_IMPLEMENTED TOM_ASSERT(!"Not implemented!")

#define CTTOM_ASSERT(Expr) static_assert(Expr, "TOM_ASSERTion failed: " #Expr)

#ifdef __cplusplus
extern "C++"
{
    template<typename _CountofType, size_t _SizeOfArray>
    char (*__countof_helper(_UNALIGNED _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];

    #define ARR_CNT(_Array) (sizeof(*__countof_helper(_Array)) + 0)
}
#else
    #define ARR_CNT(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#define OFFSET(type, member) (umm)&(((type *)0)->member

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

inline void set_flags(i32 &flags, i32 flag)
{
    flags |= flag;
}

inline void clear_flags(i32 &flags, i32 flag)
{
    flags &= ~flag;
}

inline u32 safe_truncate_u32_to_u64(u64 value)
{
    u32 result = (u32)value;
    return result;
}

#include "tom_globals.hh"

}  // namespace tom

#endif
