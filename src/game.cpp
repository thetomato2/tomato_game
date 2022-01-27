#include "game.hpp"

namespace tomato
{
namespace
{
namespace global
{
static constexpr u32 s_tile_size_pixels = 40;
static constexpr f32 s_meters_to_pixels = s_tile_size_pixels / Tile_Map::s_tile_size_meters;

#include "rng_nums.h"
}  // namespace global

void
clear_buffer(Game_Offscreen_Buffer &buffer_, Color_u32 color_ = { 0xff'ff'00'ff })
{
    s32 width  = buffer_.width;
    s32 height = buffer_.height;

    byt *row = (byt *)buffer_.memory;
    for (s32 y = 0; y < height; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = 0; x < width; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

void
draw_rect(Game_Offscreen_Buffer &buffer_, f32 f32_min_x_, f32 f_min_y_, f32 f32_max_x_,
          f32 f32_max_y_, Color_u32 color_ = { 0xffffffff })
{
    s32 min_x = math::round_f32_to_s32(f32_min_x_);
    s32 min_y = math::round_f32_to_s32(f_min_y_);
    s32 max_x = math::round_f32_to_s32(f32_max_x_);
    s32 max_y = math::round_f32_to_s32(f32_max_y_);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer_.width) max_x = buffer_.width;
    if (max_y > buffer_.height) max_y = buffer_.height;

    byt *row = ((byt *)buffer_.memory + min_x * buffer_.bytes_per_pixel + min_y * buffer_.pitch);

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = min_x; x < max_x; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

void
draw_ARGB(Game_Offscreen_Buffer &buffer_, ARGB_Img &img_, v2 pos_)
{
    s32 min_y = math::round_f32_to_s32(pos_.y - ((f32)img_.height / 2.f));
    s32 min_x = math::round_f32_to_s32(pos_.x - ((f32)img_.width / 2.f));
    s32 max_y = math::round_f32_to_s32(pos_.y + ((f32)img_.height / 2.f));
    s32 max_x = math::round_f32_to_s32(pos_.x + ((f32)img_.width / 2.f));

    s32 x_offset_left {}, x_offset_right {}, y_offset {};

    if (min_y < 0) {
        y_offset = min_y * -1;
        min_y    = 0;
    }
    if (min_x < 0) {
        // x_offset_left = (min_x * -1) + 1;
        x_offset_left = min_x * -1;
        min_x         = 0;
    }
    if (max_x > buffer_.width) {
        x_offset_right = max_x - buffer_.width;
        max_x          = buffer_.width;
    }
    if (max_y > buffer_.height) max_y = buffer_.height;

    u32 *source = img_.pixel_ptr + (y_offset * img_.width);
    byt *row    = ((byt *)buffer_.memory + min_x * buffer_.bytes_per_pixel + min_y * buffer_.pitch);

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *dest = (u32 *)row;
        source += x_offset_left;
        for (s32 x = min_x; x < max_x; ++x) {
            Color_u32 dest_col { *dest };
            Color_u32 source_col { *source };
            Color_u32 blended_col;
            blended_col.a = 0xff;

            f32 alpha = (f32)source_col.a / 255.f;

            blended_col.r = u8((1.f - alpha) * (f32)dest_col.r + alpha * (f32)source_col.r);
            blended_col.g = u8((1.f - alpha) * (f32)dest_col.g + alpha * (f32)source_col.g);
            blended_col.b = u8((1.f - alpha) * (f32)dest_col.b + alpha * (f32)source_col.b);

            *dest = blended_col.argb;

            ++dest, ++source;
        }
        source += x_offset_right;
        row += buffer_.pitch;
    }
}

inline Tile_Map_Pos
get_entity_center_pos(const Entity &entity_)
{
    return offset_pos(entity_.pos, { entity_.width / 2.f, entity_.height / 2.f });
}

void
entity_check_tile_map(Tile_Map &tile_map_, const Entity &entity_)
{
    // NOTE: this function gets the center of the player,
    // then check if the player is out of the tile map,
    // and if so moves the player to the correct position on the new tile map
    auto player_center_pos = get_entity_center_pos(entity_);
    auto *tile_map         = get_tile_chunk(tile_map_, player_center_pos.abs_tile_x,
                                            player_center_pos.abs_tile_y, player_center_pos.abs_tile_z);
    if (tile_map != nullptr && tile_map_.cur_tile_chunk != tile_map)
        tile_map_.cur_tile_chunk = tile_map;
}

void
game_ouput_sound(Game_Sound_Output_Buffer &sound_buffer_)
{
    // NOTE: outputs nothing atm
    s16 sample_value = 0;
    s16 *sampleOut   = sound_buffer_.samples;
    for (szt sampleIndex = 0; sampleIndex < sound_buffer_.sample_count; ++sampleIndex) {
        *sampleOut++ = sample_value;
        *sampleOut++ = sample_value;
    }
}

void
init_arena(Mem_Arena *arena_, mem_ind size_, byt *base_)
{
    arena_->size = size_;
    arena_->base = base_;
    arena_->used = 0;
}

Bitmap
load_bmp(Thread_Context *thread_, debug_platform_read_entire_file *read_entire_file_,
         const char *file_name_)
{
    debug_ReadFileResult read_result = read_entire_file_(thread_, file_name_);
    Bitmap result;

    if (read_result.content_size != 0) {
        auto *header     = (Bitmap_Header *)read_result.contents;
        u32 *pixels      = (u32 *)((byt *)read_result.contents + header->bitmap_offset);
        result.width     = header->width;
        result.height    = header->height;
        result.pixel_ptr = pixels;
    }
    return result;
}

ARGB_Img
load_ARGB(Thread_Context *thread_, debug_platform_read_entire_file *read_entire_file_,
          const char *file_name_)
{
    const char *argb_dir = "T:/assets/argbs/";
    char img_path_buf[512];
    szt img_buf_len;
    tomato::util::cat_str(argb_dir, file_name_, &img_path_buf[0], &img_buf_len);
    img_path_buf[img_buf_len++] = '.';
    img_path_buf[img_buf_len++] = 'a';
    img_path_buf[img_buf_len++] = 'r';
    img_path_buf[img_buf_len++] = 'g';
    img_path_buf[img_buf_len++] = 'b';
    img_path_buf[img_buf_len++] = '\0';

    debug_ReadFileResult read_result = read_entire_file_(thread_, img_path_buf);
    ARGB_Img result;

    if (read_result.content_size != 0) {
        auto *file_ptr   = (u32 *)read_result.contents;
        result.width     = *file_ptr++;
        result.height    = *file_ptr++;
        result.size      = *file_ptr++;
        result.pixel_ptr = file_ptr;
    }
    return result;
}

void
init_player(Entity &player_, ARGB_Img *sprites = nullptr)
{
    player_.exists         = false;
    player_.height         = .6f;
    player_.width          = 0.6f * player_.height;
    player_.pos.offset.x   = 0.f;
    player_.pos.offset.y   = 0.f;
    player_.pos.abs_tile_x = 3;
    player_.pos.abs_tile_y = 3;
    player_.color          = { 0xff'00'00'ff };
    player_.direction      = 0;
    player_.stair_cd       = 0;
    player_.vel            = {};

    player_.sprites = sprites;
}

void
move_camera(Camera &camera_, Entity &entity_)
{
    // NOTE: moves the camera_ to follow the player in set increments
    Tile_Map_Dif entity_dif = get_tile_diff(entity_.pos, camera_.pos);
    if (entity_dif.dif_xy.x >
        (Game_State::s_num_tiles_per_screen_x * Tile_Map::s_tile_size_meters) / 2) {
        camera_.pos.abs_tile_x += Game_State::s_num_tiles_per_screen_x / 2;
    }
    if (entity_dif.dif_xy.x <
        (Game_State::s_num_tiles_per_screen_x * Tile_Map::s_tile_size_meters) / -2) {
        camera_.pos.abs_tile_x -= Game_State::s_num_tiles_per_screen_x / 2;
    }
    if (entity_dif.dif_xy.y >
        (Game_State::s_num_tiles_per_screen_y * Tile_Map::s_tile_size_meters) / 2) {
        camera_.pos.abs_tile_y += Game_State::s_num_tiles_per_screen_y / 2;
    }
    if (entity_dif.dif_xy.y <
        (Game_State::s_num_tiles_per_screen_y * Tile_Map::s_tile_size_meters) / -2) {
        camera_.pos.abs_tile_y -= Game_State::s_num_tiles_per_screen_y / 2;
    }
}

Player_Actions
process_keyboard(const Game_Keyboard_Input &keyboard)
{
    Player_Actions result {};

    if (is_key_up(keyboard.t)) result.start = true;
    if (keyboard.w.ended_down) result.dir.y += 1.f;
    if (keyboard.s.ended_down) result.dir.y += -1.f;
    if (keyboard.a.ended_down) result.dir.x += -1.f;
    if (keyboard.d.ended_down) result.dir.x += 1.f;
    if (keyboard.left_shift.ended_down) result.sprint = true;

    return result;
}

Player_Actions
process_controller(const Game_Controller_Input &controller_)
{
    Player_Actions result {};

    if (is_button_up(controller_.button_start)) result.start = true;

    if (controller_.is_analog) {
        result.dir = controller_.end_left_stick;
    }

    if (controller_.button_A.ended_down) result.sprint = true;

    return result;
}

void
update_player(Entity &player_, const Player_Actions &player_actions_, Tile_Map &tile_map_,
              const f32 delta_time_, Game_State *game_state_ = nullptr)
{
    v2 r {};  // reflect vector
    v2 player_acc { player_actions_.dir };
    // NOTE: normalize vector to unit length
    f32 player_acc_length = math::length_sq(player_acc);
    if (player_acc_length > 1.0f) player_acc *= (1.f / math::sqrt_f32(player_acc_length));
    f32 player_speed = player_actions_.sprint ? 50.f : 25.f;
    player_acc *= player_speed;
    player_acc -= player_.vel * 10.f;

    auto old_player_pos { player_.pos };
    auto new_player_pos { player_.pos };
    v2 player_delta { (.5f * player_acc * math::square(delta_time_) + player_.vel * delta_time_) };
    player_.vel += player_acc * delta_time_;
    new_player_pos = offset_pos(new_player_pos, player_delta);

    u32 min_tile_x { math::min(old_player_pos.abs_tile_x, new_player_pos.abs_tile_x) };
    u32 min_tile_y { math::min(old_player_pos.abs_tile_y, new_player_pos.abs_tile_y) };
    u32 max_tile_x { math::max(old_player_pos.abs_tile_x, new_player_pos.abs_tile_x) };
    u32 max_tile_y { math::max(old_player_pos.abs_tile_y, new_player_pos.abs_tile_y) };

    u32 player_tile_width  = math::ceil_f32_to_s32(player_.width / Tile_Map::s_tile_size_meters);
    u32 player_tile_height = math::ceil_f32_to_s32(player_.height / Tile_Map::s_tile_size_meters);

    min_tile_x -= player_tile_width;
    min_tile_y -= player_tile_height;
    max_tile_x += player_tile_width;
    max_tile_y += player_tile_height;

    u32 abs_tile_z { player_.pos.abs_tile_z };
    f32 t_remain = 1.f;

    // NOTE: how many iterations/time resolution
    for (u32 i {}; i < 4 && t_remain >= 0.f; ++i) {
        f32 t_min { 1.f };
        v2 wall_nrm {};
        assert((max_tile_x - min_tile_x) < 32);
        assert((max_tile_y - min_tile_y) < 32);

        for (u32 abs_tile_y { min_tile_y }; abs_tile_y <= max_tile_y; ++abs_tile_y) {
            for (u32 abs_tile_x = { min_tile_x }; abs_tile_x <= max_tile_x; ++abs_tile_x) {
                Tile_Map_Pos test_tile_pos { get_centered_tile_point(abs_tile_x, abs_tile_y,
                                                                     abs_tile_z) };
                u32 tile_value { get_tile_value(tile_map_, test_tile_pos) };
                if (!is_tile_value_empty(tile_value)) {
                    // NOTE: Minkowski sum
                    f32 radius_w = Tile_Map::s_tile_size_meters + player_.width;
                    f32 radius_h = Tile_Map::s_tile_size_meters + player_.height;

                    v2 min_corner { -.5f * v2 { radius_w, radius_h } };
                    v2 max_corner { .5f * v2 { radius_w, radius_h } };

                    Tile_Map_Dif rel_old_player_pos = get_tile_diff(player_.pos, test_tile_pos);
                    v2 rel { rel_old_player_pos.dif_xy };

                    // TODO: maybe pull this out into a real function
                    auto test_wall = [&t_min](f32 wall_x, f32 rel_x, f32 rel_y, f32 player_delta_x,
                                              f32 player_delta_y, f32 min_y, f32 max_y) -> bool {
                        bool hit { false };

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

                    if (test_wall(min_corner.x, rel.x, rel.y, player_delta.x, player_delta.y,
                                  min_corner.y, max_corner.y)) {
                        wall_nrm = { -1.f, 0.f };
                    }
                    if (test_wall(max_corner.x, rel.x, rel.y, player_delta.x, player_delta.y,
                                  min_corner.y, max_corner.y)) {
                        wall_nrm = { 1.f, 0.f };
                    }
                    if (test_wall(min_corner.y, rel.y, rel.x, player_delta.y, player_delta.x,
                                  min_corner.x, max_corner.x)) {
                        wall_nrm = { 0.f, -1.f };
                    }
                    if (test_wall(max_corner.y, rel.y, rel.x, player_delta.y, player_delta.x,
                                  min_corner.x, max_corner.x)) {
                        wall_nrm = { 0.f, 1.f };
                    }
                }
            }
        }
        player_.pos = offset_pos(player_.pos, t_min * player_delta);
        player_.vel -= 1.f * math::inner(player_.vel, wall_nrm) * wall_nrm;
        player_delta -= 1.f * math::inner(player_delta, wall_nrm) * wall_nrm;
        t_remain -= t_min * t_remain;
    }

    player_.vel = player_acc * delta_time_ + player_.vel;
    entity_check_tile_map(tile_map_, player_);

    player_.stair_cd += delta_time_;
    // NOTE: checks to see if on stair tile which switches z levels
    if (player_.stair_cd > .5f &&
        get_tile_value(tile_map_, player_.pos.abs_tile_x, player_.pos.abs_tile_y,
                       player_.pos.abs_tile_z) == 3) {
        player_.pos.abs_tile_z == 0 ? player_.pos.abs_tile_z = 1 : player_.pos.abs_tile_z = 0;
        player_.stair_cd = 0.f;
    }

    v2 pv = player_.vel;
    if (math::abs_f32(pv.x) > math::abs_f32(pv.y)) {
        pv.x > 0.f ? player_.direction = Dir::right : player_.direction = Dir::left;
    } else if (math::abs_f32(pv.y) > math::abs_f32(pv.x)) {
        pv.y > 0.f ? player_.direction = Dir::up : player_.direction = Dir::down;
    }
}

}  // namespace

void *
push_size(Mem_Arena *arena_, mem_ind size_)
{
    assert((arena_->used + size_) <= arena_->size);
    void *result = arena_->base + arena_->used;
    arena_->used += size_;

    return result;
}

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    auto *state = (Game_State *)memory_.permanent_storage;
    game_ouput_sound(sound_buffer_);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    assert(sizeof(Game_State) <= memory_.permanent_storage_size);

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    auto &game_state = (Game_State &)(*(Game_State *)memory_.permanent_storage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory_.is_initialized) {
        const char *bg            = "uv_color_squares_960x540";
        const char *seaside_cliff = "bg_seaside_cliff";

        const char *player_front = "shitty_link_front";
        const char *player_back  = "shitty_link_back";
        const char *player_left  = "shitty_link_left";
        const char *player_right = "shitty_link_right";

        // game_state.bitmap   = load_bmp(thread_, memory_.platfrom_read_entire_file, bmp_path);
        game_state.bg_img = load_ARGB(thread_, memory_.platfrom_read_entire_file, bg);
        game_state.seaside_cliff =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, seaside_cliff);

        game_state.crosshair_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "crosshair");

        game_state.red_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "red_square");
        game_state.green_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "green_square");
        game_state.blue_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "blue_square");

