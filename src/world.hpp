#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_

#include "common.hpp"

namespace tom
{
// TODO: change to v3
struct world_dif
{
    v2 dif_xy;
    f32 dif_z;
};

struct world_pos
{
    // NOTE: these are fixed point positioins. the high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    s32 chunk_x, chunk_y, chunk_z;
    // NOTE: from the chunk center
    v2 offset;
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

void
init_world(world *world, f32 tile_sizes_in_meters);

world_dif
get_world_diff(const world_pos &pos_a, const world_pos &pos_b);

world_pos
map_into_chunk_space(const world_pos &pos, const v2 offset);

world_chunk *
get_world_chunk(world *world, const s32 chunk_x, const s32 chunk_y, const s32 chunk_z,
                memory_arena *arena = nullptr);

world_pos
abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z);

void
change_entity_location(memory_arena *arena, world *world, u32 ent_i, world_pos *old_pos,
                       world_pos *new_pos);

}  // namespace tom

#endif  // TOMATO_WORLD_HPP_
