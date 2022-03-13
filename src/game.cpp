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

internal void
init_arena(memory_arena *arena, const mem_ind size, byt *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
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

internal entity_ref
add_wall(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    auto wall     = add_entity(state, entity_type::wall);
    world_pos pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    wall.low->pos      = pos;
    wall.low->height   = 1.f;
    wall.low->width    = 1.f;
    wall.low->color    = { 0xff'dd'dd'dd };
    wall.low->collides = true;
    wall.low->barrier  = true;
    wall.low->sprite   = &state.tree_sprite;

    return wall;
}

internal entity_ref
add_stairs(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    auto stairs   = add_low_entity(state, entity_type::stairs);
    world_pos pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    stairs.low->height      = 1.f;
    stairs.low->width       = 1.f;
    stairs.low->pos         = pos;
    stairs.low->color       = { 0xff'1e'1e'1e };
    stairs.low->argb_offset = 16.f;
    stairs.low->collides    = true;
    stairs.low->barrier     = false;
    stairs.low->sprite      = &state.stair_sprite;

    return stairs;
}

internal entity_ref
add_monster(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    auto monster  = add_low_entity(state, entity_type::monster);
    world_pos pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    monster.low->pos         = pos;
    monster.low->height      = .6f;
    monster.low->width       = .6f * .6f;
    monster.low->color       = { 0xff'dd'dd'dd };
    monster.low->argb_offset = 16.f;
    monster.low->collides    = true;
    monster.low->barrier     = true;
    monster.low->sprite      = &state.monster_sprites[0];

    init_hit_points(monster.low, 6);

    return monster;
}

internal entity_ref
add_cat(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z)
{
    auto cat      = add_low_entity(state, entity_type::familiar);
    world_pos pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    cat.low->pos         = pos;
    cat.low->height      = .6f;
    cat.low->width       = .8f;
    cat.low->color       = { 0xff'dd'dd'dd };
    cat.low->argb_offset = 5.f;
    cat.low->collides    = true;
    cat.low->barrier     = true;
    cat.low->sprite      = &state.cat_sprites[0];

    return cat;
}

internal entity_ref
add_sword(game_state &state, argb_img *sprite = nullptr)
{
    auto sword = add_low_entity(state, entity_type::sword, {}, false);

    sword.low->pos         = {};
    sword.low->height      = .6f;
    sword.low->width       = .8f;
    sword.low->color       = { 0xff'dd'dd'dd };
    sword.low->argb_offset = 5.f;
    sword.low->collides    = false;
    sword.low->barrier     = false;
    sword.low->hurtbox     = true;

    return sword;
}

internal void
add_player(game_state &state, const u32 player_i, const f32 x, const f32 y, const f32 z,
           argb_img *sprite)
{
    // NOTE: the first 5 entities are reserved for players
    TOM_ASSERT(player_i <= state.player_cnt);
    if (player_i <= state.player_cnt) {
        auto player = add_low_entity(state, entity_type::player);
        TOM_ASSERT(player_i == player.low_i);
        if (player_i == player.low_i) {
            auto pos                = abs_pos_to_world_pos(x, y, z);
            player.low->height      = .6f;
            player.low->width       = 0.6f * player.low->height;
            player.low->pos         = pos;
            player.low->color       = { 0xff'00'00'ff };
            player.low->argb_offset = 16.f;
            player.low->collides    = true;
            player.low->barrier     = true;
            player.low->sprite      = sprite;

            init_hit_points(player.low, 10);
            force_entity_into_high(state, player_i);

            auto sword           = add_sword(state);
            player.low->weapon_i = sword.low_i;
            sword.low->parent_i  = player.low_i;
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

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    // TODO: just fucking use a pointer?
    auto &state = (game_state &)(*(game_state *)memory.permanent_storage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory.is_initialized) {
        // init memory
        init_arena(&state.world_arena, memory.permanent_storage_size - sizeof(state),
                   (u8 *)memory.permanent_storage + sizeof(state));

        state.world = PushStruct(&state.world_arena, World);

        world *world = state.world;
        init_world(*world, 1.4f);

        state.debug_draw_collision = false;

        const char *bg = "uv_color_squares_960x540";

        // load textures
        state.player_sprites[entity_direction::north] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_n");
        state.player_sprites[entity_direction::east] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_e");
        state.player_sprites[entity_direction::south] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_s");
        state.player_sprites[entity_direction::west] =
            load_argb(thread, memory.platfrom_read_entire_file, "player_w");

        state.monster_sprites[entity_direction::north] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_n");
        state.monster_sprites[entity_direction::east] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_e");
        state.monster_sprites[entity_direction::south] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_s");
        state.monster_sprites[entity_direction::west] =
            load_argb(thread, memory.platfrom_read_entire_file, "monster_w");

        state.sword_sprites[entity_direction::north] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_n");
        state.sword_sprites[entity_direction::east] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_e");
        state.sword_sprites[entity_direction::south] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_s");
        state.sword_sprites[entity_direction::west] =
            load_argb(thread, memory.platfrom_read_entire_file, "sword_w");

        state.cat_sprites[0] = load_ARGB(thread, memory.platfrom_read_entire_file, "cat_e");
        state.cat_sprites[1] = load_ARGB(thread, memory.platfrom_read_entire_file, "cat_w");

        state.bg_img        = load_ARGB(thread, memory.platfrom_read_entire_file, bg);
        state.crosshair_img = load_ARGB(thread, memory.platfrom_read_entire_file, "crosshair");
        state.tree_sprite   = load_ARGB(thread, memory.platfrom_read_entire_file, "shitty_tree");
        state.stair_sprite  = load_ARGB(thread, memory.platfrom_read_entire_file, "stairs");

        s32 screen_base_x {}, screen_base_y {}, screen_base_z {}, virtual_z {}, rng_ind {};
        s32 screen_x { screen_base_x }, screen_y { screen_base_y }, screen_z { screen_base_z };

        // set world render size
        state.camera.pos.chunk_x = 0;
        state.camera.pos.chunk_y = 0;
        state.camera.pos.offset.x += screen_base_x / 2.f;
        state.camera.pos.offset.y += screen_base_y / 2.f;

        // NOTE: entity 0 is the null entity
        add_low_entity(state, entity_type::null);
        state.high_entities[0] = {};
        ++state.high_cnt;
        // state.player_cnt               = Game_Input::s_input_cnt;
        state.player_cnt               = 1;
        state.entity_camera_follow_ind = 1;

        // add the player entites
        for (u32 player_i = 1; player_i <= state.player_cnt; ++player_i) {
            add_player(state, player_i, 0.f, 0.f, 0.f, &state.player_sprites[0]);
        }

        f32 x_len = 55.f;
        for (f32 x = -20.f; x < x_len; ++x) {
            add_wall(state, x, 15, 0.0f);
            add_wall(state, x, -5, 0.0f);
            if (scast(s32, x) % 17 == 0) continue;
            add_wall(state, x, 5, 0.0f);
        }

        for (f32 x = -20.f; x <= x_len; x += 15.f) {
            for (f32 y = -5; y < 15; ++y) {
                if ((y == 0.f || y == 10.f) && (x != -20.f && x != 55.f)) continue;
                add_wall(state, x, y, 0.0);
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

    // REVIEW: if you put Game_World & it won't convert??????
    auto &world    = state.world;
    camera &camera = state.camera;

    auto p1 = get_entity_from_low_i(state, 1);

    // get input
    // NOTE: only doing one player
    stored_entity *player = get_low_entity(state, 1);
    assert(player->type == entity_type::player);
    entity_actions player_action = {};
    process_keyboard(input.keyboard, &player_action);
    process_controller(input.controllers[0], &player_action);
    state.player_acts[1] = player_action;

    // player sword attack
    if (state.player_acts[1].attack) {
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

    if (is_key_up(input.keyboard.d1)) state.debug_draw_collision = !state.debug_draw_collision;

    entity cam_ent       = get_entity_from_low_i(state, state.entity_camera_follow_ind);
    world_dif entity_dif = get_world_diff(cam_ent.low->pos, camera.pos);

    camera.pos.chunk_z = cam_ent.low->pos.chunk_z;

    // NOTE: camera is following the player
    world_pos new_cam_pos     = p1.low->pos;
    f32 screen_test_size_mult = 2.f;
    v2 test_screen_size       = { (global::screen_size_x * screen_test_size_mult),
                            global::screen_size_y * screen_test_size_mult };
    rect cam_bounds           = rect::center_half_dim({ 0.f, 0.f }, test_screen_size);

    sim_region *sim_region = begin_sim(state, origin, bounds);

    v2 screen_center = { .5f * (f32)video_buffer.width, .5f * (f32)video_buffer.height };
    entity_visble_piece_group piece_group = {};

    // NOTE: *not* using PatBlt in the win32 layer
    color clear_color { 0xff'4e'4e'4e };
    clear_buffer(video_buffer, clear_color);

    for (u32 ent_i = 0; ent_i < sim_region->sim_entity_cnt; ++ent_i) {
        piece_group.piece_cnt = 0;
        entity ent            = get_entity_from_high_i(state, ent_i);
        if (!ent.low->active) continue;  // don't draw inactive entities

        auto ent_dif = get_world_diff(ent.low->pos, camera.pos);
        v2 ent_mid   = { (screen_center.x + (ent_dif.dif_xy.x * global::meters_to_pixels)),
                       (screen_center.y - (ent_dif.dif_xy.y * global::meters_to_pixels)) };

        // TODO: pull this out?
        auto push_hp = [](entity_visble_piece_group &piece_group, entity ent, v2 argb_mid) {
            for (u32 i {}; i < ent.low->hit_points; ++i) {
                push_piece(&piece_group, 3.f, 6.f, { colors::red },
                           v2 { argb_mid.x - (ent.low->width / 2.f) * global::meters_to_pixels -
                                    10.f + scast(f32, i) * 4.f,
                                argb_mid.y - ent.low->height * global::meters_to_pixels - 10.f },
                           ent.high->z);
            }
        };

        switch (ent.low->type) {
            case entity_type::none: {
                draw_rect(
                    video_buffer, ent_mid.x - (ent.low->width * global::meters_to_pixels) / 2.f,
                    ent_mid.y - (ent.low->height * global::meters_to_pixels) / 2.f,
                    ent_mid.x + (ent.low->width * global::meters_to_pixels) / 2.f,
                    ent_mid.y + (ent.low->height * global::meters_to_pixels) / 2.f, { 0xffff00ff });
            } break;
            case entity_type::player: {
                // TODO: get player index from entity?
                update_player(state, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, ent.low->sprite, argb_mid, ent.high->z);
                push_hp(piece_group, ent, argb_mid);

            } break;
            case entity_type::wall: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, ent.low->sprite, argb_mid, ent.high->z);
            } break;
            case entity_type::stairs: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, ent.low->sprite, argb_mid, ent.high->z);
            } break;
            case entity_type::familiar: {
                update_familiar(state, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, ent.low->sprite, argb_mid, ent.high->z);
            } break;
            case entity_type::monster: {
                update_monster(state, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, ent.low->sprite, argb_mid, ent.high->z);
                push_hp(piece_group, ent, argb_mid);
            } break;
            case entity_type::sword: {
                update_monster(state, ent, input.delta_time);
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, ent.low->sprite, argb_mid, ent.high->z);
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
                draw_ARGB(video_buffer, *piece->img, piece->mid_p);
            } else {
                draw_rect(video_buffer, piece->rect, { colors::red });
            }
        }

        // NOTE:collision box
        if (state.debug_draw_collision) {
            draw_rect_outline(
                video_buffer, ent_mid.x - (ent.low->width * global::meters_to_pixels) / 2.f,
                ent_mid.y - (ent.low->height * global::meters_to_pixels) / 2.f,
                ent_mid.x + (ent.low->width * global::meters_to_pixels) / 2.f,
                ent_mid.y + (ent.low->height * global::meters_to_pixels) / 2.f, 1, { 0xffff0000 });
        }
    }

#if 0
    // HACK: hacky way to draw a debug postion
    auto test_dif = get_world_diff(state.test_pos, camera.pos);
    v2 test_mid   = { (screen_center.x + (test_dif.dif_xy.x * global::g_meters_to_pixels)),
                    (screen_center.y - (test_dif.dif_xy.y * global::g_meters_to_pixels)) };
    draw_ARGB(video_buffer, state.crosshair_img, test_mid);
#endif

    end_sim(state, *sim_region);
}

}  // namespace tom
