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

struct Sim_Entity;

union Entity_Ref
{
    Sim_Entity *ptr;
    u32 ind;
};

struct Sim_Entity
{
    v2 pos;
    v2 vel;
    u32 chunk_z;
    f32 z;
    f32 vel_z;

    b32 active;
    b32 collides;
    b32 barrier;
    b32 hurtbox;
    s32 hit_points;
    u32 max_hit_points;
    s32 virtual_z;
    f32 width, height;
    f32 argb_offset;

    u32 stored_i;

    Entity_Ref weapon_i;
    Entity_Ref parent_i;

    Color color;
    Entity_Type type;
    Entity_Direction direction;
    ARGB_img *sprite;
};

struct Stored_Entity
{
    Sim_Entity sim;
    World_Pos world_pos;
};

}  // namespace tom
#endif  // ENTITY_HPP_
