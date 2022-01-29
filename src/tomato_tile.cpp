#include "tomato_tile.hpp"

void
recanonicalize_coord(u32 &tile_, f32 &tile_rel_)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    s32 offset = round_f32_to_s32(tile_rel_ / (f32)Tile_Map::s_tile_size_meters);

    tile_ += offset;
    tile_rel_ -= offset * (f32)Tile_Map::s_tile_size_meters;

    assert(tile_rel_ >= -.5f * Tile_Map::s_tile_size_meters);
    assert(tile_rel_ <= .5f * Tile_Map::s_tile_size_meters);
}

Tile_Map_Pos
recanonicalize_pos(const Tile_Map_Pos &pos_)
{
    auto new_pos = pos_;

    recanonicalize_coord(new_pos.abs_tile_x, new_pos.offset.x);
    recanonicalize_coord(new_pos.abs_tile_y, new_pos.offset.y);

    return new_pos;
}

Tile_Chunk_Pos
get_chunk_pos(const u32 abs_tile_x_, const u32 abs_tile_y_, const u32 abs_tile_z_)
{
    Tile_Chunk_Pos chunk_pos;

    chunk_pos.chunk_tile_x = abs_tile_x_ >> Tile_Map::s_chunk_bit_shift;
    chunk_pos.chunk_tile_y = abs_tile_y_ >> Tile_Map::s_chunk_bit_shift;
    chunk_pos.chunk_tile_z = abs_tile_z_;
    chunk_pos.rel_tile_x   = abs_tile_x_ & Tile_Map::s_chunk_bit_mask;
    chunk_pos.rel_tile_y   = abs_tile_y_ & Tile_Map::s_chunk_bit_mask;

    return chunk_pos;
}

Tile_Chunk_Pos
get_chunk_pos(const Tile_Map_Pos &pos_)
{
    Tile_Chunk_Pos chunk_pos;

    chunk_pos = get_chunk_pos(pos_.abs_tile_x, pos_.abs_tile_y, pos_.abs_tile_z);

    return chunk_pos;
}

Tile_Chunk *
get_tile_chunk(const Tile_Map &tile_map_, const u32 tile_chunk_x_, const u32 tile_chunk_y_,
               const u32 tile_chunk_z_)
{
    Tile_Chunk *TileChunk = nullptr;

    if (tile_chunk_x_ >= 0 && tile_chunk_x_ < Tile_Map::s_chunk_count && tile_chunk_y_ >= 0 &&
        tile_chunk_y_ < Tile_Map::s_chunk_count && tile_chunk_z_ >= 0 &&
        tile_chunk_z_ < Tile_Map::s_chunk_count_z) {
        TileChunk = &tile_map_.tile_chunks[tile_chunk_z_ * Tile_Map::s_chunk_count *
                                               Tile_Map::s_chunk_tile_count +
                                           tile_chunk_y_ * Tile_Map::s_chunk_count + tile_chunk_x_];
    }
    return TileChunk;
}

u32
get_tile_value_unchecked(const Tile_Chunk &tile_chunk_, const u32 tile_x_, const u32 tile_y_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= Tile_Map::s_chunk_tile_count && tile_y_ <= Tile_Map::s_chunk_tile_count);
    return tile_chunk_.tiles[tile_y_ * Tile_Map::s_chunk_tile_count + tile_x_];
}

u32
get_tile_value(Tile_Chunk *const tile_chunk_, const u32 tile_x_, const u32 tile_y_)
{
    u32 tile_value {};
    if (tile_chunk_ && tile_chunk_->tiles) {
        tile_value = get_tile_value_unchecked(*tile_chunk_, tile_x_, tile_y_);
    }

    return tile_value;
}

u32
get_tile_value(const Tile_Map &tile_map_, const u32 abs_tile_x_, const u32 abs_tile_y_,
               const u32 abs_tile_z_)
{
    u32 tile_value {};

    Tile_Chunk_Pos chunk_pos   = get_chunk_pos(abs_tile_x_, abs_tile_y_, abs_tile_z_);
    Tile_Chunk *cur_tile_chunk = get_tile_chunk(tile_map_, chunk_pos.chunk_tile_x,
                                                chunk_pos.chunk_tile_y, chunk_pos.chunk_tile_z);
    if (cur_tile_chunk) {
        tile_value = get_tile_value(cur_tile_chunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y);
    }

    return tile_value;
}

