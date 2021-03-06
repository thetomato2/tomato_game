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
init_hit_points(sim_entity *ent, u32 hp)
{
    ent->max_hp = hp;
    ent->hp     = ent->max_hp;
}

entity *
add_new_entity(game_state *state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    entity *ent = nullptr;

    if (state->ent_cnt < global::max_ent_cnt) {
        u32 ent_i      = state->ent_cnt++;
        ent            = state->entities + ent_i;
        *ent           = {};
        ent->sim.ent_i = ent_i;
        ent->sim.type  = entity_type::null;
        ent->sim.flags = sim_entity_flags::active;
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
add_wall(game_state *state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);

    ent->sim.type  = entity_type::wall;
    ent->sim.dim.y = 1.f;
    ent->sim.dim.x = 1.f;
    ent->sim.dim.z = 1.f;
    set_flag(ent, sim_entity_flags::barrier);

    return ent;
}

entity *
add_tree(game_state *state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);

    ent->sim.type  = entity_type::tree;
    ent->sim.dim.y = 1.f;
    ent->sim.dim.x = 0.5f;
    ent->sim.dim.z = 1.f;
    set_flag(ent, sim_entity_flags::barrier);

    return ent;
}

entity *
add_stair(game_state *state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);

    ent->sim.type        = entity_type::stair;
    ent->sim.dim.y       = 0.5f;
    ent->sim.dim.x       = 0.8f;
    ent->sim.dim.z       = 1.f;
    ent->sim.argb_offset = 12.f;
    set_flag(ent, sim_entity_flags::collides);

    return ent;
}

entity *
add_monster(game_state *state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);

    ent->sim.type        = entity_type::monster;
    ent->sim.dim.y       = .6f;
    ent->sim.dim.x       = .6f * .6f;
    ent->sim.dim.z       = .6f * .6f;
    ent->sim.argb_offset = 16.f;
    set_flag(ent, sim_entity_flags::collides);

    init_hit_points(&ent->sim, 6);

    return ent;
}

entity *
add_cat(game_state *state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    entity *ent = add_new_entity(state, abs_x, abs_y, abs_z);
    TOM_ASSERT(ent);
    ent->world_pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    ent->sim.type        = entity_type::familiar;
    ent->sim.dim.y       = 0.6f;
    ent->sim.dim.x       = 0.8f;
    ent->sim.dim.z       = 0.2f;
    ent->sim.argb_offset = 5.0f;
    set_flag(ent, sim_entity_flags::collides);

    return ent;
}

entity *
add_sword(game_state *state, u32 parent_i)
{
    entity *ent = add_new_entity(state);
    TOM_ASSERT(ent);

    ent->sim.type        = entity_type::sword;
    ent->sim.pos         = {};
    ent->sim.dim.y       = 0.2f;
    ent->sim.dim.x       = 0.2f;
    ent->sim.dim.z       = 0.2f;
    ent->sim.argb_offset = 5.0f;
    ent->sim.parent_i    = parent_i;
    set_flag(ent, sim_entity_flags::hurtbox | sim_entity_flags::nonspatial);

    // REVIEW: what happens if I add a second weapon?
    entity *parent_ent       = state->entities + parent_i;
    parent_ent->sim.weapon_i = ent->sim.ent_i;

    return ent;
}

void
add_player(game_state *state, u32 player_i, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    // NOTE: the first 5 entities are reserved for players
    if (player_i <= state->player_cnt) {
        entity *player = add_new_entity(state);
        if (player_i == player->sim.ent_i) {
            player->sim.type        = entity_type::player;
            player->sim.dim.y       = 0.6f;
            player->sim.dim.x       = 0.6f * player->sim.dim.y;
            player->sim.dim.z       = 0.6f * player->sim.dim.y;
            player->sim.argb_offset = 16.0f;
            set_flag(player, sim_entity_flags::collides);

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

}  // namespace tom