        init_arena(&game_state.world_arena, memory_.permanent_storage_size - sizeof(game_state),
                   (u8 *)memory_.permanent_storage + sizeof(game_state));

        game_state.world = PushStruct(&game_state.world_arena, World);

        World *world       = game_state.world;
        world->tile_map    = PushStruct(&game_state.world_arena, Tile_Map);
        Tile_Map *tile_map = world->tile_map;
        tile_map->tile_chunks =
            PushArray(&game_state.world_arena,
                      Tile_Map::s_chunk_count * Tile_Map::s_chunk_count * Tile_Map::s_chunk_count_z,
                      Tile_Chunk);

// NOTE: very basic level generator
#if 0
        //TODO: sparse storage
        u32 screen_y {UINT32_MAX / 2};
        u32 screen_x {UINT32_MAX / 2};
#else
        u32 screen_y {};
        u32 screen_x {};
#endif
        u32 screen_z {};
        u32 rng_ind {};

        bool door_left   = false;
        bool door_right  = false;
        bool door_top    = false;
        bool door_bottom = false;
        bool stairs      = false;
        bool stairs_back = false;

        for (u32 screen_ind {}; screen_ind < Game_State::s_num_screens; ++screen_ind) {
            u32 rng_choice = global::rng_table[rng_ind++] % 3;

            switch (rng_choice) {
                case 0: {
                    door_right = true;
                } break;
                case 1: {
                    door_top = true;
                } break;
                case 2: {
                    stairs_back ? door_top = true : stairs = true;
                } break;
            }

            for (u32 tile_y {}; tile_y < Game_State::s_num_tiles_per_screen_y; ++tile_y) {
                for (u32 tile_x {}; tile_x < Game_State::s_num_tiles_per_screen_x; ++tile_x) {
                    u32 abs_tile_x = screen_x * Game_State::s_num_tiles_per_screen_x + tile_x;
                    u32 abs_tile_y = screen_y * Game_State::s_num_tiles_per_screen_y + tile_y;
                    u32 tile_value = 1;

                    if (tile_x == 0 &&
                        !((tile_y == Game_State::s_num_tiles_per_screen_y / 2) && door_left)) {
                        tile_value = 2;
                    }
                    if (tile_x == Game_State::s_num_tiles_per_screen_x - 1 &&
                        !((tile_y == Game_State::s_num_tiles_per_screen_y / 2) && door_right)) {
                        tile_value = 2;
                    }
                    if (tile_y == 0 &&
                        !((tile_x == Game_State::s_num_tiles_per_screen_x / 2) && door_bottom)) {
                        tile_value = 2;
                    }

                    if (tile_y == Game_State::s_num_tiles_per_screen_y - 1 &&
                        !((tile_x == Game_State::s_num_tiles_per_screen_x / 2) && door_top)) {
                        tile_value = 2;
                    }

                    set_tile_value(&game_state.world_arena, *tile_map, abs_tile_x, abs_tile_y,
                                   screen_z, tile_value);
                }
            }

            if (stairs || stairs_back) {
                u32 mid_tile_x = screen_x * Game_State::s_num_tiles_per_screen_x +
                                 Game_State::s_num_tiles_per_screen_x / 2;
                u32 mid_tile_y = screen_y * Game_State::s_num_tiles_per_screen_y +
                                 Game_State::s_num_tiles_per_screen_y / 2;
                set_tile_value(&game_state.world_arena, *tile_map, mid_tile_x, mid_tile_y, screen_z,
                               3);
            }

            if (door_right) {
                ++screen_x;
            } else if (door_top) {
                ++screen_y;
            } else if (stairs) {
                screen_z == 0 ? screen_z = 1 : screen_z = 0;
            }

            stairs_back = stairs;
            door_left   = door_right;
            door_bottom = door_top;

            door_right = false;
            door_top   = false;
            stairs     = false;
        }

