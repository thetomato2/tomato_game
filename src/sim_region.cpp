#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{

internal sim_entity *
add_sim_entity_to_region(game_state *state, sim_region *region, u32 ent_i, entity *source_ent,
                         v2 *sim_pos);

internal sim_entity_hash *
get_hash_from_ind(sim_region *region, u32 stored_i)
{
    TOM_ASSERT(stored_i);
    sim_entity_hash *result = nullptr;
    u32 hash_val            = stored_i;
    for (u32 offset = 0; offset < ARRAY_COUNT(region->hash); ++offset) {
        sim_entity_hash *entry =
            region->hash + ((hash_val + offset) & (ARRAY_COUNT(region->hash) - 1));
        if (entry->ind == 0 || entry->ind == stored_i) {
            result = entry;
            break;
        }
    }

    return result;
}

internal void
map_storage_ind_to_ent(sim_region *sim_region, u32 stored_i, sim_entity &entity)
{
    sim_entity_hash *entry = get_hash_from_ind(sim_region, stored_i);
    TOM_ASSERT(entry->ind == 0 || entry->ind == stored_i);
    entry->ind = stored_i;
    entry->ptr = &entity;
}

internal sim_entity *
get_entity_from_ind(sim_region *region, u32 stored_i)
{
    sim_entity_hash *entry = get_hash_from_ind(region, stored_i);
    return entry->ptr;
}

internal void
store_entity_ref(entity_ref *ref)
{
    if (ref->ptr != 0) {
        ref->ind = ref->ptr->ent_i;
    }
}

internal void
load_entity_ref(game_state *state, sim_region *region, entity_ref *ref)
{
    TOM_ASSERT(state && region && ref);

    if (ref->ind) {
        sim_entity_hash *entry = get_hash_from_ind(region, ref->ind);
        if (entry->ptr == nullptr) {
            entry->ind = ref->ind;
            entry->ptr = add_sim_entity_to_region(state, region, ref->ind,
                                                  get_entity(state, ref->ind), nullptr);
        }
        ref->ptr = entry->ptr;
    }
}

internal v2
get_cam_space_pos(const game_state &state, entity *stored_ent)
{
    world_dif diff = get_world_diff(stored_ent->world_pos, state.camera.pos);
    v2 result      = { diff.dif_xy };

    return result;
}

internal v2
get_sim_space_pos(const sim_region &region, const entity &ent)
{
    // TODO: what is the null/invalid value here?
    v2 result = { 100'000'000.0f, 100'000'000.0f };
    if (!is_flag_set(ent.sim.flags, sim_entity_flags::nonspatial)) {
        world_dif dif = get_world_diff(ent.world_pos, region.origin);
        result        = dif.dif_xy;
    }

    return result;
}

internal sim_entity *
add_sim_entity_to_region_raw(game_state *state, sim_region *region, u32 ent_i, entity *source_ent)
{
    // because why use a const reference?
    TOM_ASSERT(state && region && ent_i);
    sim_entity *entity = nullptr;

    if (region->sim_entity_cnt < region->max_sim_entity_cnt) {
        // TODO: should be a decrompression step, not a copy!
        entity = region->sim_entities + region->sim_entity_cnt++;
        map_storage_ind_to_ent(region, ent_i, *entity);
        if (source_ent) {
            *entity = source_ent->sim;
            TOM_ASSERT(!is_flag_set(entity->flags, sim_entity_flags::simming));
            set_flag(entity->flags, sim_entity_flags::simming);
            // load_entity_ref(state, region, entity->weapon_i);
        }
        entity->ent_i      = ent_i;
        entity->updateable = false;
    } else {
        INVALID_CODE_PATH;
    }

    return entity;
}

internal sim_entity *
add_sim_entity_to_region(game_state *state, sim_region *region, u32 ent_i, entity *source_ent,
                         v2 *sim_pos)

{
    TOM_ASSERT(state && region && ent_i);

    sim_entity *dest_ent = add_sim_entity_to_region_raw(state, region, ent_i, source_ent);
    if (dest_ent) {
        if (sim_pos) {
            dest_ent->pos        = *sim_pos;
            dest_ent->updateable = rec::is_inside(region->update_bounds, dest_ent->pos);
        } else {
            dest_ent->pos = get_sim_space_pos(*region, *source_ent);
        }
    }

    return dest_ent;
}

sim_region *
begin_sim(memory_arena *arena, game_state *state, world_pos origin, rect bounds)
{
    TOM_ASSERT(arena && state);

    // NOTE: clear the hash table!
    sim_region *region = PUSH_STRUCT(arena, sim_region);
    ZERO_STRUCT(region->hash);

    // TODO: IMPORTANT-> calc this from the max value of all entities radius + speed
    f32 update_safety_margin = 1.0f;

    region->origin        = origin;
    region->update_bounds = bounds;
    region->bounds        = rec::add_radius(bounds, update_safety_margin);

    region->max_sim_entity_cnt = 4096;  // TODO: how many max entities?
    region->sim_entity_cnt     = 0;
    region->sim_entities       = PUSH_ARRAY(arena, region->max_sim_entity_cnt, sim_entity);

    world_pos min_chunk_pos = map_into_chunk_space(origin, rec::min_corner(bounds));
    world_pos max_chunk_pos = map_into_chunk_space(origin, rec::max_corner(bounds));

    // make all entities outside camera space stored
    for (s32 chunk_y = min_chunk_pos.chunk_y; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x = min_chunk_pos.chunk_x; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            world_chunk *chunk = get_world_chunk(state->world, chunk_x, chunk_y, origin.chunk_z,
                                                 &state->world_arena);
            if (chunk) {
                for (world_entity_block *block = &chunk->first_block; block; block = block->next) {
                    for (u32 ent_i = 0; ent_i < block->ent_cnt; ++ent_i) {
                        u32 block_ent_i = block->ent_inds[ent_i];
                        entity *ent     = state->entities + block_ent_i;
                        if (!is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)) {
                            v2 sim_space_pos = get_sim_space_pos(*region, *ent);
                            if (rec::is_inside(region->bounds, sim_space_pos)) {
                                add_sim_entity_to_region(state, region, block_ent_i, ent,
                                                         &sim_space_pos);
                                // local_persist u32 ent_cnt = 0;
                                // printf("added ent %d - %d : %d\n", block_ent_i, ent_cnt++,
                                //        region->sim_entity_cnt);
                            }
                        }
                    }
                }
            }
        }
    }

    return region;
}

void
end_sim(game_state *state, sim_region *region)
{
    // TODO: low entities stored in the world and not games state?
    for (sim_entity *sim_ent = region->sim_entities;
         sim_ent != region->sim_entities + region->sim_entity_cnt; ++sim_ent) {
        entity *ent = state->entities + sim_ent->ent_i;
        ent->sim    = *sim_ent;
        // if (ent->type == entity_type::player && state->debug_flag) __debugbreak();

        // store_entity_ref(&ent->sim.weapon_i);

#if 0
        // REVIEW: do I need this since I am using pointers everywhere?
        // TODO: save state back to stored sim_entity, once high entities do state decompression
        world_pos new_pos = !is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)
                                ? map_into_chunk_space(region->origin, sim_ent->pos)
                                : null_world_pos();
        // TODO: this is unneeded? I already do this in the update
        if (ent->world_pos != new_pos)
            change_entity_location(&state->world_arena, state->world, ent, new_pos);
#endif
        TOM_ASSERT(is_flag_set(ent->sim.flags, sim_entity_flags::simming));
        clear_flag(ent->sim.flags, sim_entity_flags::simming);
    }
}

}  // namespace tom
