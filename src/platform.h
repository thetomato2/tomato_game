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
#include <io.h>
#include <fcntl.h>

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

#ifdef __cplusplus
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

#define BIT(x) (1 << x)

#define KILOBYTES(val) ((val)*1024)
#define MEGABYTES(val) (KILOBYTES(val) * 1024)
#define GIGABYTES(val) (MEGABYTES(val) * 1024)
#define TERABYTES(val) (GIGABYTES(val) * 1024)

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))

// NOTE: this breaks tree-sitter >:|
#define internal      static
#define local_persist static
#define global_var    static

// NOTE: only win32 is supported currently
#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif
#ifdef TOM_INTERNAL
    #define TOM_ASSERT(x)                                               \
        if (!(x)) {                                                     \
            printf("FAILED ASSERT -> %s at :%d\n", __FILE__, __LINE__); \
            __debugbreak();                                             \
        }                                                               \
        assert(x)

    #define TOM_ASSERT_MSG(x, msg)                                                \
        if (!(x)) {                                                               \
            printf("FAILED ASSERT -> %s at :%d - %s\n", __FILE__, __LINE__, msg); \
            __debugbreak();                                                       \
        }                                                                         \
        assert(x)
#else
    #define TOM_ASSERT(x)
    #define TOM_ASSERT_MSG(x, msg)
#endif

#define INVALID_CODE_PATH TOM_ASSERT(!"Invalid code path!")

#define REPLAY_BUFFERS 0

    // TODO: implement this
    typedef struct thread_context
    {
        s32 place_holder;
    } thread_context;

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

    typedef struct debug_read_file_result
    {
        u32 content_size;
        void *contents;
    } debug_read_file_result;

    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *memory)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_read_file_result name(thread_context *thread, const char *file_name)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
        b32 name(thread_context *thread, const char *file_name, u64 memory_size, void *memory)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

    typedef struct game_offscreen_buffer

    {
        void *memory;
        s32 width;
        s32 height;
        s32 pitch;
        s32 bytes_per_pixel;
    } game_offscreen_buffer;

    typedef struct game_sound_output_buffer
    {
        s32 samples_per_second;
        s32 sample_count;
        s16 *samples;
        s32 tone_hertz;
    } game_sound_output_buffer;

    typedef struct game_button_state
    {
        s32 half_transition_count;
        b32 ended_down;
    } game_button_state;

    typedef struct game_controller_input
    {
        bool is_connected;
        bool is_analog;

        f32 min_x;
        f32 min_y;

        f32 max_x;
        f32 max_y;

        f32 start_left_stick_x;
        f32 start_left_stick_y;

        f32 start_right_stick_y;
        f32 start_right_stick_x;

        f32 end_left_stick_x;
        f32 end_left_stick_y;

        f32 end_right_stick_x;
        f32 end_right_stick_y;

        union
        {
            game_button_state buttons[12];
            struct
            {
                game_button_state dpad_up;
                game_button_state dpad_right;
                game_button_state dpad_down;
                game_button_state dpad_left;
                game_button_state button_a;
                game_button_state button_b;
                game_button_state button_x;
                game_button_state button_y;
                game_button_state button_rb;
                game_button_state button_lb;
                game_button_state button_back;
                game_button_state button_start;
            };
        };
    } game_controller_input;

    typedef struct game_keyboard_input
    {
        union
        {
            game_button_state keys[14];
            struct
            {
                game_button_state enter;
                game_button_state w;
                game_button_state s;
                game_button_state a;
                game_button_state d;
                game_button_state space;
                game_button_state left_shift;
                game_button_state p;
                game_button_state t;
                game_button_state d1;
                game_button_state d2;
                game_button_state d3;
                game_button_state d4;
                game_button_state d5;
            };
        };
    } game_keyboard_input;

#define INPUT_CNT        5
#define MOUSE_BUTTON_CNT 3

    typedef struct game_input
    {
        f32 delta_time;
        game_button_state mouse_buttons[MOUSE_BUTTON_CNT];
        s32 mouse_x, mouse_y, mouse_z;
        game_keyboard_input keyboard;
        game_controller_input controllers[INPUT_CNT - 1];
    } game_input;

    typedef struct game_memory
    {
        bool is_initialized;
        u64 permanent_storage_size;
        void *permanent_storage;  // NOTE: required to be cleared to 0!

        u64 transient_storage_size;
        void *transient_storage;  // NOTE: required to be cleared to 0!

#ifdef TOM_INTERNAL
        debug_platform_free_file_memory *platform_free_file_memory;
        debug_platform_read_entire_file *platfrom_read_entire_file;
        debug_platform_write_entire_file *platform_write_entire_file;
#endif
    } game_memory;

#define GAME_UPDATE_AND_RENDER(name)                                          \
    void name(thread_context *thread, game_memory &memory, game_input &input, \
              game_offscreen_buffer &video_buffer, game_sound_output_buffer &sound_buffer)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(thread_context *thread, game_memory &memory, game_sound_output_buffer &sound_buffer)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

    inline u32
    safe_truncate_u32_to_u64(u64 value)
    {
        // TODO: defines for max values
        TOM_ASSERT(value <= 0xFFFFFFFF);
        u32 result = (u32)value;
        return result;
    }

#ifdef __cplusplus
}
#endif

#endif  // TOMATO_PLATFORM_H_
