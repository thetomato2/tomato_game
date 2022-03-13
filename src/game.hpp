#ifndef GAME_HPP_
#define GAME_HPP_

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

struct game_state
{
    memory_arena world_arena;
    world *world;

    u32 entity_camera_follow_ind;
    camera camera;
    bitmap_img bitmap;
    u32 player_controller_ind[5];
    u32 player_cnt;
    entity_actions player_acts[5];

    u32 stored_cnt;
    stored_entity stored_entities[global::max_low_cnt];

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
};

inline bool
is_key_up(const game_button_state &key)
{
    return key.half_transition_count > 0 && key.ended_down == 0;
}

inline bool
is_button_up(const game_button_state &button)
{
    return is_key_up(button);
}
}  // namespace tom
#endif
