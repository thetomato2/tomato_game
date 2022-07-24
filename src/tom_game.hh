
namespace tom
{

global constexpr u32 max_ent_cnt            = 65536;
global constexpr u32 num_screens            = 10;
global constexpr u32 num_tiles_per_screen_y = 11;
global constexpr i32 chunk_safe_margin      = I32_MAX / 64;
global constexpr f32 chunk_size_meters      = 22.0f;
global constexpr f32 tile_size_meters       = 1.4f;
global constexpr f32 meters_to_pixels       = 60.0f;
global constexpr f32 screen_size_x          = chunk_size_meters;
global constexpr f32 screen_size_y          = chunk_size_meters * 9.f / 16.0f;
global constexpr f32 jump_vel               = 2.0f;
global constexpr f32 gravity                = -9.8f;
global constexpr f32 epsilon                = 0.0001f;
global constexpr v3f chunk_dim_meters       = { chunk_size_meters, chunk_size_meters, 1.0f };
global constexpr f32 max_entity_r           = 5.0f;
global constexpr f32 max_entity_vel         = 10.0f;
global constexpr f32 update_margin          = 10.0f;

struct Camera
{
    WorldPos pos;
};

struct PairwiseCollisionRule
{
    b32 should_collide;
    u32 ent_i_a;
    u32 ent_i_b;

    PairwiseCollisionRule* next;
};

struct GameSoundOutputBuffer
{
    i32 samples_per_second;
    i32 sample_count;
    i16* samples;
    i32 tone_hertz;
};

struct GameState
{
    Arena world_arena;
    World* world;

    u32 entity_camera_follow_ind;
    Camera camera;

    u32 player_cnt;
    EntityActions player_acts[4];

    u32 ent_cnt;
    Entity entities[max_ent_cnt];

    // NOTE: must be power of two
    PairwiseCollisionRule* collision_rule_hash[256];
    PairwiseCollisionRule* first_free_collision_rule;

    // TODO: make a struct for all textures?
    argb_img default_img;
    argb_img bg_img;
    argb_img crosshair_img;
    argb_img player_sprites[4];
    argb_img monster_sprites[4];
    argb_img sword_sprites[4];
    argb_img cat_sprites[2];
    argb_img tree_sprite;
    argb_img bush_sprite;
    argb_img stair_sprite;
    argb_img wall_sprite;

    WorldPos test_pos;

    b32 debug_draw_collision;
    b32 debug_flag;
    u32 col_ind;
    Color test_col = v3_to_color(red_v3);
};

}  // namespace tom