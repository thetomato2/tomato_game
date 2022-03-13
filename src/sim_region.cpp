#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{

// ===============================================================================================
// #INTERNAL
// ===============================================================================================

internal sim_entity *
add_entity(game_state &state, sim_region &region, u32 stored_i, stored_entity &source_ent,
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
map_storage_ind_to_ent(sim_region &sim_region, u32 stored_i, sim_entity &entity)
{
    sim_entity_hash *entry = get_hash_from_ind(sim_region, stored_i);
    TOM_ASSERT(entry->ind == 0 || entry->ind == stored_i);
    entry->ind = stored_i;
    entry->ptr = &entity;
}

internal void
store_entity_ref(entity_ref *ref)
{
    if (ref->ptr != 0) {
        ref->ind = ref->ptr->stored_i;
    }
}
internal sim_entity *
get_entity_from_ind(sim_region *region, u32 stored_i)
{
    sim_entity_hash *entry = get_hash_from_ind(region, stored_i);
    return entry->ptr;
}

internal void
load_entity_ref(game_state &state, sim_region region, entity_ref &ref)
{
    if (ref.ind) {
        sim_entity_hash *entry = get_hash_from_ind(region, ref.ind);
        if (entry->ptr == nullptr) {
        }
        ref.ptr = entry->ptr;
    }
}

internal v2
get_cam_space_pos(const game_state &state, stored_entity *stored_ent)
{
    world_dif diff = get_world_diff(stored_ent->world_pos, state.camera.pos);
    v2 result      = { diff.dif_xy };

    return result;
}

internal v2
get_sim_space_pos(const sim_region &region, const stored_entity &stored_ent)
{
    world_dif dif = get_world_diff(stored_ent.world_pos, region.origin);
    return dif.dif_xy;
}

internal sim_entity *
add_entity(game_state &state, sim_region &region, u32 stored_i, stored_entity *source_ent)
{
    TOM_ASSERT(stored_i);
    sim_entity *entity = nullptr;

    if (region.sim_entity_cnt < region.max_sim_entity_cnt) {
        // TODO: should be a decrompression step, not a copy!
        entity = region.sim_entities + region.sim_entity_cnt++;
        map_storage_ind_to_ent(region, stored_i, *entity);

        if (source_ent) {
            *entity = source_ent->sim;
            load_entity_ref(state, region, entity->weapon_i);
        }

        entity->stored_i = stored_i;

    } else {
        INVALID_CODE_PATH;
    }

    return entity;
}

internal sim_entity *
add_entity(game_state &state, sim_region &region, u32 stored_i, stored_entity &source_ent,
           v2 *sim_pos)
{
    sim_entity *dest_ent = add_entity(region, stored_i, &source_ent);
    if (dest_ent) {
        if (sim_pos) {
            dest_ent->pos = *sim_pos;
        } else {
            dest_ent->pos = get_sim_space_pos(region, source_ent);
        }
    }
}
internal void
add_hit_points(sim_entity &ent, s32 hp)
{
    ent.hp += hp;
    if (ent.hp > ent.max_hp) ent.hp = ent.max_hp;
}

internal void
subtract_hit_points(sim_entity &ent, s32 hp)
{
    ent.hp -= hp;
    if (ent.hp < 0) {
        ent.hp     = 0;
        ent.active = false;
    }
}

internal void
init_hit_points(sim_entity *ent, const u32 hp)
{
    ent->max_hp = hp;
    ent->hp     = ent->max_hp;
}

internal void
move_entity(game_state *state, sim_region *region, sim_entity *ent,
            const entity_actions ent_actions, const entity_move_spec move_spec, const f32 dt)
{
    TOM_ASSERT(state && region);

    v2 entity_acc = ent_actions.dir;

    // NOTE: normalize vector to unit length
    f32 ent_acc_length = vec::length_sq(entity_acc);
    // TODO: make speed spefific to ent type

    if (ent_acc_length > 1.f) entity_acc *= (1.f / math::sqrt_f32(ent_acc_length));
    entity_acc *= move_spec.speed;
    entity_acc -= ent->vel * move_spec.drag;

    v2 player_delta   = (.5f * entity_acc * math::square(dt) + ent->vel * dt);
    v2 new_player_pos = ent->pos + player_delta;

    ent->vel += entity_acc * dt;

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 t_min           = 1.0f;
        u32 hit_ent_ind     = 0;  // 0 is the null ent
        v2 wall_nrm         = {};
        v2 desired_position = ent->pos + player_delta;

        // FIXME: this is N * N bad
        for (u32 test_ent_i = 1; test_ent_i < region->sim_entity_cnt; ++test_ent_i) {
            if (test_ent_i == ent->stored_i) continue;  // don't test against self

            sim_entity *test_ent = get_entity_from_ind(region, test_ent_i);
            TOM_ASSERT(test_ent);  // a nullptr probrably means something broke

            if (!test_ent->active || !test_ent->collides)
                continue;  // skip inactive and non-collision entities
            if (ent->weapon_i.ind == test_ent->stored_i) continue;  // skip ent's weapon
            if (ent->parent_i.ind == test_ent->stored_i) continue;  // skip parent ent

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

            if (test_wall(min_corner.x, rel.x, rel.y, player_delta.x, player_delta.y, min_corner.y,
                          max_corner.y)) {
                wall_nrm    = v2 { -1.f, 0.f };
                hit_ent_ind = test_ent_i;
            }
            if (test_wall(max_corner.x, rel.x, rel.y, player_delta.x, player_delta.y, min_corner.y,
                          max_corner.y)) {
                wall_nrm    = v2 { 1.f, 0.f };
                hit_ent_ind = test_ent_i;
            }
            if (test_wall(min_corner.y, rel.y, rel.x, player_delta.y, player_delta.x, min_corner.x,
                          max_corner.x)) {
                wall_nrm    = v2 { 0.f, -1.f };
                hit_ent_ind = test_ent_i;
            }
            if (test_wall(max_corner.y, rel.y, rel.x, player_delta.y, player_delta.x, min_corner.x,
                          max_corner.x)) {
                wall_nrm    = v2 { 0.f, 1.f };
                hit_ent_ind = test_ent_i;
            }
        }

        sim_entity *hit_ent = region->sim_entities + hit_ent_ind;

        if (!hit_ent->barrier) {
            wall_nrm = v2 { 0.f, 0.f };
        }

        ent->pos += t_min * player_delta;
        if (hit_ent_ind) {
            ent->vel -= 1.f * vec::inner(ent->vel, wall_nrm) * wall_nrm;
            player_delta -= 1.f * vec::inner(player_delta, wall_nrm) * wall_nrm;

            // printf("%d hit %d!\n", ent->low->high_i, hit_ent_ind);

            // TODO: temp player hitting monster logic
            if (ent->hit_cd > .5f) {
                switch (ent->type) {
                    case entity_type::player: {
                        if (hit_ent->type == entity_type::monster) {
                            ent->hit_cd = 0.f;
                            subtract_hit_points(ent, 1);
                        } else if (hit_ent->type == entity_type::familiar) {
                            ent->hit_cd = 0.f;
                            add_hit_points(ent, 1);
                        }
                    } break;
                    case entity_type::sword: {
                        if (hit_ent->type == entity_type::monster) {
                            if (hit_ent->hit_cd <= 0.0f) {
                                hit_ent->hit_cd = 0.5f;
                                subtract_hit_points(*hit_ent, 1);
                            } else {
                                hit_ent->hit_cd -= dt;
                            }
                        }
                    } break;
                }
            } else {
                break;
            }
        }
    }

    ent->hit_cd += dt;

    // NOTE: changes the players direction for the sprite
    v2 pv = { ent->vel };
    if (math::abs_f32(pv.x) > math::abs_f32(pv.y)) {
        pv.x > 0.f ? ent->dir = entity_direction::east : ent->dir = entity_direction::west;
    } else if (math::abs_f32(pv.y) > math::abs_f32(pv.x)) {
        pv.y > 0.f ? ent->dir = entity_direction::north : ent->dir = entity_direction::south;
    }

    // TODO:
    world_pos new_pos = map_into_chunk_space(state->camera.pos, ent->pos);
    change_entity_location(&state->world_arena, *state->world, ent->stored_i,
                           &state->stored_entities[ent->stored_i].world_pos, &new_pos);
    state->stored_entities[ent->stored_i].world_pos = new_pos;
}

