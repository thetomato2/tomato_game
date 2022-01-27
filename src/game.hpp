#pragma once
#include "framework.hpp"
#include "tile.hpp"

#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

#define REPLAY_BUFFERS 1

namespace tom
{

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

// TODO: implement this
struct Thread_Context
{
    s32 place_holder;
};

struct debug_ReadFileResult
{
    u32 content_size;
    void *contents;
};

    //! all these C shenanigans...
    // TODO:  C++-ify this
    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(Thread_Context *thread_, void *memory_)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_ReadFileResult name(Thread_Context *thread_, const char *file_name_)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
        b32 name(Thread_Context *thread_, const char *file_name_, u64 memory_size_, void *memory_)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

struct Game_Offscreen_Buffer

{
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 bytes_per_pixel;
};

struct Game_Sound_Output_Buffer
{
    s32 samples_per_second;
    s32 sample_count;
    s16 *samples;
    s32 tone_hertz;
};

struct Game_Button_State
{
    s32 half_transition_count;
    b32 ended_down;
};

struct Game_Controller_Input
{
    static constexpr szt s_button_cnt { 12 };

    bool is_connected;
    bool is_analog;

    v2 min;
    v2 max;
    v2 start_left_stick;
    v2 start_right_stick;

    v2 end_left_stick;
    v2 end_right_stick;

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
};

struct Game_Keyboard_Input
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
};

struct Game_Input
{
    static constexpr szt s_input_cnt { 5 };
    static constexpr szt s_mouse_button_cnt { 3 };

    f32 delta_time;

    Game_Button_State mouse_buttons[3];
    s32 mouse_x, mouse_y, mouse_z;
    Game_Keyboard_Input keyboard;
    Game_Controller_Input controllers[4];
};

struct Player_Actions
{
    bool start;

    v2 dir;

    bool sprint;
};

struct Game_Mem
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

struct Color_u32
{
    union
    {
        u32 argb;
        struct
        {
            u8 b;
            u8 g;
            u8 r;
            u8 a;
        };
    };
};

#pragma pack(push, 1)
struct Bitmap_Header
{
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
};

struct ARGB_Header
{
    u32 width;
    u32 height;
    u32 size;
};
#pragma pack(pop)

struct Bitmap
{
    s32 width;
    s32 height;
    u32 *pixel_ptr;
};

struct ARGB_Img
{
    u32 width;
    u32 height;
    u32 size;
    u32 *pixel_ptr;
};

#define GAME_UPDATE_AND_RENDER(name)                                          \
    void name(Thread_Context *thread_, Game_Mem &memory_, Game_Input &input_, \
              Game_Offscreen_Buffer &video_buffer_, Game_Sound_Output_Buffer &sound_buffer_)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(Thread_Context *thread_, Game_Mem &memory_, Game_Sound_Output_Buffer &sound_buffer_)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

struct Mem_Arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

enum Dir : s32
{
    down = 0,
    right,
    up,
    left

};

struct Entity
{
    b32 exists;

    f32 height;
    f32 width;

    Tile_Map_Pos pos;
    Color_u32 color;

    u32 direction;
    ARGB_Img *sprites;

    f32 stair_cd;
    v2 vel;
};

struct World
{
    Tile_Map *tile_map;
};

struct Camera
{
    Tile_Map_Pos pos;
};

struct Game_State
{
    static constexpr szt s_max_entities { 256 };
    u32 static constexpr s_num_screens { 100 };
    u32 static constexpr s_num_tiles_per_screen_x { 20 };
    u32 static constexpr s_num_tiles_per_screen_y { 11 };

    Mem_Arena world_arena;
    World *world;

    szt entity_camera_follow_ind;
    Camera camera;

    Bitmap bitmap;

    szt player_controller_ind[Game_Input::s_input_cnt];
    Entity entities[s_max_entities];

    ARGB_Img bg_img;
    ARGB_Img seaside_cliff;

    ARGB_Img crosshair_img;

    ARGB_Img red_square_img;
    ARGB_Img green_square_img;
    ARGB_Img blue_square_img;

    ARGB_Img player_sprites[4];

    Tile_Map_Pos test_pos;
};

void *
push_size(Mem_Arena *arena_, mem_ind size_);

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))

inline bool
is_key_up(const Game_Button_State &key_)
{
    return key_.half_transition_count > 0 && key_.ended_down == 0;
}

inline bool
is_button_up(const Game_Button_State &button_)
{
    return is_key_up(button_);
}

}  // namespace tom
