#ifndef TOM_APP_HH
#define TOM_APP_HH

#include "tom_core.hh"
#include "tom_math.hh"
#include "tom_color.hh"
#include "tom_memory.hh"
#include "tom_string.hh"
#include "tom_time.hh"
#include "tom_thread.hh"
#include "tom_texture.hh"
#if USE_DS5
    #include "tom_DS5.hh"
#endif
#include "tom_input.hh"
#include "tom_sound.hh"
#include "tom_win32.hh"
#include "tom_d3d11.hh"
#include "tom_dx_error.hh"
// #include "tom_font.hh"
// #include "tom_camera.hh"
#include "tom_game.hh"

namespace tom
{

struct AppMemory
{
    u64 permanent_storage_size;
    void *permanent_storage;  // NOTE: required to be cleared to 0!
    u64 transient_storage_size;
    void *transient_storage;  // NOTE: required to be cleared to 0!

#ifdef TOM_INTERNAL
    debug_CycleCounter counters[256];
#endif
};

struct AppState
{
    AppMemory memory;
    void *memory_block;
    szt total_size;
    ThreadInfo threads[THREAD_CNT];
    Win32State win32;
    D3D11State d3d11;
    SoundState sound;
    Input input;
    GameState *game;
    Texture back_buffer;

    u32 game_update_hertz;
    u32 dpi;
    f32 target_secs_per_frame;
    i64 performance_counter_frequency;
    u64 frame_cnt;
    u32 work_ind;
    f32 work_secs[256];
    f32 ms_frame;
    i32 fps;
    i32 target_fps;
    char exe_path[MAX_PATH];
    f32 dt;
    f32 time;
    f32 fov;

    f32 device_changed_delay;
    bool suspend_lost_focus;

    D3D11ShaderProg main_shader;
};

i32 app_start(HINSTANCE hinst);

}  // namespace tom
#endif
