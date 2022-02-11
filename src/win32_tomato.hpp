#pragma once
#include "tomato_platform.h"

#ifdef _EMACS
using wchar_t = uint16_t;
#endif
// WinHelp is deprecate
#define NOHELP
// DirectX apps don't need GDI
// NOTE: I am using GDI to slowly blit to the screen
//#define NODRAWTEXT
//#define NOGDI
//#define NOBITMAP
//#define WIN32_LEAN_AND_MEAN

// Use the C++ standard templated min/max
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <xinput.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

struct window_dims
{
    s32 width;
    s32 height;
};

struct offscreen_buffer
{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 bytes_per_pixel;
};

struct sound_output
{
    s32 samples_per_sec;
    u32 running_sample_index;
    s32 bytes_per_sample;
    DWORD secondary_buf_size;
    s32 latency_sample_count;
};

struct replay_buffer
{
    _TCHAR file_name[512];
    HANDLE file_handle;
    HANDLE memory_map;
    void *memory_block;
};

struct win32_state
{
    szt total_size;

    char exe_path[MAX_PATH];

    void *game_memory_block;

#if REPLAY_BUFFERS
    HANDLE recording_handle;
    s32 input_recording_index;
    HANDLE playback_handle;
    s32 input_playback_index;
    replay_buffer replay_buffers[4];
#endif
};

struct game_code
{
    HMODULE game_code_DLL;
    FILETIME last_write_time_DLL;
    game_update_and_render_stub *update_and_render;
    game_get_sound_samples_stub *get_sound_samples;
    bool is_valid;
};

#ifdef TOM_INTERNAL
struct debug_sound_time_marker
{
    DWORD play_cursor;
    DWORD write_cursor;
};
#endif

enum keys : byt
{
    none           = 0,
    back           = 0x8,
    tab            = 0x9,
    enter          = 0xd,
    pause          = 0x13,
    caps_lock      = 0x14,
    kana           = 0x15,
    kanji          = 0x19,
    escape         = 0x1b,
    ime_convert    = 0x1c,
    ime_no_convert = 0x1d,
    space          = 0x20,
    pageUp         = 0x21,
    pageDown       = 0x22,
    end            = 0x23,
    home           = 0x24,
    left           = 0x25,
    up             = 0x26,
    right          = 0x27,
    down           = 0x28,
    // select         = 0x29,
    print          = 0x2a,
    execute        = 0x2b,
    print_screen   = 0x2c,
    insert         = 0x2d,
    delete_key     = 0x2e,
    help           = 0x2f,
    d0             = 0x30,
    d1             = 0x31,
    d2             = 0x32,
    d3             = 0x33,
    d4             = 0x34,
    d5             = 0x35,
    d6             = 0x36,
    d7             = 0x37,
    d8             = 0x38,
    d9             = 0x39,
    a              = 0x41,
    b              = 0x42,
    c              = 0x43,
    d              = 0x44,
    e              = 0x45,
    f              = 0x46,
    g              = 0x47,
    h              = 0x48,
    i              = 0x49,
    j              = 0x4a,
    k              = 0x4b,
    l              = 0x4c,
    m              = 0x4d,
    n              = 0x4e,
    o              = 0x4f,
    p              = 0x50,
    q              = 0x51,
    r              = 0x52,
    s              = 0x53,
    t              = 0x54,
    u              = 0x55,
    v              = 0x56,
    w              = 0x57,
    x              = 0x58,
    y              = 0x59,
    z              = 0x5a,
    left_windows   = 0x5b,
    right_windows  = 0x5c,
    apps           = 0x5d,
    sleep          = 0x5f,
    num_pad_0      = 0x60,
    num_pad_1      = 0x61,
    num_pad_2      = 0x62,
    num_pad_3      = 0x63,
    num_pad_4      = 0x64,
    num_pad_5      = 0x65,
    num_pad_6      = 0x66,
    num_pad_7      = 0x67,
    num_pad_8      = 0x68,
    num_pad_9      = 0x69,
    multiply       = 0x6a,
    add            = 0x6b,
    separator      = 0x6c,
    subtract       = 0x6d,
    decimal        = 0x6e,
    divide         = 0x6f,
    f1             = 0x70,
    f2             = 0x71,
    f3             = 0x72,
    f4             = 0x73,
    f5             = 0x74,
    f6             = 0x75,
    f7             = 0x76,
    f8             = 0x77,
    f9             = 0x78,
    f10            = 0x79,
    f11            = 0x7a,
    f12            = 0x7b,
    f13            = 0x7c,
    f14            = 0x7d,
    f15            = 0x7e,
    f16            = 0x7f,
    f17            = 0x80,
    f18            = 0x81,
    f19            = 0x82,
    f20            = 0x83,
    f21            = 0x84,
    f22            = 0x85,
    f23            = 0x86,
    f24            = 0x87,
    num_lock       = 0x90,
    scroll         = 0x91,
    left_shift     = 0xa0,
    right_shift    = 0xa1,
    left_control   = 0xa2,
    right_control  = 0xa3,
    left_alt       = 0xa4,
    right_alt      = 0xa5,
    semicolon      = 0xba,
    plus           = 0xbb,
    comma          = 0xbc,
    minus          = 0xbd,
    period         = 0xbe,
    question       = 0xbf,
    tilde          = 0xc0,
    open_brackets  = 0xdb,
    pipe           = 0xdc,
    close_brackets = 0xdd,
    quotes         = 0xde,
    oem8           = 0xdf,
    backslash      = 0xe2,
    process_key    = 0xe5,
};
