#include "game.hpp"

namespace tomato
{

namespace global
{
static constexpr u32 s_tile_size_pixels = 40;
static constexpr f32 s_meters_to_pixels = s_tile_size_pixels / Tile_Map::s_tile_size_meters;
static constexpr f32 s_player_max_vel   = 50.f;

#include "rng_nums.h"
}  // namespace global

internal void
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

internal void
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

internal void
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
inline internal bool
check_player_collision(Tile_Map &tile_map_, Player player_, Tile_Map_Pos test_pos_)
{
    bool result                = false;
    auto test_pos_top_left     = test_pos_;
    auto test_pos_top_right    = test_pos_;
    auto test_pos_bottom_left  = test_pos_;
    auto test_pos_bottom_right = test_pos_;

    test_pos_top_right.offset.x += Player::s_width;
    test_pos_bottom_left.offset.y += Player::s_height;
    test_pos_bottom_right.offset.x += Player::s_width;
    test_pos_bottom_right.offset.y += Player::s_height;

    test_pos_top_left     = recanonicalize_pos(tile_map_, test_pos_top_left);
    test_pos_top_right    = recanonicalize_pos(tile_map_, test_pos_top_right);
    test_pos_bottom_left  = recanonicalize_pos(tile_map_, test_pos_bottom_left);
    test_pos_bottom_right = recanonicalize_pos(tile_map_, test_pos_bottom_right);

    // NOTE: checking each corner
    if (is_world_tile_empty(tile_map_, test_pos_top_left) &&
        is_world_tile_empty(tile_map_, test_pos_top_right) &&
        is_world_tile_empty(tile_map_, test_pos_bottom_left) &&
        is_world_tile_empty(tile_map_, test_pos_bottom_right))
        result = true;

    return result;
}

inline internal Tile_Map_Pos
get_player_center_pos(Tile_Map &tile_map_, Player &player_)
{
    auto center_pos = player_.pos;

    center_pos.offset.x += Player::s_width / 2;
    center_pos.offset.y += Player::s_height / 2;
    center_pos = recanonicalize_pos(tile_map_, center_pos);

    return center_pos;
}

internal void
player_check_tile_map(Tile_Map &tile_map_, Player &player_)
{
    // NOTE: this function gets the center of the player,
    // then check if the player is out of the tile map,
    // and if so moves the player to the correct position on the new tile map
    auto player_center_pos = get_player_center_pos(tile_map_, player_);
    auto *tile_map         = get_tile_chunk(tile_map_, player_center_pos.abs_tile_x,
                                            player_center_pos.abs_tile_y, player_center_pos.abs_tile_z);
    if (tile_map != nullptr && tile_map_.cur_tile_chunk != tile_map)
        tile_map_.cur_tile_chunk = tile_map;
}

internal void
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

internal void
init_arena(Mem_Arena *arena_, mem_ind size_, byt *base_)
{
    arena_->size = size_;
    arena_->base = base_;
    arena_->used = 0;
}

internal Bitmap
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

internal ARGB_Img
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

internal Tile_Map_Dif
get_dif(Tile_Map_Pos pos_a, Tile_Map_Pos pos_b)
{
    Tile_Map_Dif result;
    v2 dif_tile_xy;
    dif_tile_xy.x  = (f32)pos_a.abs_tile_x - (f32)pos_b.abs_tile_x;
    dif_tile_xy.y  = (f32)pos_a.abs_tile_y - (f32)pos_b.abs_tile_y;
    f32 dif_tile_z = (f32)pos_a.abs_tile_z - (f32)pos_b.abs_tile_z;

    result.dif_xy = Tile_Map::s_tile_size_meters * dif_tile_xy + (pos_a.offset - pos_b.offset);
    result.dif_z  = 0.f;

    return result;
}

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

