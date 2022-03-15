#include "game.hpp"
#include "rng_nums.h"

namespace tom
{

internal void
clear_buffer(game_offscreen_buffer &buffer, const color color = colors::pink)
{
    const s32 width  = buffer.width;
    const s32 height = buffer.height;

    byt *row = scast(byt *, buffer.memory);
    for (s32 y = 0; y < height; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = 0; x < width; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

internal void
draw_rect(game_offscreen_buffer &buffer, const f32 min_x_f32, const f32 min_y_f32,
          const f32 max_x_f32, const f32 max_y_f32, const color color = colors::pink)
{
    s32 min_x = math::round_f32_to_s32(min_x_f32);
    s32 min_y = math::round_f32_to_s32(min_y_f32);
    s32 max_x = math::round_f32_to_s32(max_x_f32);
    s32 max_y = math::round_f32_to_s32(max_y_f32);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row = scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch;

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = min_x; x < max_x; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

internal void
draw_rect(game_offscreen_buffer &buffer, const rect rect, const color color = colors::pink)
{
    s32 min_x = math::round_f32_to_s32(rect.min.x);
    s32 min_y = math::round_f32_to_s32(rect.min.y);
    s32 max_x = math::round_f32_to_s32(rect.max.x);
    s32 max_y = math::round_f32_to_s32(rect.max.y);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row = scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch;

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = min_x; x < max_x; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

internal void
draw_rect_outline(game_offscreen_buffer &buffer, const f32 min_x_f32, const f32 min_y_f32,
                  const f32 max_x_f32, const f32 max_y_f32, const s32 thickness,
                  const color color = colors::pink)
{
    s32 min_x = math::round_f32_to_s32(min_x_f32);
    s32 min_y = math::round_f32_to_s32(min_y_f32);
    s32 max_x = math::round_f32_to_s32(max_x_f32);
    s32 max_y = math::round_f32_to_s32(max_y_f32);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row = scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch;

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = min_x; x < max_x; ++x) {
            if (x <= min_x + thickness || x >= max_x - thickness - 1 || y <= min_y + thickness ||
                y >= max_y - thickness - 1) {
                *pixel = color.argb;
            }
            ++pixel;
        }
        row += buffer.pitch;
    }
}

internal void
draw_argb(game_offscreen_buffer &buffer, const argb_img &img, const v2 pos)
{
    s32 min_y = math::round_f32_to_s32(pos.y - (scast(f32, img.height) / 2.f));
    s32 min_x = math::round_f32_to_s32(pos.x - (scast(f32, img.width) / 2.f));
    s32 max_y = math::round_f32_to_s32(pos.y + (scast(f32, img.height) / 2.f));
    s32 max_x = math::round_f32_to_s32(pos.x + (scast(f32, img.width) / 2.f));

    s32 x_offset_left = 0, x_offset_right = 0, y_offset = 0;

    if (min_y < 0) {
        y_offset = min_y * -1;
        min_y    = 0;
    }
    if (min_x < 0) {
        x_offset_left = min_x * -1;
        min_x         = 0;
    }
    if (max_x > buffer.width) {
        x_offset_right = max_x - buffer.width;
        max_x          = buffer.width;
    }
    if (max_y > buffer.height) max_y = buffer.height;

    u32 *source = img.pixel_ptr + (y_offset * img.width);
    byt *row = scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch;

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *dest = rcast(u32 *, row);
        source += x_offset_left;
        for (s32 x = min_x; x < max_x; ++x) {
            color dest_col   = { *dest };
            color source_col = { *source };
            color blended_col;
            blended_col.a = 0xff;

            f32 alpha = scast(f32, source_col.a) / 255.f;

            blended_col.r = scast(
                u8, (1.f - alpha) * scast(f32, dest_col.r) + alpha * scast(f32, source_col.r));
            blended_col.g = scast(
                u8, (1.f - alpha) * scast(f32, dest_col.g) + alpha * scast(f32, source_col.g));
            blended_col.b = scast(
                u8, (1.f - alpha) * scast(f32, dest_col.b) + alpha * scast(f32, source_col.b));

            *dest = blended_col.argb;

            ++dest, ++source;
        }
        source += x_offset_right;
        row += buffer.pitch;
    }
}

internal void
push_piece(entity_visble_piece_group *group, argb_img *img, const v2 mid_p, const f32 z_offset,
           const f32 alpha = 1.0f)
{
    TOM_ASSERT(group->piece_cnt < ARRAY_COUNT(group->pieces));
    entity_visible_piece *piece = group->pieces + group->piece_cnt++;
    piece->img                  = img;
    piece->mid_p                = mid_p;
    piece->z                    = z_offset;
    piece->alpha                = alpha;
}

internal void
push_piece(entity_visble_piece_group *group, const f32 width, const f32 height, const color color,
           const v2 mid_p, const f32 z_offset, const f32 alpha = 1.0f)
{
    TOM_ASSERT(group->piece_cnt < ARRAY_COUNT(group->pieces));
    entity_visible_piece *piece = group->pieces + group->piece_cnt++;
    piece->img                  = nullptr;
    piece->mid_p                = mid_p;
    piece->z                    = z_offset;
    piece->alpha                = alpha;
    piece->rect.min.x           = mid_p.x - width / 2;
    piece->rect.min.y           = mid_p.y - height / 2;
    piece->rect.max.x           = mid_p.x + width / 2;
    piece->rect.max.y           = mid_p.y + height / 2;
    piece->color                = color;
}

internal void
game_output_sound(game_sound_output_buffer &sound_buffer)
{
    // NOTE: outputs nothing atm
    s16 sample_value = 0;
    s16 *sampleOut   = sound_buffer.samples;
    for (szt sampleIndex = 0; sampleIndex < sound_buffer.sample_count; ++sampleIndex) {
        *sampleOut++ = sample_value;
        *sampleOut++ = sample_value;
    }
}

internal bitmap_img
load_bmp(thread_context *thread, debug_platform_read_entire_file *read_entire_file,
         const char *file_name)
{
    debug_read_file_result read_result = read_entire_file(thread, file_name);
    bitmap_img result;

    if (read_result.content_size != 0) {
        auto *header  = scast(bitmap_header *, read_result.contents);
        u32 *pixels   = rcast(u32 *, (scast(byt *, read_result.contents) + header->bitmap_offset));
        result.width  = header->width;
        result.height = header->height;
        result.pixel_ptr = pixels;
    }

    return result;
}

internal argb_img
load_argb(thread_context *thread, debug_platform_read_entire_file *read_entire_file,
          const char *file_name, const char *name = nullptr)
{
    const char *argb_dir = "T:/assets/argbs/";
    char img_path_buf[512];
    szt img_buf_len;
    cat_str(argb_dir, file_name, &img_path_buf[0], &img_buf_len);
    img_path_buf[img_buf_len++] = '.';
    img_path_buf[img_buf_len++] = 'a';
    img_path_buf[img_buf_len++] = 'r';
    img_path_buf[img_buf_len++] = 'g';
    img_path_buf[img_buf_len++] = 'b';
    img_path_buf[img_buf_len++] = '\0';

    debug_read_file_result read_result = read_entire_file(thread, img_path_buf);
    argb_img result;

    TOM_ASSERT(read_result.content_size != 0);
    if (read_result.content_size != 0) {
        if (name)
            result.name = name;
        else
            result.name = file_name;

        u32 *file_ptr    = scast(u32 *, read_result.contents);
        result.width     = *file_ptr++;
        result.height    = *file_ptr++;
        result.size      = *file_ptr++;
        result.pixel_ptr = file_ptr;
    }

    return result;
}

internal void
process_keyboard(const game_keyboard_input &keyboard, entity_actions *entity_action)
{
    if (entity_action) {
        if (is_key_up(keyboard.t)) entity_action->start = true;
        if (keyboard.space.ended_down) entity_action->jump = true;
        if (keyboard.w.ended_down) entity_action->dir.y += 1.f;
        if (keyboard.s.ended_down) entity_action->dir.y += -1.f;
        if (keyboard.a.ended_down) entity_action->dir.x += -1.f;
        if (keyboard.d.ended_down) entity_action->dir.x += 1.f;
        if (keyboard.left_shift.ended_down) entity_action->sprint = true;
    }
}

internal void
process_controller(const game_controller_input &controller, entity_actions *entity_action)
{
    if (entity_action) {
        if (is_button_up(controller.button_start)) entity_action->start = true;
        if (is_button_up(controller.button_a)) entity_action->attack = true;
        if (controller.button_b.ended_down) entity_action->sprint = true;

        constexpr f32 stick_deadzone = 0.1f;
        if (controller.is_analog && (math::abs_f32(controller.end_left_stick_x) > stick_deadzone ||
                                     math::abs_f32(controller.end_left_stick_y) > stick_deadzone)) {
            entity_action->dir = { controller.end_left_stick_x, controller.end_left_stick_y };
        }
    }
}

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    auto *state = (game_state *)memory.permanent_storage;
    game_output_sound(sound_buffer);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    TOM_ASSERT(sizeof(game_state) <= memory.permanent_storage_size);

    auto state   = (game_state *)memory.permanent_storage;
    world *world = nullptr;

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory.is_initialized) {
        // init memory
        init_arena(&state->world_arena, memory.permanent_storage_size - sizeof(*state),
                   (u8 *)memory.permanent_storage + sizeof(*state));

        state->world = PUSH_STRUCT(&state->world_arena, tom::world);

        world = state->world;
        TOM_ASSERT(world);
        init_world(world, global::tile_size_meters);

        state->debug_draw_collision = false;

        const char *bg = "uv_color_squares_960x540";

        // load textures
        state->player_sprites[entity_direction::north] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_n");
        state->player_sprites[entity_direction::east] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_e");
        state->player_sprites[entity_direction::south] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_s");
        state->player_sprites[entity_direction::west] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_w");

        state->monster_sprites[entity_direction::north] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_n");
        state->monster_sprites[entity_direction::east] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_e");
        state->monster_sprites[entity_direction::south] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_s");
        state->monster_sprites[entity_direction::west] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_w");

        state->sword_sprites[entity_direction::north] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_n");
        state->sword_sprites[entity_direction::east] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_e");
        state->sword_sprites[entity_direction::south] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_s");
        state->sword_sprites[entity_direction::west] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_w");

        state->cat_sprites[0] = load_argb(thread, memory.platfrom_read_entire_file, "cat_e");
        state->cat_sprites[1] = load_argb(thread, memory.platfrom_read_entire_file, "cat_w");

        state->bg_img        = load_argb(thread, memory.platfrom_read_entire_file, bg);
        state->crosshair_img = load_argb(thread, memory.platfrom_read_entire_file, "crosshair");
        state->tree_sprite   = load_argb(thread, memory.platfrom_read_entire_file, "shitty_tree");
        state->stair_sprite  = load_argb(thread, memory.platfrom_read_entire_file, "stairs");

        s32 screen_base_x {}, screen_base_y {}, screen_base_z {}, virtual_z {}, rng_ind {};
        s32 screen_x { screen_base_x }, screen_y { screen_base_y }, screen_z { screen_base_z };

        // set world render size
        state->camera.pos.chunk_x = 0;
        state->camera.pos.chunk_y = 0;
        state->camera.pos.offset.x += screen_base_x / 2.f;
        state->camera.pos.offset.y += screen_base_y / 2.f;

        // NOTE: entity 0 is the null entity
        // TODO: do I even need this?
        add_new_entity(state);
        // state->player_cnt               = Game_Input::s_input_cnt;
        state->player_cnt               = 1;
        state->entity_camera_follow_ind = 1;

        // add the player entites
        for (u32 player_i = 1; player_i <= state->player_cnt; ++player_i) {
            add_player(state, player_i, 0.f, 0.f, 0.f);
        }

        f32 x_len = 55.f;
        for (f32 x = -20.f; x < x_len; ++x) {
            add_tree(state, x, 15, 0.0f);
            add_tree(state, x, -5, 0.0f);
            if (scast(s32, x) % 17 == 0) continue;
            add_tree(state, x, 5, 0.0f);
        }

        for (f32 x = -20.f; x <= x_len; x += 15.f) {
            for (f32 y = -5; y < 15; ++y) {
                if ((y == 0.f || y == 10.f) && (x != -20.f && x != 55.f)) continue;
                add_tree(state, x, y, 0.0);
            }
        }

        add_monster(state, 5.f, 0.f, 0.f);
        add_cat(state, -1.f, 1.f, 0.f);

        // TODO: this might be more appropriate in the platform layer
        memory.is_initialized = true;
    }

