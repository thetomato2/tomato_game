
#include "platform.h"
#include "intrinsic.hpp"
#include "common.hpp"

#include "math.hpp"
#include "utils.hpp"
#include "world.cpp"

namespace tom
{

struct Entity_Actions
{
    bool start;
    bool jump;

    v2 dir;

    bool sprint;
};

struct Color
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

struct ARGB_header
{
    u32 width;
    u32 height;
    u32 size;
};
#pragma pack(pop)

struct Bitmap_Img
{
    s32 width;
    s32 height;
    u32 *pixel_ptr;
};

struct ARGB_img
{
    const char *name;
    u32 width;
    u32 height;
    u32 size;
    u32 *pixel_ptr;
};

enum Entity_Direction : s32
{
    down = 0,
    right,
    up,
    left

};

enum class Entity_Type
{
    null = 0,
    none,
    player,
    wall,
    stairs,
    familiar,
    monster
};

struct Entity_High
{
    b32 exists;
    b32 is_jumping;

    // NOTE: relative to camera
    v2 pos, vel;
    f32 vel_z;
    f32 z;
    u32 abs_tile_z;
    u32 direction;
    f32 stair_cd;
    f32 hit_cd;

    u32 low_i;
};

struct Entity_Low
{
    b32 collides;
    b32 barrier;
    u32 high_i;
    u32 hit_points;
    u32 max_hit_points;
    s32 virtual_z;
    f32 width, height;
    f32 argb_offset;
    World_Pos pos;
    Color color;
    Entity_Type type;
};

struct Entity
{
    u32 low_i;
    Entity_Low *low;
    Entity_High *high;
};

struct Entity_Low_Chunk_Ref
{
    World_Chunk *tile_chunk;
    u32 i_in_chunk;
};

struct Entity_Visible_Piece
{
    ARGB_img *img;
    v2 mid_p;
    f32 z;
    f32 alpha;
    rect::Rect_v2 rect;
    Color color;
};

struct Entity_Visble_Group_Piece
{
    u32 piece_cnt;
    Entity_Visible_Piece pieces[64];
};

struct Camera
{
    World_Pos pos;
};

struct Game_State
{
    Memory_Arena world_arena;
    Game_World *world;

    u32 entity_camera_follow_ind;
    Camera camera;
    Bitmap_Img bitmap;
    u32 player_controller_ind[Game_Input::s_input_cnt];
    u32 player_cnt;
    Entity_Actions player_acts[Game_Input::s_input_cnt];

    u32 low_cnt;
    u32 high_cnt;
    Entity_Low low_entities[global::max_low_cnt];
    Entity_High high_entities[global::max_high_cnt];

    ARGB_img bg_img;
    ARGB_img grass_bg;
    ARGB_img crosshair_img;
    ARGB_img red_square_img;
    ARGB_img green_square_img;
    ARGB_img blue_square_img;
    ARGB_img player_sprites[4];
    ARGB_img monster_sprites[4];
    ARGB_img cat_sprite;
    ARGB_img tree_sprite;
    ARGB_img stair_sprite;

    World_Pos test_pos;

    b32 debug_draw_collision;
};

inline bool
is_key_up(const Game_Button_State &key)
{
    return key.half_transition_count > 0 && key.ended_down == 0;
}

inline bool
is_button_up(const Game_Button_State &button)
{
    return is_key_up(button);
}
}  // namespace tom
