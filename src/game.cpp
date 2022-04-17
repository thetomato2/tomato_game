#include "game.hpp"
#include "rng_nums.h"

namespace tom
{

internal void
clear_buffer(game_offscreen_buffer &buffer, const color_argb color = colors::pink)
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
          const f32 max_x_f32, const f32 max_y_f32, const color_argb color = colors::pink)
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
draw_rect(game_offscreen_buffer &buffer, const rect2 rect, const color_argb color = colors::pink)
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
                  const color_argb color = colors::pink)
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
            color_argb dest_col   = { *dest };
            color_argb source_col = { *source };
            color_argb blended_col;
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
           const f32 alpha = 1.f)
{
    TOM_ASSERT(group->piece_cnt < ARRAY_COUNT(group->pieces));
    entity_visible_piece *piece = group->pieces + group->piece_cnt++;
    piece->img                  = img;
    piece->mid_p                = mid_p;
    piece->z                    = z_offset;
    piece->alpha                = alpha;
}

internal void
push_piece(entity_visble_piece_group *group, const f32 width, const f32 height,
           const color_argb color, const v2 mid_p, const f32 z_offset, const f32 alpha = 1.f)
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
    TOM_ASSERT(read_result.content_size != 0);  // file not found?
    argb_img result;

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

internal argb_img
load_argb_or_default(thread_context *thread, game_state *state,
                     debug_platform_read_entire_file *read_entire_file, const char *file_name,
                     const char *name = nullptr)
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
    if (read_result.content_size == 0) {
        return state->default_img;
    }

    argb_img result;

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
        if (is_key_up(keyboard.space)) entity_action->attack = true;
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

internal void
clear_collision_rule(game_state *state, u32 ent_i)
{
    // TODO: make better data structure that allows removal of collision rules without searching
    // the entire table
    for (u32 hash_bucket = 0; hash_bucket < ARRAY_COUNT(state->collision_rule_hash);
         ++hash_bucket) {
        for (pairwise_collision_rule **rule = &state->collision_rule_hash[hash_bucket]; *rule;) {
            if ((*rule)->ent_i_a == ent_i || (*rule)->ent_i_b == ent_i) {
                pairwise_collision_rule *removed_rule = *rule;
                *rule                                 = (*rule)->next;

                removed_rule->next               = state->first_free_collision_rule;
                state->first_free_collision_rule = removed_rule;
            } else {
                rule = &(*rule)->next;
            }
        }
    }
}

internal void
add_collsion_rule(game_state *state, u32 ent_i_a, u32 ent_i_b, bool should_collide)
{
    // TODO: collapse this with should_collide()
    auto ent_a = state->entities + ent_i_a;
    auto ent_b = state->entities + ent_i_b;
    if (ent_a->sim.type > ent_b->sim.type) {
        // TODO: swap func/template/macro
        auto temp = ent_i_a;
        ent_i_a   = ent_i_b;
        ent_i_b   = temp;
    }

    pairwise_collision_rule *found = nullptr;

    u32 hash_bucket = ent_i_a & (ARRAY_COUNT(state->collision_rule_hash) - 1);
    for (pairwise_collision_rule *rule = state->collision_rule_hash[hash_bucket]; rule;
         rule                          = rule->next) {
        if (rule->ent_i_a == ent_i_a && rule->ent_i_b == ent_i_b) {
            found = rule;
            break;
        }
    }

    if (!found) {
        found = state->first_free_collision_rule;
        if (found) {
            state->first_free_collision_rule = found->next;
        } else {
            found = PUSH_STRUCT(&state->world_arena, pairwise_collision_rule);
        }
        found->next                             = state->collision_rule_hash[hash_bucket];
        state->collision_rule_hash[hash_bucket] = found;
    }

    TOM_ASSERT(found);
    if (found) {
        found->ent_i_a        = ent_i_a;
        found->ent_i_b        = ent_i_b;
        found->should_collide = scast(b32, should_collide);
    }
}

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    auto *state = (game_state *)memory->permanent_storage;
    game_output_sound(sound_buffer);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    TOM_ASSERT(sizeof(game_state) <= memory->permanent_storage_size);

    auto state   = (game_state *)memory->permanent_storage;
    world *world = nullptr;

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory->is_initialized) {
        // init memory
        init_arena(&state->world_arena, memory->permanent_storage_size - sizeof(*state),
                   (u8 *)memory->permanent_storage + sizeof(*state));

        state->world = PUSH_STRUCT(&state->world_arena, tom::world);

        world = state->world;
        TOM_ASSERT(world);
        init_world(world, global::tile_size_meters);

        state->debug_draw_collision = false;
        state->debug_flag           = false;

        const char *bg = "uv_color_squares_960x540";

        // load textures
        state->default_img =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "pink");
        state->player_sprites[entity_direction::north] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "player_n");
        state->player_sprites[entity_direction::east] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "player_e");
        state->player_sprites[entity_direction::south] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "player_s");
        state->player_sprites[entity_direction::west] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "player_w");

        state->monster_sprites[entity_direction::north] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "monster_n");
        state->monster_sprites[entity_direction::east] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "monster_e");
        state->monster_sprites[entity_direction::south] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "monster_s");
        state->monster_sprites[entity_direction::west] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "monster_w");

        state->sword_sprites[entity_direction::north] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "sword_n");
        state->sword_sprites[entity_direction::east] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "sword_e");
        state->sword_sprites[entity_direction::south] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "sword_s");
        state->sword_sprites[entity_direction::west] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "sword_w");

        state->cat_sprites[0] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "cat_e");
        state->cat_sprites[1] =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "cat_w");

        state->bg_img = load_argb_or_default(thread, state, memory->platform_read_entire_file, bg);
        state->crosshair_img =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "crosshair");
        state->tree_sprite =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "shitty_tree");
        state->stair_sprite =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "stairs");
        state->wall_sprite =
            load_argb_or_default(thread, state, memory->platform_read_entire_file, "wall");

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

        state->entities[2].world_pos = state->entities[1].world_pos;

