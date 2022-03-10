#ifndef TOMATO_WORLD_HPP_
#define TOMATO_WORLD_HPP_
#include "Common.hpp"

namespace tom
{
// TODO: change to V3
struct WorldDif
{
    v2 difXy;
    f32 difZ;
};

struct WorldPos
{
    // NOTE: these are fixed point positioins. The high bits are the tile
    // chunk index, and the lower bits are the tile index in the chunk
    s32 chunkX, chunkY, chunkZ;
    // NOTE: from the chunk center
    v2 offset;
};

struct WorldEntityBlock
{
    u32 storedEntityCnt;
    u32 storedEntsInds[16];
    WorldEntityBlock *next;
};

struct WorldChunk
{
    s32 x, y, z;
    WorldEntityBlock firstBlock;
    WorldChunk *nextInHash;
};

struct World
{
    WorldChunk worldChunkHash[4096];
    WorldEntityBlock *firstFree;
};

void
initWorld(World &world, f32 tileSizesInMeters);

WorldDif
getWorldDiff(WorldPos &posA, WorldPos &posB);

WorldPos
mapIntoChunkSpace(WorldPos &pos, v2 offset);

WorldChunk *
getWorldChunk(World &world, s32 chunkX, s32 chunkY, s32 chunkZ, MemoryArena *arena);

WorldPos
absPosToWorldPos(f32 absX, f32 absY, f32 absZ);

void
changeEntityLocation(MemoryArena *arena, World &world, u32 lowInd, WorldPos *oldPos,
                     WorldPos *newPos);

}  // namespace tom

#endif  // TOMATO_WORLD_HPP_
