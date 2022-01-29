#include "tomato_platform.hpp"
#include "tomato_intrinsic.hpp"

struct Mem_Arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

inline void *
push_size(Mem_Arena *arena_, mem_ind size_)
{
    assert((arena_->used + size_) <= arena_->size);
    void *result = arena_->base + arena_->used;
    arena_->used += size_;

    return result;
}

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))

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

struct High_Entity
{
    b32 exists;
    // NOTE: relative to camera
    v2 pos, vel;
    u32 abs_tile_z;
    u32 direction;
    f32 stair_cd;
};

struct Low_Entity
{
};

struct Dormant_Entity
{
    Tile_Map_Pos pos;
    f32 width, height;
    Color_u32 color;
    ARGB_Img *sprites;

    b32 collides;
    b32 stairs;
};

enum Entity_Residence
{
    non_existent,
    dormant,
    low,
    high
};

struct Entity
{
    Entity_Residence *residence;
    Low_Entity *low;
    High_Entity *high;
    Dormant_Entity *dormant;
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

    u32 entity_cnt;
    u32 player_cnt;
    Entity_Residence entity_residence[s_max_entities];
    High_Entity high_entities[s_max_entities];
    Low_Entity low_entities[s_max_entities];
    Dormant_Entity dormant_entities[s_max_entities];

    ARGB_Img bg_img;
    ARGB_Img seaside_cliff;

    ARGB_Img crosshair_img;

    ARGB_Img red_square_img;
    ARGB_Img green_square_img;
    ARGB_Img blue_square_img;

    ARGB_Img player_sprites[4];

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
