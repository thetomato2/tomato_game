#include "entity.hpp"
#include "game.hpp"
#include "image.hpp"

namespace tom
{

entity *
get_entity(game_state *state, const u32 ind)
{
    entity *result = nullptr;

    if (ind > 0 && ind < state->ent_cnt) {
        result = state->entities + ind;
    }

    return result;
}

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
init_hit_points(sim_entity *ent, u32 hp)
{
    ent->max_hp = hp;
    ent->hp     = ent->max_hp;
}

entity *
add_new_entity(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    entity *ent = nullptr;

    if (state->ent_cnt < global::max_ent_cnt) {
        u32 ent_i      = state->ent_cnt++;
        ent            = state->entities + ent_i;
        *ent           = {};
        ent->sim.ent_i = ent_i;
        ent->type      = entity_type::null;
        ent->world_pos = null_world_pos();

        // NOTE: this inserts the entity into the corresponding  world chunk
        // REVIEW: make add_entity_to_world()?
        world_pos start_pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);
        change_entity_location(&state->world_arena, state->world, ent, start_pos);
    } else {
        INVALID_CODE_PATH;
    }

    return ent;
}

entity *
add_tree(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);

    ent->type       = entity_type::wall;
    ent->color      = { 0xff'dd'dd'dd };
    ent->sprite     = &state->tree_sprite;
    ent->sim.height = 1.f;
    ent->sim.width  = 1.f;
    ent->sim.flags =
        sim_entity_flags::active | sim_entity_flags::collides | sim_entity_flags::barrier;

    return ent;
}

entity *
add_monster(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);

    ent->type            = entity_type::monster;
    ent->color           = { 0xff'dd'dd'dd };
    ent->sprite          = state->monster_sprites;
    ent->sim.height      = .6f;
    ent->sim.width       = .6f * .6f;
    ent->sim.argb_offset = 16.f;
    ent->sim.flags =
        sim_entity_flags::active | sim_entity_flags::collides | sim_entity_flags::barrier;

    init_hit_points(&ent->sim, 6);

    return ent;
}

entity *
add_cat(game_state *state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);
    ent->world_pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    ent->type            = entity_type::familiar;
    ent->color           = { 0xff'dd'dd'dd };
    ent->sprite          = state->cat_sprites;
    ent->sim.height      = .6f;
    ent->sim.width       = .8f;
    ent->sim.argb_offset = 5.f;
    ent->sim.flags =
        sim_entity_flags::active | sim_entity_flags::collides | sim_entity_flags::barrier;

    return ent;
}

