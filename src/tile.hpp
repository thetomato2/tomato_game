#pragma once
#include "base_types.hpp"
#include "math.hpp"

namespace tomato
{

struct Mem_Arena;

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
    u32 abs_tile_x;
    u32 abs_tile_y;
    u32 abs_tile_z;

    v2 offset;
};

struct Tile_Chunk_Pos
{
    u32 chunk_tile_x;
    u32 chunk_tile_y;
    u32 chunk_tile_z;

    u32 rel_tile_x;
    u32 rel_tile_y;
};

struct Tile_Chunk
{
    u32 *tiles;
};

struct Tile_Map
{
    // NOTE: set to 16x16 tiles per chunk
    static constexpr u32 s_chunk_bit_shift        = 4;
    static constexpr u32 s_chunk_bit_mask         = (1 << s_chunk_bit_shift) - 1;
    static constexpr u32 s_chunk_tile_count       = (1 << s_chunk_bit_shift);
    static constexpr u32 s_chunk_tile_count_total = s_chunk_tile_count * s_chunk_tile_count;
    static constexpr u32 s_chunk_count            = 128;
    static constexpr u32 s_chunk_count_z          = 2;

    static constexpr f32 s_tile_size_meters = 1.4f;

    Tile_Chunk *tile_chunks;
    Tile_Chunk *cur_tile_chunk;
};

void
recanonicalize_coord(u32 &tile_, f32 &tile_rel_);

Tile_Map_Pos
recanonicalize_pos(const Tile_Map_Pos &pos_);

Tile_Chunk_Pos
get_chunk_pos(u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_);

Tile_Chunk_Pos
get_chunk_pos(const Tile_Map_Pos &pos_);

Tile_Chunk *
get_tile_chunk(const Tile_Map &tile_map_, u32 tile_chunk_x_, u32 tile_chunk_y_, u32 tile_chunk_z_);

u32
get_tile_value_unchecked(const Tile_Chunk &tile_chunk_, u32 tile_x_, u32 tile_y_);

u32
get_tile_value(Tile_Chunk *const tile_chunk_, u32 tile_x_, u32 tile_y_);

u32
get_tile_value(const Tile_Map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_);

u32
get_tile_value(const Tile_Map &tile_map_, const Tile_Map_Pos &pos_);

void
set_tile_value_unchecked(const Tile_Chunk &tile_chunk_, u32 tile_x_, u32 tile_y_, u32 tile_value_);

void
set_tile_value(Tile_Chunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_, u32 tile_value_);

void
set_tile_value(Mem_Arena *arena_, Tile_Map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_,
               u32 abs_tile_z_, u32 tile_value_);
bool
is_tile_value_empty(u32 tile_value_);
bool
is_world_tile_empty(const Tile_Map &tile_map_, const Tile_Map_Pos &test_pos_);

Tile_Map_Dif
get_tile_diff(const Tile_Map_Pos &pos_a_, const Tile_Map_Pos &pos_b_);

Tile_Map_Pos
get_centered_tile_point(u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_);

Tile_Map_Pos
offset_pos(const Tile_Map_Pos &pos_, v2 offset_);

}  // namespace tomato
