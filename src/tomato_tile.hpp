#ifndef TOMATO_TILE_HPP_
#define TOMATO_TILE_HPP_

#include "tomato_platform.h"
#include "tomato_math.hpp"
#include "tomato_common.hpp"

// TODO: change to V3
struct Tile_Map_Dif
{
    v2 dif_xy;
    f32 dif_z;
};

struct Tile_Map_Pos
{
    // NOTE: these are fixed point positioins. The high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    i32 abs_tile_x;
    i32 abs_tile_y;
    i32 abs_tile_z;

    v2 offset;
};

inline bool
operator==(const Tile_Map_Pos &lhs, const Tile_Map_Pos &rhs)
{
    return (lhs.abs_tile_x == rhs.abs_tile_x && lhs.abs_tile_y == rhs.abs_tile_y &&
            lhs.abs_tile_z == rhs.abs_tile_z && lhs.offset.x == rhs.offset.x &&
            lhs.offset.y == rhs.offset.y);
}
inline bool
operator!=(const Tile_Map_Pos &lhs, const Tile_Map_Pos &rhs)
{
    return !(lhs == rhs);
}

struct Tile_Chunk_Pos
{
    i32 chunk_tile_x;
    i32 chunk_tile_y;
    i32 chunk_tile_z;

    i32 rel_tile_x;
    i32 rel_tile_y;
};

struct Tile_Chunk
{
    i32 x;
    i32 y;
    i32 z;

    i32 *tiles;

    Tile_Chunk *next_in_hash;
};

struct Tile_Map
{
    // NOTE: set to 16x16 tiles per chunk
    static constexpr i32 s_chunk_bit_shift        = 4;
    static constexpr i32 s_chunk_bit_mask         = (1 << s_chunk_bit_shift) - 1;
    static constexpr i32 s_chunk_tile_count       = (1 << s_chunk_bit_shift);
    static constexpr i32 s_chunk_tile_count_total = s_chunk_tile_count * s_chunk_tile_count;
    static constexpr i32 s_chunk_count            = 128;
    static constexpr i32 s_chunk_count_z          = 2;
    static constexpr i32 s_tile_chunk_safe_margin = INT32_MAX / 64;

    static constexpr f32 s_tile_size_meters = 1.4f;

    Tile_Chunk tile_chunk_hash[4096];
};

#endif  // TOMATO_TILE_HPP_