entity *
add_sword(game_state *state, u32 parent_i, argb_img *sprite)
{
    entity *ent = add_new_entity(state);
    TOM_ASSERT(ent);

    ent->type            = entity_type::sword;
    ent->color           = { 0xff'dd'dd'dd };
    ent->sim.pos         = {};
    ent->sim.height      = .6f;
    ent->sim.width       = .8f;
    ent->sim.argb_offset = 5.f;
    ent->sim.flags       = sim_entity_flags::hurtbox | sim_entity_flags::nonspatial;

    return ent;
}

void
add_player(game_state *state, u32 player_i, f32 abs_x, f32 abs_y, f32 abs_z)
{
    // NOTE: the first 5 entities are reserved for players
    if (player_i <= state->player_cnt) {
        entity *player = add_new_entity(state);
        if (player_i == player->sim.ent_i) {
            player->type            = entity_type::player;
            player->color           = { 0xff'00'00'ff };
            player->sprite          = state->player_sprites;
            player->sim.height      = .6f;
            player->sim.width       = 0.6f * player->sim.height;
            player->sim.argb_offset = 16.f;
            player->sim.flags =
                sim_entity_flags::active | sim_entity_flags::collides | sim_entity_flags::barrier;

            init_hit_points(&player->sim, 10);

            entity *sword        = add_sword(state, player->sim.ent_i);
            player->sim.weapon_i = sword->sim.ent_i;

        } else {
            INVALID_CODE_PATH;
        }
    } else {
        INVALID_CODE_PATH;
    }
}

void
move_entity(game_state *state, sim_region *region, entity *ent, v2 ent_delta, const f32 dt)
{
    TOM_ASSERT(state && region);

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 t_min           = 1.0f;
        v2 wall_nrm         = {};
        v2 desired_position = ent->sim.pos + ent_delta;
        sim_entity *hit_ent = nullptr;

        // FIXME: this is N * N bad
        for (sim_entity *test_ent = region->sim_entities;
             test_ent != region->sim_entities + region->sim_entity_cnt; ++test_ent) {
            if (test_ent->ent_i == ent->sim.ent_i) continue;  // don't test against self

            TOM_ASSERT(test_ent);  // a nullptr probably means something broke

            if (!is_flag_set(test_ent->flags, sim_entity_flags::active) ||
                !is_flag_set(test_ent->flags, sim_entity_flags::collides))
                continue;  // skip inactive and non-collision entities
            if (ent->sim.weapon_i == test_ent->ent_i) continue;  // skip ent's weapon
            if (ent->sim.parent_i == test_ent->ent_i) continue;  // skip parent ent

            // NOTE: Minkowski sum
            f32 r_w = ent->sim.width + test_ent->width;
            f32 r_h = ent->sim.height + test_ent->height;

            v2 min_corner = { -.5f * v2 { r_w, r_h } };
            v2 max_corner = { .5f * v2 { r_w, r_h } };
            v2 rel        = ent->sim.pos - test_ent->pos;

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

        if (hit_ent && !is_flag_set(hit_ent->flags, sim_entity_flags::barrier)) {
            wall_nrm = v2 { 0.f, 0.f };
        }

        ent->sim.pos += t_min * ent_delta;
        if (hit_ent) {
            ent->sim.vel -= 1.f * vec::inner(ent->sim.vel, wall_nrm) * wall_nrm;
            ent_delta -= 1.f * vec::inner(ent_delta, wall_nrm) * wall_nrm;

            // printf("%d hit %d!\n", ent->sim.low->high_i, hit_ent);

            // TODO: temp player hitting monster logic
            if (ent->sim.hit_cd > .5f) {
                auto hit_ent_type = state->entities[hit_ent->ent_i].type;
                switch (ent->type) {
                    case entity_type::player: {
                        if (hit_ent_type == entity_type::monster) {
                            ent->sim.hit_cd = 0.f;
                            subtract_hit_points(&ent->sim, 1);
                        } else if (hit_ent_type == entity_type::familiar) {
                            ent->sim.hit_cd = 0.f;
                            add_hit_points(&ent->sim, 1);
                        }
                    } break;
                    case entity_type::sword: {
                        if (hit_ent_type == entity_type::monster) {
                            if (hit_ent->hit_cd <= 0.0f) {
                                hit_ent->hit_cd = 0.5f;
                                subtract_hit_points(hit_ent, 1);
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

    ent->sim.hit_cd += dt;

    // NOTE: changes the players direction for the sprite
    v2 pv = { ent->sim.vel };

    constexpr f32 dir_eps = 0.1f;

    if (math::abs_f32(pv.x) > math::abs_f32(pv.y)) {
        if (pv.x > 0.0f + dir_eps) {
            ent->dir = entity_direction::east;
        } else if (pv.x < 0.0f - dir_eps) {
            ent->dir = entity_direction::west;
        }

    } else if (math::abs_f32(pv.y) > math::abs_f32(pv.x)) {
        if (pv.y > 0.0f + dir_eps) {
            ent->dir = entity_direction::north;
        } else if (pv.y < 0.0f - dir_eps) {
            ent->dir = entity_direction::south;
        }
    }

    world_pos new_pos = map_into_chunk_space(state->camera.pos, ent->sim.pos);
    change_entity_location(&state->world_arena, state->world, ent, new_pos);

    // TODO: move this out of here?
    // state->entities[ent->sim.ent_i].world_pos = new_pos;
}

}  // namespace tom
