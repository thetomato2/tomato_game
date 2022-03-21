#ifndef TOMATO_GAME_HPP_
#define TOMATO_GAME_HPP_

#include "platform.h"
#include "common.hpp"
#include "image.hpp"
#include "entity.hpp"
#include "world.hpp"
#include "sim_region.hpp"

namespace tom
{

struct camera
{
    world_pos pos;
};

struct pairwise_collision_rule
{
    b32 should_collide;
    u32 ent_i_a;
    u32 ent_i_b;

    pairwise_collision_rule *next;
};

struct game_state
{
    memory_arena world_arena;
    world *world;

    u32 entity_camera_follow_ind;
    camera camera;

    u32 player_cnt;
    u32 player_controller_ind[INPUT_CNT];
    entity_actions player_acts[INPUT_CNT];

    u32 ent_cnt;
    entity entities[global::max_ent_cnt];

    // NOTE: must be power of two
    pairwise_collision_rule *collision_rule_hash[256];
    pairwise_collision_rule *first_free_collision_rule;

    // TODO: make a struct for all textures?
    argb_img bg_img;
    argb_img crosshair_img;
    argb_img player_sprites[4];
    argb_img monster_sprites[4];
    argb_img sword_sprites[4];
    argb_img cat_sprites[2];
    argb_img tree_sprite;
    argb_img stair_sprite;

    world_pos test_pos;

    b32 debug_draw_collision;
    b32 debug_flag;
};

inline bool
is_key_up(const game_button_state key)
{
    return key.half_transition_count > 0 && key.ended_down == 0;
}

inline bool
is_button_up(const game_button_state button)
{
    return is_key_up(button);
}

}  // namespace tom
#endif