internal void
update_familiar(game_state *state, sim_region *region, stored_entity fam, const f32 dt)
{
    stored_entity *closest_player = nullptr;
    f32 closest_player_dist_sq    = math::square(10.f);
    for (u32 high_i = 1; high_i < state.high_cnt; ++high_i) {
        entity test_ent = get_entity_from_high_i(state, high_i);
        if (test_ent.low->type == entity_type::player) {
            f32 test_dist_sq = vec::length_sq(test_ent.high->pos - fam.high->pos);
            if (closest_player_dist_sq > test_dist_sq) {
                closest_player         = test_ent;
                closest_player_dist_sq = test_dist_sq;
            }
        }
    }

    if (closest_player.high) {
        entity_actions fam_acts = {};
        f32 one_over_len        = 1.f / sqrt_f32(closest_player_dist_sq);
        f32 min_dist            = 2.f;
        v2 dif                  = closest_player.high->pos - fam.high->pos;
        if (abs_f32(dif.x) > min_dist || abs_f32(dif.y) > min_dist)
            fam_acts.dir = one_over_len * (dif);

        auto move_spec = get_default_move_spec();
        move_entity(state, fam, fam_acts, move_spec, dt);
        if (fam.high->dir == entity_direction::east)
            fam.low->sprite = &state.cat_sprites[0];
        else if (fam.high->dir == entity_direction::west)
            fam.low->sprite = &state.cat_sprites[1];
    }
}

