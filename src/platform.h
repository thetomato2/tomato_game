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
extern "C"
{
#else
    #include <stdio.h>
    #include <assert.h>
    #include "math.h"
    #include "stdint.h"
    #include "wchar.h"
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

#define internal        static
#define local_persist   static
#define global_variable static

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

#define KILOBYTES(val) ((val)*1024)
#define MEGABYTES(val) (KILOBYTES(val) * 1024)
#define GIGABYTES(val) (MEGABYTES(val) * 1024)
#define TERABYTES(val) (GIGABYTES(val) * 1024)

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif
#ifdef TOM_INTERNAL
    #define TOM_ASSERT(x)                                                   \
        {                                                                   \
            if (!(x)) {                                                     \
                printf("FAILED ASSERT -> %s at :%d\n", __FILE__, __LINE__); \
                __debugbreak();                                             \
            }                                                               \
            assert(x);                                                      \
        }
#else
    #define TOM_ASSERT(x)
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
        static const szt s_button_cnt = 12;

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
            game_button_state buttons[s_button_cnt];
            struct
            {
                game_button_state dpad_up;
                game_button_state dpad_right;
                game_button_state dpad_down;
                game_button_state dpad_left;
                game_button_state button_A;
                game_button_state button_B;
                game_button_state button_X;
                game_button_state button_Y;
                game_button_state button_RB;
                game_button_state button_LB;
                game_button_state button_back;
                game_button_state button_start;
            };
        };
    } game_controller_input;

    typedef struct game_keyboard_input
    {
        static constexpr szt s_key_cnt { 14 };

        union
        {
            game_button_state keys[s_key_cnt];
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

    typedef struct game_input
    {
        static const szt s_input_cnt        = 5;
        static const szt s_mouse_button_cnt = 3;

        f32 delta_time;

        game_button_state mouse_buttons[3];
        s32 mouse_x, mouse_y, mouse_z;
        game_keyboard_input keyboard;
        game_controller_input controllers[4];
    } game_input;

    typedef struct game_memory
    {
        bool is_initialized;
        u64 permanent_storage_size;
        void *permanent_storage;  //! required to be cleared to 0!

        u64 transient_storage_size;
        void *transient_storage;  //! required to be cleared to 0!

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
