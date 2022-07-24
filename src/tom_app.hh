#include "tom_core.hh"
#include "tom_intrinsic.hh"
#include "tom_math.hh"
#include "tom_color.hh"
#include "tom_memory.hh"
#include "tom_string.hh"
#include "tom_time.hh"
#include "tom_file_io.hh"
#include "tom_DS5.hh"
#include "tom_input.hh"
#include "tom_sound.hh"
#include "tom_win32.hh"
#include "tom_graphics.hh"
#include "tom_dx_error.hh"
// #include "tom_font.hh"
// #include "tom_camera.hh"
#include "tom_world.hh"
#include "tom_entity.hh"
#include "tom_sim_region.hh"
#include "tom_game.hh"

namespace tom
{

struct AppInput
{
    Input* current;
    Input* last;
};

struct AppVars
{
    bool line_mode;
    f32 unit;
};

struct AppMemory
{
    u64 permanent_storage_size;
    void* permanent_storage;  // NOTE: required to be cleared to 0!
    u64 transient_storage_size;
    void* transient_storage;  // NOTE: required to be cleared to 0!
};

struct AppState
{
    AppMemory memory;
    void* memory_block;
    Win32State win32;
    GfxState gfx;
    SoundState sound;
    Input input;
    GameState* game;
    BackBuffer back_buffer;
    szt total_size;
    u32 game_update_hertz;
    u32 dpi;
    f32 target_frames_per_second;
    AppVars vars;
    i64 performance_counter_frequency;
    u64 frame_cnt;
    u32 work_ind;
    f32 work_secs[256];
    f32 ms_frame;
    i32 fps;
    i32 target_fps;
    char exe_path[MAX_PATH];
    f32 dt;
    f32 fov;
    f32 key_repeat_delay;
    f32 key_repeat_speed;

    m4 World;
    m4 view;
    m4 proj;
    m4 wvp;

    v4f clear_color;
    v4f text_color;

    ShaderProg main_shader;
    // FontSheet font_sheet;
    char key_buf[256];
};
}  // namespace tom