internal void
update_sword(game_state &state, entity sword, const f32 dt)
{
    entity_actions sword_acts = {};
    constexpr f32 sword_vel   = 5.0f;

    switch (sword.high->dir) {
        case entity_direction::north: {
            sword.high->vel.y = sword_vel;
            sword.low->sprite = &state.sword_sprites[entity_direction::north];
        } break;
        case entity_direction::east: {
            sword.high->vel.x = sword_vel;
            sword.low->sprite = &state.sword_sprites[entity_direction::east];
        } break;
        case entity_direction::south: {
            sword.high->vel.y = -sword_vel;
            sword.low->sprite = &state.sword_sprites[entity_direction::south];
        } break;
        case entity_direction::west: {
            sword.high->vel.x = -sword_vel;
            sword.low->sprite = &state.sword_sprites[entity_direction::west];
        } break;
    }
    auto move_spec = get_default_move_spec();
    move_spec.drag = 0.0f;
    move_entity(state, sword, sword_acts, move_spec, dt);
}

internal void
update_monster(game_state &state, entity monster, f32 dt)
{
}

internal void
update_player(game_state &state, entity player, f32 dt)
{
    auto move_spec = get_default_move_spec();

    f32 ent_speed = state.player_acts[1].sprint ? move_spec.speed = 25.f : move_spec.drag = 10.f;
    move_entity(state, player, state.player_acts[1], move_spec, dt);
    switch (player.high->dir) {
        case entity_direction::north: {
            player.low->sprite = &state.player_sprites[entity_direction::north];
        } break;
        case entity_direction::east: {
            player.low->sprite = &state.player_sprites[entity_direction::east];
        } break;
        case entity_direction::south: {
            player.low->sprite = &state.player_sprites[entity_direction::south];
        } break;
        case entity_direction::west: {
            player.low->sprite = &state.player_sprites[entity_direction::west];
        } break;
    }
}

// ===============================================================================================
// #EXTERNAL
// ===============================================================================================

stored_entity *
get_stored_entity(game_state &state, u32 ind)
{
    stored_entity *result = nullptr;

    if (ind > 0 && ind < state.stored_cnt) {
        result = state.stored_entities + ind;
    }

    return result;
}

sim_region *
begin_sim(memory_arena *arena, game_state &state, world_pos origin, rect bounds)
{
    sim_region *region         = PUSH_STRUCT(arena, sim_region);
    region->origin             = origin;
    region->bounds             = bounds;
    region->max_sim_entity_cnt = 4096;  // TODO: how many max entities?
    region->sim_entity_cnt     = 0;
    region->sim_entities       = PUSH_ARRAY(arena, region->max_sim_entity_cnt, sim_entity);

    world_pos min_chunk_pos = map_into_chunk_space(origin, rec::min_corner(bounds));
    world_pos max_chunk_pos = map_into_chunk_space(origin, rec::max_corner(bounds));

    // make all entities outside camera space stored
    for (s32 chunk_y = min_chunk_pos.chunk_y; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x = min_chunk_pos.chunk_x; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            world_chunk *chunk = get_world_chunk(*state.world, chunk_x, chunk_y, origin.chunk_z);
            if (chunk) {
                for (world_entity_block *block = &chunk->first_block; block; block = block->next) {
                    for (u32 ent_i = 0; ent_i < block->stored_entity_cnt; ++ent_i) {
                        u32 stored_i              = block->stored_ents_inds[ent_i];
                        stored_entity *stored_ent = state.stored_entities + stored_i;
                        v2 sim_space_pos          = get_sim_space_pos(*region, *stored_ent);
                        if (rec::is_inside(region->bounds, sim_space_pos)) {
                            add_entity(state, *region, stored_i, *stored_ent, &sim_space_pos);
                            printf("rect is inside!\n");
                        }
                    }
                }
            }
        }
    }

    return region;
}

void
end_sim(game_state &state, sim_region &region)
{
    // TODO: low entities stored in the world and not games state?
    sim_entity *entity = region.sim_entities;
    for (u32 ent_i = 0; ent_i < region.sim_entity_cnt; ++ent_i, ++entity) {
        stored_entity &stored_ent = state.stored_entities[ent_i];

        stored_ent.sim = *entity;
        store_entity_ref(&stored_ent.sim.weapon_i);

        // TODO: save state back to stored entity, once high entities do state decompression
        world_pos new_pos = map_into_chunk_space(region.origin, entity->pos);
        change_entity_location(state.world_arena, state.world, entity->stored_i, &stored_ent.pos,
                               &new_pos);
    }
}

}  // namespace tom
