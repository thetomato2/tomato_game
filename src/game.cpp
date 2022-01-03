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

inline Tile_map*
get_tile_map(World& world_, World_pos pos_)
{
	Tile_map* tile_map = nullptr;

	if (pos_.tile_map_x >= 0 && pos_.tile_map_x < World::s_tile_map_count_x &&
		pos_.tile_map_y >= 0 && pos_.tile_map_y < World::s_tile_map_count_y) {
		tile_map = &world_.tile_maps[pos_.tile_map_y * World::s_tile_map_count_x + pos_.tile_map_x];
	}
	return tile_map;
}

inline u32
get_tile_value_unchecked(Tile_map& tile_map_, i32 tile_x_, i32 tile_y_)
{
	assert(tile_map_.tiles);
	assert((tile_x_ >= 0) && (tile_x_ < World::s_tile_count_x) && (tile_y_ >= 0) &&
		   (tile_y_ < World::s_tile_count_y));
	return tile_map_.tiles[tile_y_ * World::s_tile_count_x + tile_x_];
}

bool
is_tile_empty(Tile_map& tile_map_, i32 test_tile_x_, i32 test_tile_y_)
{
	bool is_empty { false };

	// printf("x: %d, y: %d", player_tile_x, player_tile_y);

	// NOTE: if tiles is null map does not exists
	if (tile_map_.tiles) {
		if (test_tile_x_ >= 0 && test_tile_x_ < World::s_tile_count_x && test_tile_y_ >= 0 &&
			test_tile_y_ < World::s_tile_count_y) {
			auto tile_map_value = get_tile_value_unchecked(tile_map_, test_tile_x_, test_tile_y_);
			is_empty			= (tile_map_value == 0);
		}
	}

	return is_empty;
}

inline void
recanonicalize_coord(const World& world_, const i32 tile_count_, i32& tile_map_, i32& tile_,
					 f32& tile_rel_)
{
	i32 offset = math::floorf_to_i32(tile_rel_ / (f32)World::s_tile_size_meters);
	tile_ += offset;
	tile_rel_ -= offset * (f32)World::s_tile_size_meters;

	assert(tile_rel_ >= 0);
	assert(tile_rel_ <= World::s_tile_size_meters);

	// check bounds to see if player is in a neighboring tile map
	if (tile_ < 0) {
		tile_ = tile_count_ + tile_;
		--tile_map_;
	}
	if (tile_ > tile_count_ - 1) {
		tile_ = tile_ - tile_count_;
		++tile_map_;
	}
}

inline World_pos
recanonicalize_pos(World& world_, World_pos pos_)
{
	auto result = pos_;

	recanonicalize_coord(world_, world_.s_tile_count_x, result.tile_map_x, result.tile_x,
						 result.tile_rel_x);
	recanonicalize_coord(world_, world_.s_tile_count_y, result.tile_map_y, result.tile_y,
						 result.tile_rel_y);

	return result;
}

bool
is_world_tile_empty(World& world_, World_pos test_pos_)
{
	bool is_empty { false };

	World_pos can_pos  = test_pos_;
	Tile_map& tile_map = *(get_tile_map(world_, test_pos_));
	is_empty		   = is_tile_empty(tile_map, can_pos.tile_x, can_pos.tile_y);

	return is_empty;
}

inline bool
check_player_collision(World& world_, Player player_, World_pos test_pos_)
{
	bool result				   = false;
	auto test_pos_top_left	   = test_pos_;
	auto test_pos_top_right	   = test_pos_;
	auto test_pos_bottom_left  = test_pos_;
	auto test_pos_bottom_right = test_pos_;

	test_pos_top_right.tile_rel_x += Player::s_width;
	test_pos_bottom_left.tile_rel_y += Player::s_height;
	test_pos_bottom_right.tile_rel_x += Player::s_width;
	test_pos_bottom_right.tile_rel_y += Player::s_height;

	test_pos_top_left	  = recanonicalize_pos(world_, test_pos_top_left);
	test_pos_top_right	  = recanonicalize_pos(world_, test_pos_top_right);
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
	auto result = player_.pos;

	result.tile_rel_x += Player::s_width / 2;
	result.tile_rel_y += Player::s_height / 2;
	result = recanonicalize_pos(world_, result);

	return result;
}

