#ifndef TOMATO_GAME_H_
#define TOMATO_GAME_H_

#include "tomato_framework.h"

#define TOM_WIN32
#ifdef TOM_WIN32
	#define TOM_DLL_EXPORT __declspec(dllexport)
#else
	#define TOM_DLL_EXPORT
#endif

namespace tomato
{

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

struct Debug_read_file_result
{
	u32 content_size;
	void* contents;
};

	//! all these C shenanigans...
	// TODO:  C++-ify this
	#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void* memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

	#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) Debug_read_file_result name(const char* file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

	#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
		bool32 name(const char* file_name, u64 memory_size, void* memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#if TOM_INTERNAL
	#define Assert(expression) \
		if (!(expression)) {   \
			*(int*)0 = 0;      \
		}
#elif
	#define Assert(expression)
#endif

struct Game_offscreen_buffer
{
	void* memory;
	i32 width;
	i32 height;
	i32 pitch;
	i32 bytes_per_pixel;
};

struct Game_sound_output_buffer
{
	i32 samples_per_second;
	i32 sample_count;
	i16* samples;
	i32 tone_hertz;
};

struct Game_button_state
{
	i32 half_transition_count;
	bool ended_down;
};

struct Game_controller_input
{
	bool is_connected;
	bool is_analog;

	f32 start_left_stick_x;
	f32 start_left_stick_y;
	f32 start_right_stick_x;
	f32 start_right_stick_y;

	f32 min_x;
	f32 min_y;

	f32 max_x;
	f32 max_y;

	f32 end_left_stick_x;
	f32 end_left_stick_y;
	f32 end_right_stick_x;
	f32 end_right_stick_y;

	union
	{
		Game_button_state buttons[12];
		struct
		{
			Game_button_state dpad_up;
			Game_button_state dpad_right;
			Game_button_state dpad_down;
			Game_button_state dpad_left;
			Game_button_state button_A;
			Game_button_state button_B;
			Game_button_state button_X;
			Game_button_state button_Y;
			Game_button_state button_RB;
			Game_button_state button_LB;
			Game_button_state button_back;
			Game_button_state button_start;
		};
	};
};

struct Game_keyboard
{
	union
	{
		Game_button_state keys[11];
		struct
		{
			Game_button_state w;
			Game_button_state s;
			Game_button_state a;
			Game_button_state d;
			Game_button_state space;
			Game_button_state left_shift;
			Game_button_state p;
			Game_button_state d1;
			Game_button_state d2;
			Game_button_state d3;
			Game_button_state d4;
		};
	};
};

struct Game_input
{
	f32 deltaTime;

	static constexpr szt mouse_button_count = 3;
	Game_button_state mouse_buttons[3];
	i32 mouse_x, mouse_y, mouse_z;
	Game_keyboard keyboard;
	Game_controller_input controllers[4];
};

struct Game_memory
{
	bool is_initialized;
	u64 permanent_storage_size;
	void* permanent_storage;  //! required to be cleared to 0!

	u64 transient_storage_size;
	void* transient_storage;  //! required to be cleared to 0!

#ifdef TOM_INTERNAL
	debug_platform_free_file_memory* debug_platform_free_file_memory;
	debug_platform_read_entire_file* debug_platfrom_read_entire_file;
	debug_platform_write_entire_file* debug_platform_write_entire_file;
#endif
};

struct Color_u32
{
	Color_u32() : argb(0xFFFFFFFF) {}
	Color_u32(u32 color) : argb(color) {}

	union
	{
		u32 argb;
		struct
		{
			u8 a;
			u8 r;
			u8 g;
			u8 b;
		};
	};
};

struct Vector2_s32
{
	Vector2_s32() : x(0), y(0) {}
	Vector2_s32(i32 x, i32 y)
	{
		this->x = x;
		this->y = y;
	}

	i32 x;
	i32 y;
};

struct Vector2_f32
{
	Vector2_f32() : x(0.f), y(0.f) {}
	Vector2_f32(f32 x, f32 y)
	{
		this->x = x;
		this->y = y;
	}

	f32 x;
	f32 y;
};

struct Col_debug
{
	Vector2_f32 pos;
	bool is_valid;
};

// TODO: implement this
struct Thread_context
{
	i32 place_holder;
};

#define GAME_UPDATE_AND_RENDER(name)                                         \
	void name(Thread_context thread, Game_memory& memory, Game_input& input, \
			  Game_offscreen_buffer& video_buffer, Game_sound_output_buffer& sound_buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
	void name(Thread_context thread, Game_memory& memory, Game_sound_output_buffer& sound_buffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

struct Tile_map
{
	static constexpr i32 s_count_x = 16;
	static constexpr i32 s_count_y = 9;

	static constexpr f32 s_upper_left_x = 10.f;
	static constexpr f32 s_upper_left_y = 10.f;
	static constexpr f32 s_tile_width	= 50.f;
	static constexpr f32 s_tile_height	= 50.f;

	u32* tiles;
};

struct Canonical_pos
{
	i32 tile_map_x;
	i32 tile_map_y;

	i32 tile_x;
	i32 tile_y;

	// NOTE: relative to tile
	f32 x;
	f32 y;
};

struct Raw_pos
{
	i32 tile_map_x;
	i32 tile_map_y;

	// NOTE: relative to tile map
	f32 x;
	f32 y;
};

struct Player
{
	Raw_pos pos;
	Color_u32 color;
	f32 height;
	f32 width;
};

struct World
{
	static constexpr i32 s_tile_map_count_x = 2;
	static constexpr i32 s_tile_map_count_y = 2;

	Tile_map* cur_tile_map;
	Tile_map* tile_maps;
};

struct Game_state
{
	Player player;
};

}  // namespace tomato
#endif
