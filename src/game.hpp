#pragma once
#include "framework.hpp"
#include "tile.hpp"

#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

#define REPLAY_BUFFERS 0

namespace tomato
{

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

// TODO: implement this
struct thread_context
{
    i32 place_holder;
};

struct debug_read_file_result
{
    u32 content_size;
    void *contents;
};

    //! all these C shenanigans...
    // TODO:  C++-ify this
    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *thread_, void *memory_)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_read_file_result name(thread_context *thread_, const char *file_name_)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)                                     \
        bool32 name(thread_context *thread_, const char *file_name_, u64 memory_size_, \
                    void *memory_)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

struct game_offscreen_buffer
{
    void *memory;
    i32 width;
    i32 height;
    i32 pitch;
    i32 bytes_per_pixel;
};

struct game_sound_output_buffer
{
    i32 samples_per_second;
    i32 sample_count;
    i16 *samples;
    i32 tone_hertz;
};

struct game_button_state
{
    i32 half_transition_count;
    bool ended_down;
};

struct game_controller_input
{
    bool is_connected;
    bool is_analog;

    f32 start_left_stick_x;
    f32 start_left_stick_y;
    f32 start_right_stick_x;
    f32 start_right_stick_y;

    f32 min_x;
    f32 min_y;

    f32 max_x;
    f32 max_y;

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
};

struct game_keyboard
{
    union
    {
        game_button_state keys[11];
        struct
        {
            game_button_state w;
            game_button_state s;
            game_button_state a;
            game_button_state d;
            game_button_state space;
            game_button_state left_shift;
            game_button_state p;
            game_button_state d1;
            game_button_state d2;
            game_button_state d3;
            game_button_state d4;
        };
    };
};

struct game_input
{
    f32 deltaTime;

    static constexpr szt mouse_button_count = 3;
    game_button_state mouse_buttons[3];
    i32 mouse_x, mouse_y, mouse_z;
    game_keyboard keyboard;
    game_controller_input controllers[4];
};

struct game_mem
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
};

struct color_u32
{
    color_u32() : argb(0xffffffff) {}
    color_u32(u32 color) : argb(color) {}

    union
    {
        u32 argb;
        struct
        {
            u8 a;
            u8 r;
            u8 g;
            u8 b;
        };
    };
};

struct vector2_s32
{
    vector2_s32() : x(0), y(0) {}
    vector2_s32(i32 x, i32 y)
    {
        this->x = x;
        this->y = y;
    }

    i32 x;
    i32 y;
};

struct vector2_f32
{
    vector2_f32() : x(0.f), y(0.f) {}
    vector2_f32(f32 x, f32 y)
    {
        this->x = x;
        this->y = y;
    }

    f32 x;
    f32 y;
};

struct col_debug
{
    vector2_f32 pos;
    bool is_valid;
};

#define GAME_UPDATE_AND_RENDER(name)                                          \
    void name(thread_context *thread_, game_mem &memory_, game_input &input_, \
              game_offscreen_buffer &video_buffer_, game_sound_output_buffer &sound_buffer_)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(thread_context *thread_, game_mem &memory_, game_sound_output_buffer &sound_buffer_)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

struct mem_arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

struct player
{
    static constexpr f32 s_height = .75f;
    static constexpr f32 s_width  = 0.75f * s_height;

    tile_map_pos pos;
    color_u32 color;
};

struct world
{
    tile_map *tile_map;
};

struct game_state
{
    mem_arena world_arena;
    world *world;
    player player;
};

void *
push_size(mem_arena *arena_, mem_ind size_);

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))

}  // namespace tomato
