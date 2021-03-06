#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{

internal sim_entity *
add_sim_entity_to_region(game_state *state, sim_region *region, u32 ent_i, entity *source_ent,
                         v3 *sim_pos);

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
        clear_flag(ent, sim_entity_flags::active);
        set_flag(ent, sim_entity_flags::nonspatial);
    }
}

internal bool
should_collide(game_state *state, sim_entity *ent_a, sim_entity *ent_b)
{
    TOM_ASSERT(ent_a && ent_b);

    bool result = false;

    if (ent_a->type > ent_b->type) {
        // TODO: swap func/template/macro?
        auto temp = ent_a;
        ent_a     = ent_b;
        ent_b     = temp;
    }

    // if and entity "collides" barriers will stop them
    if ((is_flag_set(ent_a, sim_entity_flags::barrier) &&
         is_flag_set(ent_b, sim_entity_flags::collides)) ||
        (is_flag_set(ent_b, sim_entity_flags::barrier) &&
         is_flag_set(ent_a, sim_entity_flags::collides)))
        result = true;

    // DEBUG_BREAK(ent_a->type == entity_type::monster && ent_b->type == entity_type::sword);

    // TODO: better hash function (lol)
    u32 hash_bucket = ent_a->ent_i & (ARRAY_COUNT(state->collision_rule_hash) - 1);
    for (pairwise_collision_rule *rule = state->collision_rule_hash[hash_bucket]; rule;
         rule                          = rule->next) {
        if (rule->ent_i_a == ent_a->ent_i && rule->ent_i_b == ent_b->ent_i) {
            result = rule->should_collide;
            break;
        }
    }

    return result;
}

internal bool
handle_collision(sim_entity *ent_a, sim_entity *ent_b)
{
    TOM_ASSERT(ent_a && ent_b);

    bool stop            = false;
    constexpr f32 hit_cd = 0.2f;

    if (ent_a->type > ent_b->type) {
        // TODO: swap func/template/macro?
        auto temp = ent_a;
        ent_a     = ent_b;
        ent_b     = temp;
    }

    // if and entity "collides" barriers will stop them
    if ((is_flag_set(ent_a, sim_entity_flags::barrier) &&
         is_flag_set(ent_b, sim_entity_flags::collides)) ||
        (is_flag_set(ent_b, sim_entity_flags::barrier) &&
         is_flag_set(ent_a, sim_entity_flags::collides)))
        stop = true;

    if (ent_a->type == entity_type::player && ent_b->type == entity_type::monster) {
        if (ent_a->hit_cd > hit_cd) {
            ent_a->hit_cd = 0.f;
            subtract_hit_points(ent_a, 1);
        }
        stop = true;
    }

    if (ent_a->type == entity_type::player && ent_b->type == entity_type::familiar) {
        if (ent_a->hit_cd > hit_cd) {
            ent_a->hit_cd = 0.f;
            add_hit_points(ent_a, 1);
        }
        stop = true;
    }

    if (ent_a->type == entity_type::player && ent_b->type == entity_type::stair) {
        ent_a->pos.x += 3.f;
    }
    if (ent_a->type == entity_type::monster && ent_b->type == entity_type::sword) {
        if (ent_a->hit_cd > hit_cd) {
            ent_a->hit_cd = 0.f;
            subtract_hit_points(ent_a, 1);
        }
    }

    return stop;
}

v3
calc_entity_delta(sim_entity *ent, entity_actions ent_act, entity_move_spec move_spec, const f32 dt)
{
    v3 ent_accel = ent_act.dir;

    // NOTE: normalize vector to unit length
    f32 ent_accel_len = vec::length(ent_accel);
    // TODO: make speed specific to ent type

    if (ent_accel_len > 1.f) ent_accel *= (1.f / math::sqrt_f32(ent_accel_len));
    ent_accel *= move_spec.speed;
    ent_accel -= ent->vel * move_spec.drag;
    ent->vel += ent_accel * dt;

    v3 ent_delta = (.5f * ent_accel * math::square(dt) + ent->vel * dt);

    return ent_delta;
}

