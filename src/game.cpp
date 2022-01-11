#include "game.hpp"

namespace tomato
{

namespace global
{
static constexpr u32 s_tile_size_pixels = 60;
static constexpr f32 s_meters_to_pixels = s_tile_size_pixels / TileMap::s_tile_size_meters;
}  // namespace global

namespace
{
#include "rng_nums.h"

void
clear_buffer(GameOffscreenBuffer &buffer_, u32_Color color_ = { 0xff'ff'00'ff })
{
    i32 width  = buffer_.width;
    i32 height = buffer_.height;

    byt *row = (byt *)buffer_.memory;
    for (i32 y = 0; y < height; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = 0; x < width; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

void
draw_rect(GameOffscreenBuffer &buffer_, f32 f32_min_x_, f32 f_min_y_, f32 f32_max_x_,
          f32 f32_max_y_, u32_Color color_ = { 0xffffffff })
{
    i32 min_x = math::round_f32_to_i32(f32_min_x_);
    i32 min_y = math::round_f32_to_i32(f_min_y_);
    i32 max_x = math::round_f32_to_i32(f32_max_x_);
    i32 max_y = math::round_f32_to_i32(f32_max_y_);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer_.width) max_x = buffer_.width;
    if (max_y > buffer_.height) max_y = buffer_.height;

    byt *row = ((byt *)buffer_.memory + min_x * buffer_.bytes_per_pixel + min_y * buffer_.pitch);

    for (i32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = min_x; x < max_x; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

inline bool
check_player_collision(TileMap &tile_map_, Player player_, TileMapPos test_pos_)
{
    bool result                = false;
    auto test_pos_top_left     = test_pos_;
    auto test_pos_top_right    = test_pos_;
    auto test_pos_bottom_left  = test_pos_;
    auto test_pos_bottom_right = test_pos_;

    test_pos_top_right.off_rel_x += Player::s_width;
    test_pos_bottom_left.off_rel_y += Player::s_height;
    test_pos_bottom_right.off_rel_x += Player::s_width;
    test_pos_bottom_right.off_rel_y += Player::s_height;

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

inline TileMapPos
get_player_center_pos(TileMap &tile_map_, Player &player_)
{
    auto center_pos = player_.pos;

    center_pos.off_rel_x += Player::s_width / 2;
    center_pos.off_rel_y += Player::s_height / 2;
    center_pos = recanonicalize_pos(tile_map_, center_pos);

    return center_pos;
}

void
player_check_tile_map(TileMap &tile_map_, Player &player_)
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

void
game_ouput_sound(GameSoundOutputBuffer &sound_buffer_)
{
    // NOTE: outputs nothing atm
    i16 sample_value = 0;
    i16 *sampleOut   = sound_buffer_.samples;
    for (szt sampleIndex = 0; sampleIndex < sound_buffer_.sample_count; ++sampleIndex) {
        *sampleOut++ = sample_value;
        *sampleOut++ = sample_value;
    }
}

void
init_arena(MemArena *arena_, mem_ind size_, byt *base_)
{
    arena_->size = size_;
    arena_->base = base_;
    arena_->used = 0;
}

}  // namespace

Bitmap
load_bmp(ThreadContext *thread_, debug_platform_read_entire_file *read_entire_file_,
         const char *file_name_)
{
    debug_ReadFileResult read_result = read_entire_file_(thread_, file_name_);
    Bitmap result;

    if (read_result.content_size != 0) {
        BitmapHeader *header = (BitmapHeader *)read_result.contents;
        u32 *pixels          = (u32 *)((byt *)read_result.contents + header->bitmap_offset);
        result.width         = header->width;
        result.height        = header->height;
        result.pixel_ptr     = pixels;
    }
    return result;
}

void *
push_size(MemArena *arena_, mem_ind size_)
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
    auto *state = (GameState *)memory_.permanent_storage;
    game_ouput_sound(sound_buffer_);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    assert(sizeof(GameState) <= memory_.permanent_storage_size);

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    auto &game_state = (GameState &)(*(GameState *)memory_.permanent_storage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory_.is_initialized) {
        const char *bmp_path = "C:/dev/tomato_game/assets/images/uv_color_square_1280x720.bmp";
        game_state.bitmap    = load_bmp(thread_, memory_.platfrom_read_entire_file, bmp_path);

        init_arena(&game_state.world_arena, memory_.permanent_storage_size - sizeof(game_state),
                   (u8 *)memory_.permanent_storage + sizeof(game_state));

        game_state.world = PushStruct(&game_state.world_arena, World);

        World *world          = game_state.world;
        world->tile_map       = PushStruct(&game_state.world_arena, TileMap);
        TileMap *tile_map     = world->tile_map;
        tile_map->tile_chunks = PushArray(
            &game_state.world_arena,
            TileMap::s_chunk_count * TileMap::s_chunk_count * TileMap::s_chunk_count_z, TileChunk);

        u32 constexpr num_screens { 50 };
        u32 constexpr num_tiles_per_screen_x { 16 };
        u32 constexpr num_tiles_per_screen_y { 9 };

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

            for (u32 tile_y {}; tile_y < num_tiles_per_screen_y; ++tile_y) {
                for (u32 tile_x {}; tile_x < num_tiles_per_screen_x; ++tile_x) {
                    u32 abs_tile_x = screen_x * num_tiles_per_screen_x + tile_x;
                    u32 abs_tile_y = screen_y * num_tiles_per_screen_y + tile_y;
                    u32 tile_value = 1;

                    if (tile_x == 0 && !(tile_y == num_tiles_per_screen_y / 2 && door_left)) {
                        tile_value = 2;
                    }
                    if (tile_x == num_tiles_per_screen_x - 1 &&
                        !(tile_y == num_tiles_per_screen_y / 2 && door_right)) {
                        tile_value = 2;
                    }
                    if (tile_y == 0 && !(tile_x == num_tiles_per_screen_x / 2 && door_bottom)) {
                        tile_value = 2;
                    }

                    if (tile_y == num_tiles_per_screen_y - 1 &&
                        !(tile_x == num_tiles_per_screen_x / 2 && door_top)) {
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

        game_state.player.pos.off_rel_x  = .5f;
        game_state.player.pos.off_rel_y  = .5f;
        game_state.player.pos.abs_tile_x = 3;
        game_state.player.pos.abs_tile_y = 3;
        game_state.player.color          = { 0xff'00'00'ff };

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

    GameControllerInput &controller_0 = input_.controllers[0];
    if (controller_0.is_analog) {
        // gameState.xOffset += i32(speed * (controller0.endLX));
        // gameState.yOffset += i32(speed * (controller0.endLY));

    } else {
        // TODO: handle digital input_
    }

    auto &player = game_state.player;
    // TODO: temp
    player.pos          = recanonicalize_pos(*world->tile_map, player.pos);
    auto new_player_pos = player.pos;

    f32 player_speed {};
    player_speed = input_.keyboard.left_shift.ended_down ? 20.f : 5.f;

    if (input_.keyboard.w.ended_down) {
        new_player_pos.off_rel_y += player_speed * input_.deltaTime;
    } else if (input_.keyboard.s.ended_down) {
        new_player_pos.off_rel_y -= player_speed * input_.deltaTime;
    }

    if (input_.keyboard.d.ended_down) {
        new_player_pos.off_rel_x += player_speed * input_.deltaTime;
    } else if (input_.keyboard.a.ended_down) {
        new_player_pos.off_rel_x -= player_speed * input_.deltaTime;
    }

    if (check_player_collision(*world->tile_map, player, new_player_pos)) {
        player.pos = recanonicalize_pos(*world->tile_map, new_player_pos);
        player_check_tile_map(*world->tile_map, player);

        if (get_tile_value(*world->tile_map, player.pos.abs_tile_x, player.pos.abs_tile_y,
                           player.pos.abs_tile_z) == 3) {
            player.pos.abs_tile_z == 0 ? player.pos.abs_tile_z = 1 : player.pos.abs_tile_z = 0;
            player.pos.abs_tile_x += 2;
            player.pos.abs_tile_y += 2;

            printf("player touched stairs!\n");
        }
    }

    // ===============================================================================================
    // #Draw
    // ===============================================================================================

    // NOTE: *not* using PatBlt in the win32 layer
    u32_Color clear_color { 0xff'00'00'00 };
    clear_buffer(video_buffer_, clear_color);

    // NOTE: caching for clarity, not perf
    auto player_center_pos = get_player_center_pos(*world->tile_map, player);

    i32 num_draw_tiles  = 12;
    f32 screen_center_x = .5f * (f32)video_buffer_.width;
    f32 screen_center_y = .5f * (f32)video_buffer_.height;

    for (i32 rel_y = -1 * num_draw_tiles; rel_y < num_draw_tiles; ++rel_y) {
        for (i32 rel_x = -1 * num_draw_tiles; rel_x < num_draw_tiles; ++rel_x) {
            u32 x = player.pos.abs_tile_x + rel_x;
            u32 y = player.pos.abs_tile_y + rel_y;

            u32 tile = get_tile_value(*world->tile_map, x, y, player.pos.abs_tile_z);
            u32_Color tile_color;
            if (tile == 3) {
                tile_color.argb = 0xff'1e'1e'1e;
            } else if (tile == 2) {
                tile_color.argb = 0xff'dd'dd'dd;
            } else if (tile == 1) {
                tile_color.argb = 0xff'88'88'88;
            } else {
                tile_color.argb = 0xff'ff'00'00;
            }

            f32 cen_x = screen_center_x - (player.pos.off_rel_x * global::s_meters_to_pixels) +
                        (f32)rel_x * global::s_tile_size_pixels;
            f32 cen_y = screen_center_y + (player.pos.off_rel_y * global::s_meters_to_pixels) -
                        (f32)rel_y * global::s_tile_size_pixels;
            f32 min_x = cen_x - .5f * global::s_tile_size_pixels;
            f32 min_y = cen_y - .5f * global::s_tile_size_pixels;
            f32 max_x = cen_x + .5f * global::s_tile_size_pixels;
            f32 max_y = cen_y + .5f * global::s_tile_size_pixels;

            draw_rect(video_buffer_, min_x, min_y, max_x, max_y, tile_color);
        }
    }

    f32 x = screen_center_x;
    f32 y = screen_center_y - (player.s_height * global::s_meters_to_pixels);

    draw_rect(video_buffer_, x, y, x + Player::s_width * global::s_meters_to_pixels,
              y + Player::s_height * global::s_meters_to_pixels, player.color);

    // FIXME: test code
    u32 *source = game_state.bitmap.pixel_ptr;
    source += game_state.bitmap.width * game_state.bitmap.height;
    u32 *dest = (u32 *)video_buffer_.memory;
    for (i32 y {}; y < game_state.bitmap.height && y < video_buffer_.height; ++y) {
        source -= game_state.bitmap.width;
        for (i32 x {}; x < game_state.bitmap.width && x < video_buffer_.width; ++x) {
            *dest++ = *source++;
        }
        source -= game_state.bitmap.width;
        dest += video_buffer_.width - game_state.bitmap.width;
    }
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