    u32 constexpr num_screens { 100 };
    u32 constexpr num_tiles_per_screen_x { 20 };
    u32 constexpr num_tiles_per_screen_y { 11 };

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory_.is_initialized) {
        const char *bg            = "uv_color_squares_960x540";
        const char *seaside_cliff = "bg_seaside_cliff";
        const char *red_square    = "red_square";
        const char *green_square  = "green_square";
        const char *blue_square   = "blue_square";

        const char *player_front = "girl_chibi_front";
        const char *player_back  = "girl_chibi_back";
        const char *player_left  = "girl_chibi_left";
        const char *player_right = "girl_chibi_right";

        // game_state.bitmap   = load_bmp(thread_, memory_.platfrom_read_entire_file, bmp_path);
        game_state.bg_img = load_ARGB(thread_, memory_.platfrom_read_entire_file, bg);
        game_state.seaside_cliff =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, seaside_cliff);

        game_state.red_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, red_square);
        game_state.green_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, green_square);
        game_state.blue_square_img =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, blue_square);

        game_state.player_img[0] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_front);
        game_state.player_img[1] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_right);
        game_state.player_img[2] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_back);
        game_state.player_img[3] =
            load_ARGB(thread_, memory_.platfrom_read_entire_file, player_left);

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

        u32 screen_y {};
        u32 screen_x {};
        u32 screen_z {};
        u32 rng_ind {};

        bool door_left   = false;
        bool door_right  = false;
        bool door_top    = false;
        bool door_bottom = false;
        bool stairs      = false;
        bool stairs_back = false;

        for (u32 screen_ind {}; screen_ind < num_screens; ++screen_ind) {
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

            for (u32 tile_y {}; tile_y < num_tiles_per_screen_y; ++tile_y) {
                for (u32 tile_x {}; tile_x < num_tiles_per_screen_x; ++tile_x) {
                    u32 abs_tile_x = screen_x * num_tiles_per_screen_x + tile_x;
                    u32 abs_tile_y = screen_y * num_tiles_per_screen_y + tile_y;
                    u32 tile_value = 1;

                    if (tile_x == 0 && !((tile_y == num_tiles_per_screen_y / 2 ||
                                          (tile_y == num_tiles_per_screen_y / 2) - 1) &&
                                         door_left)) {
                        tile_value = 2;
                    }
                    if (tile_x == num_tiles_per_screen_x - 1 &&
                        !((tile_y == num_tiles_per_screen_y / 2 ||
                           (tile_y == num_tiles_per_screen_y / 2) - 1) &&
                          door_right)) {
                        tile_value = 2;
                    }
                    if (tile_y == 0 && !((tile_x == num_tiles_per_screen_x / 2 ||
                                          (tile_x == num_tiles_per_screen_x / 2) - 1) &&
                                         door_bottom)) {
                        tile_value = 2;
                    }

                    if (tile_y == num_tiles_per_screen_y - 1 &&
                        !((tile_x == num_tiles_per_screen_x / 2 ||
                           (tile_x == num_tiles_per_screen_x / 2) - 1) &&
                          door_top)) {
                        tile_value = 2;
                    }

                    set_tile_value(&game_state.world_arena, *tile_map, abs_tile_x, abs_tile_y,
                                   screen_z, tile_value);
                }
            }

            if (stairs || stairs_back) {
                u32 mid_tile_x = screen_x * num_tiles_per_screen_x + num_tiles_per_screen_x / 2;
                u32 mid_tile_y = screen_y * num_tiles_per_screen_y + num_tiles_per_screen_y / 2;
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

        game_state.camera.pos.abs_tile_x = num_tiles_per_screen_x / 2;
        game_state.camera.pos.abs_tile_y = num_tiles_per_screen_y / 2;

        game_state.player.pos.offset.x   = .5f;
        game_state.player.pos.offset.y   = .5f;
        game_state.player.pos.abs_tile_x = 3;
        game_state.player.pos.abs_tile_y = 3;
        game_state.player.color          = { 0xff'00'00'ff };
        game_state.player.direction      = 0;
        game_state.player.vel            = {};

        // TODO: this might be more appropriate in the platform layer
        memory_.is_initialized = true;
    }

    // ===============================================================================================
    // #Start
    // ===============================================================================================

    World *world = game_state.world;

    world->tile_map->cur_tile_chunk =
        get_tile_chunk(*world->tile_map, game_state.player.pos.abs_tile_x,
                       game_state.player.pos.abs_tile_y, game_state.player.pos.abs_tile_z);

    Game_Controller_Input &controller_0 = input_.controllers[0];
    if (controller_0.is_analog) {
        // gameState.xOffset += s32(speed * (controller0.endLX));
        // gameState.yOffset += s32(speed * (controller0.endLY));

    } else {
        // TODO: handle digital input_
    }

    auto &player = game_state.player;
    auto &camera = game_state.camera;

    v2 player_acc {};

    f32 player_speed {};
    player_speed = input_.keyboard.left_shift.ended_down ? 50.f : 10.f;

    if (input_.keyboard.w.ended_down) {
        player_acc.y     = player_speed;
        player.direction = 2;
    } else if (input_.keyboard.s.ended_down) {
        player_acc.y     = -player_speed;
        player.direction = 0;
    }

    if (input_.keyboard.d.ended_down) {
        player_acc.x     = player_speed;
        player.direction = 1;
    } else if (input_.keyboard.a.ended_down) {
        player_acc.x     = -player_speed;
        player.direction = 3;
    }
    player_acc -= player.vel * 2.f;

    player.pos          = recanonicalize_pos(*world->tile_map, player.pos);
    auto new_player_pos = player.pos;
    new_player_pos.offset +=
        (.5f * player_acc * math::square(input_.delta_time) + player.vel * input_.delta_time);

    if (check_player_collision(*world->tile_map, player, new_player_pos)) {
        player.pos = recanonicalize_pos(*world->tile_map, new_player_pos);
        player.vel = player_acc * input_.delta_time + player.vel;
        player_check_tile_map(*world->tile_map, player);

        if (get_tile_value(*world->tile_map, player.pos.abs_tile_x, player.pos.abs_tile_y,
                           player.pos.abs_tile_z) == 3) {
            player.pos.abs_tile_z == 0 ? player.pos.abs_tile_z = 1 : player.pos.abs_tile_z = 0;
            player.pos.abs_tile_x += 1;
            player.pos.abs_tile_y += 1;
        }
        camera.pos.abs_tile_z = player.pos.abs_tile_z;

        Tile_Map_Dif player_dif = get_dif(player.pos, camera.pos);
        if (player_dif.dif_xy.x > (num_tiles_per_screen_x * Tile_Map::s_tile_size_meters) / 2) {
            camera.pos.abs_tile_x += num_tiles_per_screen_x / 2;
        }
        if (player_dif.dif_xy.x < (num_tiles_per_screen_x * Tile_Map::s_tile_size_meters) / -2) {
            camera.pos.abs_tile_x -= num_tiles_per_screen_x / 2;
        }
        if (player_dif.dif_xy.y > (num_tiles_per_screen_y * Tile_Map::s_tile_size_meters) / 2) {
            camera.pos.abs_tile_y += num_tiles_per_screen_y / 2;
        }
        if (player_dif.dif_xy.y < (num_tiles_per_screen_y * Tile_Map::s_tile_size_meters) / -2) {
            camera.pos.abs_tile_y -= num_tiles_per_screen_y / 2;
        }
    } else {
        player.vel = { 0.f, 0.f };
    }
    player.vel.x = check_bounds(player.vel.x, -global::s_player_max_vel, global::s_player_max_vel);
    player.vel.y = check_bounds(player.vel.y, -global::s_player_max_vel, global::s_player_max_vel);

    // ===============================================================================================
    // #Draw
    // ===============================================================================================

    // NOTE: *not* using PatBlt in the win32 layer
    Color_u32 clear_color { 0xff'00'00'00 };
    clear_buffer(video_buffer_, clear_color);

    // FIXME: test code
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

    Tile_Map_Dif player_dif = get_dif(player.pos, camera.pos);

    v2 player_mid { (screen_center.x + (player_dif.dif_xy.x * global::s_meters_to_pixels)),
                    (screen_center.y - (player_dif.dif_xy.y * global::s_meters_to_pixels) -
                     (player.s_height * global::s_meters_to_pixels)) };

    v2 argb_mid { player_mid.x + (((f32)Player::s_width / 2.f) * global::s_meters_to_pixels),
                  player_mid.y + (((f32)Player::s_height / 2.f) * global::s_meters_to_pixels) -
                      40 };

    draw_ARGB(video_buffer_, game_state.player_img[player.direction], argb_mid);

    constexpr s32 square_offset = 0;
    draw_ARGB(video_buffer_, game_state.red_square_img,
              { f32(64 + square_offset), f32(video_buffer_.height - 64) });
    draw_ARGB(video_buffer_, game_state.green_square_img,
              { f32(64 + square_offset), f32(video_buffer_.height - 192) });
    draw_ARGB(video_buffer_, game_state.blue_square_img,
              { f32(64 + square_offset), f32(video_buffer_.height - 320) });
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