void
player_check_tile_map(World& world_, Player& player_)
{
	// NOTE: this function gets the center of the player,
	// then check if the player is out of the tile map,
	// and if so moves the player to the correct position on the new tile map
	auto player_center_pos = get_player_center_pos(world_, player_);
	auto* tile_map		   = get_tile_map(world_, player_center_pos);
	if (tile_map != nullptr && world_.cur_tile_map != tile_map) world_.cur_tile_map = tile_map;
}

void
game_ouput_sound(Game_sound_output_buffer& sound_buffer_)
{
	// NOTE: outputs nothing atm
	i16 sample_value = 0;
	i16* sampleOut	 = sound_buffer_.samples;
	for (szt sampleIndex = 0; sampleIndex < sound_buffer_.sample_count; ++sampleIndex) {
		*sampleOut++ = sample_value;
		*sampleOut++ = sample_value;
	}
}

}  // namespace

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

		game_state.player.pos.tile_rel_x = .5f;
		game_state.player.pos.tile_rel_y = .5f;
		game_state.player.pos.tile_map_x = 0;
		game_state.player.pos.tile_map_y = 0;
		game_state.player.pos.tile_x	 = 3;
		game_state.player.pos.tile_y	 = 3;

		game_state.player.color = { 0xFF'FF'FF'00 };

		// TODO: this might be more appropriate in the platform layer
		memory_.is_initialized = true;
	}

	// ===============================================================================================
	// #Start
	// ===============================================================================================

	u32 tiles_0[World::s_tile_count_y][World::s_tile_count_x] = {
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1 },
		{ 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
		{ 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 }
	};

	u32 tiles_1[World::s_tile_count_y][World::s_tile_count_x] = {
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 }
	};

	u32 tiles_2[World::s_tile_count_y][World::s_tile_count_x] = {
		{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
	};

	u32 tiles_3[World::s_tile_count_y][World::s_tile_count_x] = {
		{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
	};

	World world {};
	Tile_map tile_maps[World::s_tile_map_count_x][World::s_tile_map_count_y] {};
	world.s_lower_left_x = (f32)World::s_tile_size_pixels / 2.f;
	world.s_lower_left_y = (f32)video_buffer_.height;

	tile_maps[0][0].tiles = (u32*)tiles_0;
	tile_maps[0][1].tiles = (u32*)tiles_1;
	tile_maps[1][0].tiles = (u32*)tiles_2;
	tile_maps[1][1].tiles = (u32*)tiles_3;

	world.tile_maps = tile_maps[0];

	world.cur_tile_map = get_tile_map(world, game_state.player.pos);
	if (!world.cur_tile_map) {
		printf("Error-> Tile map is null!\n");
	}

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
	player.pos			= recanonicalize_pos(world, player.pos);
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

	for (i32 y {}; y < World::s_tile_count_y; ++y) {
		for (i32 x {}; x < World::s_tile_count_x; ++x) {
			u32 tile = get_tile_value_unchecked(*world.cur_tile_map, x, y);
			Color_u32 tile_color;
			if (player_center_pos.tile_x == x && player_center_pos.tile_y == y) {
				tile_color.argb = 0xFF'00'00'00;
			} else if (tile) {
				tile_color.argb = 0xFF'DD'DD'DD;
			} else {
				tile_color.argb = 0xFF'88'88'88;
			}

			f32 min_x = world.s_lower_left_x + ((f32)x) * World::s_tile_size_pixels;
			f32 min_y = world.s_lower_left_y - ((f32)y) * World::s_tile_size_pixels;
			f32 max_x = min_x + World::s_tile_size_pixels;
			f32 max_y = min_y - World::s_tile_size_pixels;

			draw_rect(video_buffer_, min_x, max_y, max_x, min_y, tile_color);
		}
	}

	f32 x = (player_center_pos.tile_rel_x * World::s_meters_to_pixels) +
			(world.s_tile_size_pixels * player_center_pos.tile_x) +
			((player.s_width * World::s_meters_to_pixels) / 2);

	f32 y = world.s_lower_left_y - ((player_center_pos.tile_rel_y * World::s_meters_to_pixels) +
									(world.s_tile_size_pixels * player_center_pos.tile_y) +
									(player.s_height * World::s_meters_to_pixels) / 2);

	draw_rect(video_buffer_, x, y, x + Player::s_width * World::s_meters_to_pixels,
			  y + Player::s_height * World::s_meters_to_pixels, player.color);
}

#if 0
	#ifdef TOM_WIN32
namespace win32
{
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN	 // Exclude rarely-used stuff from Windows headerfs
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
