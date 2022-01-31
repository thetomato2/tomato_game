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

#define INVALID_CODE_PATH assert(!"Invalid code path!")

#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

#define REPLAY_BUFFERS 1

    // TODO: implement this
    typedef struct Thread_Context
    {
        s32 place_holder;
    } Thread_Context;

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

    typedef struct debug_Read_File_Result
    {
        u32 content_size;
        void *contents;
    } debug_Read_File_Result;

    //! all these C shenanigans...
    // TODO:  C++-ify this
    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(Thread_Context *thread_, void *memory_)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_Read_File_Result name(Thread_Context *thread_, const char *file_name_)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
        b32 name(Thread_Context *thread_, const char *file_name_, u64 memory_size_, void *memory_)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

    typedef struct Game_Offscreen_Buffer

    {
        void *memory;
        s32 width;
        s32 height;
        s32 pitch;
        s32 bytes_per_pixel;
    } Game_Offscreen_Buffer;

    typedef struct Game_Sound_Output_Buffer
    {
        s32 samples_per_second;
        s32 sample_count;
        s16 *samples;
        s32 tone_hertz;
    } Game_Sound_Output_Buffer;

    typedef struct Game_Button_State
    {
        s32 half_transition_count;
        b32 ended_down;
    } Game_Button_State;

    typedef struct Game_Controller_Input
    {
        static constexpr szt s_button_cnt { 12 };

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
            Game_Button_State buttons[s_button_cnt];
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

    typedef struct Game_Keyboard_Input
    {
        static constexpr szt s_key_cnt { 14 };

        union
        {
            Game_Button_State keys[s_key_cnt];
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

    typedef struct Game_Input
    {
        static constexpr szt s_input_cnt { 5 };
        static constexpr szt s_mouse_button_cnt { 3 };

        f32 delta_time;

        Game_Button_State mouse_buttons[3];
        s32 mouse_x, mouse_y, mouse_z;
        Game_Keyboard_Input keyboard;
        Game_Controller_Input controllers[4];
    } Game_Input;

    typedef struct Game_Mem
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
    } Game_Mem;

#define GAME_UPDATE_AND_RENDER(name)                                          \
    void name(Thread_Context *thread_, Game_Mem &memory_, Game_Input &input_, \
              Game_Offscreen_Buffer &video_buffer_, Game_Sound_Output_Buffer &sound_buffer_)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(Thread_Context *thread_, Game_Mem &memory_, Game_Sound_Output_Buffer &sound_buffer_)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

    inline u32
    safe_truncate_u32_to_u64(u64 val_)
    {
        // TODO: defines for max values
        assert(val_ <= 0xFFFFFFFF);
        u32 result = (u32)val_;
        return result;
    }

#ifdef __cplusplus
}
#endif

#endif  // TOMATO_PLATFORM_H_
