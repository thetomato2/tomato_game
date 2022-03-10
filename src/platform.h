#ifndef TOMATO_PLATFORM_H_
#define TOMATO_PLATFORM_H_
/*
** Anything that needs to used in a platform layer gets put here
*/
#include <tchar.h>

#ifdef __cplusplus
    #include <cassert>
    #include <cmath>
    #include <cstdint>
    #include <cstdio>
    #include <cwchar>

    // NOTE: for grep purposes
    // TODO: improve this?
    #define scast(t, v) static_cast<t>(v)
    #define rcast(t, v) reinterpret_cast<t>(v)
    #define ccast(t, v) const_cast<t>(v)

extern "C"
{
#else
    #include <stdio.h>
    #include <assert.h>
    #include "math.h"
    #include "stdint.h"
    #include "wchar.h"

    #define scast(t, v) ((t)(v))
    #define rcast(t, v) ((t)(v))
    #define ccast(t, v) ((t)(v))
#endif

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

    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef float f32;
    typedef double f64;

    typedef size_t szt;
    typedef size_t mem_ind;
    typedef u8 byt;

#ifdef _WIN32
    typedef wchar_t wchar;
    typedef unsigned long ul;
    typedef unsigned long long ull;
#endif

    typedef int32_t b32;

#ifdef Cplusplus
    static_assert(sizeof(s8) == 1, "s8 isn't 1 byte!");
    static_assert(sizeof(s16) == 2, "s16 isn't 2 byte!s");
    static_assert(sizeof(s32) == 4, "s32 isn't 4 byte!s");
    static_assert(sizeof(s64) == 8, "s64 isn't 8 byte!s");
    static_assert(sizeof(u8) == 1, "u8 isn't 1 byte!");
    static_assert(sizeof(u16) == 2, "u16 isn't 2 byte!s");
    static_assert(sizeof(u32) == 4, "u32 isn't 4 byte!s");
    static_assert(sizeof(u64) == 8, "u64 isn't 8 byte!s");
    static_assert(sizeof(f32) == 4, "f32 isn't 4 byte!s");
    static_assert(sizeof(f64) == 8, "f64 isn't 8 byte!s");
    static_assert(sizeof(b32) == 4, "b32 isn't 4 byte!s");
#endif

#define U8_MIN 0u
#define U8_MAX 0xffu
#define S8_MIN (-0x7f - 1)
#define S8_MAX 0x7f

#define U16_MIN 0u
#define U16_MAX 0xffffu
#define S16_MIN (-0x7fff - 1)
#define S16_MAX 0x7fff

#define U32_MIN 0u
#define U32_MAX 0xffffffffu
#define S32_MIN (-0x7fffffff - 1)
#define S32_MAX 0x7fffffff

#define U64_MIN 0ull
#define U64_MAX 0xffffffffffffffffull
#define S64_MIN (-0x7fffffffffffffffll - 1)
#define S64_MAX 0x7fffffffffffffffll

#define KILOBYTES(val) ((val)*1024)
#define MEGABYTES(val) (KILOBYTES(val) * 1024)
#define GIGABYTES(val) (MEGABYTES(val) * 1024)
#define TERABYTES(val) (GIGABYTES(val) * 1024)

#define ARRAY_COUNT(Array) (sizeof((Array)) / sizeof((Array)[0]))

// NOTE: this breaks tree-sitter >:|
#define internal      static
#define local_persist static
#define global_var    static

#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

#ifdef TOM_INTERNAL
    #if Msvc
        #define TOM_ASSERT(x)                                               \
            if (!(x)) {                                                     \
                printf("FAILED ASSERT -> %s at :%d\n", __FILE__, __LINE__); \
                __debugbreak();                                             \
            }                                                               \
            assert(x)
    #else
        #define TOM_ASSERT(x)                                               \
            if (!(x)) {                                                     \
                printf("FAILED ASSERT -> %s at :%d\n", __FILE__, __LINE__); \
            }                                                               \
            assert(x)
    #endif
#else
    #define TOM_ASSERT(x)
#endif

#define INVALID_CODEPATH TOM_ASSERT(!"Invalid code path!")

#define REPLAY_BUFFERS 0

    // TODO: implement this
    typedef struct ThreadContext
    {
        s32 placeHolder;
    } ThreadContext;

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

    typedef struct debug_ReadFileResult
    {
        u32 contentSz;
        void *contents;
    } debug_ReadFileResult;

    #define debug_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread, void *memory)
    typedef debug_PLATFORM_FREE_FILE_MEMORY(debug_platformFreeFileMemory);

    #define debug_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_ReadFileResult name(ThreadContext *thread, const char *fileName)
    typedef debug_PLATFORM_READ_ENTIRE_FILE(debug_platformReadEntireFile);

    #define debug_PLATFORM_WRITE_ENTIRE_FILE(name) \
        b32 name(ThreadContext *thread, const char *fileName, u64 memorySize, void *memory)
    typedef debug_PLATFORM_WRITE_ENTIRE_FILE(debug_platformWriteEntireFile);
#endif

    typedef struct GameOffscreenBuffer
    {
        void *memory;
        s32 width;
        s32 height;
        s32 pitch;
        s32 bytesPerPixel;
    } GameOffscreenBuffer;

    typedef struct GameSoundOutputBuffer
    {
        s32 samplesPerSecond;
        s32 sampleCount;
        s16 *samples;
        s32 toneHertz;
    } GameSoundOutputBuffer;

    typedef struct GameButtonState
    {
        s32 halfTransitionCount;
        b32 endedDown;
    } GameButtonState;

    typedef struct GameControllerInput
    {
        bool isConnected;
        bool isAnalog;

        f32 minX;
        f32 minY;

        f32 maxX;
        f32 maxY;

        f32 startLeftStickX;
        f32 startLeftStickY;

        f32 startRightStickY;
        f32 startRightStickX;

        f32 endLeftStickX;
        f32 endLeftStickY;

        f32 endRightStickX;
        f32 endRightStickY;

        union
        {
            GameButtonState buttons[12];
            struct
            {
                GameButtonState dpadUp;
                GameButtonState dpadRight;
                GameButtonState dpadDown;
                GameButtonState dpadLeft;
                GameButtonState buttonA;
                GameButtonState buttonB;
                GameButtonState buttonX;
                GameButtonState buttonY;
                GameButtonState buttonRb;
                GameButtonState buttonLb;
                GameButtonState buttonBack;
                GameButtonState buttonStart;
            };
        };
    } GameControllerInput;

    typedef struct GameKeyboardInput
    {
        union
        {
            GameButtonState keys[14];
            struct
            {
                GameButtonState enter;
                GameButtonState w;
                GameButtonState s;
                GameButtonState a;
                GameButtonState d;
                GameButtonState space;
                GameButtonState leftShift;
                GameButtonState p;
                GameButtonState t;
                GameButtonState d1;
                GameButtonState d2;
                GameButtonState d3;
                GameButtonState d4;
                GameButtonState d5;
            };
        };
    } GameKeyboardInput;

#define INPUT_CNT     5
#define MOUSE_BUT_CNT 3
    typedef struct GameInput
    {
        f32 deltaTime;
        GameButtonState mouseButtons[MOUSE_BUT_CNT];
        s32 mouseX, mouseY, mouseZ;
        GameKeyboardInput keyboard;
        GameControllerInput controllers[INPUT_CNT - 1];
    } GameInput;

    typedef struct GameMemory
    {
        bool isInitialized;
        u64 permanentStorageSize;
        void *permanentStorage;  //! required to be cleared to 0!

        u64 transientStorageSize;
        void *transientStorage;  //! required to be cleared to 0!

#ifdef TOMInternal
        debug_platformFreeFileMemory *platformFreeFileMemory;
        debug_platformReadEntireFile *platfromReadEntireFile;
        debug_platformWriteEntireFile *platformWriteEntireFile;
#endif
    } GameMemory;

#define GAME_UPDATE_AND_RENDER(name)                                       \
    void name(ThreadContext *thread, GameMemory &memory, GameInput &input, \
              GameOffscreenBuffer &videoBuffer, GameSoundOutputBuffer &soundBuffer)
    typedef GAME_UPDATE_AND_RENDER(stub_gameUpdateAndRender);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(ThreadContext *thread, GameMemory &memory, GameSoundOutputBuffer &soundBuffer)
    typedef GAME_GET_SOUND_SAMPLES(stub_gameGetSoundSamples);

    inline u32
    safeTruncate_u64_u32(u64 value)
    {
        // TODO: defines for max values
        TOM_ASSERT(value <= 0xFFFFFFFF);
        u32 result = (u32)value;
        return result;
    }

#ifdef __cplusplus
}
#endif

#endif  // TOMATOPlatformH
