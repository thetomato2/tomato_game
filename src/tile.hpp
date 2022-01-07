#pragma once
#include "base_types.hpp"

namespace tomato
{

struct mem_arena;

struct tile_map_pos
{
    // NOTE: these are fixed point positioins. The high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    u32 abs_tile_x;
    u32 abs_tile_y;

    f32 tile_rel_x;
    f32 tile_rel_y;
};

struct tile_chunk_pos
{
    u32 chunk_tile_x;
    u32 chunk_tile_y;

    u32 rel_tile_x;
    u32 rel_tile_y;
};

struct tile_chunk
{
    u32 *tiles;
};

struct tile_map
{
    // NOTE: set to 256x256 tiles per chunk
    static constexpr u32 s_chunk_bit_shift  = 8;
    static constexpr u32 s_chunk_bit_mask   = (1 << s_chunk_bit_shift) - 1;
    static constexpr u32 s_chunk_tile_count = (1 << s_chunk_bit_shift);
    static constexpr u32 s_chunk_count      = 4;

    static constexpr f32 s_tile_size_meters = 1.4f;
    static constexpr u32 s_tile_size_pixels = 60;
    static constexpr f32 s_meters_to_pixels = s_tile_size_pixels / s_tile_size_meters;

    tile_chunk *tile_chunks;
    tile_chunk *cur_tile_chunk;
};

void
recanonicalize_coord(const tile_map &tile_map_, u32 &tile_, f32 &tile_rel_);

tile_map_pos
recanonicalize_pos(tile_map &tile_map_, tile_map_pos pos_);

tile_chunk_pos
get_chunk_pos(u32 abs_tile_x_, u32 abs_tile_y_);

tile_chunk_pos
get_chunk_pos(tile_map_pos pos_);

tile_chunk *
get_tile_chunk(tile_map &tile_map_, i32 tile_chunk_x_, i32 tile_chunk_y_);

u32
get_tile_value_unchecked(tile_chunk &tile_chunk_, u32 tile_x_, u32 tile_y_);

u32
get_tile_value(tile_chunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_);

u32
get_tile_value(tile_map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_);

void
set_tile_value_unchecked(tile_chunk &tile_chunk_, u32 tile_x_, u32 tile_y_, u32 tile_value_);

void
set_tile_value(tile_chunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_, u32 tile_value_);

void
set_tile_value(mem_arena *arena_, tile_map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_,
               u32 tile_value_);

bool
is_world_tile_empty(tile_map &tile_map_, tile_map_pos test_pos_);

}  // namespace tomato
