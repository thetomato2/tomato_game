#include "game.hpp"

namespace tomato
{

namespace
{

void
clear_buffer(Game_offscreen_buffer& buffer_, Color_u32 color_ = { 0xFF'FF'00'FF })
{
    i32 width  = buffer_.width;
    i32 height = buffer_.height;

    u8* row = (u8*)buffer_.memory;
    for (i32 y = 0; y < height; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = 0; x < width; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

void
draw_rect(Game_offscreen_buffer& buffer_, f32 f32_min_x_, f32 f_min_y_, f32 f32_max_x_,
          f32 f32_max_y_, Color_u32 color_ = { 0xFFFFFFFF })
{
    i32 min_x = math::round_f32_to_i32(f32_min_x_);
    i32 min_y = math::round_f32_to_i32(f_min_y_);
    i32 max_x = math::round_f32_to_i32(f32_max_x_);
    i32 max_y = math::round_f32_to_i32(f32_max_y_);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer_.width) max_x = buffer_.width;
    if (max_y > buffer_.height) max_y = buffer_.height;

    u8* row = ((u8*)buffer_.memory + min_x * buffer_.bytes_per_pixel + min_y * buffer_.pitch);

    for (i32 y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = min_x; x < max_x; ++x) {
            *pixel++ = color_.argb;
        }
        row += buffer_.pitch;
    }
}

inline Tile_chunk_pos
get_chunk_pos(u32 abs_tile_x_, u32 abs_tile_y_)
{
    Tile_chunk_pos chunk_pos;

    chunk_pos.chunk_tile_x = abs_tile_x_ >> World::s_chunk_bit_shift;
    chunk_pos.chunk_tile_y = abs_tile_y_ >> World::s_chunk_bit_shift;
    chunk_pos.rel_tile_x   = abs_tile_x_ & World::s_chunk_bit_mask;
    chunk_pos.rel_tile_y   = abs_tile_y_ & World::s_chunk_bit_mask;

    return chunk_pos;
}

inline Tile_chunk_pos
get_chunk_pos(World_pos pos_)
{
    Tile_chunk_pos chunk_pos;

    chunk_pos = get_chunk_pos(pos_.abs_tile_x, pos_.abs_tile_y);

    return chunk_pos;
}

inline Tile_chunk*
get_tile_chunk(World& world_, i32 tile_chunk_x_, i32 tile_chunk_y_)
{
    Tile_chunk* tile_chunk = nullptr;

    if (tile_chunk_x_ >= 0 && tile_chunk_x_ < World::s_chunk_count && tile_chunk_y_ >= 0 &&
        tile_chunk_y_ < World::s_chunk_count) {
        tile_chunk = &world_.tile_chunks[tile_chunk_y_ * World::s_chunk_count + tile_chunk_x_];
    }
    return tile_chunk;
}

inline u32
get_tile_value_unchecked(Tile_chunk& tile_chunk_, u32 tile_x_, u32 tile_y_)
{
    assert(tile_chunk_.tiles);
    assert(tile_x_ <= World::s_chunk_tile_count && tile_y_ <= World::s_chunk_tile_count);
    return tile_chunk_.tiles[tile_y_ * World::s_chunk_tile_count + tile_x_];
}

u32
get_tile_value(World& world_, u32 abs_tile_x_, u32 abs_tile_y_)
{
    u32 tile_value {};

    Tile_chunk_pos chunk_pos = get_chunk_pos(abs_tile_x_, abs_tile_y_);
    Tile_chunk* tile_chunk = get_tile_chunk(world_, chunk_pos.chunk_tile_x, chunk_pos.chunk_tile_y);
    if (tile_chunk) {
        tile_value =
            get_tile_value_unchecked(*tile_chunk, chunk_pos.rel_tile_x, chunk_pos.rel_tile_y);
    }

    return tile_value;
}

bool
is_world_tile_empty(World& world_, World_pos test_pos_)
{
    bool is_empty = get_tile_value(world_, test_pos_.abs_tile_x, test_pos_.abs_tile_y) == 0;

    return is_empty;
}

inline void
recanonicalize_coord(const World& world_, u32& tile_, f32& tile_rel_)
{
    i32 offset = math::floorf_to_i32(tile_rel_ / (f32)World::s_tile_size_meters);

    // NOTE: wrold is assumed to be torodial (torus shaped world),
    // if you step off one end where you wrap around

    tile_ += offset;
    tile_rel_ -= offset * (f32)World::s_tile_size_meters;

    assert(tile_rel_ >= 0);
    assert(tile_rel_ <= World::s_tile_size_meters);
}

inline World_pos
recanonicalize_pos(World& world_, World_pos pos_)
{
    auto new_pos = pos_;

    recanonicalize_coord(world_, new_pos.abs_tile_x, new_pos.tile_rel_x);
    recanonicalize_coord(world_, new_pos.abs_tile_y, new_pos.tile_rel_y);

    return new_pos;
}

inline bool
check_player_collision(World& world_, Player player_, World_pos test_pos_)
{
    bool result                = false;
    auto test_pos_top_left     = test_pos_;
    auto test_pos_top_right    = test_pos_;
    auto test_pos_bottom_left  = test_pos_;
    auto test_pos_bottom_right = test_pos_;

    test_pos_top_right.tile_rel_x += Player::s_width;
    test_pos_bottom_left.tile_rel_y += Player::s_height;
    test_pos_bottom_right.tile_rel_x += Player::s_width;
    test_pos_bottom_right.tile_rel_y += Player::s_height;

    test_pos_top_left     = recanonicalize_pos(world_, test_pos_top_left);
    test_pos_top_right    = recanonicalize_pos(world_, test_pos_top_right);
    test_pos_bottom_left  = recanonicalize_pos(world_, test_pos_bottom_left);
    test_pos_bottom_right = recanonicalize_pos(world_, test_pos_bottom_right);

    // NOTE: checking each corner
    if (is_world_tile_empty(world_, test_pos_top_left) &&
        is_world_tile_empty(world_, test_pos_top_right) &&
        is_world_tile_empty(world_, test_pos_bottom_left) &&
        is_world_tile_empty(world_, test_pos_bottom_right))
        result = true;

    return result;
}

inline World_pos
get_player_center_pos(World& world_, Player& player_)
{
    auto center_pos = player_.pos;

    center_pos.tile_rel_x += Player::s_width / 2;
    center_pos.tile_rel_y += Player::s_height / 2;
    center_pos = recanonicalize_pos(world_, center_pos);

    return center_pos;
}

void
player_check_tile_map(World& world_, Player& player_)
{
    // NOTE: this function gets the center of the player,
    // then check if the player is out of the tile map,
    // and if so moves the player to the correct position on the new tile map
    auto player_center_pos = get_player_center_pos(world_, player_);
    auto* tile_map =
        get_tile_chunk(world_, player_center_pos.abs_tile_x, player_center_pos.abs_tile_y);
    if (tile_map != nullptr && world_.cur_tile_chunk != tile_map) world_.cur_tile_chunk = tile_map;
}

void
game_ouput_sound(Game_sound_output_buffer& sound_buffer_)
{
    // NOTE: outputs nothing atm
    i16 sample_value = 0;
    i16* sampleOut   = sound_buffer_.samples;
    for (szt sampleIndex = 0; sampleIndex < sound_buffer_.sample_count; ++sampleIndex) {
        *sampleOut++ = sample_value;
        *sampleOut++ = sample_value;
    }
}

}  // namespace

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
    auto* game_state = (Game_state*)memory_.permanent_storage;
    game_ouput_sound(sound_buffer_);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    assert(sizeof(Game_state) <= memory_.permanent_storage_size);

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    auto& game_state = (Game_state&)(*(Game_state*)memory_.permanent_storage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory_.is_initialized) {
        const char* file_name = __FILE__;

        printf("File name: %s\n", file_name);

        game_state.player.pos.tile_rel_x = .5f;
        game_state.player.pos.tile_rel_y = .5f;
        game_state.player.pos.abs_tile_x = 3;
        game_state.player.pos.abs_tile_y = 3;

        game_state.player.color = { 0xFF'FF'FF'00 };

        // TODO: this might be more appropriate in the platform layer
        memory_.is_initialized = true;
    }

    // ===============================================================================================
    // #Start
    // ===============================================================================================

    u32 tiles[World::s_chunk_tile_count][World::s_chunk_tile_count] = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1,
          1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
          1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    };

    World world {};

    f32 lower_left_x = (f32)World::s_tile_size_pixels / 2.f;
    f32 lower_left_y = (f32)video_buffer_.height;

    Tile_chunk tile_chunk;
    tile_chunk.tiles     = (u32*)tiles;
    world.tile_chunks    = &tile_chunk;
    world.cur_tile_chunk = &tile_chunk;

    world.cur_tile_chunk =
        get_tile_chunk(world, game_state.player.pos.abs_tile_x, game_state.player.pos.abs_tile_y);

    Game_controller_input& controller_0 = input_.controllers[0];
    if (controller_0.is_analog) {
        // gameState.xOffset += i32(speed * (controller0.endLX));
        // gameState.yOffset += i32(speed * (controller0.endLY));

    } else {
        // TODO: handle digital input_
    }

    static constexpr f32 player_speed = 5.f;

    auto& player = game_state.player;
    // TODO: temp
    player.pos          = recanonicalize_pos(world, player.pos);
    auto new_player_pos = player.pos;

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

    if (check_player_collision(world, player, new_player_pos)) {
        player.pos = recanonicalize_pos(world, new_player_pos);
        player_check_tile_map(world, player);
    }

    // ===============================================================================================
    // #Draw
    // ===============================================================================================

    // NOTE: *not* using PatBlt in the win32 layer
    Color_u32 clear_color { 0xFF'00'00'00 };
    clear_buffer(video_buffer_, clear_color);

    // NOTE: caching for clarity, not perf
    auto player_center_pos = get_player_center_pos(world, player);

    i32 num_draw_tiles = 10;
    f32 center_x       = .5f * (f32)video_buffer_.width;
    f32 center_y       = .5f * (f32)video_buffer_.height;

    for (i32 rel_y = -10; rel_y < num_draw_tiles; ++rel_y) {
        for (i32 rel_x = -10; rel_x < num_draw_tiles; ++rel_x) {
            u32 x = player.pos.abs_tile_x + rel_x;
            u32 y = player.pos.abs_tile_y + rel_y;

            u32 tile = get_tile_value(world, x, y);
            Color_u32 tile_color;
            if (player_center_pos.abs_tile_x == x && player_center_pos.abs_tile_y == y) {
                tile_color.argb = 0xFF'00'00'00;
            } else if (tile) {
                tile_color.argb = 0xFF'DD'DD'DD;
            } else {
                tile_color.argb = 0xFF'88'88'88;
            }

            f32 min_x = center_x - (player.pos.tile_rel_x * World::s_meters_to_pixels) +
                        (f32)rel_x * World::s_tile_size_pixels;
            f32 min_y = center_y + (player.pos.tile_rel_y * World::s_meters_to_pixels) -
                        (f32)rel_y * World::s_tile_size_pixels;
            f32 max_x = min_x + World::s_tile_size_pixels;
            f32 max_y = min_y - World::s_tile_size_pixels;

            draw_rect(video_buffer_, min_x, max_y, max_x, min_y, tile_color);
        }
    }

    f32 x = center_x;
    f32 y = center_y - (player.s_height * World::s_meters_to_pixels);

    draw_rect(video_buffer_, x, y, x + Player::s_width * World::s_meters_to_pixels,
              y + Player::s_height * World::s_meters_to_pixels, player.color);
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
