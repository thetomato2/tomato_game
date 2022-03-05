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

#define KILOBYTES(val) ((val)*1024)
#define MEGABYTES(val) (KILOBYTES(val) * 1024)
#define GIGABYTES(val) (MEGABYTES(val) * 1024)
#define TERABYTES(val) (GIGABYTES(val) * 1024)

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

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
    #define TomAssert(x)                                                \
        if (!(x)) {                                                     \
            printf("FAILED ASSERT -> %s at :%d\n", __FILE__, __LINE__); \
            __debugbreak();                                             \
        }                                                               \
        assert(x)
#else
    #define TomAssert(x)
#endif

#define INVALID_CODE_PATH TomAssert(!"Invalid code path!")

#define REPLAY_BUFFERS 0

    // TODO: implement this
    typedef struct _thread_context
    {
        s32 place_holder;
    } Thread_Context;

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

    typedef struct debug_read_file_result
    {
        u32 content_size;
        void *contents;
    } Debug_Read_File_Result;

    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(Thread_Context *thread, void *memory)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_read_file_result name(Thread_Context *thread, const char *file_name)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
        b32 name(Thread_Context *thread, const char *file_name, u64 memory_size, void *memory)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

    typedef struct _game_offscreen_buffer

    {
        void *memory;
        s32 width;
        s32 height;
        s32 pitch;
        s32 bytes_per_pixel;
    } Game_Offscreen_Buffer;

    typedef struct _game_sound_output_buffer
    {
        s32 samples_per_second;
        s32 sample_count;
        s16 *samples;
        s32 tone_hertz;
    } Game_Sound_Output_Buffer;

    typedef struct _game_button_state
    {
        s32 half_transition_count;
        b32 ended_down;
    } Game_Button_State;

    typedef struct _game_controller_input
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
            Game_Button_State buttons[12];
            struct
            {
                Game_Button_State dpad_up;
                Game_Button_State dpad_right;
                Game_Button_State dpad_down;
                Game_Button_State dpad_left;
                Game_Button_State button_A;
                Game_Button_State button_B;
                Game_Button_State button_X;
                Game_Button_State button_Y;
                Game_Button_State button_RB;
                Game_Button_State button_LB;
                Game_Button_State button_back;
                Game_Button_State button_start;
            };
        };
    } Game_Controller_Input;

    typedef struct _game_keyboard_input
    {
        union
        {
            Game_Button_State keys[14];
            struct
            {
                Game_Button_State enter;
                Game_Button_State w;
                Game_Button_State s;
                Game_Button_State a;
                Game_Button_State d;
                Game_Button_State space;
                Game_Button_State left_shift;
                Game_Button_State p;
                Game_Button_State t;
                Game_Button_State d1;
                Game_Button_State d2;
                Game_Button_State d3;
                Game_Button_State d4;
                Game_Button_State d5;
            };
        };
    } Game_Keyboard_Input;

    typedef struct _game_input
    {
        static const szt s_input_cnt        = 5;
        static const szt s_mouse_button_cnt = 3;

        f32 delta_time;

        Game_Button_State mouse_buttons[3];
        s32 mouse_x, mouse_y, mouse_z;
        Game_Keyboard_Input keyboard;
        Game_Controller_Input controllers[4];
    } Game_Input;

    typedef struct _game_memory
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
    } Game_Memory;

#define GAME_UPDATE_AND_RENDER(name)                                          \
    void name(Thread_Context *thread, Game_Memory &memory, Game_Input &input, \
              Game_Offscreen_Buffer &video_buffer, Game_Sound_Output_Buffer &sound_buffer)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(Thread_Context *thread, Game_Memory &memory, Game_Sound_Output_Buffer &sound_buffer)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

    inline u32
    safe_truncate_u32_to_u64(u64 value)
    {
        // TODO: defines for max values
        TomAssert(value <= 0xFFFFFFFF);
        u32 result = (u32)value;
        return result;
    }

#ifdef __cplusplus
}
#endif

#endif  // TOMATO_PLATFORM_H_
