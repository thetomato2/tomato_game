#pragma once
#include "game.hpp"

namespace tomato::win32
{
struct WinDim
{
    i32 width;
    i32 height;
};

struct OffscreenBuffer
{
    BITMAPINFO info;
    void *memory;
    i32 width;
    i32 height;
    i32 pitch;
    i32 bytes_per_pixel;
};

struct SoundOutput
{
    i32 samples_per_sec;
    u32 running_sample_index;
    i32 bytes_per_sample;
    DWORD secondary_buf_size;
    f32 tSine;
    i32 latency_sample_count;
};

struct ReplayBuffer
{
    _TCHAR file_name[512];
    HANDLE file_handle;
    HANDLE memory_map;
    void *memory_block;
};

struct win32_State
{
    szt total_size;

    char exe_path[MAX_PATH];

    void *game_memory_block;

#if REPLAY_BUFFERS
    HANDLE recording_handle;
    i32 input_recording_index;
    HANDLE playback_handle;
    i32 input_playback_index;
    replay_buffer replay_buffers[4];
#endif
};

struct GameCode
{
    HMODULE game_code_DLL;
    FILETIME last_write_time_DLL;
    game_update_and_render_stub *update_and_render;
    game_get_sound_samples_stub *get_sound_samples;
    bool is_valid;
};

#ifdef TOM_INTERNAL
struct debug_SoundTimeMarker
{
    DWORD play_cursor;
    DWORD write_cursor;
};
#endif

}  // namespace tomato::win32
