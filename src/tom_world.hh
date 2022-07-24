#define CHUNK_UNITIALIZED I32_MAX

namespace tom
{
struct Entity;

struct WorldPos
{
    // NOTE: these are fixed point positioins. the high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    i32 chunk_x, chunk_y, chunk_z;
    // NOTE: from the chunk center
    v3f offset;
};

struct WorldEntityBlock
{
    u32 ent_cnt;
    u32 ent_inds[16];
    WorldEntityBlock* next;
};

struct WorldChunk
{
    i32 x, y, z;
    WorldEntityBlock first_block;
    WorldChunk* next_in_hash;
};

struct World
{
    WorldChunk world_chunk_hash[4096];
    WorldEntityBlock* first_free;
};

inline WorldPos null_world_pos()
{
    WorldPos result = {};
    result.chunk_x  = CHUNK_UNITIALIZED;
    return result;
}

inline bool is_valid(WorldPos pos)
{
    return pos.chunk_x != CHUNK_UNITIALIZED;
}

inline bool is_valid(WorldPos* pos)
{
    if (pos) return is_valid(*pos);

    return false;
}

inline bool operator==(WorldPos& lhs, WorldPos& rhs)
{
    return lhs.chunk_x == rhs.chunk_x && lhs.chunk_y == rhs.chunk_y && lhs.chunk_z == rhs.chunk_z &&
           lhs.offset == rhs.offset;
}

inline bool operator!=(WorldPos& lhs, WorldPos& rhs)
{
    return !(lhs == rhs);
}

}  // namespace tom