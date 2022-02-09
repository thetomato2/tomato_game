#include "tomato_world.hpp"

#define CHUNK_UNITIALIZED INT32_MAX

static void
recanonicalize_coord(i32 &tile_, f32 &tile_rel_)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    i32 offset { round_f32_to_i32(tile_rel_ / (f32)World::s_tile_size_meters) };

    tile_ += offset;
    tile_rel_ -= offset * (f32)World::s_tile_size_meters;

    TOM_ASSERT(tile_rel_ >= -.5f * World::s_tile_size_meters);
    TOM_ASSERT(tile_rel_ <= .5f * World::s_tile_size_meters);
}

static World_Pos
map_into_tile_space(const World_Pos &pos_, v2 offset_)
{
    auto result { pos_ };

    result.offset += offset_;
    recanonicalize_coord(result.x, result.offset.x);
    recanonicalize_coord(result.y, result.offset.y);

    return result;
}

#if 0
static Tile_Chunk_Pos
get_chunk_pos(const i32 abs_tile_x_, const i32 abs_tile_y_, const i32 abs_tile_z_)
{
    Tile_Chunk_Pos chunk_pos;

    chunk_pos.x     = abs_tile_x_ >> World::s_chunk_bit_shift;
    chunk_pos.y     = abs_tile_y_ >> World::s_chunk_bit_shift;
    chunk_pos.z     = abs_tile_z_;
    chunk_pos.rel_x = abs_tile_x_ & World::s_chunk_bit_mask;
    chunk_pos.rel_y = abs_tile_y_ & World::s_chunk_bit_mask;

    return chunk_pos;
}

static Tile_Chunk_Pos
get_chunk_pos(const World_Pos &pos_)
{
    Tile_Chunk_Pos chunk_pos;

    chunk_pos = get_chunk_pos(pos_.abs_tile_x, pos_.abs_tile_y, pos_.abs_tile_z);

    return chunk_pos;
}
#endif

static World_Chunk *
get_chunk(World &world_, const i32 chunk_x_, const i32 chunk_y_, const i32 chunk_z_,
          Mem_Arena *arena_ = nullptr)
{
    TOM_ASSERT(chunk_x_ > -World::s_chunk_safe_margin);
    TOM_ASSERT(chunk_y_ > -World::s_chunk_safe_margin);
    TOM_ASSERT(chunk_z_ > -World::s_chunk_safe_margin);
    TOM_ASSERT(chunk_x_ < World::s_chunk_safe_margin);
    TOM_ASSERT(chunk_y_ < World::s_chunk_safe_margin);
    TOM_ASSERT(chunk_z_ < World::s_chunk_safe_margin);

    // TODO: BETTER HASH FUNCTION!
    i32 hash_val  = 19 * chunk_x_ + 7 * chunk_y_ + 3 * chunk_z_;
    i32 hash_slot = hash_val & (ArrayCount(world_.world_chunk_hash) - 1);
    TOM_ASSERT(hash_slot < ArrayCount(world_.world_chunk_hash));

    World_Chunk *chunk = world_.world_chunk_hash + hash_slot;
    do {
        // found chunk
        if (chunk_x_ == chunk->x && chunk_y_ == chunk->y && chunk_z_ == chunk->z) {
            break;
        }
        // didn't find chunk but there isn't a next chunk
        // so allocate a new one and move the pointer there
        if (arena_ && chunk->x == CHUNK_UNITIALIZED && !chunk->next_in_hash) {
            chunk->next_in_hash = PushStruct(arena_, World_Chunk);
            chunk               = chunk->next_in_hash;
            chunk->x            = CHUNK_UNITIALIZED;
        }

        // if chunk is empty (0) allocate the tiles
        if (arena_ && chunk->x == CHUNK_UNITIALIZED) {
            chunk->x = chunk_x_;
            chunk->y = chunk_y_;
            chunk->z = chunk_z_;

            // do we want to always initialize?
            chunk->next_in_hash = nullptr;
            break;
        }
        chunk = chunk->next_in_hash;
    } while (chunk);

    return chunk;
}

static World_Dif
get_diff(const World_Pos &pos_a_, const World_Pos &pos_b_)
{
    World_Dif result;

    v2 diff_xy;
    diff_xy.x = (f32)pos_a_.x - (f32)pos_b_.x;
    diff_xy.y = (f32)pos_a_.y - (f32)pos_b_.y;
    f32 dif_z = (f32)pos_a_.z - (f32)pos_b_.z;

    result.dif_xy = World::s_tile_size_meters * diff_xy + (pos_a_.offset - pos_b_.offset);
    result.dif_z  = 0.f;

    return result;
}

static World_Pos
get_centered_point(const i32 x_, const i32 y_, const i32 z_)
{
    World_Pos result {};

    result.x = x_;
    result.y = y_;
    result.z = z_;

    return result;
}

static void
init_world(World &world_)
{
    for (i32 chunk_i = 0; chunk_i < ArrayCount(world_.world_chunk_hash); ++chunk_i) {
        world_.world_chunk_hash[chunk_i].x = CHUNK_UNITIALIZED;  // null chunk
    }
}
