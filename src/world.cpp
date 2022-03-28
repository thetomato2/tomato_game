#include "world.hpp"
#include "entity.hpp"

namespace tom
{

internal bool
is_canonical(f32 rel_coord)
{
    return rel_coord >= global::chunk_size_meters * -0.5f - global::epsilon &&
           rel_coord <= global::chunk_size_meters * 0.5f + global::epsilon;
}

internal bool
is_canonical(v3 rel_coord)
{
    return is_canonical(rel_coord.x) && is_canonical(rel_coord.y) && is_canonical(rel_coord.z);
}

internal void
recanonicalize_coord(s32 &coord, f32 &rel_coord)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    s32 offset = math::round_f32_to_s32(rel_coord / scast(f32, global::chunk_size_meters));
    coord += offset;
    rel_coord -= offset * scast(f32, global::chunk_size_meters);

    TOM_ASSERT(is_canonical(rel_coord));
}

internal bool
is_same_chunk(const world_pos a, const world_pos b)
{
    TOM_ASSERT(is_canonical(a.offset));
    TOM_ASSERT(is_canonical(b.offset));

    return (a.chunk_x == b.chunk_x && a.chunk_y == b.chunk_y && a.chunk_z == b.chunk_z);
}

internal world_pos
get_centered_point(const s32 x, const s32 y, const s32 z)
{
    world_pos result;

    result.chunk_x = x;
    result.chunk_y = y;
    result.chunk_z = z;

    return result;
}

void
init_world(world *world, f32 tile_sizes_in_meters)
{
    world->first_free = nullptr;
    for (s32 chunk_i = 0; chunk_i < ARRAY_COUNT(world->world_chunk_hash); ++chunk_i) {
        world->world_chunk_hash[chunk_i].x                   = CHUNK_UNITIALIZED;  // null chunk
        world->world_chunk_hash[chunk_i].first_block.ent_cnt = 0;
    }
}

v3
get_world_diff(world_pos pos_a, world_pos pos_b)
{
    v3 dif = { .x = scast(f32, pos_a.chunk_x) - scast(f32, pos_b.chunk_x),
               .y = dif.y = scast(f32, pos_a.chunk_y) - scast(f32, pos_b.chunk_y),
               .z = dif.z = scast(f32, pos_a.chunk_z) - scast(f32, pos_b.chunk_z) };

    v3 result = vec::hadamard(global::chunk_dim_meters, dif) + (pos_a.offset - pos_b.offset);

    return result;
}

world_pos
map_into_chunk_space(world_pos pos, const v3 offset)
{
    auto result = pos;

    // TODO: decide on tile chunk alignment
    result.offset += offset;
    recanonicalize_coord(result.chunk_x, result.offset.x);
    recanonicalize_coord(result.chunk_y, result.offset.y);
    recanonicalize_coord(result.chunk_z, result.offset.z);

    return result;
}

world_pos
map_into_chunk_space(world_pos pos, const v2 offset)
{
    v3 offset_v3 = v3_init(offset);
    auto result  = map_into_chunk_space(pos, offset_v3);

    return result;
}

