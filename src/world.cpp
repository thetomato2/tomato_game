#include "world.hpp"

#define CHUNK_UNITIALIZED INT32_MAX

namespace tom
{
static void
init_world(game_world &world, f32 tile_sizes_in_meters)
{
    world.first_free = nullptr;
    for (s32 chunk_i = 0; chunk_i < ArrayCount(world.world_chunk_hash); ++chunk_i) {
        world.world_chunk_hash[chunk_i].x = CHUNK_UNITIALIZED;  // null chunk
        world.world_chunk_hash[chunk_i].first_block.low_entity_cnt = 0;
    }
}

inline bool
is_canonical(f32 rel_coord)
{
    return rel_coord >= global::chunk_size_meters * -.5f &&
           rel_coord <= global::chunk_size_meters * .5f;
}

inline bool
is_canonical(v2 rel_coord)
{
    return is_canonical(rel_coord.x) && is_canonical(rel_coord.y);
}

static void
recanonicalize_coord(s32 &coord, f32 &rel_coord)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    s32 offset = round_f32_to_s32(rel_coord / (f32)global::chunk_size_meters);

    coord += offset;
    rel_coord -= offset * (f32)global::chunk_size_meters;

    TOM_ASSERT(is_canonical(rel_coord));
}

inline bool
is_same_chunk(const world_pos a, const world_pos b)
{
    TOM_ASSERT(is_canonical(a.offset));
    TOM_ASSERT(is_canonical(b.offset));

    return (a.chunk_x == b.chunk_x && a.chunk_y == b.chunk_y && a.chunk_z == b.chunk_z);
}

static world_pos
map_into_chunk_space(const world_pos &pos, const v2 offset)
{
    auto result = pos;

    // TODO: decide on tile chunk alignment
    result.offset += offset;
    recanonicalize_coord(result.chunk_x, result.offset.x);
    recanonicalize_coord(result.chunk_y, result.offset.y);

    return result;
}

static world_chunk *
get_world_chunk(game_world &world, const s32 chunk_x, const s32 chunk_y, const s32 chunk_z,
                memory_arena *arena = nullptr)
{
    TOM_ASSERT(chunk_x > -global::chunk_safe_margin);
    TOM_ASSERT(chunk_y > -global::chunk_safe_margin);
    TOM_ASSERT(chunk_z > -global::chunk_safe_margin);
    TOM_ASSERT(chunk_x < global::chunk_safe_margin);
    TOM_ASSERT(chunk_y < global::chunk_safe_margin);
    TOM_ASSERT(chunk_z < global::chunk_safe_margin);

    // TODO: BETTER HASH FUNCTION!
    s32 hash_val  = 19 * chunk_x + 7 * chunk_y + 3 * chunk_z;
    s32 hash_slot = hash_val & (ArrayCount(world.world_chunk_hash) - 1);
    TOM_ASSERT(hash_slot < ArrayCount(world.world_chunk_hash));

    world_chunk *chunk = world.world_chunk_hash + hash_slot;
    do {
        // found chunk
        if (chunk_x == chunk->x && chunk_y == chunk->y && chunk_z == chunk->z) {
            break;
        }
        // didn't find chunk but there isn't a next chunk
        // so allocate a new one and move the pointer there
        if (arena && chunk->x == CHUNK_UNITIALIZED && !chunk->next_in_hash) {
            chunk->next_in_hash = PushStruct(arena, world_chunk);
            chunk               = chunk->next_in_hash;
            chunk->x            = CHUNK_UNITIALIZED;
        }

        // if chunk is empty (0) allocate the tiles
        if (arena && chunk->x == CHUNK_UNITIALIZED) {
            chunk->x = chunk_x;
            chunk->y = chunk_y;
            chunk->z = chunk_z;

            // do we want to always initialize?
            chunk->next_in_hash = nullptr;
            break;
        }
        chunk = chunk->next_in_hash;
    } while (chunk);

    return chunk;
}

