#include "tom_graphics.hh"
// #include "tom_world.hh"
#include "tom_entity.hh"
#include "tom_sim.hh"

namespace tom
{

global constexpr u32 g_max_ent_cnt      = 65536;
global constexpr f32 g_meters_to_pixels = 60.0f;

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

    u32 player_cnt;
    EntityActions player_acts[4];

    u32 ent_cnt;
    Entity entities[g_max_ent_cnt];

    // TODO: make a struct for all textures?
    Texture player_sprites[4];
    Texture cat_sprites[2];
    Texture tree_sprite;
    Texture bush_sprite;
    Texture stair_sprite;
    Texture wall_sprite;
    Texture sword_sprites[4];

    b32 debug_draw_collision;
    b32 debug_flag;
    v2f debug_origin;

    Texture test_alb;
    Texture test_nrm;
    v2i env_map_dims;
    // NOTE: 0 -> bot/ground, 1 -> mid, 2 -> top/sky
    EnviromentMap env_maps[3];
};

}  // namespace tom