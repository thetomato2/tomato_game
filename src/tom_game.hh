#ifndef TOM_GAME_HH
#define TOM_GAME_HH

#include "tom_core.hh"
#include "tom_memory.hh"
#include "tom_graphics.hh"
#include "tom_texture.hh"
#include "tom_entity.hh"
#include "tom_sim.hh"

namespace tom
{

////////////////////////////////////////////////////////////////////////////////////////////////
// Game Types
struct SpriteSheet
{
    u32 x_cnt;
    u32 y_cnt;
    u32 cur_x;
    u32 cur_y;
    Texture sheet;
};

struct GameSoundOutputBuffer
{
    i32 samples_per_second;
    i32 sample_count;
    i16 *samples;
    i32 tone_hertz;
};

struct GameState
{
    u32 entity_camera_follow_ind;
    Camera camera;
    Arena *tran_arena;
    WorkQueue render_queue;
    f32 meters_to_pixels;

    u32 player_cnt;
    EntityActions player_acts[4];

    u32 ent_cnt;
    Entity entities[tom::g_max_ent_cnt];

    Texture player_sprites[4];
    Texture cat_sprites[2];
    Texture tree_sprite;
    Texture bush_sprite;
    Texture stair_sprite;
    Texture wall_sprite;
    Texture sword_sprites[4];
    SpriteSheet player_sprite;
    Texture bg;

    b32 debug_draw_collision;
    b32 debug_flag;
    v2f debug_origin;

    Texture test_alb;
    Texture test_nrm;
    v2i env_map_dims;
    // NOTE: 0 -> bot/ground, 1 -> mid, 2 -> top/sky
    EnviromentMap env_maps[3];
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Game Functions
struct AppState;
void game_init(ThreadContext *thread, AppState *app);
void game_update_and_render(ThreadContext *thread, AppState *app);

}  // namespace tom

#endif
