#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_

#include "tomato_platform.h"
#include "tomato_math.hpp"
#include "tomato_common.hpp"

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
    i32 x;
    i32 y;
    i32 z;

    v2 offset;
};

inline bool
operator==(const World_Pos &lhs_, const World_Pos &rhs_)
{
    return (lhs_.x == rhs_.x && lhs_.y == rhs_.y && lhs_.z == rhs_.z &&
            lhs_.offset.x == rhs_.offset.x && lhs_.offset.y == rhs_.offset.y);
}
inline bool
operator!=(const World_Pos &lhs_, const World_Pos &rhs_)
{
    return !(lhs_ == rhs_);
}

struct World_Entity_Block
{
    u32 entity_cnt;
    u32 entity_i[16];
    World_Entity_Block *next;
};

struct World_Chunk
{
    i32 x;
    i32 y;
    i32 z;

    World_Entity_Block first_block;

    World_Chunk *next_in_hash;
};

struct World
{
    // NOTE: set to 16x16 tiles per chunk
    static constexpr i32 s_chunk_bit_shift        = 4;
    static constexpr i32 s_chunk_bit_mask         = (1 << s_chunk_bit_shift) - 1;
    static constexpr i32 s_chunk_tile_count       = (1 << s_chunk_bit_shift);
    static constexpr i32 s_chunk_tile_count_total = s_chunk_tile_count * s_chunk_tile_count;
    static constexpr i32 s_chunk_count            = 128;
    static constexpr i32 s_chunk_count_z          = 2;
    static constexpr i32 s_chunk_safe_margin      = INT32_MAX / 64;

    static constexpr f32 s_tile_size_meters = 1.4f;

    // TODO:  shold probably switch to a pointer if tile entity blocks
    //  continue to be stored en masse direclty in the tile chunk
    World_Chunk world_chunk_hash[4096];
};

#endif  // TOMATO_WORLD_HPP_