    // ===============================================================================================
    // #START
    // ===============================================================================================

    camera *cam = &state->camera;
    auto p1     = get_entity(state, 1);

    // get input
    // NOTE: only doing one player
    entity *player = p1;
    assert(player->type == entity_type::player);
    entity_actions player_action = {};
    process_keyboard(input.keyboard, &player_action);
    process_controller(input.controllers[0], &player_action);
    state->player_acts[1] = player_action;

    // player sword attack
    // TODO:  update this to sim region stuff
#if 0
    if (state->player_acts[1].attack) {
        if (!p1.high->is_attacking) {
            p1.high->is_attacking = true;
            entity sword          = force_entity_into_high(state, p1.low->weapon_i);
            sword.high->dir       = p1.high->dir;
            sword.low->active     = true;
            // TODO: player weapon pos offset
            sword.high->pos    = p1.high->pos;
            p1.high->attack_cd = 0.5f;
        }
    }
    if (p1.high->is_attacking) {
        p1.high->attack_cd -= input.delta_time;
        entity sword = force_entity_into_high(state, p1.low->weapon_i);
        update_sword(state, sword, input.delta_time);
        if (p1.high->attack_cd <= 0.f) {
            p1.high->is_attacking = false;
            sword.low->active     = false;
        }
    }
#endif

