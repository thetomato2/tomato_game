#pragma once
#include "tomato_game.h"

namespace tomato::win32
{
struct Window_dimensions
{
	s32 width;
	s32 height;
};

struct Off_screen_buffer
{
	BITMAPINFO info;
	void* memory;
	s32 width;
	s32 height;
	s32 pitch;
	s32 bytes_per_pixel;
};

struct Sound_output
{
	s32 samples_per_sec;
	u32 running_sample_index;
	s32 bytes_per_sample;
	DWORD secondary_buf_size;
	f32 tSine;
	s32 latency_sample_count;
};

struct Replay_buffer
{
	_TCHAR file_name[512];
	HANDLE file_handle;
	HANDLE memory_map;
	void* memory_block;
};

struct Win32_state
{
	szt total_size;

	HANDLE recording_handle;
	s32 input_recording_index;

	HANDLE playback_handle;
	s32 input_playback_index;

	char exe_path[MAX_PATH];

	void* game_memory_block;
	Replay_buffer replay_buffers[4];
};

#ifdef TOM_INTERNAL
struct Debug_sound_time_marker
{
	DWORD play_cursor;
	DWORD write_cursor;
};
#endif

template<typename T>
consteval T
debug_calculate_time_marker_count(const T size)
{
	return size / 2;
}
}  // namespace tomato::win32