u32
get_tile_value(const Tile_Map &tile_map_, const Tile_Map_Pos &pos_)
{
    u32 tile_value { get_tile_value(tile_map_, pos_.abs_tile_x, pos_.abs_tile_y, pos_.abs_tile_z) };
    return tile_value;
}

void
set_tile_value_unchecked(Tile_Chunk &tile_chunk_, const u32 tile_x_, const u32 tile_y_,
                         const u32 tile_value_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= Tile_Map::s_chunk_tile_count && tile_y_ <= Tile_Map::s_chunk_tile_count);
    tile_chunk_.tiles[tile_y_ * Tile_Map::s_chunk_tile_count + tile_x_] = tile_value_;
}

void
set_tile_value(Tile_Chunk *tile_chunk_, const u32 abs_tile_x_, const u32 abs_tile_y_,
               const u32 tile_value_)
{
    if (tile_chunk_ && tile_chunk_->tiles) {
        set_tile_value_unchecked(*tile_chunk_, abs_tile_x_, abs_tile_y_, tile_value_);
    }
}

void
set_tile_value(Mem_Arena *arena_, Tile_Map &tile_map_, const u32 abs_tile_x_, const u32 abs_tile_y_,
               const u32 abs_tile_z_, const u32 tile_value_)
{
    Tile_Chunk_Pos chunk_pos = get_chunk_pos(abs_tile_x_, abs_tile_y_, abs_tile_z_);
    Tile_Chunk *TileChunk    = get_tile_chunk(tile_map_, chunk_pos.chunk_tile_x,
                                              chunk_pos.chunk_tile_y, chunk_pos.chunk_tile_z);

    assert(TileChunk);
    if (!TileChunk->tiles) {
        TileChunk->tiles = PushArray(arena_, Tile_Map::s_chunk_tile_count_total, u32);
        for (u32 tile_ind {}; tile_ind < Tile_Map::s_chunk_tile_count_total; ++tile_ind) {
            TileChunk->tiles[tile_ind] = 1;
        }
    }

    set_tile_value(TileChunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y, tile_value_);
}

bool
is_tile_value_empty(const u32 tile_value_)
{
    bool is_empty = tile_value_ == 1 || tile_value_ == 3;
    return is_empty;
}

bool
is_world_tile_empty(const Tile_Map &tile_map_, const Tile_Map_Pos &test_pos_)
{
    u32 tile_value =
        get_tile_value(tile_map_, test_pos_.abs_tile_x, test_pos_.abs_tile_y, test_pos_.abs_tile_z);

    bool is_empty = is_tile_value_empty(tile_value);

    return is_empty;
}

Tile_Map_Dif
get_tile_diff(const Tile_Map_Pos &pos_a_, const Tile_Map_Pos &pos_b_)
{
    Tile_Map_Dif result;

    v2 diff_tile_xy;
    diff_tile_xy.x = (f32)pos_a_.abs_tile_x - (f32)pos_b_.abs_tile_x;
    diff_tile_xy.y = (f32)pos_a_.abs_tile_y - (f32)pos_b_.abs_tile_y;
    f32 dif_tile_z = (f32)pos_a_.abs_tile_z - (f32)pos_b_.abs_tile_z;

    result.dif_xy = Tile_Map::s_tile_size_meters * diff_tile_xy + (pos_a_.offset - pos_b_.offset);
    result.dif_z  = 0.f;

    return result;
}

Tile_Map_Pos
get_centered_tile_point(const u32 abs_tile_x_, const u32 abs_tile_y_, const u32 abs_tile_z_)
{
    Tile_Map_Pos result {};

    result.abs_tile_x = abs_tile_x_;
    result.abs_tile_y = abs_tile_y_;
    result.abs_tile_z = abs_tile_z_;

    return result;
}

Tile_Map_Pos
offset_pos(const Tile_Map_Pos &pos_, const v2 offset_)
{
    auto result = pos_;

    result.offset += offset_;
    result = recanonicalize_pos(result);

    return result;
}
