#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_

#include "common.hpp"

#define CHUNK_UNITIALIZED S32_MAX

namespace tom
{

struct entity;

struct world_pos
{
    // NOTE: these are fixed point positioins. the high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    s32 chunk_x, chunk_y, chunk_z;
    // NOTE: from the chunk center
    v3 offset;
};

struct world_entity_block
{
    u32 ent_cnt;
    u32 ent_inds[16];
    world_entity_block *next;
};

struct world_chunk
{
    s32 x, y, z;
    world_entity_block first_block;
    world_chunk *next_in_hash;
};

struct world
{
    world_chunk world_chunk_hash[4096];
    world_entity_block *first_free;
};

inline world_pos
null_world_pos()
{
    world_pos result = {};
    result.chunk_x   = CHUNK_UNITIALIZED;
    return result;
}

inline bool
is_valid(world_pos pos)
{
    return pos.chunk_x != CHUNK_UNITIALIZED;
}

inline bool
is_valid(world_pos *pos)
{
    if (pos) return is_valid(*pos);

    return false;
}

inline bool
operator==(world_pos &lhs, world_pos &rhs)
{
    return lhs.chunk_x == rhs.chunk_x && lhs.chunk_y == rhs.chunk_y && lhs.chunk_z == rhs.chunk_z &&
           lhs.offset == rhs.offset;
}

inline bool
operator!=(world_pos &lhs, world_pos &rhs)
{
    return !(lhs == rhs);
}

void
init_world(world *world, f32 tile_sizes_in_meters);

v3
get_world_diff(world_pos pos_a, world_pos pos_b);

world_pos
map_into_chunk_space(world_pos pos, v3 offset = { 0.f, 0.f, 0.f });

world_pos
map_into_chunk_space(world_pos pos, v2 offset = { 0.f, 0.f });

world_chunk *
get_world_chunk(world *world, s32 chunk_x, s32 chunk_y, s32 chunk_z, memory_arena *arena = nullptr);

world_pos
abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z);

void
change_entity_location(memory_arena *arena, world *world, entity *ent, world_pos new_pos_init);

}  // namespace tom

#endif  // TOMATO_WORLD_HPP_
