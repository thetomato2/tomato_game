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

struct Camera
{
    World_Pos pos;
};

struct Game_State
{
    Memory_Arena world_arena;
    World *world;

    u32 entity_camera_follow_ind;
    Camera camera;
    Bitmap_Img bitmap;
    u32 player_controller_ind[5];
    u32 player_cnt;
    Entity_Actions player_acts[5];

    u32 stored_cnt;
    Stored_Entity stored_entities[global::max_low_cnt];

    ARGB_img bg_img;
    ARGB_img crosshair_img;
    ARGB_img player_sprites[4];
    ARGB_img monster_sprites[4];
    ARGB_img sword_sprites[4];
    ARGB_img cat_sprites[2];
    ARGB_img tree_sprite;
    ARGB_img stair_sprite;

    World_Pos test_pos;

    b32 debug_draw_collision;
};

inline bool
is_key_up(const Game_Button_State &key)
{
    return key.half_transition_count > 0 && key.ended_down == 0;
}

inline bool
is_button_up(const Game_Button_State &button)
{
    return is_key_up(button);
}
}  // namespace tom
#endif
