#include "game.hpp"

namespace tomato
{

namespace
{

void
clear_buffer(game_offscreen_buffer &buffer_, color_u32 color_ = { 0xFF'FF'00'FF })
{
    i32 width  = buffer_.width;
    i32 height = buffer_.height;

    u8 *row = (u8 *)buffer_.memory;
    for (i32 y = 0; y < height; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = 0; x < width; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

void
draw_rect(game_offscreen_buffer &buffer_, f32 f32_min_x_, f32 f_min_y_, f32 f32_max_x_,
          f32 f32_max_y_, color_u32 color_ = { 0xFFFFFFFF })
{
    i32 min_x = math::round_f32_to_i32(f32_min_x_);
    i32 min_y = math::round_f32_to_i32(f_min_y_);
    i32 max_x = math::round_f32_to_i32(f32_max_x_);
    i32 max_y = math::round_f32_to_i32(f32_max_y_);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer_.width) max_x = buffer_.width;
    if (max_y > buffer_.height) max_y = buffer_.height;

    u8 *row = ((u8 *)buffer_.memory + min_x * buffer_.bytes_per_pixel + min_y * buffer_.pitch);

    for (i32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = min_x; x < max_x; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

inline bool
check_player_collision(tile_map &tile_map_, player player_, tile_map_pos test_pos_)
{
    bool result                = false;
    auto test_pos_top_left     = test_pos_;
    auto test_pos_top_right    = test_pos_;
    auto test_pos_bottom_left  = test_pos_;
    auto test_pos_bottom_right = test_pos_;

    test_pos_top_right.tile_rel_x += player::s_width;
    test_pos_bottom_left.tile_rel_y += player::s_height;
    test_pos_bottom_right.tile_rel_x += player::s_width;
    test_pos_bottom_right.tile_rel_y += player::s_height;

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

inline tile_map_pos
get_player_center_pos(tile_map &tile_map_, player &player_)
{
    auto center_pos = player_.pos;

    center_pos.tile_rel_x += player::s_width / 2;
    center_pos.tile_rel_y += player::s_height / 2;
    center_pos = recanonicalize_pos(tile_map_, center_pos);

    return center_pos;
}

void
player_check_tile_map(tile_map &tile_map_, player &player_)
{
    // NOTE: this function gets the center of the player,
    // then check if the player is out of the tile map,
    // and if so moves the player to the correct position on the new tile map
    auto player_center_pos = get_player_center_pos(tile_map_, player_);
    auto *tile_map =
        get_tile_chunk(tile_map_, player_center_pos.abs_tile_x, player_center_pos.abs_tile_y);
    if (tile_map != nullptr && tile_map_.cur_tile_chunk != tile_map)
        tile_map_.cur_tile_chunk = tile_map;
}

void
game_ouput_sound(game_sound_output_buffer &sound_buffer_)
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
init_arena(mem_arena *arena_, mem_ind size_, u8 *base_)
{
    arena_->size = size_;
    arena_->base = base_;
    arena_->used = 0;
}

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))

void *
push_size(mem_arena *arena_, mem_ind size_)
{
    assert((arena_->used + size_) <= arena_->size);
    void *result = arena_->base + arena_->used;
    arena_->used += size_;

    return result;
}

}  // namespace

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    auto *state = (game_state *)memory_.permanent_storage;
    game_ouput_sound(sound_buffer_);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    assert(sizeof(game_state) <= memory_.permanent_storage_size);

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    auto &state = (game_state &)(*(game_state *)memory_.permanent_storage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory_.is_initialized) {
        init_arena(&state.world_arena, memory_.permanent_storage_size - sizeof(state),
                   (u8 *)memory_.permanent_storage + sizeof(state));

        state.world = PushStruct(&state.world_arena, world);

        world *world          = state.world;
        world->tile_map       = PushStruct(&state.world_arena, tile_map);
        tile_map *tile_map    = world->tile_map;
        tile_map->tile_chunks = PushArray(
            &state.world_arena, tile_map::s_chunk_count * tile_map::s_chunk_count, tile_chunk);

        u32 constexpr num_screens { 16 };
        u32 constexpr num_tiles_per_screen_x { 16 };
        u32 constexpr num_tiles_per_screen_y { 9 };

        for (u32 y {}; y < tile_map::s_chunk_count; ++y) {
            for (u32 x {}; x < tile_map::s_chunk_count; ++x) {
                tile_map->tile_chunks[y * tile_map::s_chunk_count * x].tiles = PushArray(
                    &state.world_arena, tile_map::s_chunk_count * tile_map::s_chunk_count, u32);
            }
        }

        for (u32 screen_y {}; screen_y < num_screens; ++screen_y) {
            for (u32 screen_x {}; screen_x < num_screens; ++screen_x) {
                for (u32 tile_y {}; tile_y < num_tiles_per_screen_y; ++tile_y) {
                    for (u32 tile_x {}; tile_x < num_tiles_per_screen_x; ++tile_x) {
                        u32 abs_tile_x = screen_x * num_tiles_per_screen_x + tile_x;
                        u32 abs_tile_y = screen_y * num_tiles_per_screen_y + tile_y;
                        u32 tile_value = 0;

                        if (tile_x == 0 || tile_x == num_tiles_per_screen_x - 1) {
                            tile_value = tile_y == num_tiles_per_screen_y / 2 ? 0 : 1;
                        }

                        if (tile_y == 0 || tile_y == num_tiles_per_screen_y - 1) {
                            tile_value = tile_x == num_tiles_per_screen_x / 2 ? 0 : 1;
                        }

                        set_tile_value(&state.world_arena, *tile_map, abs_tile_x, abs_tile_y,
                                       tile_value);
                    }
                }
            }
        }

        state.player.pos.tile_rel_x = .5f;
        state.player.pos.tile_rel_y = .5f;
        state.player.pos.abs_tile_x = 3;
        state.player.pos.abs_tile_y = 3;
        state.player.color          = { 0xFF'FF'FF'00 };

        // TODO: this might be more appropriate in the platform layer
        memory_.is_initialized = true;
    }

    // ===============================================================================================
    // #Start
    // ===============================================================================================

    world *world = state.world;

    world->tile_map->cur_tile_chunk =
        get_tile_chunk(*world->tile_map, state.player.pos.abs_tile_x, state.player.pos.abs_tile_y);

    game_controller_input &controller_0 = input_.controllers[0];
    if (controller_0.is_analog) {
        // gameState.xOffset += i32(speed * (controller0.endLX));
        // gameState.yOffset += i32(speed * (controller0.endLY));

    } else {
        // TODO: handle digital input_
    }

    auto &player = state.player;
    // TODO: temp
    player.pos          = recanonicalize_pos(*world->tile_map, player.pos);
    auto new_player_pos = player.pos;

    f32 player_speed {};
    player_speed = input_.keyboard.left_shift.ended_down ? 20.f : 5.f;

    if (input_.keyboard.w.ended_down) {
        new_player_pos.tile_rel_y += player_speed * input_.deltaTime;
    } else if (input_.keyboard.s.ended_down) {
        new_player_pos.tile_rel_y -= player_speed * input_.deltaTime;
    }

    if (input_.keyboard.d.ended_down) {
        new_player_pos.tile_rel_x += player_speed * input_.deltaTime;
    } else if (input_.keyboard.a.ended_down) {
        new_player_pos.tile_rel_x -= player_speed * input_.deltaTime;
    }

    if (check_player_collision(*world->tile_map, player, new_player_pos)) {
        player.pos = recanonicalize_pos(*world->tile_map, new_player_pos);
        player_check_tile_map(*world->tile_map, player);
    }

    // ===============================================================================================
    // #Draw
    // ===============================================================================================

    // NOTE: *not* using PatBlt in the win32 layer
    color_u32 clear_color { 0xFF'00'00'00 };
    clear_buffer(video_buffer_, clear_color);

    // NOTE: caching for clarity, not perf
    auto player_center_pos = get_player_center_pos(*world->tile_map, player);

    i32 num_draw_tiles  = 10;
    f32 screen_center_x = .5f * (f32)video_buffer_.width;
    f32 screen_center_y = .5f * (f32)video_buffer_.height;

    for (i32 rel_y = -10; rel_y < num_draw_tiles; ++rel_y) {
        for (i32 rel_x = -10; rel_x < num_draw_tiles; ++rel_x) {
            u32 x = player.pos.abs_tile_x + rel_x;
            u32 y = player.pos.abs_tile_y + rel_y;

            u32 tile = get_tile_value(*world->tile_map, x, y);
            color_u32 tile_color;
            if (player_center_pos.abs_tile_x == x && player_center_pos.abs_tile_y == y) {
                tile_color.argb = 0xFF'00'00'00;
            } else if (tile) {
                tile_color.argb = 0xFF'DD'DD'DD;
            } else {
                tile_color.argb = 0xFF'88'88'88;
            }

            f32 cen_x = screen_center_x - (player.pos.tile_rel_x * tile_map::s_meters_to_pixels) +
                        (f32)rel_x * tile_map::s_tile_size_pixels;
            f32 cen_y = screen_center_y + (player.pos.tile_rel_y * tile_map::s_meters_to_pixels) -
                        (f32)rel_y * tile_map::s_tile_size_pixels;
            f32 min_x = cen_x - .5f * tile_map::s_tile_size_pixels;
            f32 min_y = cen_y - .5f * tile_map::s_tile_size_pixels;
            f32 max_x = cen_x + .5f * tile_map::s_tile_size_pixels;
            f32 max_y = cen_y + .5f * tile_map::s_tile_size_pixels;

            draw_rect(video_buffer_, min_x, min_y, max_x, max_y, tile_color);
        }
    }

    f32 x = screen_center_x;
    f32 y = screen_center_y - (player.s_height * tile_map::s_meters_to_pixels);

    draw_rect(video_buffer_, x, y, x + player::s_width * tile_map::s_meters_to_pixels,
              y + player::s_height * tile_map::s_meters_to_pixels, player.color);
}

#if 0
    #ifdef TOM_WIN32
namespace win32
{
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headerfs
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
