
#include "platform.h"
#include "intrinsic.hpp"
#include "common.hpp"

#include "math.hpp"
#include "utils.hpp"
#include "world.cpp"

namespace tom
{
struct entity_actions
{
    bool start;
    bool jump;

    v2 dir;

    bool sprint;
};

struct color_u32
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
struct bitmap_header
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

struct bitmap_img
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

enum entity_direction : s32
{
    down = 0,
    right,
    up,
    left

};

enum class entity_type
{
    null = 0,
    none,
    player,
    wall,
    stairs,
    familiar,
    monster
};

struct entity_high
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

    u32 low_i;
};

struct entity_low
{
    world_pos pos;
    s32 virtual_z;
    f32 width, height;
    color_u32 color;
    b32 collides;
    b32 barrier;
    entity_type type;
    u32 high_i;
    f32 argb_offset;
};

struct entity
{
    u32 low_i;
    entity_low *low;
    entity_high *high;
};

struct entity_low_chunk_ref
{
    world_chunk *tile_chunk;
    u32 i_in_chunk;
};

struct entity_visible_piece
{
    ARGB_img *img;
    v2 mid_p;
    f32 z;
    f32 alpha;
};

struct entity_visible_piece_group
{
    u32 piece_cnt;
    entity_visible_piece pieces[8];
};

struct camera
{
    world_pos pos;
};

struct game_state
{
    memory_arena world_arena;
    game_world *world;

    u32 entity_camera_follow_ind;
    camera camera;
    bitmap_img bitmap;
    u32 player_controller_ind[game_input::s_input_cnt];
    u32 player_cnt;
    entity_actions player_acts[game_input::s_input_cnt];

    u32 low_cnt;
    u32 high_cnt;
    entity_low low_entities[global::max_low_cnt];
    entity_high high_entities[global::max_high_cnt];

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

    world_pos test_pos;

    b32 debug_draw_collision;
};

inline bool
is_key_up(const game_button_state &key)
{
    return key.half_transition_count > 0 && key.ended_down == 0;
}

inline bool
is_button_up(const game_button_state &button)
{
    return is_key_up(button);
}
}  // namespace tom
