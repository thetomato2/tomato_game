#ifndef ENTITY_HPP_
#define ENTITY_HPP_
#include "common.hpp"
#include "world.hpp"
#include "image.hpp"

namespace tom
{
struct Entity_Actions
{
    bool start;
    bool jump;
    bool attack;

    v2 dir;

    bool sprint;
};

enum Entity_Direction : s32
{
    north = 0,
    east,
    south,
    west
};

enum class Entity_Type
{
    null = 0,
    none,
    player,
    wall,
    stairs,
    familiar,
    monster,
    sword
};

struct Entity_High
{
    b32 is_jumping;
    b32 is_attacking;

    // NOTE: relative to camera
    v2 pos, vel;
    f32 vel_z;
    f32 z;
    u32 abs_tile_z;
    Entity_Direction dir;
    f32 stair_cd;
    f32 hit_cd;
    f32 attack_cd;

    u32 low_i;
};

struct Entity_Low
{
    b32 active;
    b32 collides;
    b32 barrier;
    b32 hurtbox;
    u32 high_i;
    u32 weapon_i;
    u32 parent_i;
    s32 hit_points;
    u32 max_hit_points;
    s32 virtual_z;
    f32 width, height;
    f32 argb_offset;
    World_Pos pos;
    Color color;
    Entity_Type type;
    ARGB_img *sprite;
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
    Rect rect;
    Color color;
};

struct Entity_Visble_Piece_Group
{
    u32 piece_cnt;
    Entity_Visible_Piece pieces[64];
};
}  // namespace tom
#endif  // ENTITY_HPP_
