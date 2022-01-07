#include "tile.hpp"
#include "framework.hpp"
#include "game.hpp"

namespace tomato
{

void
recanonicalize_coord(const Tile_map &tile_map_, u32 &tile_, f32 &tile_rel_)
{
    // NOTE: world is assumed to be torodial (torus shaped world),
    // if you step off one end where you wrap around
    i32 offset = math::round_f32_to_i32(tile_rel_ / (f32)Tile_map::s_tile_size_meters);

    tile_ += offset;
    tile_rel_ -= offset * (f32)Tile_map::s_tile_size_meters;

    assert(tile_rel_ >= -.5f * Tile_map::s_tile_size_meters);
    assert(tile_rel_ <= .5f * Tile_map::s_tile_size_meters);
}

Tile_map_pos
recanonicalize_pos(Tile_map &tile_map_, Tile_map_pos pos_)
{
    auto new_pos = pos_;

    recanonicalize_coord(tile_map_, new_pos.abs_tile_x, new_pos.tile_rel_x);
    recanonicalize_coord(tile_map_, new_pos.abs_tile_y, new_pos.tile_rel_y);

    return new_pos;
}

Tile_chunk_pos
get_chunk_pos(u32 abs_tile_x_, u32 abs_tile_y_)
{
    Tile_chunk_pos chunk_pos;

    chunk_pos.chunk_tile_x = abs_tile_x_ >> Tile_map::s_chunk_bit_shift;
    chunk_pos.chunk_tile_y = abs_tile_y_ >> Tile_map::s_chunk_bit_shift;
    chunk_pos.rel_tile_x   = abs_tile_x_ & Tile_map::s_chunk_bit_mask;
    chunk_pos.rel_tile_y   = abs_tile_y_ & Tile_map::s_chunk_bit_mask;

    return chunk_pos;
}

Tile_chunk_pos
get_chunk_pos(Tile_map_pos pos_)
{
    Tile_chunk_pos chunk_pos;

    chunk_pos = get_chunk_pos(pos_.abs_tile_x, pos_.abs_tile_y);

    return chunk_pos;
}

Tile_chunk *
get_tile_chunk(Tile_map &tile_map_, i32 tile_chunk_x_, i32 tile_chunk_y_)
{
    Tile_chunk *tile_chunk = nullptr;

    if (tile_chunk_x_ >= 0 && tile_chunk_x_ < Tile_map::s_chunk_count && tile_chunk_y_ >= 0 &&
        tile_chunk_y_ < Tile_map::s_chunk_count) {
        tile_chunk =
            &tile_map_.tile_chunks[tile_chunk_y_ * Tile_map::s_chunk_count + tile_chunk_x_];
    }
    return tile_chunk;
}

u32
get_tile_value_unchecked(Tile_chunk &tile_chunk_, u32 tile_x_, u32 tile_y_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= Tile_map::s_chunk_tile_count && tile_y_ <= Tile_map::s_chunk_tile_count);
    return tile_chunk_.tiles[tile_y_ * Tile_map::s_chunk_tile_count + tile_x_];
}

u32
get_tile_value(Tile_chunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_)
{
    u32 tile_value {};
    if (tile_chunk_) {
        tile_value = get_tile_value_unchecked(*tile_chunk_, abs_tile_x_, abs_tile_y_);
    }

    return tile_value;
}

u32
get_tile_value(Tile_map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_)
{
    u32 tile_value {};

    Tile_chunk_pos chunk_pos = get_chunk_pos(abs_tile_x_, abs_tile_y_);
    Tile_chunk *tile_chunk =
        get_tile_chunk(tile_map_, chunk_pos.chunk_tile_x, chunk_pos.chunk_tile_y);
    if (tile_chunk) {
        tile_value = get_tile_value(tile_chunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y);
    }

    return tile_value;
}

void
set_tile_value_unchecked(Tile_chunk &tile_chunk_, u32 tile_x_, u32 tile_y_, u32 tile_value_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= Tile_map::s_chunk_tile_count && tile_y_ <= Tile_map::s_chunk_tile_count);
    tile_chunk_.tiles[tile_y_ * Tile_map::s_chunk_tile_count + tile_x_] = tile_value_;
}

void
set_tile_value(Tile_chunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_, u32 tile_value_)
{
    if (tile_chunk_) {
        set_tile_value_unchecked(*tile_chunk_, abs_tile_x_, abs_tile_y_, tile_value_);
    }
}

void
set_tile_value(Mem_arena *arena_, Tile_map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_,
               u32 tile_value_)
{
    Tile_chunk_pos chunk_pos = get_chunk_pos(abs_tile_x_, abs_tile_y_);
    Tile_chunk *tile_chunk =
        get_tile_chunk(tile_map_, chunk_pos.chunk_tile_x, chunk_pos.chunk_tile_y);

    // TODO: on-demand tile chunk creation
    assert(tile_chunk);
    set_tile_value(tile_chunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y, tile_value_);
}

bool
is_world_tile_empty(Tile_map &tile_map_, Tile_map_pos test_pos_)
{
    bool is_empty = get_tile_value(tile_map_, test_pos_.abs_tile_x, test_pos_.abs_tile_y) == 0;

    return is_empty;
}

}  // namespace tomato
