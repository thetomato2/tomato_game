#ifndef TOM_ENTITY_HH
#define TOM_ENTITY_HH

#include "tom_core.hh"

namespace tom
{

struct EntityActions
{
    bool start;
    bool jump;
    bool attack;

    v3f dir;

    bool sprint;
};

enum EntityDirection : i32
{
    north = 0,
    east,
    south,
    west,
    north_east,
    north_west,
    south_east,
    south_west
};

enum EntityFlags : i32
{
    active     = Bit(0),
    collides   = Bit(1),
    barrier    = Bit(2),
    nonspatial = Bit(3),
    hurtbox    = Bit(4),
    updateable = Bit(5),

    simming = Bit(31),
};

enum class EntityType
{
    null = 0,
    none,
    player,
    wall,
    tree,
    bush,
    stairs,
    familiar,
    monster,
    sword,
    stair
};

struct EntityMoveSpec
{
    f32 speed;
    f32 drag;
};

struct Entity
{
    u32 ind;
    EntityType type;
    i32 flags;
    v3f pos;
    v3f vel;
    v3f dims;
    f32 hit_cd;
    f32 exists_cd;
    f32 dist_limit;
    i32 hp;
    u32 max_hp;
    u32 weapon_i;
    u32 parent_i;
    f32 next_sprite;
    i32 cur_sprite;
    v2f sprite_off;
    EntityDirection dir;
};

inline EntityMoveSpec default_move_spec()
{
    return { 10.0f, 10.0f };
}

////////////////////////////////////////////////////////////////////////////////////////////////
// #DECLARES

struct GameState;

Entity *get_entity(GameState *state, u32 ind);
Entity* add_entity(GameState *game, EntityType type, v3f pos = {});

}  // namespace tom

#endif