    if (is_key_up(input.keyboard.d1)) state->debug_draw_collision = !state->debug_draw_collision;

    entity *cam_ent      = get_entity(state, state->entity_camera_follow_ind);
    world_dif entity_dif = get_world_diff(cam_ent->world_pos, cam->pos);

    cam->pos.chunk_z = cam_ent->world_pos.chunk_z;

    // NOTE: cam is following the player
    world_pos new_cam_pos     = p1->world_pos;
    f32 screen_test_size_mult = 2.f;
    v2 test_screen_size       = { (global::screen_size_x * screen_test_size_mult),
                            global::screen_size_y * screen_test_size_mult };
    rect cam_bounds           = rec::center_half_dim({ 0.f, 0.f }, test_screen_size);

    memory_arena sim_arena;
    init_arena(&sim_arena, memory.transient_storage_size, memory.transient_storage);

    sim_region *region = begin_sim(&sim_arena, state, cam->pos, cam_bounds);

    v2 screen_center = { .5f * (f32)video_buffer.width, .5f * (f32)video_buffer.height };
    entity_visble_piece_group piece_group = {};

    // NOTE: *not* using PatBlt in the win32 layer
    color clear_color { 0xff'4e'4e'4e };
    clear_buffer(video_buffer, clear_color);

    for (sim_entity *sim_ent = region->sim_entities;
         sim_ent != region->sim_entities + region->sim_entity_cnt; ++sim_ent) {
        piece_group.piece_cnt = 0;
        entity *ent           = get_entity(state, sim_ent->ent_i);
        TOM_ASSERT(ent);  // there theoretically shoudln't be a null entities

        // TODO: abstract active check into func?
        if (!is_flag_set(ent->sim.flags, sim_entity_flags::active))
            continue;  // don't draw inactive entities

        auto ent_dif = get_world_diff(ent->world_pos, cam->pos);
        v2 ent_mid   = { (screen_center.x + (ent_dif.dif_xy.x * global::meters_to_pixels)),
                       (screen_center.y - (ent_dif.dif_xy.y * global::meters_to_pixels)) };

        // TODO: pull this out?
        auto push_hp = [](entity_visble_piece_group *piece_group, entity *ent, v2 argb_mid) {
            for (u32 i = 0; i < ent->sim.hp; ++i) {
                push_piece(piece_group, 3.f, 6.f, { colors::red },
                           v2 { argb_mid.x - (ent->sim.width / 2.f) * global::meters_to_pixels -
                                    10.f + scast(f32, i) * 4.f,
                                argb_mid.y - ent->sim.height * global::meters_to_pixels - 10.f },
                           ent->sim.z);
            }
        };

        switch (ent->type) {
            case entity_type::none: {
                draw_rect(
                    video_buffer, ent_mid.x - (ent->sim.width * global::meters_to_pixels) / 2.f,
                    ent_mid.y - (ent->sim.height * global::meters_to_pixels) / 2.f,
                    ent_mid.x + (ent->sim.width * global::meters_to_pixels) / 2.f,
                    ent_mid.y + (ent->sim.height * global::meters_to_pixels) / 2.f, { 0xffff00ff });
            } break;
            case entity_type::player: {
                // TODO: get player index from entity?
                update_player(state, region, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent->sim.argb_offset };
                push_piece(&piece_group, ent->sprite, argb_mid, ent->sim.z);
                push_hp(&piece_group, ent, argb_mid);

            } break;
            case entity_type::wall: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent->sim.argb_offset };
                push_piece(&piece_group, ent->sprite, argb_mid, ent->sim.z);
            } break;
            case entity_type::stairs: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent->sim.argb_offset };
                push_piece(&piece_group, ent->sprite, argb_mid, ent->sim.z);
            } break;
            case entity_type::familiar: {
                update_familiar(state, region, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent->sim.argb_offset };
                push_piece(&piece_group, ent->sprite, argb_mid, ent->sim.z);
            } break;
            case entity_type::monster: {
                update_monster(state, region, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent->sim.argb_offset };
                push_piece(&piece_group, ent->sprite, argb_mid, ent->sim.z);
                push_hp(&piece_group, ent, argb_mid);
            } break;
            case entity_type::sword: {
                update_monster(state, region, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent->sim.argb_offset };
                push_piece(&piece_group, ent->sprite, argb_mid, ent->sim.z);
            } break;
            default: {
                INVALID_CODE_PATH;
            } break;
        }

        // ===============================================================================================
        // #DRAW
        // ===============================================================================================

        for (u32 piece_i = 0; piece_i < piece_group.piece_cnt; ++piece_i) {
            entity_visible_piece *piece = &piece_group.pieces[piece_i];
            if (piece->img) {
                draw_argb(video_buffer, *piece->img, piece->mid_p);
            } else {
                draw_rect(video_buffer, piece->rect, { colors::red });
            }
        }

        // NOTE:collision box
        if (state->debug_draw_collision) {
            draw_rect_outline(
                video_buffer, ent_mid.x - (ent->sim.width * global::meters_to_pixels) / 2.f,
                ent_mid.y - (ent->sim.height * global::meters_to_pixels) / 2.f,
                ent_mid.x + (ent->sim.width * global::meters_to_pixels) / 2.f,
                ent_mid.y + (ent->sim.height * global::meters_to_pixels) / 2.f, 1, { 0xffff0000 });
        }
    }

#if 0
    // HACK: hacky way to draw a debug postion
    auto test_dif = get_world_diff(state->test_pos, cam->pos);
    v2 test_mid   = { (screen_center.x + (test_dif.dif_xy.x * global::g_meters_to_pixels)),
                    (screen_center.y - (test_dif.dif_xy.y * global::g_meters_to_pixels)) };
    draw_ARGB(video_buffer, state->crosshair_img, test_mid);
#endif

    end_sim(state, region);
}

}  // namespace tom
