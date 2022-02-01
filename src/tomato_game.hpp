#include "tomato_platform.h"
#include "tomato_intrinsic.hpp"
#include "tomato_common.hpp"

#include "tomato_math.hpp"
#include "tomato_utils.hpp"
#include "tomato_tile.cpp"

struct Player_Actions
{
    bool start;

    v2 dir;

    bool sprint;
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
    const char *name;
    u32 width;
    u32 height;
    u32 size;
    u32 *pixel_ptr;
};

enum Dir : s32
{
    down = 0,
    right,
    up,
    left

};

enum class Entity_Residence
{
    non_existent,
    dormant,
    low,
    high
};

enum class Entity_Type
{
    null,
    none,
    player,
    wall,
    stairs
};

struct High_Entity
{
    b32 exists;

    // NOTE: relative to camera
    v2 pos, vel;
    u32 abs_tile_z;
    u32 direction;
    f32 stair_cd;

    u32 low_i;
};

struct Low_Entity
{
    Tile_Map_Pos pos;
    f32 width, height;
    Color_u32 color;
    ARGB_Img *sprites;

    b32 collides;
    b32 barrier;
    Entity_Type type;

    u32 high_i;
};

struct Entity
{
    u32 low_i;
    Low_Entity *low;
    High_Entity *high;
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
    static constexpr u32 s_max_low_cnt { 4096 };
    static constexpr u32 s_max_high_cnt { 256 };
    static constexpr u32 s_num_screens { 10 };
    static constexpr u32 s_num_tiles_per_screen_x { 20 };
    static constexpr u32 s_num_tiles_per_screen_y { 11 };

    Mem_Arena world_arena;
    World *world;

    u32 entity_camera_follow_ind;
    Camera camera;

    Bitmap bitmap;

    u32 player_controller_ind[Game_Input::s_input_cnt];

    u32 player_cnt;

    u32 low_cnt;
    Low_Entity low_entities[s_max_low_cnt];
    u32 high_cnt;
    High_Entity high_entities[s_max_high_cnt];

    ARGB_Img bg_img;
    ARGB_Img seaside_cliff;
    ARGB_Img crosshair_img;
    ARGB_Img red_square_img;
    ARGB_Img green_square_img;
    ARGB_Img blue_square_img;
    ARGB_Img player_sprites[4];
    ARGB_Img tree_sprite;
    ARGB_Img stair_sprite;

    Tile_Map_Pos test_pos;
};

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
