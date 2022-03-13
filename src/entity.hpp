#ifndef ENTITY_HPP_
#define ENTITY_HPP_
#include "common.hpp"
#include "world.hpp"
#include "image.hpp"

namespace tom
{
struct entity_actions
{
    bool start;
    bool jump;
    bool attack;

    v2 dir;

    bool sprint;
};

enum entity_direction : s32
{
    north = 0,
    east,
    south,
    west
};

enum class entity_type
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

struct entity_low_chunk_ref
{
    world_chunk *tile_chunk;
    u32 i_in_chunk;
};

struct entity_visible_piece
{
    argb_img *img;
    v2 mid_p;
    f32 z;
    f32 alpha;
    rect rect;
    color color;
};

struct entity_visble_piece_group
{
    u32 piece_cnt;
    entity_visible_piece pieces[64];
};

struct sim_entity;

union entity_ref
{
    sim_entity *ptr;
    u32 ind;
};

struct sim_entity
{
    v2 pos;
    v2 vel;
    u32 chunk_z;
    f32 z;
    f32 vel_z;

    f32 hit_cd;

    b32 active;
    b32 collides;
    b32 barrier;
    b32 hurtbox;
    s32 hp;
    u32 max_hp;
    s32 virtual_z;
    f32 width, height;
    f32 argb_offset;

    u32 stored_i;

    entity_ref weapon_i;
    entity_ref parent_i;

    color color;
    entity_type type;
    entity_direction dir;
    argb_img *sprite;
};

struct stored_entity
{
    sim_entity sim;
    world_pos world_pos;
};

}  // namespace tom
#endif  // ENTITY_HPP_
