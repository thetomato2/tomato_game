#include "tile.hpp"
#include "framework.hpp"
#include "game.hpp"

namespace tomato
{

void
recanonicalize_coord(const tile_map &tile_map_, u32 &tile_, f32 &tile_rel_)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    i32 offset = math::round_f32_to_i32(tile_rel_ / (f32)tile_map::s_tile_size_meters);

    tile_ += offset;
    tile_rel_ -= offset * (f32)tile_map::s_tile_size_meters;

    assert(tile_rel_ >= -.5f * tile_map::s_tile_size_meters);
    assert(tile_rel_ <= .5f * tile_map::s_tile_size_meters);
}

tile_map_pos
recanonicalize_pos(tile_map &tile_map_, tile_map_pos pos_)
{
    auto new_pos = pos_;

    recanonicalize_coord(tile_map_, new_pos.abs_tile_x, new_pos.tile_rel_x);
    recanonicalize_coord(tile_map_, new_pos.abs_tile_y, new_pos.tile_rel_y);

    return new_pos;
}

tile_chunk_pos
get_chunk_pos(u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_)
{
    tile_chunk_pos chunk_pos;

    chunk_pos.chunk_tile_x = abs_tile_x_ >> tile_map::s_chunk_bit_shift;
    chunk_pos.chunk_tile_y = abs_tile_y_ >> tile_map::s_chunk_bit_shift;
    chunk_pos.chunk_tile_z = abs_tile_z_;
    chunk_pos.rel_tile_x   = abs_tile_x_ & tile_map::s_chunk_bit_mask;
    chunk_pos.rel_tile_y   = abs_tile_y_ & tile_map::s_chunk_bit_mask;

    return chunk_pos;
}

tile_chunk_pos
get_chunk_pos(tile_map_pos pos_)
{
    tile_chunk_pos chunk_pos;

    chunk_pos = get_chunk_pos(pos_.abs_tile_x, pos_.abs_tile_y, pos_.abs_tile_z);

    return chunk_pos;
}

tile_chunk *
get_tile_chunk(tile_map &tile_map_, u32 tile_chunk_x_, u32 tile_chunk_y_, u32 tile_chunk_z_)
{
    tile_chunk *tile_chunk = nullptr;

    if (tile_chunk_x_ >= 0 && tile_chunk_x_ < tile_map::s_chunk_count && tile_chunk_y_ >= 0 &&
        tile_chunk_y_ < tile_map::s_chunk_count && tile_chunk_z_ >= 0 &&
        tile_chunk_z_ < tile_map::s_chunk_count_z) {
        tile_chunk =
            &tile_map_.tile_chunks[tile_chunk_z_ * tile_map::s_chunk_count_z +
                                   tile_chunk_y_ * tile_map::s_chunk_count + tile_chunk_x_];
    }
    return tile_chunk;
}

u32
get_tile_value_unchecked(tile_chunk &tile_chunk_, u32 tile_x_, u32 tile_y_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= tile_map::s_chunk_tile_count && tile_y_ <= tile_map::s_chunk_tile_count);
    return tile_chunk_.tiles[tile_y_ * tile_map::s_chunk_tile_count + tile_x_];
}

u32
get_tile_value(tile_chunk *tile_chunk_, u32 tile_x_, u32 tile_y_)
{
    u32 tile_value {};
    if (tile_chunk_ && tile_chunk_->tiles) {
        tile_value = get_tile_value_unchecked(*tile_chunk_, tile_x_, tile_y_);
    }

    return tile_value;
}

u32
get_tile_value(tile_map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_, u32 abs_tile_z_)
{
    u32 tile_value {};

    tile_chunk_pos chunk_pos   = get_chunk_pos(abs_tile_x_, abs_tile_y_, abs_tile_z_);
    tile_chunk *cur_tile_chunk = get_tile_chunk(tile_map_, chunk_pos.chunk_tile_x,
                                                chunk_pos.chunk_tile_y, chunk_pos.chunk_tile_z);
    if (cur_tile_chunk) {
        tile_value = get_tile_value(cur_tile_chunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y);
    }

    return tile_value;
}

void
set_tile_value_unchecked(tile_chunk &tile_chunk_, u32 tile_x_, u32 tile_y_, u32 tile_value_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= tile_map::s_chunk_tile_count && tile_y_ <= tile_map::s_chunk_tile_count);
    tile_chunk_.tiles[tile_y_ * tile_map::s_chunk_tile_count + tile_x_] = tile_value_;
}

void
set_tile_value(tile_chunk *tile_chunk_, u32 abs_tile_x_, u32 abs_tile_y_, u32 tile_value_)
{
    if (tile_chunk_ && tile_chunk_->tiles) {
        set_tile_value_unchecked(*tile_chunk_, abs_tile_x_, abs_tile_y_, tile_value_);
    }
}

void
set_tile_value(mem_arena *arena_, tile_map &tile_map_, u32 abs_tile_x_, u32 abs_tile_y_,
               u32 abs_tile_z_, u32 tile_value_)
{
    tile_chunk_pos chunk_pos = get_chunk_pos(abs_tile_x_, abs_tile_y_, abs_tile_z_);
    tile_chunk *tile_chunk   = get_tile_chunk(tile_map_, chunk_pos.chunk_tile_x,
                                              chunk_pos.chunk_tile_y, chunk_pos.chunk_tile_z);

    assert(tile_chunk);
    if (!tile_chunk->tiles) {
        tile_chunk->tiles = PushArray(arena_, tile_map::s_chunk_tile_count_total, u32);
        for (u32 tile_ind {}; tile_ind < tile_map::s_chunk_tile_count_total; ++tile_ind) {
            tile_chunk->tiles[tile_ind] = 1;
        }
    }

    set_tile_value(tile_chunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y, tile_value_);
}

bool
is_world_tile_empty(tile_map &tile_map_, tile_map_pos test_pos_)
{
    u32 tile_value =
        get_tile_value(tile_map_, test_pos_.abs_tile_x, test_pos_.abs_tile_y, test_pos_.abs_tile_z);

    bool is_empty = tile_value == 1 || tile_value == 3;

    return is_empty;
}

}  // namespace tomato
