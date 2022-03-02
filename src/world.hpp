#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_
#include "common.hpp"

namespace tom
{
// TODO: change to V3
struct World_Dif
{
    v2 dif_xy;
    f32 dif_z;
};

struct World_Pos
{
    // NOTE: these are fixed point positioins. The high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    s32 chunk_x, chunk_y, chunk_z;
    // NOTE: from the chunk center
    v2 offset;
};

struct World_Entity_Block
{
    u32 low_entity_cnt;
    u32 low_ent_inds[16];
    World_Entity_Block *next;
};

struct World_Chunk
{
    s32 x, y, z;
    World_Entity_Block first_block;
    World_Chunk *next_in_hash;
};

struct World
{
    World_Chunk world_chunk_hash[4096];
    World_Entity_Block *first_free;
};

void
init_world(World &world, f32 tile_sizes_in_meters);

World_Dif
get_world_diff(const World_Pos &pos_a, const World_Pos &pos_b);

World_Pos
map_into_chunk_space(const World_Pos &pos, const v2 offset);

World_Chunk *
get_world_chunk(World &world, const s32 chunk_x, const s32 chunk_y, const s32 chunk_z,
                Memory_Arena *arena = nullptr);

World_Pos
abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z);

void
change_entity_location(Memory_Arena *arena, World &world, const u32 low_i, const World_Pos *old_pos,
                       World_Pos *new_pos);

}  // namespace tom

#endif  // TOMATO_WORLD_HPP_