void
move_entity(game_state *state, sim_region *region, sim_entity *ent, v3 ent_delta, const f32 dt)
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
        sim_entity *hit_ent = nullptr;

        if (!is_flag_set(ent, sim_entity_flags::nonspatial)) {
            // FIXME: this is N * N bad
            for (sim_entity *test_ent = region->sim_entities;
                 test_ent != region->sim_entities + region->sim_entity_cnt; ++test_ent) {
                if (test_ent->ent_i == ent->ent_i) continue;  // don't test against self

                TOM_ASSERT(test_ent);  // nullptr probably means something broke
                if (!should_collide(state, ent, test_ent)) continue;
                // TODO: need this flag anymore?

                // TODO: this is redundant, leaving it for now
                if (ent->weapon_i == test_ent->ent_i || ent->parent_i == test_ent->ent_i)
                    continue;  // skip parent and weapon

                // NOTE: Minkowski sum
                f32 r_w = ent->dim.x + test_ent->dim.x;
                f32 r_h = ent->dim.y + test_ent->dim.y;

                v2 min_corner = { -.5f * v2 { r_w, r_h } };
                v2 max_corner = { .5f * v2 { r_w, r_h } };
                v3 rel        = ent->pos - test_ent->pos;

                auto test_wall = [&t_min](f32 wall_x, f32 rel_x, f32 rel_y, f32 player_delta_x,
                                          f32 player_delta_y, f32 min_y, f32 max_y) -> bool {
                    bool hit = false;

                    if (player_delta_x != 0.f) {
                        f32 t_res = (wall_x - rel_x) / player_delta_x;
                        f32 y     = rel_y + t_res * player_delta_y;

                        if (t_res >= 0.f && (t_min > t_res)) {
                            if (y >= min_y && y <= max_y) {
                                t_min = math::max(0.f, t_res - global::epsilon);
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

        if (hit_ent) {
            bool stop_on_collision = handle_collision(ent, hit_ent);
            if (stop_on_collision) {
                v3 wall_nrm_v3 = v3_init(wall_nrm);
                ent->vel -= 1.f * vec::inner(ent->vel, wall_nrm_v3) * wall_nrm_v3;
                ent_delta -= 1.f * vec::inner(ent_delta, wall_nrm_v3) * wall_nrm_v3;
            } else {
                t_min = 1.f;
            }
        }
        // REVIEW: t_min is causing collision to always happen
        ent->pos += t_min * ent_delta;
        dist_remain -= t_min * ent_delta_len;
    }
    if (ent->dist_limit != 0.f) {
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

// TODO: do I need this? its not called anywhere but it might be useful idk
internal v3
get_cam_space_pos(const game_state &state, entity *ent)
{
    v3 result = get_world_diff(ent->world_pos, state.camera.pos);

    return result;
}

internal v3
get_sim_space_pos(const sim_region &region, const entity &ent)
{
    // TODO: what is the null/invalid value here?
    v3 result = { 100'000'000.0f, 100'000'000.0f, 100'000'000.0f };
    if (!is_flag_set(ent.sim.flags, sim_entity_flags::nonspatial)) {
        result = get_world_diff(ent.world_pos, region.origin);
    }

    return result;
}

v3
get_sim_space_pos(const game_state &state, const sim_region &region, u32 ent_i)
{
    // TODO: what is the null/invalid value here?
    v3 result         = { 100'000'000.0f, 100'000'000.0f, 100'000'000.0f };
    const entity &ent = state.entities[ent_i];
    if (!is_flag_set(ent.sim.flags, sim_entity_flags::nonspatial)) {
        result = get_world_diff(ent.world_pos, region.origin);
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

internal bool
entity_overlap_rect(const rect3 rect, sim_entity *ent)
{
    rect3 grown = rec::add_dim(rect, ent->dim);
    bool result = rec::is_inside(rect, ent->pos);

    return result;
}

internal sim_entity *
add_sim_entity_to_region(game_state *state, sim_region *region, u32 ent_i, entity *source_ent,
                         v3 *sim_pos)
{
    TOM_ASSERT(state && region && ent_i);

    sim_entity *dest_ent = add_sim_entity_to_region_raw(state, region, ent_i, source_ent);
    if (dest_ent) {
        if (sim_pos) {
            dest_ent->pos        = *sim_pos;
            dest_ent->updateable = entity_overlap_rect(region->update_bounds, dest_ent);
        } else {
            dest_ent->pos = get_sim_space_pos(*region, *source_ent);
        }
    }

    return dest_ent;
}

sim_region *
begin_sim(memory_arena *arena, game_state *state, world_pos origin, rect3 bounds, const f32 dt)
{
    TOM_ASSERT(arena && state);

    // NOTE: clear the hash table!
    sim_region *region = PUSH_STRUCT(arena, sim_region);
    ZERO_STRUCT(region->hash);

    // TODO: IMPORTANT-> calc this from the max value of all entities radius + speed
    f32 update_safety_margin =
        global::max_entity_r + global::max_entity_vel * dt + global::update_margin;

    region->origin        = origin;
    region->update_bounds = bounds;
    region->bounds        = rec::add_radius(bounds, update_safety_margin);

    region->max_sim_entity_cnt = 4096;  // TODO: how many max entities?
    region->sim_entity_cnt     = 0;
    region->sim_entities       = PUSH_ARRAY(arena, region->max_sim_entity_cnt, sim_entity);

    world_pos min_chunk_pos = map_into_chunk_space(origin, rec::min_corner(region->bounds));
    world_pos max_chunk_pos = map_into_chunk_space(origin, rec::max_corner(region->bounds));

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
                            v3 sim_space_pos = get_sim_space_pos(*region, *ent);
                            if (entity_overlap_rect(region->bounds, &ent->sim)) {
                                add_sim_entity_to_region(state, region, block_ent_i, ent,
                                                         &sim_space_pos);
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
