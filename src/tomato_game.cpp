#include "tomato_game.h"

namespace tomato
{
namespace
{
[[nodiscard]] s32
round_f32_to_s32(f32 value)
{
	return s32(value + 0.5f);
}

[[nodiscard]] u32
rnd_f32_to_u32(f32 value)
{
	return u32(value + 0.5f);
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
	s32 width  = buffer.width;
	s32 height = buffer.height;

	u8* row = (u8*)buffer.memory;
	for (s32 y = 0; y < height; ++y) {
		u32* pixel = (u32*)row;
		for (s32 x = 0; x < width; ++x) {
			*pixel++ = color.argb;
		}
		row += buffer.pitch;
	}
}

void
draw_rect(Game_offscreen_buffer& buffer, f32 f32_min_x, f32 f_min_y, f32 f32_max_x, f32 f32_max_y,
		  Color_u32 color = { 0xFFFFFFFF })
{
	s32 min_x = round_f32_to_s32(f32_min_x);
	s32 min_y = round_f32_to_s32(f_min_y);
	s32 max_x = round_f32_to_s32(f32_max_x);
	s32 max_y = round_f32_to_s32(f32_max_y);

	if (min_x < 0) min_x = 0;
	if (min_y < 0) min_y = 0;
	if (max_x > buffer.width) max_x = buffer.width;
	if (max_y > buffer.height) max_x = buffer.height;

	u8* row = ((u8*)buffer.memory + min_x * buffer.bytes_per_pixel + min_y * buffer.pitch);

	for (s32 y = min_y; y < max_y; ++y) {
		u32* pixel = (u32*)row;
		for (s32 x = min_x; x < max_x; ++x) {
			*pixel++ = color.argb;
		}
		row += buffer.pitch;
	}
}

void
game_ouput_sound(Game_sound_output_buffer& sound_buffer)
{
	// NOTE: outputs nothing atm
	s16 sample_value = 0;
	s16* sampleOut	 = sound_buffer.samples;
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

	if (!memory.is_initialized) {
		const char* file_name = __FILE__;

#ifdef TOM_INTERNAL
		Debug_read_file_result file = memory.debug_platfrom_read_entire_file(file_name);
		if (file.contents) {
			memory.debug_platform_write_entire_file("test.txt", file.content_size, file.contents);
			memory.debug_platform_free_file_memory(file.contents);
		}
#endif
		game_state.tone_hertz = 256;

		// TODO: this might be more appropriate in the platform layer
		memory.is_initialized = true;
	}

	Game_controller_input& controller_0 = input.controllers[0];
	if (controller_0.is_analog) {
		// gameState.xOffset += s32(speed * (controller0.endLX));
		// gameState.yOffset += s32(speed * (controller0.endLY));
		game_state.tone_hertz = 256 + (s32)(64.0f * (controller_0.end_left_stick_y)) +
								(s32)(64.0f * (controller_0.end_left_stick_x));

	} else {
		// TODO: handle digital input
	}

	u32 tile_map[9][16] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

	constexpr f32 upper_left_x = 10;
	constexpr f32 upper_left_y = 10;
	constexpr f32 tile_width   = 100;
	constexpr f32 tile_height  = 100;

	for (s32 row {}; row < 9; ++row) {
		for (s32 col {}; col < 16; ++col) {
			u32 tile = tile_map[row][col];
			Color_u32 tile_color;
			if (tile) {
				tile_color.argb = 0xFFDDDDDD;
			}

			// FIXME: in progress
			// f32 min_x = (f32)col * tile_color;

			// draw_rect(video_buffer, f32 fMinX, f32 fMinY, f32 fMaxX, f32 fMaxY)
		}
	}

	local_persist f32 time {};
	time += input.seconds_per_frame * 0.2f;

	Color_u32 clear_color { 0xFFFF00FF };

	clear_buffer(video_buffer, clear_color);

	draw_rect(video_buffer, 10.0f, 10.0f, 30.0f, 30.0f);
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
