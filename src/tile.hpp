#pragma once
#include "base_types.hpp"

namespace tomato
{

struct MemArena;

struct TileMapDif
{
    f32 dif_x;
    f32 dif_y;
    f32 dif_z;
};

struct TileMapPos
{
    // NOTE: these are fixed point positioins. The high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    u32 abs_tile_x;
    u32 abs_tile_y;
    u32 abs_tile_z;

    f32 off_rel_x;
    f32 off_rel_y;
};

struct TileChunkPos
{
    u32 chunk_tile_x;
    u32 chunk_tile_y;
    u32 chunk_tile_z;

    u32 rel_tile_x;
    u32 rel_tile_y;
};

struct TileChunk
{
    u32 *tiles;
};

struct TileMap
{
    // NOTE: set to 16x16 tiles per chunk
    static constexpr u32 s_chunk_bit_shift        = 4;
    static constexpr u32 s_chunk_bit_mask         = (1 << s_chunk_bit_shift) - 1;
    static constexpr u32 s_chunk_tile_count       = (1 << s_chunk_bit_shift);
    static constexpr u32 s_chunk_tile_count_total = s_chunk_tile_count * s_chunk_tile_count;
    static constexpr u32 s_chunk_count            = 128;
    static constexpr u32 s_chunk_count_z          = 2;

    static constexpr f32 s_tile_size_meters = 1.4f;

    TileChunk *tile_chunks;
    TileChunk *cur_tile_chunk;
};

void
recanonicalize_coord(const TileMap &tile_map_, u32 &tile_, f32 &tile_rel_);

TileMapPos
recanonicalize_pos(TileMap &tile_map_, TileMapPos pos_);

TileChunkPos
get_chunk_pos(u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_);

TileChunkPos
get_chunk_pos(TileMapPos pos_);

TileChunk *
get_tile_chunk(TileMap &tile_map_, u32 tile_chunk_x_, u32 tile_chunk_y_, u32 tile_chunk_z_);

u32
get_tile_value_unchecked(TileChunk &tile_chunk_, u32 tile_x_, u32 tile_y_);

u32
get_tile_value(TileChunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_);

u32
get_tile_value(TileMap &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_);

void
set_tile_value_unchecked(TileChunk &tile_chunk_, u32 tile_x_, u32 tile_y_, u32 tile_value_);

void
set_tile_value(TileChunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_,
               u32 tile_value_);

void
set_tile_value(MemArena *arena_, TileMap &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_,
               u32 abs_tile_z_, u32 tile_value_);

bool
is_world_tile_empty(TileMap &tile_map_, TileMapPos test_pos_);

}  // namespace tomato