#if 1
        f32 x_len = 55.f;
        for (f32 x = -20.f; x < x_len; ++x) {
            add_wall(state, x, 15, 0.f);
            add_wall(state, x, -5, 0.f);
            if (scast(s32, x) % 17 == 0) continue;
            add_wall(state, x, 5, 0.f);
        }

        for (f32 x = -20.f; x <= x_len; x += 15.f) {
            for (f32 y = -5; y < 15; ++y) {
                if ((y == 0.f || y == 10.f) && (x != -20.f && x != 55.f)) continue;
                add_wall(state, x, y, 0.);
            }
        }
#endif

        for (f32 x = 0.f; x < 5.f; x += 0.5f) {
            add_tree(state, x, -2.f, 0.f);
        }

        {
            auto p1          = get_entity(state, 1);
            auto monster_ent = add_monster(state, 5.f, 0.f, 0.f);
            auto cat_ent     = add_cat(state, -1.f, 1.f, 0.f);
            auto sword_ent   = get_entity(state, p1->sim.weapon_i);
            auto stair_ent   = add_stair(state, 2.5f, 3.f, 0.f);
            add_collsion_rule(state, sword_ent->sim.ent_i, monster_ent->sim.ent_i, true);
            add_collsion_rule(state, p1->sim.ent_i, monster_ent->sim.ent_i, true);
            add_collsion_rule(state, p1->sim.ent_i, cat_ent->sim.ent_i, true);
            add_collsion_rule(state, p1->sim.ent_i, stair_ent->sim.ent_i, true);
        }

        // TODO: this might be more appropriate in the platform layer
        memory->is_initialized = true;
    }

    // ===============================================================================================
    // #START
    // ===============================================================================================

    f32 dt      = input.delta_time;
    camera *cam = &state->camera;
    auto p1     = get_entity(state, 1);
    // printf("%f, %f\n", p1->world_pos.offset.x, p1->world_pos.offset.y);

    // get input
    // NOTE: only doing one player
    entity *player = p1;
    TOM_ASSERT(player->sim.type == entity_type::player);
    entity_actions player_action = {};
    process_keyboard(input.keyboard, &player_action);
    process_controller(input.controllers[0], &player_action);
    state->player_acts[1] = player_action;

    if (is_key_up(input.keyboard.d1)) state->debug_draw_collision = !state->debug_draw_collision;

    // sword attack
    if (state->player_acts[1].attack && p1->sim.weapon_i &&
        is_flag_set(state->entities[p1->sim.weapon_i].sim.flags, sim_entity_flags::nonspatial)) {
        entity *sword_ent = state->entities + p1->sim.weapon_i;
        clear_flag(sword_ent->sim.flags, sim_entity_flags::nonspatial);
        sword_ent->world_pos      = p1->world_pos;
        constexpr f32 sword_vel   = 5.f;
        sword_ent->sim.dist_limit = 5.f;
        switch (p1->sim.dir) {
            case entity_direction::north: {
                sword_ent->sim.vel.y      = sword_vel;
                sword_ent->sim.cur_sprite = 0;
            } break;
            case entity_direction::east: {
                sword_ent->sim.vel.x      = sword_vel;
                sword_ent->sim.cur_sprite = 1;
            } break;
            case entity_direction::south: {
                sword_ent->sim.vel.y      = -sword_vel;
                sword_ent->sim.cur_sprite = 2;
            } break;
            case entity_direction::west: {
                sword_ent->sim.vel.x      = -sword_vel;
                sword_ent->sim.cur_sprite = 3;
            } break;
        }
    }

    entity *cam_ent = get_entity(state, state->entity_camera_follow_ind);
    v3 entity_dif   = get_world_diff(cam_ent->world_pos, cam->pos);

    // NOTE: cam is following the player
    cam->pos         = p1->world_pos;
    cam->pos.chunk_z = cam_ent->world_pos.chunk_z;

    rect3 sim_bounds = rec::center_dim(
        { 0.f, 0.f, 0.f }, { global::chunk_size_meters * 2.f, global::chunk_size_meters * 2.f });

    memory_arena sim_arena;
    init_arena(&sim_arena, memory->transient_storage_size, memory->transient_storage);

    sim_region *region = begin_sim(&sim_arena, state, cam->pos, sim_bounds, dt);

    v2 screen_center                      = { .5f * scast(f32, video_buffer.width),
                         .5f * scast(f32, video_buffer.height) };
    entity_visble_piece_group piece_group = {};
    argb_img *sprite                      = nullptr;

    // NOTE: *not* using PatBlt in the win32 layer
    color_argb clear_color { 0xff'4e'4e'4e };
    clear_buffer(video_buffer, clear_color);

    for (sim_entity *sim_ent = region->sim_entities;
         sim_ent != region->sim_entities + region->sim_entity_cnt; ++sim_ent) {
        if (sim_ent->type == entity_type::null) continue;

        // TODO: active and nonspatial redundant? -NO spatial ents get updated
        if (!is_flag_set(sim_ent->flags, sim_entity_flags::active))
            continue;  // don't update inactive entities

        sim_ent->hit_cd += dt;

        entity_move_spec move_spec = default_move_spec();
        entity_actions ent_act     = {};

        // update logic
        if (sim_ent->updateable) {
            switch (sim_ent->type) {
                case entity_type::none: {
                } break;
                case entity_type::player: {
                    f32 ent_speed = state->player_acts[1].sprint ? move_spec.speed = 25.f
                                                                 : move_spec.speed = 10.f;
                    ent_act       = state->player_acts[1];

                } break;
                case entity_type::wall: {
                    // do nothing
                } break;
                case entity_type::tree: {
                    // do nothing
                } break;
                case entity_type::stairs: {
                    // do nothing
                } break;
                case entity_type::familiar: {
                    sim_entity *closest_player = nullptr;
                    f32 closest_player_dist_sq = math::square(10.f);

                    for (sim_entity *test_ent = region->sim_entities;
                         test_ent != region->sim_entities + region->sim_entity_cnt; ++test_ent) {
                        if (test_ent->type == entity_type::player) {
                            f32 test_dist_sq = vec::length_sq(test_ent->pos - sim_ent->pos);
                            if (closest_player_dist_sq > test_dist_sq) {
                                closest_player         = test_ent;
                                closest_player_dist_sq = test_dist_sq;
                            }
                        }
                    }

                    if (closest_player) {
                        f32 one_over_len = 1.f / math::sqrt_f32(closest_player_dist_sq);
                        f32 min_dist     = 2.f;
                        v3 dif           = closest_player->pos - sim_ent->pos;
                        if (math::abs_f32(dif.x) > min_dist || math::abs_f32(dif.y) > min_dist)
                            ent_act.dir = one_over_len * (dif);
                    }
                } break;
                case entity_type::monster: {
                    // TODO: update monster
                } break;
                case entity_type::sword: {
                    move_spec.drag = 0.f;
                    if (!is_flag_set(sim_ent->flags, sim_entity_flags::nonspatial) &&
                        sim_ent->dist_limit == 0.f) {
                        set_flag(sim_ent->flags, sim_entity_flags::nonspatial);
                    }
                } break;
                case entity_type::stair: {
                    // TODO: stair logic?
                } break;
                default: {
                    INVALID_CODE_PATH;
                } break;
            }
        }

        v3 ent_delta = calc_entity_delta(sim_ent, ent_act, move_spec, dt);

        if (math::abs_f32(sim_ent->vel.x) > 0.f + global::epsilon ||
            math::abs_f32(sim_ent->vel.y) > 0.f + global::epsilon)
            move_entity(state, region, sim_ent, ent_delta, dt);

        // NOTE: changes the players direction for the sprite
        v3 pv = sim_ent->vel;

        constexpr f32 dir_eps = 0.1f;

        if (math::abs_f32(pv.x) > math::abs_f32(pv.y)) {
            if (pv.x > 0.f + dir_eps) {
                sim_ent->dir = entity_direction::east;
            } else if (pv.x < 0.f - dir_eps) {
                sim_ent->dir = entity_direction::west;
            }

        } else if (math::abs_f32(pv.y) > math::abs_f32(pv.x)) {
            if (pv.y > 0.f + dir_eps) {
                sim_ent->dir = entity_direction::north;
            } else if (pv.y < 0.f - dir_eps) {
                sim_ent->dir = entity_direction::south;
            }
        }

        sim_ent->hit_cd += dt;

        // ===============================================================================================
        // #PUSH TO RENDER
        // ===============================================================================================
        piece_group.piece_cnt = 0;
        sprite                = nullptr;
        // the regions origin should be the center of the camera so this should line up
        v3 sim_space_pos  = get_sim_space_pos(*state, *region, sim_ent->ent_i);
        v2 ent_screen_pos = { (screen_center.x + (sim_space_pos.x * global::meters_to_pixels)),
                              (screen_center.y - (sim_space_pos.y * global::meters_to_pixels)) };

        // TODO: pull this out in render code?
        auto push_hp = [](entity_visble_piece_group *piece_group, sim_entity *ent, v2 argb_mid) {
            for (u32 i = 0; i < ent->hp; ++i) {
                push_piece(piece_group, 3.f, 6.f, { colors::red },
                           v2 { argb_mid.x - (ent->dim.x / 2.f) * global::meters_to_pixels - 10.f +
                                    scast(f32, i) * 4.f,
                                argb_mid.y - ent->dim.y * global::meters_to_pixels - 10.f },
                           ent->z);
            }
        };

        rect3 cam_bounds              = { { 0.f, 0.f, 0.f },
                             { scast(f32, video_buffer.width), scast(f32, video_buffer.height),
                               0.f } };
        constexpr f32 cam_bound_delta = 50.f;
        cam_bounds                    = rec::add_radius(cam_bounds, cam_bound_delta);

        // draw only inside window
        if (rec::is_inside(cam_bounds, v3_init(ent_screen_pos, 0.f))) {
            switch (sim_ent->type) {
                case entity_type::none: {
                    draw_rect(video_buffer,
                              ent_screen_pos.x - (sim_ent->dim.x * global::meters_to_pixels) / 2.f,
                              ent_screen_pos.y - (sim_ent->dim.y * global::meters_to_pixels) / 2.f,
                              ent_screen_pos.x + (sim_ent->dim.x * global::meters_to_pixels) / 2.f,
                              ent_screen_pos.y + (sim_ent->dim.x * global::meters_to_pixels) / 2.f,
                              colors::pink);
                } break;
                case entity_type::player: {
                    // TODO: get player index from entity?
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    // TODO: this can probably be extracted out for general use
                    sim_ent->cur_sprite = sim_ent->dir;
                    sprite              = state->player_sprites + sim_ent->cur_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                    push_hp(&piece_group, sim_ent, argb_mid);
                } break;
                case entity_type::wall: {
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite      = &state->wall_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case entity_type::tree: {
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite      = &state->tree_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case entity_type::stairs: {
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite      = &state->stair_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case entity_type::familiar: {
                    if (sim_ent->dir == entity_direction::east)
                        sim_ent->cur_sprite = 0;
                    else if (sim_ent->dir == entity_direction::west)
                        sim_ent->cur_sprite = 1;
                    sprite      = state->cat_sprites + sim_ent->cur_sprite;
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case entity_type::monster: {
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite      = state->monster_sprites;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                    push_hp(&piece_group, sim_ent, argb_mid);
                } break;
                case entity_type::sword: {
                    sim_ent->cur_sprite = sim_ent->dir;
                    sprite              = state->sword_sprites + sim_ent->cur_sprite;
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case entity_type::stair: {
                    sprite      = &state->stair_sprite;
                    v2 argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                default: {
                    INVALID_CODE_PATH;
                } break;
            }
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
            draw_rect_outline(video_buffer,
                              ent_screen_pos.x - (sim_ent->dim.x * global::meters_to_pixels) / 2.f,
                              ent_screen_pos.y - (sim_ent->dim.y * global::meters_to_pixels) / 2.f,
                              ent_screen_pos.x + (sim_ent->dim.x * global::meters_to_pixels) / 2.f,
                              ent_screen_pos.y + (sim_ent->dim.y * global::meters_to_pixels) / 2.f,
                              1, { 0xffff0000 });
        }
    }

    end_sim(state, region);
}

}  // namespace tom
