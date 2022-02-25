#include "game.hpp"

#include "rng_nums.h"

namespace tom
{

static void
clear_buffer(Game_Offscreen_Buffer &buffer, const Color color = colors::pink)
{
    const s32 width { buffer.width };
    const s32 height { buffer.height };

    byt *row { scast(byt *, buffer.memory) };
    for (s32 y {}; y < height; ++y) {
        u32 *pixel { rcast(u32 *, row) };
        for (s32 x {}; x < width; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

static void
draw_rect(Game_Offscreen_Buffer &buffer, const f32 min_x_f32, const f32 min_y_f32,
          const f32 max_x_f32, const f32 max_y_f32, const Color color = colors::pink)
{
    s32 min_x { round_f32_to_s32(min_x_f32) };
    s32 min_y { round_f32_to_s32(min_y_f32) };
    s32 max_x { round_f32_to_s32(max_x_f32) };
    s32 max_y { round_f32_to_s32(max_y_f32) };

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row { scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel +
               min_y * buffer.pitch };

    for (s32 y { min_y }; y < max_y; ++y) {
        u32 *pixel { rcast(u32 *, row) };
        for (s32 x { min_x }; x < max_x; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

static void
draw_rect(Game_Offscreen_Buffer &buffer, const rect::Rect_v2 rect, const Color color = colors::pink)
{
    s32 min_x { round_f32_to_s32(rect.min.x) };
    s32 min_y { round_f32_to_s32(rect.min.y) };
    s32 max_x { round_f32_to_s32(rect.max.x) };
    s32 max_y { round_f32_to_s32(rect.max.y) };

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row { scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel +
               min_y * buffer.pitch };

    for (s32 y { min_y }; y < max_y; ++y) {
        u32 *pixel { rcast(u32 *, row) };
        for (s32 x { min_x }; x < max_x; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

static void
draw_rect_outline(Game_Offscreen_Buffer &buffer, const f32 min_x_f32, const f32 min_y_f32,
                  const f32 max_x_f32, const f32 max_y_f32, const s32 thickness,
                  const Color color = colors::pink)
{
    s32 min_x { round_f32_to_s32(min_x_f32) };
    s32 min_y { round_f32_to_s32(min_y_f32) };
    s32 max_x { round_f32_to_s32(max_x_f32) };
    s32 max_y { round_f32_to_s32(max_y_f32) };

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer.width) max_x = buffer.width;
    if (max_y > buffer.height) max_y = buffer.height;

    byt *row { scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel +
               min_y * buffer.pitch };

    for (s32 y { min_y }; y < max_y; ++y) {
        u32 *pixel { rcast(u32 *, row) };
        for (s32 x { min_x }; x < max_x; ++x) {
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
draw_ARGB(Game_Offscreen_Buffer &buffer, const ARGB_img &img, const v2 pos)
{
    s32 min_y { round_f32_to_s32(pos.y - (scast(f32, img.height) / 2.f)) };
    s32 min_x { round_f32_to_s32(pos.x - (scast(f32, img.width) / 2.f)) };
    s32 max_y { round_f32_to_s32(pos.y + (scast(f32, img.height) / 2.f)) };
    s32 max_x { round_f32_to_s32(pos.x + (scast(f32, img.width) / 2.f)) };

    s32 x_offset_left {}, x_offset_right {}, y_offset {};

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

    u32 *source { img.pixel_ptr + (y_offset * img.width) };
    byt *row { scast(byt *, buffer.memory) + min_x * buffer.bytes_per_pixel +
               min_y * buffer.pitch };

    for (s32 y { min_y }; y < max_y; ++y) {
        u32 *dest { rcast(u32 *, row) };
        source += x_offset_left;
        for (s32 x { min_x }; x < max_x; ++x) {
            Color dest_col { *dest };
            Color source_col { *source };
            Color blended_col;
            blended_col.a = 0xff;

            f32 alpha { scast(f32, source_col.a) / 255.f };

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

static void
push_piece(Entity_Visble_Piece_Group *group, ARGB_img *img, const v2 mid_p, const f32 z_offset,
           const f32 alpha = 1.0f)
{
    TomAssert(group->piece_cnt < ArrayCount(group->pieces));
    Entity_Visible_Piece *piece { group->pieces + group->piece_cnt++ };

    piece->img   = img;
    piece->mid_p = mid_p;
    piece->z     = z_offset;
    piece->alpha = alpha;
}

static void
push_piece(Entity_Visble_Piece_Group *group, const f32 width, const f32 height, const Color color,
           const v2 mid_p, const f32 z_offset, const f32 alpha = 1.0f)
{
    TomAssert(group->piece_cnt < ArrayCount(group->pieces));
    Entity_Visible_Piece *piece { group->pieces + group->piece_cnt++ };

    piece->img        = nullptr;
    piece->mid_p      = mid_p;
    piece->z          = z_offset;
    piece->alpha      = alpha;
    piece->rect.min.x = mid_p.x - width / 2;
    piece->rect.min.y = mid_p.y - height / 2;
    piece->rect.max.x = mid_p.x + width / 2;
    piece->rect.max.y = mid_p.y + height / 2;
    piece->color      = color;
}

static void
game_output_sound(Game_Sound_Output_Buffer &sound_buffer)
{
    // NOTE: outputs nothing atm
    s16 sample_value {};
    s16 *sampleOut { sound_buffer.samples };
    for (szt sampleIndex {}; sampleIndex < sound_buffer.sample_count; ++sampleIndex) {
        *sampleOut++ = sample_value;
        *sampleOut++ = sample_value;
    }
}

static void
init_arena(Memory_Arena *arena, const mem_ind size, byt *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

static Bitmap_Img
load_bmp(Thread_Context *thread, debug_platform_read_entire_file *read_entire_file,
         const char *file_name)
{
    Debug_Read_File_Result read_result { read_entire_file(thread, file_name) };
    Bitmap_Img result;

    if (read_result.content_size != 0) {
        auto *header { scast(Bitmap_Header *, read_result.contents) };
        u32 *pixels { rcast(u32 *, (scast(byt *, read_result.contents) + header->bitmap_offset)) };
        result.width     = header->width;
        result.height    = header->height;
        result.pixel_ptr = pixels;
    }

    return result;
}

static ARGB_img
load_ARGB(Thread_Context *thread, debug_platform_read_entire_file *read_entire_file,
          const char *file_name, const char *name = nullptr)
{
    const char *argb_dir { "T:/assets/argbs/" };
    char img_path_buf[512];
    szt img_buf_len;
    cat_str(argb_dir, file_name, &img_path_buf[0], &img_buf_len);
    img_path_buf[img_buf_len++] = '.';
    img_path_buf[img_buf_len++] = 'a';
    img_path_buf[img_buf_len++] = 'r';
    img_path_buf[img_buf_len++] = 'g';
    img_path_buf[img_buf_len++] = 'b';
    img_path_buf[img_buf_len++] = '\0';

    Debug_Read_File_Result read_result { read_entire_file(thread, img_path_buf) };
    ARGB_img result;

    TomAssert(read_result.content_size != 0);
    if (read_result.content_size != 0) {
        if (name)
            result.name = name;
        else
            result.name = file_name;

        u32 *file_ptr { scast(u32 *, read_result.contents) };
        result.width     = *file_ptr++;
        result.height    = *file_ptr++;
        result.size      = *file_ptr++;
        result.pixel_ptr = file_ptr;
    }

    return result;
}

static Entity_Actions
process_keyboard(const Game_Keyboard_Input &keyboard)
{
    Entity_Actions result {};

    if (is_key_up(keyboard.t)) result.start = true;
    if (keyboard.space.ended_down) result.jump = true;
    if (keyboard.w.ended_down) result.dir.y += 1.f;
    if (keyboard.s.ended_down) result.dir.y += -1.f;
    if (keyboard.a.ended_down) result.dir.x += -1.f;
    if (keyboard.d.ended_down) result.dir.x += 1.f;
    if (keyboard.left_shift.ended_down) result.sprint = true;

    return result;
}

static Entity_Actions
process_controller(const Game_Controller_Input &controller)
{
    Entity_Actions result {};

    if (is_button_up(controller.button_start)) result.start = true;

    if (controller.is_analog) {
        result.dir = { controller.end_left_stick_x, controller.end_left_stick_y };
    }

    if (controller.button_A.ended_down) result.sprint = true;

    return result;
}

inline v2
get_cam_space_pos(const Game_State &state, Entity_Low *low_ent)
{
    World_Dif diff { get_diff(low_ent->pos, state.camera.pos) };
    v2 result { diff.dif_xy };

    return result;
}

static Entity_High *
make_entity_high(Game_State &state, Entity_Low *low_ent, u32 low_i, v2 cam_space_pos)
{
    Entity_High *high_ent {};
    TomAssert(low_i < global::max_low_cnt);
    TomAssert(!low_ent->high_i);

    if (state.high_cnt < global::max_high_cnt) {
        u32 high_i { state.high_cnt++ };
        TomAssert(high_i < global::max_high_cnt);
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

inline Entity_High *
make_entity_high(Game_State &state, u32 low_i)
{
    Entity_High *high_ent {};

    Entity_Low *low_ent { state.low_entities + low_i };

    if (low_ent->high_i) {
        high_ent = state.high_entities + low_ent->high_i;
    } else {
        v2 cam_space_pos { get_cam_space_pos(state, low_ent) };
        high_ent = make_entity_high(state, low_ent, low_i, cam_space_pos);
    }

    return high_ent;
}

inline void
make_entity_low(Game_State &game_state, u32 low_i)
{
    TomAssert(low_i < global::max_low_cnt);
    Entity_Low &low_ent { game_state.low_entities[low_i] };
    u32 high_i { low_ent.high_i };
    if (high_i) {
        u32 last_high_ent { game_state.high_cnt - 1 };
        if (high_i != last_high_ent) {
            Entity_High *last_ent { game_state.high_entities + last_high_ent };
            Entity_High *del_ent { game_state.high_entities + high_i };
            *del_ent                                        = *last_ent;
            game_state.low_entities[last_ent->low_i].high_i = high_i;
        }
        --game_state.high_cnt;
        low_ent.high_i = 0;
    }
}

static Entity_Low *
get_low_entity(Game_State &game_state, u32 low_i)
{
    Entity_Low *result {};

    if (low_i > 0 && low_i < game_state.low_cnt) {
        result = game_state.low_entities + low_i;
    }

    return result;
}

static Entity
force_entity_into_high(Game_State &state, u32 low_i)
{
    Entity result {};

    // TODO: allow 0 index and returning a nullptr?
    TomAssert(low_i < global::max_low_cnt);
    result.low_i = low_i;
    result.low   = get_low_entity(state, result.low_i);
    result.high  = make_entity_high(state, result.low_i);

    return result;
}

static Entity
get_entity_from_high_i(Game_State &state, u32 high_i)
{
    Entity result {};

    // TODO: allow 0 index and returning a nullptr?
    TomAssert(high_i < state.high_cnt);
    if (high_i < state.high_cnt) {
        result.high  = state.high_entities + high_i;
        result.low_i = result.high->low_i;
        result.low   = get_low_entity(state, result.low_i);
    }

    return result;
}

static Entity
get_entity_from_low_i(Game_State &state, u32 low_i)
{
    Entity result {};

    // TODO: allow 0 index and returning a nullptr?
    assert(low_i < global::max_low_cnt);
    result.low_i = low_i;
    result.low   = get_low_entity(state, result.low_i);
    result.high  = nullptr;
    if (result.low->high_i) result.high = state.high_entities + result.low->high_i;

    return result;
}

struct Add_Low_Entity_Result
{
    Entity_Low *low;
    u32 low_i;
};

static Add_Low_Entity_Result
add_low_entity(Game_State &state, Entity_Type type = Entity_Type::none, World_Pos pos = {})
{
    TomAssert(state.low_cnt < global::max_low_cnt);
    u32 low_i { state.low_cnt++ };

    Entity_Low *ent_low { state.low_entities + low_i };
    *ent_low      = {};
    ent_low->type = type;
    ent_low->pos  = pos;

    change_entity_location(&state.world_arena, *state.world, low_i, nullptr,
                           &state.low_entities[low_i].pos);

    Add_Low_Entity_Result result {};

    result.low   = ent_low;
    result.low_i = low_i;

    return result;
}

static bool
validate_entity_pairs(const Game_State &state)
{
    bool valid { true };
    for (u32 high_i { 1 }; high_i < state.high_cnt; ++high_i) {
        valid = valid && state.low_entities[state.high_entities[high_i].low_i].high_i == high_i;
    }

    return valid;
}

static void
add_hit_points(Entity entity, u32 hp)
{
    entity.low->hit_points += hp;
    if (entity.low->hit_points > entity.low->max_hit_points)
        entity.low->hit_points = entity.low->max_hit_points;
}

static void
subtract_hit_points(Entity entity, u32 hp)
{
    entity.low->hit_points -= hp;
    if (entity.low->hit_points < 0) entity.low->hit_points = 0;
}

inline void
offset_and_check_entities_by_area(Game_State &state, const rect::Rect_v2 area, const v2 offset)
{
    for (u32 high_i { 1 }; high_i < state.high_cnt;) {
        Entity_High *high { state.high_entities + high_i };
        high->pos += offset;
        if (rect::is_inside(area, high->pos)) {
            ++high_i;
        } else {
            make_entity_low(state, high->low_i);
        }
    }
}

static Entity_Low *
add_wall(Game_State &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
         ARGB_img *sprites = nullptr)
{
    auto wall { add_low_entity(state, Entity_Type::wall) };
    World_Pos pos { abs_pos_to_world_pos(abs_x, abs_y, abs_z) };

    wall.low->pos      = pos;
    wall.low->height   = 1.f;
    wall.low->width    = 1.f;
    wall.low->color    = { 0xff'dd'dd'dd };
    wall.low->collides = true;
    wall.low->barrier  = true;

    return wall.low;
}

static Entity_Low *
add_stairs(Game_State &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
           ARGB_img *sprites = nullptr)
{
    auto stairs { add_low_entity(state, Entity_Type::stairs) };
    World_Pos pos { abs_pos_to_world_pos(abs_x, abs_y, abs_z) };

    stairs.low->height      = 1.f;
    stairs.low->width       = 1.f;
    stairs.low->pos         = pos;
    stairs.low->color       = { 0xff'1e'1e'1e };
    stairs.low->argb_offset = 16.f;
    stairs.low->collides    = true;
    stairs.low->barrier     = false;

    return stairs.low;
}

static Entity_Low *
add_monster(Game_State &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
            ARGB_img *sprites = nullptr)
{
    auto monster { add_low_entity(state, Entity_Type::monster) };
    World_Pos pos { abs_pos_to_world_pos(abs_x, abs_y, abs_z) };

    monster.low->pos            = pos;
    monster.low->height         = .6f;
    monster.low->width          = .6f * .6f;
    monster.low->color          = { 0xff'dd'dd'dd };
    monster.low->argb_offset    = 16.f;
    monster.low->collides       = true;
    monster.low->barrier        = true;
    monster.low->max_hit_points = 6;
    monster.low->hit_points     = monster.low->max_hit_points;

    return monster.low;
}

static Entity_Low *
add_cat(Game_State &state, const f32 abs_x, const f32 abs_y, const f32 abs_z,
        ARGB_img *sprites = nullptr)
{
    auto cat { add_low_entity(state, Entity_Type::familiar) };
    World_Pos pos { abs_pos_to_world_pos(abs_x, abs_y, abs_z) };

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
add_player(Game_State &state, const u32 player_i, const f32 x, const f32 y, const f32 z,
           ARGB_img *sprites = nullptr)
{
    // NOTE: the first 5 entities are reserved for players
    TomAssert(player_i <= state.player_cnt);
    if (player_i <= state.player_cnt) {
        auto player { add_low_entity(state, Entity_Type::player) };
        TomAssert(player_i == player.low_i);
        if (player_i == player.low_i) {
            World_Pos pos { abs_pos_to_world_pos(x, y, z) };
            player.low->height         = .6f;
            player.low->width          = 0.6f * player.low->height;
            player.low->pos            = pos;
            player.low->color          = { 0xff'00'00'ff };
            player.low->argb_offset    = 16.f;
            player.low->collides       = true;
            player.low->barrier        = true;
            player.low->max_hit_points = 12;
            player.low->hit_points     = player.low->max_hit_points;
            force_entity_into_high(state, player_i);
        }
    }
}

static void
move_entity(Game_State &state, Entity entity, const Entity_Actions &entity_actions, const f32 dt)
{
    v2 entity_acc { entity_actions.dir };

    // NOTE: normalize vector to unit length
    f32 ent_acc_length { length_sq(entity_acc) };
    // TODO: make speed spefific to entity type
    f32 ent_speed { entity_actions.sprint ? 100.f : 50.f };

    if (ent_acc_length > 1.f) entity_acc *= (1.f / sqrt_f32(ent_acc_length));
    entity_acc *= ent_speed;
    entity_acc -= entity.high->vel * 10.f;  // drag/friction

    v2 player_delta { (.5f * entity_acc * square(dt) + entity.high->vel * dt) };
    v2 new_player_pos { entity.high->pos + player_delta };

    entity.high->vel += entity_acc * dt;

    // NOTE: how many iterations/time resolution
    for (u32 i = {}; i < 4; ++i) {
        f32 t_min { 1.0f };
        u32 hit_ent_ind {};  // 0 is the null entity
        v2 wall_nrm {};
        v2 desired_position { entity.high->pos + player_delta };

        // FIXME: this is N * N bad
        for (u32 test_high_i { 1 }; test_high_i < state.high_cnt; ++test_high_i) {
            if (test_high_i == entity.low->high_i) continue;  // don't test against self

            Entity test_ent { get_entity_from_high_i(state, test_high_i) };

            if (!test_ent.low->collides) continue;  // skip non-collision entities

            // NOTE: Minkowski sum
            f32 radius_w { entity.low->width + test_ent.low->width };
            f32 radius_h { entity.low->height + test_ent.low->height };

            v2 min_corner { -.5f * v2 { radius_w, radius_h } };
            v2 max_corner { .5f * v2 { radius_w, radius_h } };
            v2 rel { entity.high->pos - test_ent.high->pos };

            // TODO: maybe pull this out into a free function (but why?)
            auto test_wall { [&t_min](f32 wall_x, f32 rel_x, f32 rel_y, f32 player_delta_x,
                                      f32 player_delta_y, f32 min_y, f32 max_y) -> bool {
                bool hit { false };

                f32 t_esp { .001f };
                if (player_delta_x != 0.f) {
                    f32 t_res { (wall_x - rel_x) / player_delta_x };
                    f32 y { rel_y + t_res * player_delta_y };

                    if (t_res >= 0.f && (t_min > t_res)) {
                        if (y >= min_y && y <= max_y) {
                            t_min = max(0.f, t_res - t_esp);
                            hit   = true;
                        }
                    }
                }

                return hit;
            } };

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

        Entity_High *hit_high { state.high_entities + hit_ent_ind };
        if (!state.low_entities[hit_high->low_i].barrier) {
            wall_nrm = v2 { 0.f, 0.f };
        }

        entity.high->pos += t_min * player_delta;
        if (hit_ent_ind) {
            entity.high->vel -= 1.f * inner(entity.high->vel, wall_nrm) * wall_nrm;
            player_delta -= 1.f * inner(player_delta, wall_nrm) * wall_nrm;

            printf("%d hit %d!\n", entity.low->high_i, hit_ent_ind);
            // TODO: temp player hitting monster logic
            if (entity.low->type == Entity_Type::player && entity.high->hit_cd > .5f) {
                if (state.low_entities[hit_high->low_i].type == Entity_Type::monster) {
                    entity.high->hit_cd = 0.f;
                    subtract_hit_points(entity, 1);
                } else if (state.low_entities[hit_high->low_i].type == Entity_Type::familiar) {
                    entity.high->hit_cd = 0.f;
                    add_hit_points(entity, 1);
                }
            }

            if (entity.high->stair_cd > .5f &&
                state.low_entities[hit_high->low_i].type == Entity_Type::stairs) {
                entity.low->virtual_z == 0  ? ++entity.low->pos.chunk_z,
                    ++entity.low->virtual_z : --entity.low->pos.chunk_z, --entity.low->virtual_z;
                entity.high->stair_cd = 0.f;
            }
        } else {
            break;
        }
    }

    // jump code
    // TODO: implement this
    if (entity_actions.jump && !entity.high->is_jumping) {
        entity.high->is_jumping = true;
        entity.high->vel_z      = global::jump_vel;
    }

    entity.high->stair_cd += dt;
    entity.high->hit_cd += dt;

    // NOTE: changes the players direction for the sprite
    v2 pv { entity.high->vel };
    if (abs_f32(pv.x) > abs_f32(pv.y)) {
        pv.x > 0.f ? entity.high->direction = Entity_Direction::right
                   : entity.high->direction = Entity_Direction::left;
    } else if (abs_f32(pv.y) > abs_f32(pv.x)) {
        pv.y > 0.f ? entity.high->direction = Entity_Direction::up
                   : entity.high->direction = Entity_Direction::down;
    }

    // TODO:
    World_Pos new_pos { map_into_chunk_space(state.camera.pos, entity.high->pos) };
    change_entity_location(&state.world_arena, *state.world, entity.low_i, &entity.low->pos,
                           &new_pos);
    entity.low->pos = new_pos;
}

static void
set_camera(Game_State &state, World_Pos new_cam_pos)
{
    World_Pos old_cam_pos { state.camera.pos };
    World_Dif dif_cam { get_diff(new_cam_pos, old_cam_pos) };
    state.camera.pos = new_cam_pos;

    // offset and make all entities inside camera space high
    f32 screen_test_size_mult { 2.f };
    v2 test_screen_size { (global::screen_size_x * screen_test_size_mult),
                          global::screen_size_y * screen_test_size_mult };
    rect::Rect_v2 cam_bounds { rect::center_half_dim({ 0.f, 0.f }, test_screen_size) };
    v2 ent_offset { -dif_cam.dif_xy };
    offset_and_check_entities_by_area(state, cam_bounds, ent_offset);

    World_Pos min_chunk_pos { map_into_chunk_space(new_cam_pos, rect::min_corner(cam_bounds)) };
    World_Pos max_chunk_pos { map_into_chunk_space(new_cam_pos, rect::max_corner(cam_bounds)) };

    // make all entities outside camera space low
    for (s32 chunk_y { min_chunk_pos.chunk_y }; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x { min_chunk_pos.chunk_x }; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            World_Chunk *chunk { get_world_chunk(*state.world, chunk_x, chunk_y,
                                                 new_cam_pos.chunk_z) };
            if (chunk) {
                for (World_Entity_Block *block { &chunk->first_block }; block;
                     block = block->next) {
                    for (u32 ent_i {}; ent_i < block->low_entity_cnt; ++ent_i) {
                        u32 low_i { block->low_ent_inds[ent_i] };
                        Entity_Low *low_ent { state.low_entities + low_i };
                        if (low_ent->high_i == 0) {
                            v2 cam_space_pos { get_cam_space_pos(state, low_ent) };
                            if (rect::is_inside(cam_bounds, cam_space_pos)) {
                                make_entity_high(state, low_i);
                            }
                        }
                    }
                }
            }
        }
    }

    TomAssert(validate_entity_pairs(state));
}

static void
update_familiar(Game_State &state, Entity entity, const f32 dt)
{
    Entity closest_player {};
    f32 closest_player_dist_sq { square(10.f) };
    for (u32 high_i { 1 }; high_i < state.high_cnt; ++high_i) {
        Entity test_ent { get_entity_from_high_i(state, high_i) };
        if (test_ent.low->type == Entity_Type::player) {
            f32 test_dist_sq { length_sq(test_ent.high->pos - entity.high->pos) };
            if (closest_player_dist_sq > test_dist_sq) {
                closest_player         = test_ent;
                closest_player_dist_sq = test_dist_sq;
            }
        }
    }

    if (closest_player.high) {
        Entity_Actions fam_acts {};
        f32 one_over_len { 1.f / sqrt_f32(closest_player_dist_sq) };
        f32 min_dist { 5.f };
        v2 dif { closest_player.high->pos - entity.high->pos };
        if (abs_f32(dif.x) > min_dist || abs_f32(dif.y) > min_dist)
            fam_acts.dir = one_over_len * (dif);
        move_entity(state, entity, fam_acts, dt);
    }
}

static void
update_monster(Game_State &state, Entity entity, f32 dt)
{
}

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    auto *state = (Game_State *)memory.permanent_storage;
    game_output_sound(sound_buffer);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    TomAssert(sizeof(Game_State) <= memory.permanent_storage_size);

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    // TODO: just fucking use a pointer?
    auto &state { (Game_State &)(*(Game_State *)memory.permanent_storage) };

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory.is_initialized) {
        // init memory
        init_arena(&state.world_arena, memory.permanent_storage_size - sizeof(state),
                   (u8 *)memory.permanent_storage + sizeof(state));

        state.world = PushStruct(&state.world_arena, World);

        World *world { state.world };
        init_world(*world, 1.4f);

        state.debug_draw_collision = false;

        const char *bg = "uv_color_squares_960x540";

        // load textures
        state.player_sprites[Entity_Direction::down] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_front");
        state.player_sprites[Entity_Direction::right] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_right");
        state.player_sprites[Entity_Direction::up] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_back");
        state.player_sprites[Entity_Direction::left] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "player_left");

        state.monster_sprites[Entity_Direction::down] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_front");
        state.monster_sprites[Entity_Direction::right] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_right");
        state.monster_sprites[Entity_Direction::up] =
            load_ARGB(thread, memory.platfrom_read_entire_file, "monster_back");
        state.monster_sprites[Entity_Direction::left] =
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

        s32 screen_base_x {}, screen_base_y {}, screen_base_z {}, virtual_z {}, rng_ind {};
        s32 screen_x { screen_base_x }, screen_y { screen_base_y }, screen_z { screen_base_z };

        // set world render size
        state.camera.pos.chunk_x = 0;
        state.camera.pos.chunk_y = 0;
        state.camera.pos.offset.x += screen_base_x / 2.f;
        state.camera.pos.offset.y += screen_base_y / 2.f;

        // NOTE: entity 0 is the null entity
        add_low_entity(state, Entity_Type::null);
        state.high_entities[0] = {};
        ++state.high_cnt;
        // state.player_cnt               = Game_Input::s_input_cnt;
        state.player_cnt               = 1;
        state.entity_camera_follow_ind = 1;

        // add the player entites
        for (u32 player_i { 1 }; player_i <= state.player_cnt; ++player_i) {
            add_player(state, player_i, 0.f, 0.f, 0.f, state.player_sprites);
        }

        f32 x_len { 55.f };

        for (f32 x { -20.f }; x < x_len; ++x) {
            add_wall(state, x, 15, 0.f, &state.tree_sprite);
            add_wall(state, x, -5, 0.f, &state.tree_sprite);
            if (scast(s32, x) % 17 == 0) continue;
            add_wall(state, x, 5, 0.f, &state.tree_sprite);
        }

        for (f32 x { -20.f }; x <= x_len; x += 15.f) {
            for (f32 y { -5 }; y < 15; ++y) {
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

    // REVIEW: if you put Game_World & it won't convert??????
    auto &world { state.world };
    Camera &camera { state.camera };

    auto p1 = get_entity_from_low_i(state, 1);

    // get input
    for (u32 player_i { 1 }; player_i <= state.player_cnt; ++player_i) {
        Entity_Low *low_player { get_low_entity(state, player_i) };
        assert(low_player->type == Entity_Type::player);

        Entity_Actions player_action {};
        if (player_i == 1) {
            player_action = process_keyboard(input.keyboard);
        } else {
            player_action = process_controller(input.controllers[player_i - 2]);
        }

        state.player_acts[player_i] = player_action;
    }

    if (is_key_up(input.keyboard.d1)) state.debug_draw_collision = !state.debug_draw_collision;

    Entity cam_ent { get_entity_from_low_i(state, state.entity_camera_follow_ind) };
    World_Dif entity_dif { get_diff(cam_ent.low->pos, camera.pos) };

    camera.pos.chunk_z = cam_ent.low->pos.chunk_z;

    // NOTE: camera is following the player
    World_Pos new_cam_pos { p1.low->pos };
    set_camera(state, new_cam_pos);
    v2 screen_center { .5f * (f32)video_buffer.width, .5f * (f32)video_buffer.height };
    Entity_Visble_Piece_Group piece_group {};

    // NOTE: *not* using PatBlt in the win32 layer
    Color clear_color { 0xff'4e'4e'4e };
    clear_buffer(video_buffer, clear_color);

    for (u32 high_i { 1 }; high_i < state.high_cnt; ++high_i) {
        piece_group.piece_cnt = 0;
        Entity ent { get_entity_from_high_i(state, high_i) };

        auto ent_dif { get_diff(ent.low->pos, camera.pos) };
        v2 ent_mid { (screen_center.x + (ent_dif.dif_xy.x * global::meters_to_pixels)),
                     (screen_center.y - (ent_dif.dif_xy.y * global::meters_to_pixels)) };

        // TODO: pull this out
        auto push_hp { [](Entity_Visble_Piece_Group &piece_group, Entity ent, v2 argb_mid) {
            for (u32 i {}; i < ent.low->hit_points; ++i) {
                push_piece(&piece_group, 3.f, 6.f, { colors::red },
                           v2 { argb_mid.x - (ent.low->width / 2.f) * global::meters_to_pixels -
                                    10.f + scast(f32, i) * 4.f,
                                argb_mid.y - ent.low->height * global::meters_to_pixels - 15.f },
                           ent.high->z);
            }
        } };

        switch (ent.low->type) {
            case Entity_Type::none: {
                draw_rect(
                    video_buffer, ent_mid.x - (ent.low->width * global::meters_to_pixels) / 2.f,
                    ent_mid.y - (ent.low->height * global::meters_to_pixels) / 2.f,
                    ent_mid.x + (ent.low->width * global::meters_to_pixels) / 2.f,
                    ent_mid.y + (ent.low->height * global::meters_to_pixels) / 2.f, { 0xffff00ff });
            } break;
            case Entity_Type::player: {
                // TODO: get player index from entity?
                move_entity(state, ent, state.player_acts[ent.low_i], input.delta_time);
                v2 argb_mid { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, &state.player_sprites[ent.high->direction], argb_mid,
                           ent.high->z);
                push_hp(piece_group, ent, argb_mid);

            } break;
            case Entity_Type::wall: {
                v2 argb_mid { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, &state.tree_sprite, argb_mid, ent.high->z);
            } break;
            case Entity_Type::stairs: {
                v2 argb_mid { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, &state.stair_sprite, argb_mid, ent.high->z);
            } break;
            case Entity_Type::familiar: {
                update_familiar(state, ent, input.delta_time);
                v2 argb_mid { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, &state.cat_sprite, argb_mid, ent.high->z);
            } break;
            case Entity_Type::monster: {
                update_monster(state, ent, input.delta_time);
                v2 argb_mid { ent_mid.x, ent_mid.y - ent.low->argb_offset };
                push_piece(&piece_group, &state.monster_sprites[ent.high->direction], argb_mid,
                           ent.high->z);
                push_hp(piece_group, ent, argb_mid);
            } break;
            default: {
                INVALID_CODE_PATH;
            } break;
        }

        // ===============================================================================================
        // #DRAW
        // ===============================================================================================
        //

        for (u32 piece_i {}; piece_i < piece_group.piece_cnt; ++piece_i) {
            Entity_Visible_Piece *piece { &piece_group.pieces[piece_i] };
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
    // NOTE: hacky way to draw a debug postion
    auto test_dif  { get_diff(state.test_pos, camera.pos) };
    v2 test_mid   { (screen_center.x + (test_dif.dif_xy.x * global::g_meters_to_pixels)),
                  (screen_center.y - (test_dif.dif_xy.y * global::g_meters_to_pixels)) };
    draw_ARGB(video_buffer, state.crosshair_img, test_mid);
#endif
}

}  // namespace tom
