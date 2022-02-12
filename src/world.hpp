#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_

#include "platform.h"
#include "math.hpp"
#include "common.hpp"

namespace tom
{
// TODO: change to V3
struct world_dif
{
    v2 dif_xy;
    f32 dif_z;
};

struct world_pos
{
    // NOTE: these are fixed point positioins. The high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    s32 chunk_x;
    s32 chunk_y;
    s32 chunk_z;

    // NOTE: from the chunk center
    v2 offset;
};

struct world_entity_block
{
    u32 low_entity_cnt;
    u32 low_ent_inds[16];
    world_entity_block *next;
};

struct world_chunk
{
    s32 x;
    s32 y;
    s32 z;

    world_entity_block first_block;

    world_chunk *next_in_hash;
};

struct game_world
{
    world_chunk world_chunk_hash[4096];

    world_entity_block *first_free;
};
}  // namespace tom
#endif  // TOMATO_WORLD_HPP_
