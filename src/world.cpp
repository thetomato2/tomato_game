#include "World.hpp"

#define CHUNK_UNITIALIZED S32_MAX

namespace tom
{

static bool
isCanonical(f32 relCoord)
{
    return relCoord >= global::chunk_size_meters * -.5f &&
           relCoord <= global::chunk_size_meters * .5f;
}

static bool
isCanonical(v2 relCoord)
{
    return isCanonical(relCoord.x) && isCanonical(relCoord.y);
}

static void
recanonicalizeCoord(s32 &coord, f32 &relCoord)
{
    // NOTE: world is assumed to be toroidal (torus shaped world),
    // if you step off one end where you wrap around
    s32 offset = math::round_f32_s32(relCoord / scast(f32, global::chunk_size_meters));
    coord += offset;
    relCoord -= offset * scast(f32, global::chunk_size_meters);

    TOM_ASSERT(isCanonical(relCoord));
}

static bool
isSameChunk(const WorldPos a, const WorldPos b)
{
    TOM_ASSERT(isCanonical(a.offset));
    TOM_ASSERT(isCanonical(b.offset));

    return (a.chunkX == b.chunkX && a.chunkY == b.chunkY && a.chunkZ == b.chunkZ);
}

static WorldPos
getCenteredPoint(const s32 x, const s32 y, const s32 z)
{
    WorldPos result;

    result.chunkX = x;
    result.chunkY = y;
    result.chunkZ = z;

    return result;
}

void
initWorld(World &world, f32 tileSizesInMeters)
{
    world.firstFree = nullptr;
    for (s32 chunkI = 0; chunkI < ARRAY_COUNT(world.worldChunkHash); ++chunkI) {
        world.worldChunkHash[chunkI].x                           = CHUNK_UNITIALIZED;  // null chunk
        world.worldChunkHash[chunkI].first_block.storedEntityCnt = 0;
    }
}

WorldDif
getWorldDiff(onst WorldPos &posA, const WorldPos &posB)
{
    WorldDif result;

    v2 diffXy;
    diffXy.x = scast(f32, posA.chunkX) - scast(f32, posB.chunkX);
    diffXy.y = scast(f32, posA.chunkY) - scast(f32, posB.chunkY);
    f32 difZ = scast(f32, posA.chunkZ) - scast(f32, posB.chunkZ);

    result.difXy = global::chunk_size_meters * diffXy + (posA.offset - posB.offset);
    result.difZ  = 0.f;

    return result;
}

WorldPos
mapIntoChunkSpace(const WorldPos &pos, const v2 offset)
{
    auto result = pos;

    // TODO: decide on tile chunk alignment
    result.offset += offset;
    recanonicalizeCoord(result.chunkX, result.offset.x);
    recanonicalizeCoord(result.chunkY, result.offset.y);

    return result;
}

WorldChunk *
getWorldChunk(World &world, const s32 chunkX, const s32 chunkY, const s32 chunkZ,
              MemoryArena *arena)
{
    TOM_ASSERT(chunkX > -global::chunk_safe_margin);
    TOM_ASSERT(chunkY > -global::chunk_safe_margin);
    TOM_ASSERT(chunkZ > -global::chunk_safe_margin);
    TOM_ASSERT(chunkX < global::chunk_safe_margin);
    TOM_ASSERT(chunkY < global::chunk_safe_margin);
    TOM_ASSERT(chunkZ < global::chunk_safe_margin);

    // TODO: BETTER HASH FUNCTION!
    s32 hashVal  = 19 * chunkX + 7 * chunkY + 3 * chunkZ;
    s32 hashSlot = scast(s32, hashVal & (ARRAY_COUNT(world.worldChunkHash) - 1));
    TOM_ASSERT(hashSlot < ARRAY_COUNT(world.worldChunkHash));

    WorldChunk *chunk = world.worldChunkHash + hashSlot;
    do {
        // found chunk
        if (chunkX == chunk->x && chunkY == chunk->y && chunkZ == chunk->z) {
            break;
        }
        // didn't find chunk but there isn't a next chunk
        // so allocate a new one and move the pointer there
        if (arena && chunk->x == CHUNK_UNITIALIZED && !chunk->nextInHash) {
            chunk->nextInHash = PUSH_STRUCT(arena, WorldChunk);
            chunk             = chunk->nextInHash;
            chunk->x          = CHUNK_UNITIALIZED;
        }

        // if chunk is empty (0) allocate the tiles
        if (arena && chunk->x == CHUNK_UNITIALIZED) {
            chunk->x = chunkX;
            chunk->y = chunkY;
            chunk->z = chunkZ;

            // do we want to always initialize?
            chunk->nextInHash = nullptr;
            break;
        }
        chunk = chunk->nextInHash;
    } while (chunk);

    return chunk;
    u32 lowEntInds[16];
    WorldEntityBlock *next;
}

WorldPos
absPosToWorldPos(f32 absX, f32 absY, f32 absZ)
{
    WorldPos result;

    result.chunkX = scast(s32, absX / global::chunk_size_meters);
    result.chunkY = scast(s32, absY / global::chunk_size_meters);
    result.chunkZ = scast(s32, absZ / global::chunk_size_meters);

    result.offset.x = absX - (result.chunkX * global::chunk_size_meters);
    result.offset.y = absY - (result.chunkY * global::chunk_size_meters);

    return result;
}

void
changeEntityLocation(MemoryArena *arena, World &world, const u32 lowInd, const WorldPos *oldPos,
                     WorldPos *newPos)
{
    if (oldPos && isSameChunk(*oldPos, *newPos)) {
        //  leave the entity where it is
        return;
    } else {
        if (oldPos) {
            // pull the entity out its old block
            WorldChunk *chunk =
                getWorldChunk(world, oldPos->chunkX, oldPos->chunkY, oldPos->chunkZ);
            TOM_ASSERT(chunk);
            if (chunk) {
                bool found                   = false;
                WorldEntityBlock *firstBlock = &chunk->firstBlock;
                for (WorldEntityBlock *block = &chunk->firstBlock; block && !found;
                     block                   = block->next) {
                    for (u32 i {}; i < block->storedEntityCnt; ++i) {
                        if (block->storedEntsInds[i] == lowInd) {
                            TOM_ASSERT(firstBlock->storedEntityCnt > 0);
                            block->storedEntsInds[i] =
                                firstBlock->storedEntsInds[--firstBlock->storedEntityCnt];
                            if (firstBlock->storedEntityCnt == 0) {
                                if (firstBlock->next) {
                                    WorldEntityBlock *nextBlock = firstBlock->next;
                                    *firstBlock                 = *nextBlock;
                                    nextBlock->next             = world.firstFree;
                                    world.firstFree             = nextBlock;
                                }
                            }
                            found = true;
                        }
                    }
                }
            }
        }

        //  insert the entity into its new block
        WorldChunk *chunk =
            getWorldChunk(world, newPos->chunkX, newPos->chunkY, newPos->chunkZ, arena);
        TOM_ASSERT(chunk);
        WorldEntityBlock *block = &chunk->firstBlock;
        if (block->storedEntityCnt == ARRAY_COUNT(block->storedEntsInds)) {
            // out of room! make new block
            WorldEntityBlock *oldBlock = world.firstFree;
            if (oldBlock) {
                world.firstFree = oldBlock->next;
            } else {
                oldBlock = PUSH_STRUCT(arena, WorldEntityBlock);
            }
            *oldBlock              = *block;
            block->next            = oldBlock;
            block->storedEntityCnt = 0;
        }

        TOM_ASSERT(block->storedEntityCnt < ARRAY_COUNT(block->storedEntsInds));
        block->storedEntsInds[block->storedEntityCnt++] = lowInd;
    }
}
}  // namespace tom