        // set_tile_value(&game_state.world_arena, *tile_map, 4, 0, screen_z, 1);
        // set_tile_value(&game_state.world_arena, *tile_map, 5, 0, screen_z, 1);
        // set_tile_value(&game_state.world_arena, *tile_map, 0, 3, screen_z, 1);
        // set_tile_value(&game_state.world_arena, *tile_map, 0, 4, screen_z, 1);

        game_state.camera.pos.abs_tile_x = Game_State::s_num_tiles_per_screen_x / 2;
        game_state.camera.pos.abs_tile_y = Game_State::s_num_tiles_per_screen_y / 2;

        game_state.entity_camera_follow_ind = 0;

        game_state.player_sprites[Dir::down] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_front);
        game_state.player_sprites[Dir::right] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_right);
        game_state.player_sprites[Dir::up] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_back);
        game_state.player_sprites[Dir::left] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_left);

        // NOTE: clearing out garbage data
        for (szt entity {}; entity < Game_State::s_max_entities; ++entity) {
            game_state.entities[entity].exists = false;
        }

        for (szt player {}; player < Game_Input::s_input_cnt; ++player) {
            init_player(game_state.entities[player], game_state.player_sprites);
        }

        // TODO: this might be more appropriate in the platform layer
        memory_.is_initialized = true;
    }

    // ===============================================================================================
    // #Start
    // ===============================================================================================

    auto &world  = game_state.world;
    auto &camera = game_state.camera;

    world->tile_map->cur_tile_chunk = get_tile_chunk(
        *world->tile_map, game_state.entities[0].pos.abs_tile_x,
        game_state.entities[0].pos.abs_tile_y, game_state.entities[0].pos.abs_tile_z);

    for (szt player {}; player < Game_Input::s_input_cnt; ++player) {
        Player_Actions player_action {};
        if (player == 0) {
            player_action = process_keyboard(input_.keyboard);
        } else {
            player_action = process_controller(input_.controllers[player - 1]);
        }

        auto &cur_player = game_state.entities[player];
        if (player_action.start) {
            cur_player.exists = !cur_player.exists;
            if (cur_player.exists) game_state.entity_camera_follow_ind = player;
        }

        if (cur_player.exists) {
            update_player(cur_player, player_action, *world->tile_map, input_.delta_time,
                          &game_state);
        }
    }

    game_state.entities[game_state.entity_camera_follow_ind];

    if (input_.keyboard.d1.ended_down) {
        if (game_state.entities[0].exists) game_state.entity_camera_follow_ind = 0;
    } else if (input_.keyboard.d2.ended_down) {
        if (game_state.entities[1].exists) game_state.entity_camera_follow_ind = 1;
    } else if (input_.keyboard.d3.ended_down) {
        if (game_state.entities[2].exists) game_state.entity_camera_follow_ind = 2;
    } else if (input_.keyboard.d4.ended_down) {
        if (game_state.entities[3].exists) game_state.entity_camera_follow_ind = 3;
    } else if (input_.keyboard.d5.ended_down) {
        if (game_state.entities[4].exists) game_state.entity_camera_follow_ind = 4;
    }

    camera.pos.abs_tile_z = game_state.entities[game_state.entity_camera_follow_ind].pos.abs_tile_z;
    move_camera(camera, game_state.entities[game_state.entity_camera_follow_ind]);

    // ===============================================================================================
    // #Draw
    // ===============================================================================================

    // NOTE: *not* using PatBlt in the win32 layer
    Color_u32 clear_color { 0xff'00'00'00 };
    clear_buffer(video_buffer_, clear_color);

    u32 *source = game_state.bg_img.pixel_ptr;
    u32 *dest   = (u32 *)video_buffer_.memory;

    for (u32 y {}; y < game_state.bg_img.height; ++y) {
        for (u32 x {}; x < game_state.bg_img.width; ++x) {
            *dest++ = *source++;
        }
    }

    // NOTE: caching for clarity, not perf

    s32 num_draw_tiles = 12;
    v2 screen_center { .5f * (f32)video_buffer_.width, .5f * (f32)video_buffer_.height };

    // draw_ARGB(video_buffer_, game_state.seaside_cliff, screen_center);

    for (s32 rel_y = -1 * num_draw_tiles; rel_y < num_draw_tiles; ++rel_y) {
        for (s32 rel_x = -1 * num_draw_tiles; rel_x < num_draw_tiles; ++rel_x) {
            u32 x = camera.pos.abs_tile_x + rel_x;
            u32 y = camera.pos.abs_tile_y + rel_y;

            u32 tile = get_tile_value(*world->tile_map, x, y, camera.pos.abs_tile_z);
            Color_u32 tile_color;
            if (tile == 3) {
                tile_color.argb = 0xff'1e'1e'1e;
            } else if (tile == 2) {
                tile_color.argb = 0xff'dd'dd'dd;
            } else if (tile == 1) {
                // continue;
                tile_color.argb = 0xff'88'88'88;
            } else {
                continue;
                tile_color.argb = 0xff'ff'00'00;
            }

            v2 center { (screen_center.x - (camera.pos.offset.x * global::s_meters_to_pixels) +
                         (f32)rel_x * global::s_tile_size_pixels),
                        (screen_center.y + (camera.pos.offset.y * global::s_meters_to_pixels) -
                         (f32)rel_y * global::s_tile_size_pixels) };

            v2 min { (center.x - .5f * global::s_tile_size_pixels),
                     (center.y - .5f * global::s_tile_size_pixels) };

            v2 max { (center.x + .5f * global::s_tile_size_pixels),
                     (center.y + .5f * global::s_tile_size_pixels) };

            draw_rect(video_buffer_, min.x, min.y, max.x, max.y, tile_color);
        }
    }

    for (szt player {}; player < Game_Input::s_input_cnt; ++player) {
        auto &cur_player = game_state.entities[player];

        if (cur_player.exists && camera.pos.abs_tile_z == cur_player.pos.abs_tile_z) {
            auto player_dif = get_tile_diff(cur_player.pos, camera.pos);

            v2 player_mid { (screen_center.x + (player_dif.dif_xy.x * global::s_meters_to_pixels)),
                            (screen_center.y -
                             (player_dif.dif_xy.y * global::s_meters_to_pixels)) };

            v2 argb_mid { player_mid.x, player_mid.y - 16 };

            // draw_ARGB(video_buffer_, cur_player.sprites[cur_player.direction], argb_mid);
            draw_ARGB(video_buffer_, game_state.crosshair_img, player_mid);
            draw_ARGB(video_buffer_, game_state.player_sprites[cur_player.direction], argb_mid);
        }
    }
    // NOTE: hacky way to draw a debug postion
    auto test_dif = get_tile_diff(game_state.test_pos, camera.pos);
    v2 test_mid { (screen_center.x + (test_dif.dif_xy.x * global::s_meters_to_pixels)),
                  (screen_center.y - (test_dif.dif_xy.y * global::s_meters_to_pixels)) };
    draw_ARGB(video_buffer_, game_state.crosshair_img, test_mid);

    // constexpr s32 square_offset = 0;
    // draw_ARGB(video_buffer_, game_state.red_square_img,
    //           { f32(64 + square_offset), f32(video_buffer_.height - 64) });
    // draw_ARGB(video_buffer_, game_state.green_square_img,
    //           { f32(64 + square_offset), f32(video_buffer_.height - 192) });
    // draw_ARGB(video_buffer_, game_state.blue_square_img,
    //           { f32(64 + square_offset), f32(video_buffer_.height - 320) });
}

#if 0
    #ifdef TOM_WIN32
namespace win32
{
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
        #endif
        #include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
}  // namespace win32

    #endif
#endif
}  // namespace tomato
