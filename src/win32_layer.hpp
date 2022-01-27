#pragma once
#include "game.hpp"

struct Win_Dim
{
    s32 width;
    s32 height;
};

struct Offscreen_Buffer
{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 bytes_per_pixel;
};

struct Sound_Output
{
    s32 samples_per_sec;
    u32 running_sample_index;
    s32 bytes_per_sample;
    DWORD secondary_buf_size;
    s32 latency_sample_count;
};

struct Replay_Buffer
{
    _TCHAR file_name[512];
    HANDLE file_handle;
    HANDLE memory_map;
    void *memory_block;
};

struct Win32_State
{
    szt total_size;

    char exe_path[MAX_PATH];

    void *game_memory_block;

#if REPLAY_BUFFERS
    HANDLE recording_handle;
    s32 input_recording_index;
    HANDLE playback_handle;
    s32 input_playback_index;
    Replay_Buffer replay_buffers[4];
#endif
};

struct Game_Code
{
    HMODULE game_code_DLL;
    FILETIME last_write_time_DLL;
    tom::game_update_and_render_stub *update_and_render;
    tom::game_get_sound_samples_stub *get_sound_samples;
    bool is_valid;
};

#ifdef TOM_INTERNAL
struct debug_Sound_Time_Marker
{
    DWORD play_cursor;
    DWORD write_cursor;
};
#endif
