#include "tomato_game.h"

#include <math.h>

namespace tomato
{
namespace global
{
constexpr bool grid_on	  = false;
constexpr bool corners_on = true;

i32 current_corner {};
Col_debug corners[4];

}  // namespace global

namespace
{
inline i32
round_f32_to_i32(f32 value)
{
	return i32(value + 0.5f);
}

inline u32
rnd_f32_to_u32(f32 value)
{
	return u32(value + 0.5f);
}

inline i32
floorf_to_i32(f32 val)
{
	return (i32)floorf(val);
}

void
generate_rainbow(Color_u32& color, f32 frequency, f32 time)
{
	f32 f = (f32)sin(time * frequency) / 2.0f + 0.5f;

	f32 a = (1.0f - f) / 0.2f;
	f32 x = floorf(a);
	f32 y = floorf(255.0f * (a - x));
	f32 red, green, blue;

	if (x == 0.0f) {
		red	  = 255.0f;
		green = y;
		blue  = 0;
	} else if (x == 1.0f) {
		red	  = 255.0f - y;
		green = 255.0f;
		blue  = 0;
	} else if (x == 2.0f) {
		red	  = 0.0f;
		green = 255.0f;
		blue  = y;
	} else if (x == 3.0f) {
		red	  = 0.0f;
		green = 255.0f - y;
		blue  = 255.0f;
	} else if (x == 4.0f) {
		red	  = y;
		green = 0.0f;
		blue  = 255.0f;
	} else if (x == 5.0f) {
		red	  = 255.0f;
		green = 0.0f;
		blue  = 255.0f;
	}

	color.argb = (0xFF << 24) | ((u8)red << 16) | ((u8)green << 8) | ((u8)blue);
}

void
clear_buffer(Game_offscreen_buffer& buffer, Color_u32 color = { 0xFF'FF'00'FF })
{
	i32 width  = buffer.width;
	i32 height = buffer.height;

	u8* row = (u8*)buffer.memory;
	for (i32 y = 0; y < height; ++y) {
		u32* pixel = (u32*)row;
		for (i32 x = 0; x < width; ++x) {
			*pixel++ = color.argb;
		}
		row += buffer.pitch;
	}
}

void
draw_rect(Game_offscreen_buffer& buffer, f32 f32_min_x, f32 f_min_y, f32 f32_max_x, f32 f32_max_y,
		  Color_u32 color = { 0xFFFFFFFF })
{
	i32 min_x = round_f32_to_i32(f32_min_x);
	i32 min_y = round_f32_to_i32(f_min_y);
	i32 max_x = round_f32_to_i32(f32_max_x);
	i32 max_y = round_f32_to_i32(f32_max_y);

	if (min_x < 0) min_x = 0;
	if (min_y < 0) min_y = 0;
	if (max_x > buffer.width) max_x = buffer.width;
	if (max_y > buffer.height) max_y = buffer.height;

	u8* row = ((u8*)buffer.memory + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch);

	for (i32 y = min_y; y < max_y; ++y) {
		u32* pixel = (u32*)row;
		for (i32 x = min_x; x < max_x; ++x) {
			*pixel++ = color.argb;
		}
		row += buffer.pitch;
	}
}

inline Tile_map*
get_tile_map(World& world, i32 tile_map_x, i32 tile_map_y)
{
	Tile_map* tile_map = nullptr;

	if (tile_map_x >= 0 && tile_map_x < World::s_tile_map_count_x && tile_map_y >= 0 &&
		tile_map_y < World::s_tile_map_count_y) {
		tile_map = &world.tile_maps[tile_map_y * World::s_tile_map_count_x + tile_map_x];
	}
	return tile_map;
}

inline u32
get_tile_value_unchecked(Tile_map& tile_map, i32 tile_x, i32 tile_y)
{
	assert(tile_map.tiles);
	assert((tile_x >= 0) && (tile_x < Tile_map::s_count_x) && (tile_y >= 0) &&
		   (tile_y < Tile_map::s_count_y));
	return tile_map.tiles[tile_y * Tile_map::s_count_x + tile_x];
}

bool
is_tile_empty(Tile_map& tile_map, i32 test_tile_x, i32 test_tile_y)
{
	bool is_empty { false };

	// printf("x: %d, y: %d", player_tile_x, player_tile_y);

	// NOTE: if tiles is null map does not exists
	if (tile_map.tiles) {
		if (test_tile_x >= 0 && test_tile_x < Tile_map::s_count_x && test_tile_y >= 0 &&
			test_tile_y < Tile_map::s_count_y) {
			auto tile_map_value = get_tile_value_unchecked(tile_map, test_tile_x, test_tile_y);
			is_empty			= (tile_map_value == 0);
		}
	}

	return is_empty;
}

inline Canonical_pos
get_canonical_pos(Raw_pos pos)
{
	Canonical_pos res;
	res.tile_map_x = pos.tile_map_x;
	res.tile_map_y = pos.tile_map_y;

	f32 x	   = pos.x - Tile_map::s_upper_left_x;
	f32 y	   = pos.y - Tile_map::s_upper_left_y;
	res.tile_x = floorf_to_i32(x / Tile_map::s_tile_width);
	res.tile_y = floorf_to_i32(y / Tile_map::s_tile_height);

	res.x = x - res.tile_x * Tile_map::s_tile_width;
	res.y = y - res.tile_y * Tile_map::s_tile_height;

	assert(res.x >= 0);
	assert(res.y >= 0);
	assert(res.x < Tile_map::s_tile_width);
	assert(res.y < Tile_map::s_tile_height);

	// check bounds to see if player is in a neigboring tile map
	if (res.tile_x < 0) {
		res.tile_x = Tile_map::s_count_x + res.tile_x;
		--res.tile_map_x;
	}
	if (res.tile_x > Tile_map::s_count_x - 1) {
		res.tile_x = res.tile_x - Tile_map::s_count_x;
		++res.tile_map_x;
	}

	if (res.tile_y < 0) {
		res.tile_y = Tile_map::s_count_y + res.tile_y;
		--res.tile_map_y;
	}

	if (res.tile_y > Tile_map::s_count_y - 1) {
		res.tile_y = res.tile_y - Tile_map::s_count_y;
		++res.tile_map_y;
	}

	return res;
}

bool
is_world_tile_empty(World& world, Raw_pos test_pos)
{
	bool is_empty { false };

	Canonical_pos can_pos = get_canonical_pos(test_pos);
	Tile_map& tile_map	  = *(get_tile_map(world, can_pos.tile_map_x, can_pos.tile_map_y));
	is_empty			  = is_tile_empty(tile_map, can_pos.tile_x, can_pos.tile_y);

	// debug stuff
	if (global::corners_on) {
		global::corners[global::current_corner].pos.x	 = test_pos.x;
		global::corners[global::current_corner].pos.y	 = test_pos.y;
		global::corners[global::current_corner].is_valid = !is_empty;
		++global::current_corner;
		if (global::current_corner == 4) global::current_corner = 0;
	}
	return is_empty;
}

inline bool
check_player_collsion(World& world, Player player, Raw_pos test_pos)
{
	bool result				   = false;
	auto test_pos_top_left	   = test_pos;
	auto test_pos_top_right	   = test_pos;
	auto test_pos_bottom_left  = test_pos;
	auto test_pos_bottom_right = test_pos;

	test_pos_top_right.x += player.width;
	test_pos_bottom_left.y += player.height;
	test_pos_bottom_right.x += player.width;
	test_pos_bottom_right.y += player.height;

	// NOTE: checking each corner
	if (is_world_tile_empty(world, test_pos_top_left) &&
		is_world_tile_empty(world, test_pos_top_right) &&
		is_world_tile_empty(world, test_pos_bottom_left) &&
		is_world_tile_empty(world, test_pos_bottom_right))
		result = true;

	return result;
}

void
game_ouput_sound(Game_sound_output_buffer& sound_buffer)
{
	// NOTE: outputs nothing atm
	i16 sample_value = 0;
	i16* sampleOut	 = sound_buffer.samples;
	for (szt sampleIndex = 0; sampleIndex < sound_buffer.sample_count; ++sampleIndex) {
		*sampleOut++ = sample_value;
		*sampleOut++ = sample_value;
	}
}

}  // namespace

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(game_get_sound_samples)
{
	auto* game_state = (Game_state*)memory.permanent_storage;
	game_ouput_sound(sound_buffer);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(game_update_and_render)
{
	assert(sizeof(Game_state) <= memory.permanent_storage_size);

	// NOTE: cast to GameState ptr, dereference and cast to GameState reference
	auto& game_state = (Game_state&)(*(Game_state*)memory.permanent_storage);

	// ===============================================================================================
	// #Initialization
	// ===============================================================================================
	if (!memory.is_initialized) {
		const char* file_name = __FILE__;

		game_state.player.pos.x			 = 100.f;
		game_state.player.pos.y			 = 100.f;
		game_state.player.pos.tile_map_x = 0;
		game_state.player.pos.tile_map_y = 0;
		game_state.player.width			 = 20.f;
		game_state.player.height		 = 30.f;
		game_state.player.color			 = { 0xFF'FF'FF'00 };

		// TODO: this might be more appropriate in the platform layer
		memory.is_initialized = true;
	}

	// ===============================================================================================
	// #Start
	// ===============================================================================================

	u32 tiles_0[Tile_map::s_count_y][Tile_map::s_count_x] = {
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

	u32 tiles_1[Tile_map::s_count_y][Tile_map::s_count_x] = {
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

	u32 tiles_2[Tile_map::s_count_y][Tile_map::s_count_x] = {
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

	u32 tiles_3[Tile_map::s_count_y][Tile_map::s_count_x] = {
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

	tile_maps[0][0].tiles = (u32*)tiles_0;
	tile_maps[0][1].tiles = (u32*)tiles_1;
	tile_maps[1][0].tiles = (u32*)tiles_2;
	tile_maps[1][1].tiles = (u32*)tiles_3;

	world.tile_maps = tile_maps[0];

	world.cur_tile_map =
		get_tile_map(world, game_state.player.pos.tile_map_x, game_state.player.pos.tile_map_y);
	if (!world.cur_tile_map) {
		printf("Error-> Tile map is null!\n");
	}

	Game_controller_input& controller_0 = input.controllers[0];
	if (controller_0.is_analog) {
		// gameState.xOffset += i32(speed * (controller0.endLX));
		// gameState.yOffset += i32(speed * (controller0.endLY));

	} else {
		// TODO: handle digital input
	}

	local_persist f32 time {};
	time += input.deltaTime * 0.2f;

	static constexpr f32 player_speed = 128.f;

	auto& player		= game_state.player;
	auto new_player_pos = player.pos;

	if (input.keyboard.w.ended_down) {
		new_player_pos.y -= player_speed * input.deltaTime;
	} else if (input.keyboard.s.ended_down) {
		new_player_pos.y += player_speed * input.deltaTime;
	}

	if (input.keyboard.d.ended_down) {
		new_player_pos.x += player_speed * input.deltaTime;
	} else if (input.keyboard.a.ended_down) {
		new_player_pos.x -= player_speed * input.deltaTime;
	}

	if (check_player_collsion(world, player, new_player_pos)) {
		player.pos = new_player_pos;

		// check for tile map change from center of player
		// TODO: make actual vector classes
		auto player_center_pos = player.pos;
		player_center_pos.x += player.width / 2;
		player_center_pos.y += player.height / 2;
		Canonical_pos can_pos = get_canonical_pos(player_center_pos);
		printf(" %d, %d ", can_pos.tile_x, can_pos.tile_y);
		Tile_map* tile_map = get_tile_map(world, can_pos.tile_map_x, can_pos.tile_map_y);
		if (tile_map != nullptr && tile_map != world.cur_tile_map) {
			world.cur_tile_map	  = tile_map;
			player.pos.tile_map_x = can_pos.tile_map_x;
			player.pos.tile_map_y = can_pos.tile_map_y;
			player.pos.x = Tile_map::s_upper_left_x + Tile_map::s_tile_width * can_pos.tile_x +
						   can_pos.x - player.width / 2;
			player.pos.y = Tile_map::s_upper_left_y + Tile_map::s_tile_height * can_pos.tile_y +
						   can_pos.y - player.height / 2;
		}
		printf("%f, %f\n", player.pos.x, player.pos.y);
	}

	// ===============================================================================================
	// #Draw
	// ===============================================================================================

	// NOTE: *not* using PatBlt in the win32 layer
	Color_u32 clear_color { 0xFF'00'00'00 };
	clear_buffer(video_buffer, clear_color);

	for (i32 y {}; y < Tile_map::s_count_y; ++y) {
		for (i32 x {}; x < Tile_map::s_count_x; ++x) {
			u32 tile = get_tile_value_unchecked(*world.cur_tile_map, x, y);
			Color_u32 tile_color;
			if (tile) {
				tile_color.argb = 0xFF'DD'DD'DD;
			} else {
				tile_color.argb = 0xFF'88'88'88;
			}

			f32 min_x = Tile_map::s_upper_left_x + ((f32)x) * Tile_map::s_tile_width;
			f32 min_y = Tile_map::s_upper_left_y + ((f32)y) * Tile_map::s_tile_height;
			f32 max_x = min_x + Tile_map::s_tile_width;
			f32 max_y = min_y + Tile_map::s_tile_height;

			draw_rect(video_buffer, min_x, min_y, max_x, max_y, tile_color);
			if (global::grid_on) {
				draw_rect(video_buffer, min_x, min_y, min_x + 2, max_y, { 0xFF'00'00'00 });
				draw_rect(video_buffer, min_x, min_y, max_x, min_y + 2, { 0xFF'00'00'00 });
			}
		}
	}

	draw_rect(video_buffer, player.pos.x, player.pos.y, player.pos.x + player.width,
			  player.pos.y + player.height, player.color);

	for (i32 i {}; i < 4; ++i) {
		Color_u32 tile_color;
		auto& corner = global::corners[i];

		if (corner.is_valid)
			tile_color = { 0xFF'FF'00'00 };	 // red
		else
			tile_color = { 0xFF'00'FF'00 };	 // green

		draw_rect(video_buffer, corner.pos.x, corner.pos.y, corner.pos.x + 2, corner.pos.y + 2,
				  tile_color);
	}
}

#if 0
	#ifdef TOM_WIN32
namespace win32
{
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN	 // Exclude rarely-used stuff from Windows headers
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
