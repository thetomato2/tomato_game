#ifndef TOMATO_ENTITY_HPP_
#define TOMATO_ENTITY_HPP_

#include "common.hpp"
#include "world.hpp"

namespace tom
{

struct argb_img;
struct game_state;
struct sim_region;
struct sim_entity;

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

namespace sim_entity_flags
{
enum : s32
{
    active     = BIT(0),
    collides   = BIT(1),
    barrier    = BIT(2),
    nonspatial = BIT(3),
    hurtbox    = BIT(4),

    simming = BIT(31),
};
}

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
    color_argb color;
};

struct entity_visble_piece_group
{
    u32 piece_cnt;
    // TODO: how many pieces?
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
    u32 ent_i;
    b32 updateable;

    s32 flags;
    v2 pos;
    v2 vel;
    u32 chunk_z;
    f32 z;
    f32 vel_z;
    f32 hit_cd;
    f32 exists_cd;
    f32 dist_limit;
    s32 hp;
    u32 max_hp;
    s32 virtual_z;
    f32 width, height;
    f32 argb_offset;
    u32 weapon_i;
    u32 parent_i;
    s32 cur_sprite;
    entity_direction dir;
    entity_type type;
};

struct entity
{
    sim_entity sim;
    world_pos world_pos;
};

struct entity_move_spec
{
    f32 speed;
    f32 drag;
};

inline entity_move_spec
default_move_spec()
{
    return { 10.0f, 10.0f };
}

inline void
make_entity_nonspatial(sim_entity *ent)
{
    set_flag(ent->flags, sim_entity_flags::nonspatial);
}

inline void
make_entity_spatial(sim_entity *ent, v2 pos, v2 vel = { 0.0f, 0.0f })
{
    set_flag(ent->flags, sim_entity_flags::nonspatial);
    ent->pos = pos;
    ent->vel = vel;
}

entity *
get_entity(game_state *state, u32 ind);

entity *
add_new_entity(game_state *state, f32 abs_x = 0.f, f32 abs_y = 0.f, f32 abs_z = 0.f);

entity *
add_tree(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z);

entity *
add_monster(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z);

entity *
add_cat(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z);

entity *
add_sword(game_state *state, u32 parent_i);

void
add_player(game_state *state, u32 player_i, f32 abs_x, f32 abs_y, f32 abs_z);

void
update_familiar(game_state *state, sim_region *region, entity *fam, f32 dt);

void
update_sword(game_state *state, sim_region *region, entity *sword, f32 dt);

void
update_monster(game_state *state, sim_region *region, entity *monster, f32 dt);

void
update_player(game_state *state, sim_region *region, entity *player, f32 dt);

}  // namespace tom
#endif  // ENTITY_HPP_