static world_dif
get_diff(const world_pos &pos_a, const world_pos &pos_b)
{
    world_dif result;

    v2 diff_xy;
    diff_xy.x = (f32)pos_a.chunk_x - (f32)pos_b.chunk_x;
    diff_xy.y = (f32)pos_a.chunk_y - (f32)pos_b.chunk_y;
    f32 dif_z = (f32)pos_a.chunk_z - (f32)pos_b.chunk_z;

    result.dif_xy = global::chunk_size_meters * diff_xy + (pos_a.offset - pos_b.offset);
    result.dif_z  = 0.f;

    return result;
}

static world_pos
get_centered_point(const s32 x, const s32 y, const s32 z)
{
    world_pos result {};

    result.chunk_x = x;
    result.chunk_y = y;
    result.chunk_z = z;

    return result;
}

static world_pos
abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z)
{
    world_pos result = {};

    result.chunk_x = s32(abs_x / global::chunk_size_meters);
    result.chunk_y = s32(abs_y / global::chunk_size_meters);
    result.chunk_z = s32(abs_z / global::chunk_size_meters);

    result.offset.x = abs_x - (result.chunk_x * global::chunk_size_meters);
    result.offset.y = abs_y - (result.chunk_y * global::chunk_size_meters);

    return result;
}

#if 0
static world_pos
chunk_pos_from_tile_pos(s32 abs_tile_x, s32 abs_tile_y, s32 abs_tile_z)
{
    world_pos result = {};

    // TODO: move to 3d!!!
    result.chunk_x = abs_tile_x / global::tiles_per_chunk;
    result.chunk_y = abs_tile_y / global::tiles_per_chunk;
    result.chunk_z = abs_tile_z / global::tiles_per_chunk;

    result.offset.x =
        abs_tile_x - (result.chunk_x * global::tiles_per_chunk) * global::tile_size_meters;
    result.offset.y =
        abs_tile_y - (result.chunk_y * global::tiles_per_chunk) * global::tile_size_meters;

    return result;
}
#endif

static void
change_entity_location(memory_arena *arena, game_world &world, const u32 low_i,
                       const world_pos *old_pos, world_pos *new_pos)
{
    if (old_pos && is_same_chunk(*old_pos, *new_pos)) {
        //  leave the entity where it is
        return;
    } else {
        if (old_pos) {
            // pull the entity out its old block
            world_chunk *chunk =
                get_world_chunk(world, old_pos->chunk_x, old_pos->chunk_y, old_pos->chunk_z);
            TOM_ASSERT(chunk);
            if (chunk) {
                bool found                      = false;
                world_entity_block *first_block = &chunk->first_block;
                for (world_entity_block *block = &chunk->first_block; block && !found;
                     block                     = block->next) {
                    for (u32 i = 0; i < block->low_entity_cnt; ++i) {
                        if (block->low_ent_inds[i] == low_i) {
                            TOM_ASSERT(first_block->low_entity_cnt > 0)
                            block->low_ent_inds[i] =
                                first_block->low_ent_inds[--first_block->low_entity_cnt];
                            if (first_block->low_entity_cnt == 0) {
                                if (first_block->next) {
                                    world_entity_block *next_block = first_block->next;
                                    *first_block                   = *next_block;
                                    next_block->next               = world.first_free;
                                    world.first_free               = next_block;
                                }
                            }
                            found = true;
                        }
                    }
                }
            }
        }

        //  insert the entity into its new block
        world_chunk *chunk =
            get_world_chunk(world, new_pos->chunk_x, new_pos->chunk_y, new_pos->chunk_z, arena);
        TOM_ASSERT(chunk);
        world_entity_block *block = &chunk->first_block;
        if (block->low_entity_cnt == ArrayCount(block->low_ent_inds)) {
            // out of room! make new block
            world_entity_block *old_block = world.first_free;
            if (old_block) {
                world.first_free = old_block->next;
            } else {
                old_block = PushStruct(arena, world_entity_block);
            }
            *old_block            = *block;
            block->low_entity_cnt = 0;
        }

        TOM_ASSERT(block->low_entity_cnt < ArrayCount(block->low_ent_inds));
        block->low_ent_inds[block->low_entity_cnt++] = low_i;
    }
}
}  // namespace tom
