#include "game.hpp"

#include "rng_nums.h"

namespace tom
{

static void
clear_buffer(game_offscreen_buffer &buffer, color_u32 color = { 0xff'ff'00'ff })
{
    auto width = buffer.width;
    s32 height = buffer.height;

    byt *row = (byt *)buffer.memory;
    for (s32 y = 0; y < height; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = 0; x < width; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

static void
draw_rect(game_offscreen_buffer &buffer, f32 f32_min_x, f32 f_min_y, f32 f32_max_x, f32 f32_max_y_,
          color_u32 color = { 0xffffffff })
{
    s32 min_x = round_f32_to_s32(f32_min_x);
    s32 min_y = round_f32_to_s32(f_min_y);
    s32 max_x = round_f32_to_s32(f32_max_x);
    s32 max_y = round_f32_to_s32(f32_max_y_);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row = ((byt *)buffer.memory + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch);

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = min_x; x < max_x; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

static void
draw_rect_outline(game_offscreen_buffer &buffer, f32 f32_min_x, f32 f_min_y, f32 f32_max_x,
                  f32 f32_max_y_, s32 thickness, color_u32 color = { 0xffffffff })
{
    s32 min_x = round_f32_to_s32(f32_min_x);
    s32 min_y = round_f32_to_s32(f_min_y);
    s32 max_x = round_f32_to_s32(f32_max_x);
    s32 max_y = round_f32_to_s32(f32_max_y_);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row = ((byt *)buffer.memory + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch);

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
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

static void
draw_ARGB(game_offscreen_buffer &buffer, ARGB_img &img, v2 pos)
{
    s32 min_y = round_f32_to_s32(pos.y - ((f32)img.height / 2.f));
    s32 min_x = round_f32_to_s32(pos.x - ((f32)img.width / 2.f));
    s32 max_y = round_f32_to_s32(pos.y + ((f32)img.height / 2.f));
    s32 max_x = round_f32_to_s32(pos.x + ((f32)img.width / 2.f));

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
    byt *row    = ((byt *)buffer.memory + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch);

    for (s32 y = min_y; y < max_y; ++y) {
        u32 *dest = (u32 *)row;
        source += x_offset_left;
        for (s32 x = min_x; x < max_x; ++x) {
            color_u32 dest_col { *dest };
            color_u32 source_col { *source };
            color_u32 blended_col;
            blended_col.a = 0xff;

            f32 alpha = (f32)source_col.a / 255.f;

            blended_col.r = u8((1.f - alpha) * (f32)dest_col.r + alpha * (f32)source_col.r);
            blended_col.g = u8((1.f - alpha) * (f32)dest_col.g + alpha * (f32)source_col.g);
            blended_col.b = u8((1.f - alpha) * (f32)dest_col.b + alpha * (f32)source_col.b);

            *dest = blended_col.argb;

            ++dest, ++source;
        }
        source += x_offset_right;
        row += buffer.pitch;
    }
}

static world_pos
get_entity_center_pos(const entity &entity)
{
    // FIXME: dunno if this is working right
    return map_into_chunk_space(entity.low->pos,
                                { entity.low->width / 2.f, entity.low->height / 2.f });
}

static void
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

static void
init_arena(memory_arena *arena, mem_ind size, byt *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

static bitmap_img
load_bmp(thread_context *thread, debug_platform_read_entire_file *read_entire_file,
         const char *file_name)
{
    debug_read_file_result read_result = read_entire_file(thread, file_name);
    bitmap_img result;

    if (read_result.content_size != 0) {
        auto *header     = (bitmap_header *)read_result.contents;
        u32 *pixels      = (u32 *)((byt *)read_result.contents + header->bitmap_offset);
        result.width     = header->width;
        result.height    = header->height;
        result.pixel_ptr = pixels;
    }
    return result;
}

static ARGB_img
load_ARGB(thread_context *thread, debug_platform_read_entire_file *read_entire_file,
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
    ARGB_img result;

    if (read_result.content_size != 0) {
        if (name)
            result.name = name;
        else
            result.name = file_name;
        auto *file_ptr   = (u32 *)read_result.contents;
        result.width     = *file_ptr++;
        result.height    = *file_ptr++;
        result.size      = *file_ptr++;
        result.pixel_ptr = file_ptr;
    }
    return result;
}

static player_actions
process_keyboard(const game_keyboard_input &keyboard)
{
    player_actions result = {};

    if (is_key_up(keyboard.t)) result.start = true;
    if (keyboard.w.ended_down) result.dir.y += 1.f;
    if (keyboard.s.ended_down) result.dir.y += -1.f;
    if (keyboard.a.ended_down) result.dir.x += -1.f;
    if (keyboard.d.ended_down) result.dir.x += 1.f;
    if (keyboard.left_shift.ended_down) result.sprint = true;

    return result;
}

static player_actions
process_controller(const game_controller_input &controller)
{
    player_actions result = {};

    if (is_button_up(controller.button_start)) result.start = true;

    if (controller.is_analog) {
        result.dir = { controller.end_left_stick_x, controller.end_left_stick_y };
    }

    if (controller.button_A.ended_down) result.sprint = true;

    return result;
}

inline v2
get_cam_space_pos(const game_state &state, entity_low *low_ent)
{
    world_dif diff = get_diff(low_ent->pos, state.camera.pos);
    v2 result      = diff.dif_xy;

    return result;
}

static entity_high *
make_entity_high(game_state &state, entity_low *low_ent, u32 low_i, v2 cam_space_pos)
{
    entity_high *high_ent = nullptr;
    TOM_ASSERT(low_i < global::max_low_cnt);
    TOM_ASSERT(!low_ent->high_i)

    if (state.high_cnt < global::max_high_cnt) {
        u32 high_i = state.high_cnt++;
        assert(high_i < global::max_high_cnt);
        high_ent = state.high_entities + high_i;

        high_ent->pos       = cam_space_pos;
        high_ent->vel       = { 0.f, 0.f };
        high_ent->direction = 0;
        high_ent->low_i     = low_i;

        low_ent->high_i = high_i;
    } else {
        INVALID_CODE_PATH;
    }

    return high_ent;
}

inline entity_high *
make_entity_high(game_state &state, u32 low_i)
{
    entity_high *high_ent = nullptr;

    entity_low *low_ent = state.low_entities + low_i;

    if (low_ent->high_i) {
        high_ent = state.high_entities + low_ent->high_i;
    } else {
        v2 cam_space_pos = get_cam_space_pos(state, low_ent);
        high_ent         = make_entity_high(state, low_ent, low_i, cam_space_pos);
    }

    return high_ent;
}

inline void
make_entity_low(game_state &game_state, u32 low_i)
{
    TOM_ASSERT(low_i < global::max_low_cnt);
    entity_low &low_ent = game_state.low_entities[low_i];
    u32 high_i          = low_ent.high_i;
    if (high_i) {
        u32 last_high_ent = game_state.high_cnt - 1;
        if (high_i != last_high_ent) {
            entity_high *last_ent = game_state.high_entities + last_high_ent;
            entity_high *del_ent  = game_state.high_entities + high_i;
            *del_ent              = *last_ent;
            game_state.low_entities[last_ent->low_i].high_i = high_i;
        }
        --game_state.high_cnt;
        low_ent.high_i = 0;
    }
}

static entity_low *
get_low_entity(game_state &game_state, u32 low_i)
{
    entity_low *result = nullptr;

    if (low_i > 0 && low_i < game_state.low_cnt) {
        result = game_state.low_entities + low_i;
    }

    return result;
}

static entity
get_high_entity(game_state &game_state, u32 low_i)
{
    entity result = {};

    // TODO: allow 0 index and returning a nullptr?
    assert(low_i < global::max_low_cnt);
    result.low_i = low_i;
    result.low   = get_low_entity(game_state, result.low_i);
    result.high  = make_entity_high(game_state, result.low_i);

    return result;
}
struct add_low_entity_result
{
    entity_low *low;
    u32 low_i;
};

static add_low_entity_result
add_low_entity(game_state &state, entity_type type = entity_type::none, world_pos pos = {})
{
    assert(state.low_cnt < global::max_low_cnt);
    u32 low_i = state.low_cnt++;

    entity_low *ent_low = state.low_entities + low_i;
    *ent_low            = {};
    ent_low->type       = type;
    ent_low->pos        = pos;

    change_entity_location(&state.world_arena, *state.world, low_i, nullptr,
                           &state.low_entities[low_i].pos);

    add_low_entity_result result = {};
    result.low                   = ent_low;
    result.low_i                 = low_i;

    return result;
}

static bool
validate_entity_pairs(const game_state &state)
{
    bool valid = true;
    for (u32 high_i = 1; high_i < state.high_cnt; ++high_i) {
        valid = valid && state.low_entities[state.high_entities[high_i].low_i].high_i == high_i;
    }

    return valid;
}

inline void
offset_and_check_entities_by_area(game_state &state, const rect::rect_v2 area, const v2 offset)
{
    for (u32 high_i = 1; high_i < state.high_cnt;) {
        entity_high *high = state.high_entities + high_i;
        high->pos += offset;
        if (rect::is_inside(area, high->pos)) {
            ++high_i;
        } else {
            make_entity_low(state, high->low_i);
        }
    }
}

static entity_low *
add_wall(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
         ARGB_img *sprites = nullptr)
{
    auto wall     = add_low_entity(state, entity_type::wall);
    world_pos pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    wall.low->pos      = pos;
    wall.low->height   = 1.f;
    wall.low->width    = 1.f;
    wall.low->color    = { 0xff'dd'dd'dd };
    wall.low->collides = true;
    wall.low->barrier  = true;

    return wall.low;
}

static entity_low *
add_stairs(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
           ARGB_img *sprites = nullptr)
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

    return stairs.low;
}

static entity_low *
add_monster(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
            ARGB_img *sprites = nullptr)
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

    return monster.low;
}

static entity_low *
add_cat(game_state &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
        ARGB_img *sprites = nullptr)
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

    return cat.low;
}

static void
init_player(game_state &state, const u32 player_i, const f32 x, const f32 y, const f32 z,
            ARGB_img *sprites = nullptr)
{
    // NOTE: the first 5 entities are reserved for players
    TOM_ASSERT(player_i <= state.player_cnt);
    if (player_i <= state.player_cnt) {
        auto player = add_low_entity(state, entity_type::player);
        TOM_ASSERT(player_i == player.low_i);
        if (player_i == player.low_i) {
            world_pos pos = abs_pos_to_world_pos(x, y, z);
#if 0
            make_entity_high(state, player_i);
            entity_high *player_high = state.high_entities + player->high_i;
            player_high->direction = 0;
            player_high->stair_cd  = 0;
            player_high->vel       = {};
#endif
            player.low->height      = .6f;
            player.low->width       = 0.6f * player.low->height;
            player.low->pos         = pos;
            player.low->color       = { 0xff'00'00'ff };
            player.low->argb_offset = 16.f;
            player.low->collides    = true;
            player.low->barrier     = true;
        }
    }
}

static void
move_player(game_state &state, entity player, const player_actions &player_action, const f32 dt)
{
    v2 r          = {};  // reflect vector
    v2 player_acc = { player_action.dir };

    // NOTE: normalize vector to unit length
    f32 player_acc_length = length_sq(player_acc);
    f32 player_speed      = player_action.sprint ? 100.f : 50.f;

    if (player_acc_length > 1.f) player_acc *= (1.f / sqrt_f32(player_acc_length));
    player_acc *= player_speed;
    player_acc -= player.high->vel * 10.f;  // drag/friction

    v2 player_delta   = { (.5f * player_acc * square(dt) + player.high->vel * dt) };
    v2 new_player_pos = player.high->pos + player_delta;
    player.high->vel += player_acc * dt;

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 t_min           = 1.0f;
        v2 wall_nrm         = {};
        u32 hit_ent_ind     = {};  // 0 is the null entity
        v2 desired_position = player.high->pos + player_delta;

        // TODO: this is N * N bad
        for (u32 test_high_i = 1; test_high_i < state.high_cnt; ++test_high_i) {
            if (test_high_i == player.low->high_i) continue;  // don't test against self

            entity test_ent;
            test_ent.high = state.high_entities + test_high_i,
            test_ent.low  = state.low_entities + test_ent.high->low_i;

            if (!test_ent.low->collides) continue;  // skip non-collision entities

            // NOTE: Minkowski sum
            f32 radius_w = player.low->width + test_ent.low->width;
            f32 radius_h = player.low->height + test_ent.low->height;

            v2 min_corner = { -.5f * v2 { radius_w, radius_h } };
            v2 max_corner = { .5f * v2 { radius_w, radius_h } };

            v2 rel = player.high->pos - test_ent.high->pos;

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
        entity_high *hit_high = state.high_entities + hit_ent_ind;
        if (!state.low_entities[hit_high->low_i].barrier) {
            wall_nrm = { 0.f, 0.f };
        }

        player.high->pos += t_min * player_delta;
        if (hit_ent_ind) {
            player.high->vel -= 1.f * inner(player.high->vel, wall_nrm) * wall_nrm;
            player_delta -= 1.f * inner(player_delta, wall_nrm) * wall_nrm;

            printf("%d hit %d!\n", player.low->high_i, hit_ent_ind);

            if (player.high->stair_cd > .5f &&
                state.low_entities[hit_high->low_i].type == entity_type::stairs) {
                player.low->virtual_z == 0  ? ++player.low->pos.chunk_z,
                    ++player.low->virtual_z : --player.low->pos.chunk_z, --player.low->virtual_z;
                player.high->stair_cd = 0.f;
            }
        } else {
            break;
        }
    }

    player.high->stair_cd += dt;

    // NOTE: changes the players direction for the sprite
    v2 pv = player.high->vel;
    if (abs_f32(pv.x) > abs_f32(pv.y)) {
        pv.x > 0.f ? player.high->direction = entity_direction::right
                   : player.high->direction = entity_direction::left;
    } else if (abs_f32(pv.y) > abs_f32(pv.x)) {
        pv.y > 0.f ? player.high->direction = entity_direction::up
                   : player.high->direction = entity_direction::down;
    }

    // TODO:
    world_pos new_pos = map_into_chunk_space(state.camera.pos, player.high->pos);
    change_entity_location(&state.world_arena, *state.world, player.low_i, &player.low->pos,
                           &new_pos);
    player.low->pos = new_pos;
}

static void
set_camera(game_state &state, world_pos new_cam_pos)
{
    world_pos old_cam_pos = state.camera.pos;
    world_dif dif_cam     = get_diff(new_cam_pos, old_cam_pos);
    state.camera.pos      = new_cam_pos;

    // offset and make all entities inside camera space high
    f32 screen_test_size_mult = 2.f;
    v2 test_screen_size       = { (global::screen_size_x * screen_test_size_mult),
                            global::screen_size_y * screen_test_size_mult };
    rect::rect_v2 cam_bounds  = rect::center_half_dim({ 0.f, 0.f }, test_screen_size);
    v2 ent_offset             = -dif_cam.dif_xy;
    offset_and_check_entities_by_area(state, cam_bounds, ent_offset);

    world_pos min_chunk_pos = map_into_chunk_space(new_cam_pos, rect::min_corner(cam_bounds));
    world_pos max_chunk_pos = map_into_chunk_space(new_cam_pos, rect::max_corner(cam_bounds));

    // make all entities outside camera space low
    for (s32 chunk_y = min_chunk_pos.chunk_y; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x = min_chunk_pos.chunk_x; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            world_chunk *chunk =
                get_world_chunk(*state.world, chunk_x, chunk_y, new_cam_pos.chunk_z);
            if (chunk) {
                for (world_entity_block *block = &chunk->first_block; block; block = block->next) {
                    for (u32 ent_i = 0; ent_i < block->low_entity_cnt; ++ent_i) {
                        u32 low_i           = block->low_ent_inds[ent_i];
                        entity_low *low_ent = state.low_entities + low_i;
                        if (low_ent->high_i == 0) {
                            v2 cam_space_pos = get_cam_space_pos(state, low_ent);
                            if (rect::is_inside(cam_bounds, cam_space_pos)) {
                                make_entity_high(state, low_i);
                            }
                        }
                    }
                }
            }
        }
    }

    TOM_ASSERT(validate_entity_pairs(state));
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
    auto &state = (game_state &)(*(game_state *)memory.permanent_storage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory.is_initialized) {
        // init memory
        init_arena(&state.world_arena, memory.permanent_storage_size - sizeof(state),
                   (u8 *)memory.permanent_storage + sizeof(state));

        state.world = PushStruct(&state.world_arena, game_world);

        game_world *world = state.world;
        init_world(*world, 1.4f);

        state.debug_draw_collision = false;

        const char *bg = "uv_color_squares_960x540";

        // load textures
        state.player_sprites[entity_direction::down] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_front");
        state.player_sprites[entity_direction::right] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_right");
        state.player_sprites[entity_direction::up] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_back");
        state.player_sprites[entity_direction::left] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_left");

        state.monster_sprites[entity_direction::down] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_front");
        state.monster_sprites[entity_direction::right] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_right");
        state.monster_sprites[entity_direction::up] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_back");
        state.monster_sprites[entity_direction::left] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_left");

        state.cat_sprite = load_ARGB(thread, memory.platfrom_read_entire_file, "cat");

        state.bg_img         = load_ARGB(thread, memory.platfrom_read_entire_file, bg);
        state.grass_bg       = load_ARGB(thread, memory.platfrom_read_entire_file, "grass_bg");
        state.crosshair_img  = load_ARGB(thread, memory.platfrom_read_entire_file, "crosshair");
        state.tree_sprite    = load_ARGB(thread, memory.platfrom_read_entire_file, "shitty_tree");
        state.stair_sprite   = load_ARGB(thread, memory.platfrom_read_entire_file, "stairs");
        state.red_square_img = load_ARGB(thread, memory.platfrom_read_entire_file, "red_square");
        state.green_square_img =
            load_ARGB(thread, memory.platfrom_read_entire_file, "green_square");
        state.blue_square_img = load_ARGB(thread, memory.platfrom_read_entire_file, "blue_square");

        s32 screen_base_x = 0;
        s32 screen_base_y = 0;
        s32 screen_base_z = 0;
        s32 screen_x      = screen_base_x;
        s32 screen_y      = screen_base_y;
        s32 screen_z      = screen_base_z;
        s32 virtual_z     = 0;
        s32 rng_ind       = 0;

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
            init_player(state, player_i, 0.f, 0.f, 0.f, state.player_sprites);
        }

        f32 x_len = 55.f;

        for (f32 x = -20.f; x < x_len; ++x) {
            add_wall(state, x, 15, 0.f, &state.tree_sprite);
            add_wall(state, x, -5, 0.f, &state.tree_sprite);
            if ((s32)x % 17 == 0) continue;
            add_wall(state, x, 5, 0.f, &state.tree_sprite);
        }

        for (f32 x = -20.f; x <= x_len; x += 15.f) {
            for (f32 y = -5; y < 15; ++y) {
                if ((y == 0.f || y == 10.f) && (x != -20.f && x != 55.f)) continue;
                add_wall(state, x, y, 0.f, &state.tree_sprite);
            }
        }

        add_monster(state, 5.f, 0.f, 0.f);

        add_cat(state, -1.f, 1.0f, 0.f);

        // TODO: this might be more appropriate in the platform layer
        memory.is_initialized = true;
    }

    // ===============================================================================================
    // #Start
    // ===============================================================================================

    auto &world  = state.world;
    auto &camera = state.camera;

    auto p1 = get_high_entity(state, 1);
    printf("%.2f, %.2f\n", p1.high->pos.x, p1.high->pos.y);
    printf("%.2f, %f\n", p1.low->pos.offset.x, p1.low->pos.offset.y);
    printf("%d, %d\n", p1.low->pos.chunk_x, p1.low->pos.chunk_y);

    for (u32 player_i = 1; player_i <= state.player_cnt; ++player_i) {
        entity_low *low_player = get_low_entity(state, player_i);
        assert(low_player->type == entity_type::player);

        player_actions player_action {};
        if (player_i == 1) {
            player_action = process_keyboard(input.keyboard);
        } else {
            player_action = process_controller(input.controllers[player_i - 2]);
        }
        if (player_action.start) {
            if (!low_player->high_i)
                make_entity_high(state, player_i);
            else
                make_entity_low(state, player_i);
        }

        if (low_player->high_i) {
            // TODO: get rid of Entity or make a helper func?
            entity player_ent = get_high_entity(state, player_i);
            move_player(state, player_ent, player_action, input.delta_time);
        }
    }
    if (is_key_up(input.keyboard.d1)) state.debug_draw_collision = !state.debug_draw_collision;

    entity cam_ent       = get_high_entity(state, state.entity_camera_follow_ind);
    world_dif entity_dif = get_diff(cam_ent.low->pos, camera.pos);
    camera.pos.chunk_z   = cam_ent.low->pos.chunk_z;

    // NOTE: camera is following the player
    world_pos new_cam_pos = p1.low->pos;

    set_camera(state, new_cam_pos);

    // ===============================================================================================
    // #DRAW
    // ===============================================================================================

    // NOTE: *not* using PatBlt in the win32 layer
    color_u32 clear_color = { 0xff'4e'4e'4e };
    clear_buffer(video_buffer, clear_color);

    u32 *source = state.bg_img.pixel_ptr;
    u32 *dest   = (u32 *)video_buffer.memory;

    s32 num_draw_tiles = 12;
    v2 screen_center   = { .5f * (f32)video_buffer.width, .5f * (f32)video_buffer.height };

    for (u32 high_i { 1 }; high_i < state.high_cnt; ++high_i) {
        // TODO: seems a bit convoluted...
        entity ent = {};
        ent.high   = state.high_entities + high_i;
        ent.low_i  = ent.high->low_i;
        ent.low    = state.low_entities + ent.low_i;

        auto ent_dif = get_diff(ent.low->pos, camera.pos);
        v2 ent_mid   = { (screen_center.x + (ent_dif.dif_xy.x * global::meters_to_pixels)),
                       (screen_center.y - (ent_dif.dif_xy.y * global::meters_to_pixels)) };

        switch (ent.low->type) {
            case entity_type::none: {
                draw_rect(
                    video_buffer, ent_mid.x - (ent.low->width * global::meters_to_pixels) / 2.f,
                    ent_mid.y - (ent.low->height * global::meters_to_pixels) / 2.f,
                    ent_mid.x + (ent.low->width * global::meters_to_pixels) / 2.f,
                    ent_mid.y + (ent.low->height * global::meters_to_pixels) / 2.f, { 0xffff00ff });
            } break;
            case entity_type::player: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                draw_ARGB(video_buffer, state.player_sprites[ent.high->direction], argb_mid);
            } break;
            case entity_type::wall: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                draw_ARGB(video_buffer, state.tree_sprite, argb_mid);
            } break;
            case entity_type::stairs: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                draw_ARGB(video_buffer, state.stair_sprite, argb_mid);
            } break;
            case entity_type::familiar: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                draw_ARGB(video_buffer, state.cat_sprite, argb_mid);
            } break;
            case entity_type::monster: {
                v2 argb_mid = { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                draw_ARGB(video_buffer, state.monster_sprites[ent.high->direction], argb_mid);
            } break;
            default: {
                INVALID_CODE_PATH;
            } break;
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
    // NOTE: hacky way to draw a debug postion
    auto test_dif = get_diff(state.test_pos, camera.pos);
    v2 test_mid  = { (screen_center.x + (test_dif.dif_xy.x * global::g_meters_to_pixels)),
                  (screen_center.y - (test_dif.dif_xy.y * global::g_meters_to_pixels)) };
    draw_ARGB(video_buffer, state.crosshair_img, test_mid);
#endif
}

}  // namespace tom
