#include "world.hpp"

#define CHUNK_UNITIALIZED INT32_MAX

namespace tom
{

static bool
is_canonical(f32 rel_coord)
{
    return rel_coord >= global::chunk_size_meters * -.5f &&
           rel_coord <= global::chunk_size_meters * .5f;
}

static bool
is_canonical(v2 rel_coord)
{
    return is_canonical(rel_coord.x) && is_canonical(rel_coord.y);
}

static void
recanonicalize_coord(s32 &coord, f32 &rel_coord)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    s32 offset = round_f32_to_s32(rel_coord / scast(f32, global::chunk_size_meters));
    coord += offset;
    rel_coord -= offset * scast(f32, global::chunk_size_meters);

    TomAssert(is_canonical(rel_coord));
}

static bool
is_same_chunk(const World_Pos a, const World_Pos b)
{
    TomAssert(is_canonical(a.offset));
    TomAssert(is_canonical(b.offset));

    return (a.chunk_x == b.chunk_x && a.chunk_y == b.chunk_y && a.chunk_z == b.chunk_z);
}

static World_Pos
get_centered_point(const s32 x, const s32 y, const s32 z)
{
    World_Pos result;

    result.chunk_x = x;
    result.chunk_y = y;
    result.chunk_z = z;

    return result;
}

void
init_world(World &world, f32 tile_sizes_in_meters)
{
    world.first_free = nullptr;
    for (s32 chunk_i = 0; chunk_i < ArrayCount(world.world_chunk_hash); ++chunk_i) {
        world.world_chunk_hash[chunk_i].x = CHUNK_UNITIALIZED;  // null chunk
        world.world_chunk_hash[chunk_i].first_block.stored_entity_cnt = 0;
    }
}

World_Dif
get_world_diff(const World_Pos &pos_a, const World_Pos &pos_b)
{
    World_Dif result;

    v2 diff_xy;
    diff_xy.x = scast(f32, pos_a.chunk_x) - scast(f32, pos_b.chunk_x);
    diff_xy.y = scast(f32, pos_a.chunk_y) - scast(f32, pos_b.chunk_y);
    f32 dif_z = scast(f32, pos_a.chunk_z) - scast(f32, pos_b.chunk_z);

    result.dif_xy = global::chunk_size_meters * diff_xy + (pos_a.offset - pos_b.offset);
    result.dif_z  = 0.f;

    return result;
}

World_Pos
map_into_chunk_space(const World_Pos &pos, const v2 offset)
{
    auto result = pos;

    // TODO: decide on tile chunk alignment
    result.offset += offset;
    recanonicalize_coord(result.chunk_x, result.offset.x);
    recanonicalize_coord(result.chunk_y, result.offset.y);

    return result;
}

World_Chunk *
get_world_chunk(World &world, const s32 chunk_x, const s32 chunk_y, const s32 chunk_z,
                Memory_Arena *arena)
{
    TomAssert(chunk_x > -global::chunk_safe_margin);
    TomAssert(chunk_y > -global::chunk_safe_margin);
    TomAssert(chunk_z > -global::chunk_safe_margin);
    TomAssert(chunk_x < global::chunk_safe_margin);
    TomAssert(chunk_y < global::chunk_safe_margin);
    TomAssert(chunk_z < global::chunk_safe_margin);

    // TODO: BETTER HASH FUNCTION!
    s32 hash_val  = 19 * chunk_x + 7 * chunk_y + 3 * chunk_z;
    s32 hash_slot = scast(s32, hash_val & (ArrayCount(world.world_chunk_hash) - 1));
    TomAssert(hash_slot < ArrayCount(world.world_chunk_hash));

    World_Chunk *chunk = world.world_chunk_hash + hash_slot;
    do {
        // found chunk
        if (chunk_x == chunk->x && chunk_y == chunk->y && chunk_z == chunk->z) {
            break;
        }
        // didn't find chunk but there isn't a next chunk
        // so allocate a new one and move the pointer there
        if (arena && chunk->x == CHUNK_UNITIALIZED && !chunk->next_in_hash) {
            chunk->next_in_hash = PushStruct(arena, World_Chunk);
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
    u32 low_ent_inds[16];
    World_Entity_Block *next;
}

World_Pos
abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z)
{
    World_Pos result;

    result.chunk_x = s32(abs_x / global::chunk_size_meters);
    result.chunk_y = s32(abs_y / global::chunk_size_meters);
    result.chunk_z = s32(abs_z / global::chunk_size_meters);

    result.offset.x = abs_x - (result.chunk_x * global::chunk_size_meters);
    result.offset.y = abs_y - (result.chunk_y * global::chunk_size_meters);

    return result;
}

void
change_entity_location(Memory_Arena *arena, World &world, const u32 low_i, const World_Pos *old_pos,
                       World_Pos *new_pos)
{
    if (old_pos && is_same_chunk(*old_pos, *new_pos)) {
        //  leave the entity where it is
        return;
    } else {
        if (old_pos) {
            // pull the entity out its old block
            World_Chunk *chunk =
                get_world_chunk(world, old_pos->chunk_x, old_pos->chunk_y, old_pos->chunk_z);
            TomAssert(chunk);
            if (chunk) {
                bool found                      = false;
                World_Entity_Block *first_block = &chunk->first_block;
                for (World_Entity_Block *block = &chunk->first_block; block && !found;
                     block                     = block->next) {
                    for (u32 i {}; i < block->stored_entity_cnt; ++i) {
                        if (block->stored_ents_inds[i] == low_i) {
                            TomAssert(first_block->stored_entity_cnt > 0);
                            block->stored_ents_inds[i] =
                                first_block->stored_ents_inds[--first_block->stored_entity_cnt];
                            if (first_block->stored_entity_cnt == 0) {
                                if (first_block->next) {
                                    World_Entity_Block *next_block = first_block->next;
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
        World_Chunk *chunk =
            get_world_chunk(world, new_pos->chunk_x, new_pos->chunk_y, new_pos->chunk_z, arena);
        TomAssert(chunk);
        World_Entity_Block *block = &chunk->first_block;
        if (block->stored_entity_cnt == ArrayCount(block->stored_ents_inds)) {
            // out of room! make new block
            World_Entity_Block *old_block = world.first_free;
            if (old_block) {
                world.first_free = old_block->next;
            } else {
                old_block = PushStruct(arena, World_Entity_Block);
            }
            *old_block               = *block;
            block->next              = old_block;
            block->stored_entity_cnt = 0;
        }

        TomAssert(block->stored_entity_cnt < ArrayCount(block->stored_ents_inds));
        block->stored_ents_inds[block->stored_entity_cnt++] = low_i;
    }
}
}  // namespace tom
