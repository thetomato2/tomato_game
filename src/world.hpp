#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_

#include "platform.h"
#include "math.hpp"
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
    s32 chunk_x;
    s32 chunk_y;
    s32 chunk_z;

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
    s32 x;
    s32 y;
    s32 z;

    World_Entity_Block first_block;

    World_Chunk *next_in_hash;
};

struct Game_World
{
    World_Chunk world_chunk_hash[4096];

    World_Entity_Block *first_free;
};
}  // namespace tom
#endif  // TOMATO_WORLD_HPP_
