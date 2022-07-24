namespace tom {
struct argb_img;
struct GameState;
struct sim_region;
struct SimEntity;

struct EntityActions
{
    bool start;
    bool jump;
    bool attack;

    v3f dir;

    bool sprint;
};

enum EntityDirection : i32
{
    north = 0,
    east,
    south,
    west
};

enum sim_entity_flags : i32
{
    active     = Bit(0),
    collides   = Bit(1),
    barrier    = Bit(2),
    nonspatial = Bit(3),
    hurtbox    = Bit(4),

    simming = Bit(31),
};

enum class EntityType
{
    null = 0,
    none,
    player,
    wall,
    tree,
    bush,
    stairs,
    familiar,
    monster,
    sword,
    stair
};

struct EntityLowChunkRef
{
    WorldChunk* tile_chunk;
    u32 i_in_chunk;
};

struct EntityVisiblePiece
{
    argb_img* img;
    v2f mid_p;
    f32 z;
    f32 alpha;
    r2i rect;
    Color color;
};

struct EntityVisiblePieceGroup
{
    u32 piece_cnt;
    // TODO: how many pieces?
    EntityVisiblePiece pieces[64];
};

union EntityRef
{
    SimEntity* ptr;
    u32 ind;
};

struct SimEntity
{
    u32 ent_i;
    b32 updateable;

    i32 flags;
    v3f pos;
    v3f vel;
    v3f dim;
    u32 chunk_z;
    f32 z;
    f32 vel_z;
    f32 hit_cd;
    f32 exists_cd;
    f32 dist_limit;
    i32 hp;
    u32 max_hp;
    i32 virtual_z;
    f32 argb_offset;
    u32 weapon_i;
    u32 parent_i;
    i32 cur_sprite;
    EntityDirection dir;
    EntityType type;
};

struct Entity
{
    SimEntity sim;
    WorldPos world_pos;
};

struct EntityMoveSpec
{
    f32 speed;
    f32 drag;
};

inline EntityMoveSpec default_move_spec()
{
    return { 10.0f, 10.0f };
}

inline void make_entity_nonspatial(SimEntity* ent)
{
    set_flags(ent->flags, sim_entity_flags::nonspatial);
}

inline void make_entity_spatial(SimEntity* ent, v3f pos, v3f vel = { 0.f, 0.f, 0.f })
{
    set_flags(ent->flags, sim_entity_flags::nonspatial);
    ent->pos = pos;
    ent->vel = vel;
}

inline bool is_flag_set(SimEntity* ent, i32 flag)
{
    return is_flag_set(ent->flags, flag);
}

inline void set_flags(SimEntity* ent, i32 flag)
{
    set_flags(ent->flags, flag);
}

inline void clear_flags(SimEntity* ent, i32 flag)
{
    clear_flags(ent->flags, flag);
}

inline bool is_flag_set(Entity* ent, i32 flag)
{
    return is_flag_set(ent->sim.flags, flag);
}

inline void set_flags(Entity* ent, i32 flag)
{
    set_flags(ent->sim.flags, flag);
}

inline void clear_flags(Entity* ent, i32 flag)
{
    clear_flags(ent->sim.flags, flag);
}
}