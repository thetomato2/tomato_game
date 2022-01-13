#pragma once
#include "framework.hpp"
#include "tile.hpp"

#define TOM_WIN32
#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

#define REPLAY_BUFFERS 0

namespace tomato
{

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL

// TODO: implement this
struct ThreadContext
{
    i32 place_holder;
};

struct debug_ReadFileResult
{
    u32 content_size;
    void *contents;
};

    //! all these C shenanigans...
    // TODO:  C++-ify this
    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread_, void *memory_)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) \
        debug_ReadFileResult name(ThreadContext *thread_, const char *file_name_)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
        bool32 name(ThreadContext *thread_, const char *file_name_, u64 memory_size_, void *memory_)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

struct GameOffscreenBuffer
{
    void *memory;
    i32 width;
    i32 height;
    i32 pitch;
    i32 bytes_per_pixel;
};

struct GameSoundOutputBuffer
{
    i32 samples_per_second;
    i32 sample_count;
    i16 *samples;
    i32 tone_hertz;
};

struct GameButtonState
{
    i32 half_transition_count;
    bool ended_down;
};

struct GameControllerInput
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
        GameButtonState buttons[12];
        struct
        {
            GameButtonState dpad_up;
            GameButtonState dpad_right;
            GameButtonState dpad_down;
            GameButtonState dpad_left;
            GameButtonState button_A;
            GameButtonState button_B;
            GameButtonState button_X;
            GameButtonState button_Y;
            GameButtonState button_RB;
            GameButtonState button_LB;
            GameButtonState button_back;
            GameButtonState button_start;
        };
    };
};

struct GameKeyboard
{
    union
    {
        GameButtonState keys[11];
        struct
        {
            GameButtonState w;
            GameButtonState s;
            GameButtonState a;
            GameButtonState d;
            GameButtonState space;
            GameButtonState left_shift;
            GameButtonState p;
            GameButtonState d1;
            GameButtonState d2;
            GameButtonState d3;
            GameButtonState d4;
        };
    };
};

struct GameInput
{
    f32 deltaTime;

    static constexpr szt mouse_button_count = 3;
    GameButtonState mouse_buttons[3];
    i32 mouse_x, mouse_y, mouse_z;
    GameKeyboard keyboard;
    GameControllerInput controllers[4];
};

struct GameMem
{
    bool is_initialized;
    u64 permanent_storage_size;
    void *permanent_storage;  //! required to be cleared to 0!

    u64 transient_storage_size;
    void *transient_storage;  //! required to be cleared to 0!

#ifdef TOM_INTERNAL
    debug_platform_free_file_memory *platform_free_file_memory;
    debug_platform_read_entire_file *platfrom_read_entire_file;
    debug_platform_write_entire_file *platform_write_entire_file;
#endif
};

struct Color_u32
{
    Color_u32() : argb(0xffffffff) {}
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

struct s32_Vector2
{
    s32_Vector2() : x(0), y(0) {}
    s32_Vector2(i32 x_, i32 y_)
    {
        x = x_;
        y = y_;
    }

    i32 x;
    i32 y;
};

struct f32_Vector2
{
    f32_Vector2() : x(0.f), y(0.f) {}
    f32_Vector2(f32 x_, f32 y_)
    {
        x = x_;
        y = y_;
    }

    f32 x;
    f32 y;
};
#pragma pack(push, 1)
struct BitmapHeader
{
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
};

struct ARGB_header
{
    u32 width;
    u32 height;
    u32 size;
};
#pragma pack(pop)

struct ARGB_img
{
    u32 width;
    u32 height;
    u32 size;
    u32 *pixel_ptr;
};
struct ColorDebug
{
    f32_Vector2 pos;
    bool is_valid;
};

#define GAME_UPDATE_AND_RENDER(name)                                       \
    void name(ThreadContext *thread_, GameMem &memory_, GameInput &input_, \
              GameOffscreenBuffer &video_buffer_, GameSoundOutputBuffer &sound_buffer_)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render_stub);

#define GAME_GET_SOUND_SAMPLES(name) \
    void name(ThreadContext *thread_, GameMem &memory_, GameSoundOutputBuffer &sound_buffer_)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples_stub);

struct MemArena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

struct Player
{
    static constexpr f32 s_height = .75f;
    static constexpr f32 s_width  = 0.75f * s_height;

    TileMapPos pos;
    Color_u32 color;

    u32 direction;
};

struct World
{
    TileMap *tile_map;
};

struct Bitmap
{
    i32 width;
    i32 height;
    u32 *pixel_ptr;
};

struct GameState
{
    MemArena world_arena;
    World *world;
    Player player;

    Bitmap bitmap;

    ARGB_img bg_img;
    ARGB_img player_img[4];

    ARGB_img bunny_girl_img[4];
};

void *
push_size(MemArena *arena_, mem_ind size_);

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))

}  // namespace tomato
