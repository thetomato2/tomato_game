#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{

internal sim_entity *
add_sim_entity_to_region(game_state *state, sim_region *region, u32 ent_i, entity *source_ent,
                         v2 *sim_pos);

internal void
add_hit_points(sim_entity *ent, s32 hp)
{
    ent->hp += hp;
    if (ent->hp > ent->max_hp) ent->hp = ent->max_hp;
}

internal void
subtract_hit_points(sim_entity *ent, s32 hp)
{
    ent->hp -= hp;
    if (ent->hp < 0) {
        ent->hp = 0;
        clear_flag(ent->flags, sim_entity_flags::active);
    }
}

internal void
handle_collision(sim_entity *ent_a, sim_entity *ent_b)
{
    if (ent_a->type == entity_type::monster && ent_b->type == entity_type::sword) {
        subtract_hit_points(ent_a, 1);
    }
}

void
move_entity(game_state *state, sim_region *region, sim_entity *ent, v2 ent_delta, const f32 dt)
{
    TOM_ASSERT(state && region && ent);

    f32 dist_remain = ent->dist_limit;
    if (dist_remain == 0.0f) {
        dist_remain = 1000.0f;
    }

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 t_min         = 1.0f;
        f32 ent_delta_len = vec::length(ent_delta);
        // REVIEW: use epsilon?
        if (!(ent_delta_len > 0.0f)) break;  // if the ent delta is 0 that means no movmement
        if (ent_delta_len > dist_remain) t_min = dist_remain / ent_delta_len;
        v2 wall_nrm         = {};
        v2 desired_position = ent->pos + ent_delta;
        sim_entity *hit_ent = nullptr;

        if (is_flag_set(ent->flags, sim_entity_flags::collides)) {
            // FIXME: this is N * N bad
            for (sim_entity *test_ent = region->sim_entities;
                 test_ent != region->sim_entities + region->sim_entity_cnt; ++test_ent) {
                if (test_ent->ent_i == ent->ent_i) continue;  // don't test against self

                TOM_ASSERT(test_ent);  // ent_a nullptr probably means something broke
                if (!is_flag_set(test_ent->flags, sim_entity_flags::active) ||
                    !is_flag_set(test_ent->flags, sim_entity_flags::barrier))
                    continue;  // skip inactive and non-barreirr entities
                if (ent->weapon_i == test_ent->ent_i || ent->parent_i == test_ent->ent_i)
                    continue;  // skip parent and weapon

                // NOTE: Minkowski sum
                f32 r_w = ent->width + test_ent->width;
                f32 r_h = ent->height + test_ent->height;

                v2 min_corner = { -.5f * v2 { r_w, r_h } };
                v2 max_corner = { .5f * v2 { r_w, r_h } };
                v2 rel        = ent->pos - test_ent->pos;

                // TODO: maybe pull this out into a free function (but why?)
                auto test_wall = [&t_min](f32 wall_x, f32 rel_x, f32 rel_y, f32 player_delta_x,
                                          f32 player_delta_y, f32 min_y, f32 max_y) -> bool {
                    bool hit = false;

                    f32 t_esp = .001f;
                    if (player_delta_x != 0.f) {
                        f32 t_res = (wall_x - rel_x) / player_delta_x;
                        f32 y     = rel_y + t_res * player_delta_y;

                        if (t_res >= 0.f && (t_min > t_res)) {
                            if (y >= min_y && y <= max_y) {
                                t_min = math::max(0.f, t_res - t_esp);
                                hit   = true;
                            }
                        }
                    }

                    return hit;
                };

                if (test_wall(min_corner.x, rel.x, rel.y, ent_delta.x, ent_delta.y, min_corner.y,
                              max_corner.y)) {
                    wall_nrm = v2 { -1.f, 0.f };
                    hit_ent  = test_ent;
                }
                if (test_wall(max_corner.x, rel.x, rel.y, ent_delta.x, ent_delta.y, min_corner.y,
                              max_corner.y)) {
                    wall_nrm = v2 { 1.f, 0.f };
                    hit_ent  = test_ent;
                }
                if (test_wall(min_corner.y, rel.y, rel.x, ent_delta.y, ent_delta.x, min_corner.x,
                              max_corner.x)) {
                    wall_nrm = v2 { 0.f, -1.f };
                    hit_ent  = test_ent;
                }
                if (test_wall(max_corner.y, rel.y, rel.x, ent_delta.y, ent_delta.x, min_corner.x,
                              max_corner.x)) {
                    wall_nrm = v2 { 0.f, 1.f };
                    hit_ent  = test_ent;
                }
            }
        }

        if (hit_ent && !is_flag_set(hit_ent->flags, sim_entity_flags::barrier)) {
            wall_nrm = v2 { 0.f, 0.f };
        }

        ent->pos += t_min * ent_delta;
        dist_remain -= t_min * ent_delta_len;

        if (hit_ent) {
            if (ent->dist_limit != 0.0f) {
                ent->dist_limit = 0.0f;
            }
            ent->vel -= 1.f * vec::inner(ent->vel, wall_nrm) * wall_nrm;
            ent_delta -= 1.f * vec::inner(ent_delta, wall_nrm) * wall_nrm;

            sim_entity *ent_a = ent;
            sim_entity *ent_b = hit_ent;
            if (ent_a->type > ent_b->type) {
                // TODO: swap func/template/macro?
                auto temp = ent_a;
                ent_a     = ent_b;
                ent_b     = temp;
            }
            handle_collision(ent_a, ent_b);
        }
    }
    if (ent->dist_limit != 0.0f) {
        ent->dist_limit = dist_remain;
    }
}

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
get_cam_space_pos(const game_state &state, entity *ent)
{
    world_dif diff = get_world_diff(ent->world_pos, state.camera.pos);
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

v2
get_sim_space_pos(const game_state &state, const sim_region &region, u32 ent_i)
{
    // TODO: what is the null/invalid value here?
    v2 result         = { 100'000'000.0f, 100'000'000.0f };
    const entity &ent = state.entities[ent_i];
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

        // TODO: save state back to stored sim_entity, once high entities do state decompression
        world_pos new_pos = !is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)
                                ? map_into_chunk_space(region->origin, sim_ent->pos)
                                : null_world_pos();
        if (ent->world_pos != new_pos)
            change_entity_location(&state->world_arena, state->world, ent, new_pos);
        TOM_ASSERT(is_flag_set(ent->sim.flags, sim_entity_flags::simming));
        clear_flag(ent->sim.flags, sim_entity_flags::simming);
    }
}

}  // namespace tom