world_chunk *
get_world_chunk(world *world, const s32 chunk_x, const s32 chunk_y, const s32 chunk_z,
                memory_arena *arena)
{
    TOM_ASSERT(chunk_x > -global::chunk_safe_margin);
    TOM_ASSERT(chunk_y > -global::chunk_safe_margin);
    TOM_ASSERT(chunk_z > -global::chunk_safe_margin);
    TOM_ASSERT(chunk_x < global::chunk_safe_margin);
    TOM_ASSERT(chunk_y < global::chunk_safe_margin);
    TOM_ASSERT(chunk_z < global::chunk_safe_margin);

    // TODO: better hash function!
    s32 hash_val  = 19 * chunk_x + 7 * chunk_y + 3 * chunk_z;
    s32 hash_slot = scast(s32, hash_val & (ARRAY_COUNT(world->world_chunk_hash) - 1));
    TOM_ASSERT(hash_slot < ARRAY_COUNT(world->world_chunk_hash));

    world_chunk *chunk = world->world_chunk_hash + hash_slot;
    do {
        // found chunk
        if (chunk_x == chunk->x && chunk_y == chunk->y && chunk_z == chunk->z) {
            break;
        }
        // didn't find chunk but there isn't a next chunk
        // so allocate a new one and move the pointer there
        if (arena && chunk->x == CHUNK_UNITIALIZED && !chunk->next_in_hash) {
            chunk->next_in_hash = PUSH_STRUCT(arena, world_chunk);
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

world_pos
abs_pos_to_world_pos(f32 abs_x, f32 abs_y, f32 abs_z)
{
    world_pos result;

    result.chunk_x = scast(s32, abs_x / global::chunk_dim_meters.x);
    result.chunk_y = scast(s32, abs_y / global::chunk_dim_meters.y);
    result.chunk_z = scast(s32, abs_z / global::chunk_dim_meters.z);

    result.offset.x = abs_x - (result.chunk_x * global::chunk_dim_meters.x);
    result.offset.y = abs_y - (result.chunk_y * global::chunk_dim_meters.y);
    result.offset.z = abs_z - (result.chunk_z * global::chunk_dim_meters.z);

    // TODO: use map_into_chunk_space?
    recanonicalize_coord(result.chunk_x, result.offset.x);
    recanonicalize_coord(result.chunk_y, result.offset.y);
    recanonicalize_coord(result.chunk_z, result.offset.z);

    return result;
}

internal void
change_entity_location_raw(memory_arena *arena, world *world, u32 ent_i, world_pos *old_pos,
                           world_pos *new_pos)
{
    if (is_valid(new_pos)) {
        if (is_valid(old_pos) && is_same_chunk(*old_pos, *new_pos)) {
            //  leave the entity where it is
            return;
        } else {
            if (is_valid(old_pos)) {
                // pull the entity out its old block
                world_chunk *chunk =
                    get_world_chunk(world, old_pos->chunk_x, old_pos->chunk_y, old_pos->chunk_z);
                TOM_ASSERT(chunk);
                if (chunk) {
                    bool found                      = false;
                    world_entity_block *first_block = &chunk->first_block;
                    for (world_entity_block *block = &chunk->first_block; block && !found;
                         block                     = block->next) {
                        for (u32 i {}; i < block->ent_cnt; ++i) {
                            if (block->ent_inds[i] == ent_i) {
                                TOM_ASSERT(first_block->ent_cnt > 0);
                                block->ent_inds[i] = first_block->ent_inds[--first_block->ent_cnt];
                                if (first_block->ent_cnt == 0) {
                                    if (first_block->next) {
                                        world_entity_block *next_block = first_block->next;
                                        *first_block                   = *next_block;
                                        next_block->next               = world->first_free;
                                        world->first_free              = next_block;
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
            if (block->ent_cnt == ARRAY_COUNT(block->ent_inds)) {
                // out of room! make new block
                // REVIEW: this seems kinda backwards to me, it's like a backwards pointing list
                world_entity_block *old_block = world->first_free;
                if (old_block) {
                    world->first_free = old_block->next;
                } else {
                    old_block = PUSH_STRUCT(arena, world_entity_block);
                }
                *old_block     = *block;
                block->next    = old_block;
                block->ent_cnt = 0;
            }

            TOM_ASSERT(block->ent_cnt < ARRAY_COUNT(block->ent_inds));
            block->ent_inds[block->ent_cnt++] = ent_i;
        }
    }
}

void
change_entity_location(memory_arena *arena, world *world, entity *ent, world_pos new_pos_init)
{
    world_pos *old_pos = nullptr, *new_pos = nullptr;
    if (!is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)) old_pos = &ent->world_pos;
    if (is_valid(new_pos_init)) new_pos = &new_pos_init;

    change_entity_location_raw(arena, world, ent->sim.ent_i, old_pos, new_pos);
    if (new_pos) {
        ent->world_pos = *new_pos;
        clear_flag(ent->sim.flags, sim_entity_flags::nonspatial);
    } else {
        ent->world_pos = null_world_pos();
        set_flag(ent->sim.flags, sim_entity_flags::nonspatial);
    }
}

}  // namespace tom
