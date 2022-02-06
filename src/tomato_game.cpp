#include "tomato_game.hpp"

static constexpr u32 gs_tile_size_pixels = 40;
static constexpr f32 gs_meters_to_pixels = gs_tile_size_pixels / Tile_Map::s_tile_size_meters;

#include "rng_nums.h"

static void
clear_buffer(Game_Offscreen_Buffer &buffer_, Color_u32 color_ = { 0xff'ff'00'ff })
{
    auto width = buffer_.width;
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

static void
draw_rect(Game_Offscreen_Buffer &buffer_, f32 f32_min_x_, f32 f_min_y_, f32 f32_max_x_,
          f32 f32_max_y_, Color_u32 color_ = { 0xffffffff })
{
    s32 min_x = round_f32_to_s32(f32_min_x_);
    s32 min_y = round_f32_to_s32(f_min_y_);
    s32 max_x = round_f32_to_s32(f32_max_x_);
    s32 max_y = round_f32_to_s32(f32_max_y_);

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

static void
draw_ARGB(Game_Offscreen_Buffer &buffer_, ARGB_Img &img_, v2 pos_)
{
    s32 min_y = round_f32_to_s32(pos_.y - ((f32)img_.height / 2.f));
    s32 min_x = round_f32_to_s32(pos_.x - ((f32)img_.width / 2.f));
    s32 max_y = round_f32_to_s32(pos_.y + ((f32)img_.height / 2.f));
    s32 max_x = round_f32_to_s32(pos_.x + ((f32)img_.width / 2.f));

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

static Tile_Map_Pos
get_entity_center_pos(const Entity &entity_)
{
    // FIXME: dunno if this is working right
    return map_into_tile_space(entity_.low->pos,
                               { entity_.low->width / 2.f, entity_.low->height / 2.f });
}

#if 0
static void
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
#endif

static void
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

static void
init_arena(Mem_Arena *arena_, mem_ind size_, byt *base_)
{
    arena_->size = size_;
    arena_->base = base_;
    arena_->used = 0;
}

static Bitmap
load_bmp(Thread_Context *thread_, debug_platform_read_entire_file *read_entire_file_,
         const char *file_name_)
{
    debug_Read_File_Result read_result = read_entire_file_(thread_, file_name_);
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

static ARGB_Img
load_ARGB(Thread_Context *thread_, debug_platform_read_entire_file *read_entire_file_,
          const char *file_name_, const char *name_ = nullptr)
{
    const char *argb_dir = "T:/assets/argbs/";
    char img_path_buf[512];
    szt img_buf_len;
    cat_str(argb_dir, file_name_, &img_path_buf[0], &img_buf_len);
    img_path_buf[img_buf_len++] = '.';
    img_path_buf[img_buf_len++] = 'a';
    img_path_buf[img_buf_len++] = 'r';
    img_path_buf[img_buf_len++] = 'g';
    img_path_buf[img_buf_len++] = 'b';
    img_path_buf[img_buf_len++] = '\0';

    debug_Read_File_Result read_result = read_entire_file_(thread_, img_path_buf);
    ARGB_Img result;

    if (read_result.content_size != 0) {
        if (name_)
            result.name = name_;
        else
            result.name = file_name_;
        auto *file_ptr   = (u32 *)read_result.contents;
        result.width     = *file_ptr++;
        result.height    = *file_ptr++;
        result.size      = *file_ptr++;
        result.pixel_ptr = file_ptr;
    }
    return result;
}

static Player_Actions
process_keyboard(const Game_Keyboard_Input &keyboard_)
{
    Player_Actions result {};

    if (is_key_up(keyboard_.t)) result.start = true;
    if (keyboard_.w.ended_down) result.dir.y += 1.f;
    if (keyboard_.s.ended_down) result.dir.y += -1.f;
    if (keyboard_.a.ended_down) result.dir.x += -1.f;
    if (keyboard_.d.ended_down) result.dir.x += 1.f;
    if (keyboard_.left_shift.ended_down) result.sprint = true;

    return result;
}

static Player_Actions
process_controller(const Game_Controller_Input &controller_)
{
    Player_Actions result {};

    if (is_button_up(controller_.button_start)) result.start = true;

    if (controller_.is_analog) {
        result.dir = { controller_.end_left_stick_x, controller_.end_left_stick_y };
    }

    if (controller_.button_A.ended_down) result.sprint = true;

    return result;
}

inline High_Entity *
make_entity_high(Game_State &game_state_, u32 low_i_)
{
    High_Entity *high_ent = nullptr;
    assert(low_i_ < Game_State::s_max_low_cnt);
    Low_Entity *low_ent = game_state_.low_entities + low_i_;

    if (low_ent->high_i) {
        high_ent = game_state_.high_entities + low_ent->high_i;
    } else {
        if (game_state_.high_cnt < Game_State::s_max_high_cnt) {
            u32 high_i = game_state_.high_cnt++;
            assert(high_i < Game_State::s_max_high_cnt);
            high_ent = game_state_.high_entities + high_i;

            auto diff           = get_tile_diff(low_ent->pos, game_state_.camera.pos);
            high_ent->pos       = diff.dif_xy;
            high_ent->vel       = { 0.f, 0.f };
            high_ent->direction = 0;
            high_ent->low_i     = low_i_;
            low_ent->high_i     = high_i;
        } else {
            INVALID_CODE_PATH;
        }
    }

    return high_ent;
}

inline void
make_entity_low(Game_State &game_state_, u32 low_i_)
{
    assert(low_i_ < Game_State::s_max_low_cnt);
    Low_Entity &low_ent = game_state_.low_entities[low_i_];
    u32 high_i          = low_ent.high_i;
    if (high_i) {
        u32 last_high_ent = game_state_.high_cnt - 1;
        if (high_i != last_high_ent) {
            High_Entity *last_ent = game_state_.high_entities + last_high_ent;
            High_Entity *del_ent  = game_state_.high_entities + high_i;
            game_state_.low_entities[last_ent->low_i].high_i = high_i;
        }
        --game_state_.high_cnt;
        low_ent.high_i = 0;
    }
}

static Low_Entity *
get_low_entity(Game_State &game_state_, u32 low_i_)
{
    Low_Entity *result = nullptr;

    if (low_i_ > 0 && low_i_ < game_state_.low_cnt) {
        result = game_state_.low_entities + low_i_;
    }

    return result;
}

static Entity
get_high_entity(Game_State &game_state_, u32 low_i_)
{
    Entity result {};

    // TODO: allow 0 index and returning a nullptr?
    assert(low_i_ < Game_State::s_max_low_cnt);
    result.low_i = low_i_;
    result.low   = get_low_entity(game_state_, result.low_i);
    result.high  = make_entity_high(game_state_, result.low_i);

    return result;
}

static u32
add_low_entity(Game_State &game_state_, Entity_Type type_ = Entity_Type::none)
{
    assert(game_state_.low_cnt < Game_State::s_max_low_cnt);
    u32 low_i = game_state_.low_cnt++;

    game_state_.low_entities[low_i]      = {};
    game_state_.low_entities[low_i].type = type_;

    return low_i;
}

inline void
offset_and_check_entities_by_area(Game_State &game_state_, rect_v2 area_, v2 offset_)
{
    for (u32 high_i = 1; high_i < game_state_.high_cnt;) {
        High_Entity *high = game_state_.high_entities + high_i;
        high->pos += offset_;
        if (is_in_rect_v2(area_, high->pos)) {
            ++high_i;
        } else {
            make_entity_low(game_state_, high->low_i);
        }
    }
}

static u32
add_wall(Game_State &game_state_, u32 abs_x_, u32 abs_y_, u32 abs_z_, ARGB_Img *sprites_ = nullptr)
{
    u32 wall_i       = add_low_entity(game_state_, Entity_Type::wall);
    Low_Entity *wall = get_low_entity(game_state_, wall_i);

    wall->height         = Tile_Map::s_tile_size_meters;
    wall->width          = Tile_Map::s_tile_size_meters;
    wall->pos.abs_tile_x = abs_x_;
    wall->pos.abs_tile_y = abs_y_;
    wall->pos.abs_tile_z = abs_z_;
    wall->color          = { 0xff'dd'dd'dd };
    wall->sprites        = sprites_;
    wall->collides       = true;
    wall->barrier        = true;

    return wall_i;
}

static u32
add_stairs(Game_State &game_state_, u32 abs_x_, u32 abs_y_, u32 abs_z_,
           ARGB_Img *sprites_ = nullptr)
{
    u32 wall_i       = add_low_entity(game_state_, Entity_Type::stairs);
    Low_Entity *wall = get_low_entity(game_state_, wall_i);

    wall->height         = Tile_Map::s_tile_size_meters;
    wall->width          = Tile_Map::s_tile_size_meters;
    wall->pos.abs_tile_x = abs_x_;
    wall->pos.abs_tile_y = abs_y_;
    wall->pos.abs_tile_z = abs_z_;
    wall->color          = { 0xff'1e'1e'1e };
    wall->sprites        = sprites_;
    wall->collides       = true;
    wall->barrier        = false;

    return wall_i;
}

static void
init_player(Game_State &game_state_, u32 player_i_, u32 abs_x_, u32 abs_y_, u32 abs_z_,
            ARGB_Img *sprites_ = nullptr)
{
    // NOTE: the first 5 entities are reserved for players
    assert(player_i_ <= game_state_.player_cnt);
    if (player_i_ <= game_state_.player_cnt) {
        u32 player_i = add_low_entity(game_state_, Entity_Type::player);
        assert(player_i = player_i_);
        if (player_i == player_i_) {
            Low_Entity *player = get_low_entity(game_state_, player_i);
#if 0
            make_entity_high(game_state_, player_i);
            High_Entity *player_high = game_state_.high_entities + player->high_i;

            player_high->pos       = { 0.f, 0.f };
            player_high->direction = 0;
            player_high->stair_cd  = 0;
            player_high->vel       = {};
#endif
            player->height         = .6f;
            player->width          = 0.6f * player->height;
            player->pos.abs_tile_x = abs_x_;
            player->pos.abs_tile_y = abs_y_;
            player->pos.abs_tile_z = abs_z_;
            player->color          = { 0xff'00'00'ff };
            player->sprites        = sprites_;
            player->collides       = true;
        }
    }
}

static void
move_player(Entity player_, const Player_Actions &player_actions_, Tile_Map &tile_map_,
            const f32 dt_, Game_State &game_state_)
{
    v2 r          = {};  // reflect vector
    v2 player_acc = { player_actions_.dir };

    // NOTE: normalize vector to unit length
    f32 player_acc_length = length_sq(player_acc);
    f32 player_speed      = player_actions_.sprint ? 100.f : 50.f;

    if (player_acc_length > 1.f) player_acc *= (1.f / sqrt_f32(player_acc_length));
    player_acc *= player_speed;
    player_acc -= player_.high->vel * 10.f;  // drag/friction

    v2 player_delta   = { (.5f * player_acc * square(dt_) + player_.high->vel * dt_) };
    v2 new_player_pos = player_.high->pos + player_delta;
    player_.high->vel += player_acc * dt_;

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 t_min           = 1.0f;
        v2 wall_nrm         = {};
        u32 hit_ent_ind     = {};  // 0 is the null entity
        v2 desired_position = player_.high->pos + player_delta;

        // TODO: this is N * N bad
        for (u32 test_high_i = 1; test_high_i < game_state_.high_cnt; ++test_high_i) {
            if (test_high_i == player_.low->high_i) continue;  // don't test against self

            Entity test_ent;
            test_ent.high = game_state_.high_entities + test_high_i,
            test_ent.low  = game_state_.low_entities + test_ent.high->low_i;

            if (!test_ent.low->collides) continue;  // skip non-collision entities

            // NOTE: Minkowski sum
            f32 radius_w = player_.low->width + test_ent.low->width;
            f32 radius_h = player_.low->height + test_ent.low->height;

            v2 min_corner = { -.5f * v2 { radius_w, radius_h } };
            v2 max_corner = { .5f * v2 { radius_w, radius_h } };

            v2 rel = player_.high->pos - test_ent.high->pos;

            // TODO: maybe pull this out into a real function
            auto test_wall = [&t_min](f32 wall_x, f32 rel_x, f32 rel_y, f32 player_delta_x,
                                      f32 player_delta_y, f32 min_y, f32 max_y) -> bool {
                bool hit = false;

                f32 t_esp = .001f;
                if (player_delta_x != 0.f) {
                    f32 t_res = (wall_x - rel_x) / player_delta_x;
                    f32 y     = rel_y + t_res * player_delta_y;

                    if (t_res >= 0.f && (t_min > t_res)) {
                        if (y >= min_y && y <= max_y) {
                            t_min = max(0.f, t_res - t_esp);
                            hit   = true;
                        }
                    }
                }

                return hit;
            };

            if (test_wall(min_corner.x, rel.x, rel.y, player_delta.x, player_delta.y, min_corner.y,
                          max_corner.y)) {
                wall_nrm    = v2 { -1.f, 0.f };
                hit_ent_ind = test_high_i;
            }
            if (test_wall(max_corner.x, rel.x, rel.y, player_delta.x, player_delta.y, min_corner.y,
                          max_corner.y)) {
                wall_nrm    = v2 { 1.f, 0.f };
                hit_ent_ind = test_high_i;
            }
            if (test_wall(min_corner.y, rel.y, rel.x, player_delta.y, player_delta.x, min_corner.x,
                          max_corner.x)) {
                wall_nrm    = v2 { 0.f, -1.f };
                hit_ent_ind = test_high_i;
            }
            if (test_wall(max_corner.y, rel.y, rel.x, player_delta.y, player_delta.x, min_corner.x,
                          max_corner.x)) {
                wall_nrm    = v2 { 0.f, 1.f };
                hit_ent_ind = test_high_i;
            }
        }
        High_Entity *hit_high = game_state_.high_entities + hit_ent_ind;
        if (!game_state_.low_entities[hit_high->low_i].barrier) {
            wall_nrm = { 0.f, 0.f };
        }

        player_.high->pos += t_min * player_delta;
        if (hit_ent_ind) {
            player_.high->vel -= 1.f * inner(player_.high->vel, wall_nrm) * wall_nrm;
            player_delta -= 1.f * inner(player_delta, wall_nrm) * wall_nrm;

            if (player_.high->stair_cd > .5f &&
                game_state_.low_entities[hit_high->low_i].type == Entity_Type::stairs) {
                player_.low->virtual_z == 0  ? ++player_.low->pos.abs_tile_z,
                    ++player_.low->virtual_z : --player_.low->pos.abs_tile_z,
                    --player_.low->virtual_z;
                player_.high->stair_cd = 0.f;
            }
        } else {
            break;
        }
    }

    // player_.high->vel = player_acc * dt_ + player_.high->vel;
    // entity_check_tile_map(tile_map_, player_);
    player_.high->stair_cd += dt_;

    // NOTE: changes the players direction for the sprite
    v2 pv = player_.high->vel;
    if (abs_f32(pv.x) > abs_f32(pv.y)) {
        pv.x > 0.f ? player_.high->direction = Dir::right : player_.high->direction = Dir::left;
    } else if (abs_f32(pv.y) > abs_f32(pv.x)) {
        pv.y > 0.f ? player_.high->direction = Dir::up : player_.high->direction = Dir::down;
    }

    // TODO: map entities relative to camera space
    player_.low->pos = map_into_tile_space(game_state_.camera.pos, player_.high->pos);
}

static void
set_camera(Game_State &game_state_, Tile_Map_Pos new_cam_pos_)
{
    Tile_Map_Pos old_cam_pos = game_state_.camera.pos;
    Tile_Map_Dif dif_cam     = get_tile_diff(new_cam_pos_, old_cam_pos);
    game_state_.camera.pos   = new_cam_pos_;

    // NOTE: camera space defined here
    u32 tile_span_x = Game_State::s_num_tiles_per_screen_x * 3;
    u32 tile_span_y = Game_State::s_num_tiles_per_screen_y * 3;

    // offset and make all entities inside camera space high
    v2 test_screen_size { Tile_Map::s_tile_size_meters *
                          v2 { (f32)tile_span_x, (f32)tile_span_y } };
    rect_v2 cam_bounds = rect_v2_center_half_dim({ 0.f, 0.f }, test_screen_size);
    v2 ent_offset      = -dif_cam.dif_xy;
    offset_and_check_entities_by_area(game_state_, cam_bounds, ent_offset);

    // make all entities outside camera space lowk
    u32 min_tile_x = new_cam_pos_.abs_tile_x - tile_span_x / 2;
    u32 min_tile_y = new_cam_pos_.abs_tile_y - tile_span_y / 2;
    u32 max_tile_x = new_cam_pos_.abs_tile_x + tile_span_x / 2;
    u32 max_tile_y = new_cam_pos_.abs_tile_y + tile_span_y / 2;
    for (u32 low_i = 1; low_i < game_state_.low_cnt; ++low_i) {
        Low_Entity *low = game_state_.low_entities + low_i;
        if (low->high_i == 0) {
            if (low->pos.abs_tile_z == new_cam_pos_.abs_tile_z &&
                low->pos.abs_tile_x >= min_tile_x && low->pos.abs_tile_x <= max_tile_x &&
                low->pos.abs_tile_y >= min_tile_y && low->pos.abs_tile_y <= max_tile_y) {
                make_entity_high(game_state_, low_i);
            }
        }
    }
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
        // init memory
        init_arena(&game_state.world_arena, memory_.permanent_storage_size - sizeof(game_state),
                   (u8 *)memory_.permanent_storage + sizeof(game_state));

        game_state.world = PushStruct(&game_state.world_arena, World);

        World *world       = game_state.world;
        world->tile_map    = PushStruct(&game_state.world_arena, Tile_Map);
        Tile_Map *tile_map = world->tile_map;
        init_tile_map(*tile_map);

        const char *bg           = "uv_color_squares_960x540";
        const char *player_front = "0001";
        const char *player_back  = "0003";
        const char *player_left  = "0004";
        const char *player_right = "0002";

        // load textures
        game_state.player_sprites[Dir::down] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_front);
        game_state.player_sprites[Dir::right] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_right);
        game_state.player_sprites[Dir::up] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_back);
        game_state.player_sprites[Dir::left] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_left);
        game_state.bg_img = load_ARGB(thread_, memory_.platfrom_read_entire_file, bg);
        game_state.seaside_cliff =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "bg_seaside_cliff");
        game_state.crosshair_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "crosshair");
        game_state.tree_sprite =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "shitty_tree");
        game_state.stair_sprite = load_ARGB(thread_, memory_.platfrom_read_entire_file, "stairs");
        game_state.red_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "red_square");
        game_state.green_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "green_square");
        game_state.blue_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, "blue_square");

        u32 screen_base_x = ((UINT32_MAX / 2) / Game_State::s_num_tiles_per_screen_x) / 2;
        u32 screen_base_y = ((UINT32_MAX / 2) / Game_State::s_num_tiles_per_screen_y) / 2;
        u32 screen_base_z = UINT32_MAX / 4;
        u32 screen_x      = screen_base_x;
        u32 screen_y      = screen_base_y;
        u32 screen_z      = screen_base_z;
        s32 virtual_z     = 0;
        u32 rng_ind       = 0;

        // set world render size
        game_state.camera.pos.abs_tile_x = Game_State::s_num_tiles_per_screen_x * screen_x +
                                           Game_State::s_num_tiles_per_screen_x / 2;
        game_state.camera.pos.abs_tile_y = Game_State::s_num_tiles_per_screen_y * screen_y +
                                           Game_State::s_num_tiles_per_screen_y / 2;

        // NOTE: entity 0 is the null entity
        add_low_entity(game_state, Entity_Type::null);
        game_state.player_cnt               = Game_Input::s_input_cnt;
        game_state.entity_camera_follow_ind = 1;

        // add the player entites
        for (u32 player_i = 1; player_i <= game_state.player_cnt; ++player_i) {
            init_player(game_state, player_i, screen_x, screen_y, screen_z,
                        game_state.player_sprites);
        }

        // NOTE: very basic level generator

        bool door_left   = false;
        bool door_right  = false;
        bool door_top    = false;
        bool door_bottom = false;
        bool stairs      = false;
        bool stairs_back = false;

        for (u32 screen_ind = 0; screen_ind < Game_State::s_num_screens; ++screen_ind) {
            u32 rng_choice = rng_table[rng_ind++] % 3;

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

            for (u32 tile_y = 0; tile_y < Game_State::s_num_tiles_per_screen_y; ++tile_y) {
                for (u32 tile_x = 0; tile_x < Game_State::s_num_tiles_per_screen_x; ++tile_x) {
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

                    if (tile_value == 2)
                        // ACTUALLY TREES
                        add_wall(game_state, abs_tile_x, abs_tile_y, screen_z,
                                 &game_state.tree_sprite);
                }
            }

            if (stairs || stairs_back) {
                u32 mid_tile_x = screen_x * Game_State::s_num_tiles_per_screen_x +
                                 Game_State::s_num_tiles_per_screen_x / 2;
                u32 mid_tile_y = screen_y * Game_State::s_num_tiles_per_screen_y +
                                 Game_State::s_num_tiles_per_screen_y / 2;
                set_tile_value(&game_state.world_arena, *tile_map, mid_tile_x, mid_tile_y, screen_z,
                               3);
                add_stairs(game_state, mid_tile_x, mid_tile_y, screen_z, &game_state.stair_sprite);
            }

            if (door_right) {
                ++screen_x;
            } else if (door_top) {
                ++screen_y;
            } else if (stairs) {
                virtual_z == 0 ? ++screen_z, ++virtual_z : --screen_z, --virtual_z;
            }

            stairs_back = stairs;
            door_left   = door_right;
            door_bottom = door_top;

            door_right = false;
            door_top   = false;
            stairs     = false;
        }

        // TODO: this might be more appropriate in the platform layer
        memory_.is_initialized = true;
    }

    // ===============================================================================================
    // #Start
    // ===============================================================================================

    auto &world  = game_state.world;
    auto &camera = game_state.camera;

    for (u32 player_i = 1; player_i <= game_state.player_cnt; ++player_i) {
        Low_Entity *low_player = get_low_entity(game_state, player_i);
        assert(low_player->type == Entity_Type::player);

        Player_Actions player_action {};
        if (player_i == 1) {
            player_action = process_keyboard(input_.keyboard);
        } else {
            player_action = process_controller(input_.controllers[player_i - 2]);
        }
        if (player_action.start) {
            if (!low_player->high_i)
                make_entity_high(game_state, player_i);
            else
                make_entity_low(game_state, player_i);
        }

        if (low_player->high_i) {
            // TODO: get rid of Entity or make a helper func?
            Entity player_ent = get_high_entity(game_state, player_i);
            move_player(player_ent, player_action, *world->tile_map, input_.delta_time, game_state);
        }
    }

    if (input_.keyboard.d1.ended_down) {
        if (game_state.low_entities[1].high_i) game_state.entity_camera_follow_ind = 1;
    } else if (input_.keyboard.d2.ended_down) {
        if (game_state.low_entities[2].high_i) game_state.entity_camera_follow_ind = 2;
    } else if (input_.keyboard.d3.ended_down) {
        if (game_state.low_entities[3].high_i) game_state.entity_camera_follow_ind = 3;
    } else if (input_.keyboard.d4.ended_down) {
        if (game_state.low_entities[4].high_i) game_state.entity_camera_follow_ind = 4;
    } else if (input_.keyboard.d5.ended_down) {
        if (game_state.low_entities[5].high_i) game_state.entity_camera_follow_ind = 5;
    }

    Entity cam_ent = get_high_entity(game_state, game_state.entity_camera_follow_ind);

    Tile_Map_Dif entity_dif = get_tile_diff(cam_ent.low->pos, camera.pos);

    camera.pos.abs_tile_z = cam_ent.low->pos.abs_tile_z;

    Tile_Map_Pos new_cam_pos = camera.pos;
    if (entity_dif.dif_xy.x >
        (Game_State::s_num_tiles_per_screen_x * Tile_Map::s_tile_size_meters) / 2) {
        new_cam_pos.abs_tile_x += Game_State::s_num_tiles_per_screen_x / 2;
    }
    if (entity_dif.dif_xy.x <
        (Game_State::s_num_tiles_per_screen_x * Tile_Map::s_tile_size_meters) / -2) {
        new_cam_pos.abs_tile_x -= Game_State::s_num_tiles_per_screen_x / 2;
    }
    if (entity_dif.dif_xy.y >
        (Game_State::s_num_tiles_per_screen_y * Tile_Map::s_tile_size_meters) / 2) {
        new_cam_pos.abs_tile_y += Game_State::s_num_tiles_per_screen_y / 2;
    }
    if (entity_dif.dif_xy.y <
        (Game_State::s_num_tiles_per_screen_y * Tile_Map::s_tile_size_meters) / -2) {
        new_cam_pos.abs_tile_y -= Game_State::s_num_tiles_per_screen_y / 2;
    }

    set_camera(game_state, new_cam_pos);

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
    v2 screen_center   = { .5f * (f32)video_buffer_.width, .5f * (f32)video_buffer_.height };

    for (s32 rel_y = -1 * num_draw_tiles; rel_y < num_draw_tiles; ++rel_y) {
        for (s32 rel_x = -1 * num_draw_tiles; rel_x < num_draw_tiles; ++rel_x) {
            u32 x = camera.pos.abs_tile_x + rel_x;
            u32 y = camera.pos.abs_tile_y + rel_y;

            u32 tile = get_tile_value(*world->tile_map, x, y, camera.pos.abs_tile_z);
            Color_u32 tile_color;
            if (tile == 3) {
                tile_color.argb = 0xff'2e'2e'2e;
            } else if (tile == 2) {
                tile_color.argb = 0xff'dd'dd'dd;
            } else if (tile == 1) {
                // continue;
                tile_color.argb = 0xff'88'88'88;
            } else {
                continue;
                tile_color.argb = 0xff'ff'00'00;
            }

            v2 center { (screen_center.x - (camera.pos.offset.x * gs_meters_to_pixels) +
                         (f32)rel_x * gs_tile_size_pixels),
                        (screen_center.y + (camera.pos.offset.y * gs_meters_to_pixels) -
                         (f32)rel_y * gs_tile_size_pixels) };

            v2 min { (center.x - .5f * gs_tile_size_pixels),
                     (center.y - .5f * gs_tile_size_pixels) };

            v2 max { (center.x + .5f * gs_tile_size_pixels),
                     (center.y + .5f * gs_tile_size_pixels) };

            draw_rect(video_buffer_, min.x, min.y, max.x, max.y, tile_color);
        }
    }

    for (u32 high_i { 1 }; high_i < game_state.high_cnt; ++high_i) {
        // TODO: seems a bit convoluted...
        Entity ent {};
        ent.high  = game_state.high_entities + high_i;
        ent.low_i = ent.high->low_i;
        ent.low   = game_state.low_entities + ent.low_i;

        // cur_ent.high->pos += ent_offset;
        auto ent_dif = get_tile_diff(ent.low->pos, camera.pos);
        v2 ent_mid   = { (screen_center.x + (ent_dif.dif_xy.x * gs_meters_to_pixels)),
                       (screen_center.y - (ent_dif.dif_xy.y * gs_meters_to_pixels)) };
        v2 argb_mid  = { ent_mid.x, ent_mid.y - 16 };

        draw_ARGB(video_buffer_, ent.low->sprites[ent.high->direction], argb_mid);
    }

#if 0
    // NOTE: hacky way to draw a debug postion
    auto test_dif = get_tile_diff(game_state.test_pos, camera.pos);
    v2 test_mid  = { (screen_center.x + (test_dif.dif_xy.x * gs_meters_to_pixels)),
                  (screen_center.y - (test_dif.dif_xy.y * gs_meters_to_pixels)) };
    draw_ARGB(video_buffer_, game_state.crosshair_img, test_mid);
#endif
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
