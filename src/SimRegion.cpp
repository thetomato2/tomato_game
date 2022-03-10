#include "SimRegion.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "Game.hpp"

namespace tom
{
// ===============================================================================================
// #INTERNAL
// ===============================================================================================
static SimEntity *
addEntity(GameState &state, SimRegion &region, u32 storedI, StoredEntity &sourceEnt, v2 *simPos);

static SimEntityHash *
getHashFromInd(SimRegion &simRegion, u32 storedInd)
{
    TOM_ASSERT(storedInd);
    SimEntityHash *result = nullptr;
    u32 hashVal           = storedInd;
    for (u32 offset = 0; offset < ARRAY_COUNT(simRegion.hash); ++offset) {
        SimEntityHash *entry =
            simRegion.hash + ((hashVal + offset) & (ARRAY_COUNT(simRegion.hash) - 1));
        if (entry->ind == 0 || entry->ind == storedInd) {
            result = entry;
            break;
        }
    }

    return result;
}

static void
mapStorageIndToEnt(SimRegion &simRegion, u32 storedInd, SimEntity &entity)
{
    SimEntityHash *entry = getHashFromInd(simRegion, storedInd);
    TOM_ASSERT(entry->ind == 0 || entry->ind == storedInd);
    entry->ind = storedInd;
    entry->ptr = &entity;
}

static void
storeEntityRef(EntityRef *ref)
{
    if (ref->ptr != 0) {
        ref->ind = ref->ptr->storedInd;
    }
}
static SimEntity *
getEntityByInd(SimRegion &region, u32 storedInd)
{
    SimEntityHash *entry = getHashFromInd(region, storedInd);
    return entry->ptr;
}

static void
loadEntityRef(GameState &state, SimRegion simRegion, EntityRef &ref)
{
    if (ref.ind) {
        SimEntityHash *entry = getHashFromInd(simRegion, ref.ind);
        if (entry->ptr == nullptr) {
        }
        ref.ptr = entry->ptr;
    }
}

static v2
getCamSpacePos(const GameState &state, StoredEntity *storedEnt)
{
    WorldDif diff = getWorldDiff(storedEnt->worldPos, state.camera.pos);
    v2 result     = { diff.difXy };

    return result;
}

static v2
getSimSpacePos(const SimRegion &region, const StoredEntity &storedEnt)
{
    WorldDif dif = getWorldDiff(storedEnt.worldPos, region.origin);
    return dif.difXy;
}

static SimEntity *
addEntity(GameState &state, SimRegion &region, u32 storedInd, StoredEntity *sourceEnt)
{
    TOM_ASSERT(storedInd);
    SimEntity *entity = nullptr;

    if (region.simEntityCnt < region.maxSimEntityCnt) {
        // TODO: should be a decrompression step, not a copy!
        entity = region.simEntities + region.simEntityCnt++;
        mapStorageIndToEnt(region, storedInd, *entity);

        if (sourceEnt) {
            *entity = sourceEnt->sim;
            loadEntityRef(state, region, entity->weaponInd);
        }

        entity->storedInd = storedInd;

    } else {
        INVALID_CODEPATH;
    }

    return entity;
}

static SimEntity *
addEntity(GameState &state, SimRegion &region, u32 storedInd, StoredEntity &sourceEnt, v2 *simPos)
{
    SimEntity *destEnt = addEntity(Region, storedInd, &sourceEnt);
    if (destEnt) {
        if (simPos) {
            destEnt->Pos = *simPos;
        } else {
            destEnt->Pos = getSimSpacePos(Region, sourceEnt);
        }
    }
}

// ===============================================================================================
// #EXTERNAL
// ===============================================================================================

StoredEntity *
get_stored_entity(GameState &state, u32 ind)
{
    StoredEntity *result = nullptr;

    if (ind > 0 && ind < state.storedCnt) {
        result = state.storedEntities + ind;
    }

    return result;
}

SimRegion *
beginSim(MemoryArena *simArena, GameState &state, WorldPos origin, Rect bounds)
{
    SimRegion *region       = PUSH_STRUCT(simArena, SimRegion);
    region->origin          = origin;
    region->bounds          = bounds;
    region->maxSimEntityCnt = 4096;  // TODO: how many max entities?
    region->simEntityCnt    = 0;
    region->simEntities     = PUSH_ARRAY(simArena, region->maxSimEntityCnt, SimEntity);

    WorldPos minChunkPos = mapIntoChunkSpace(Origin, rect::minCorner(Bounds));
    WorldPos maxChunkPos = mapIntoChunkSpace(Origin, rect::maxCorner(Bounds));

    // make all entities outside camera space low
    for (s32 chunkY = minChunkPos.chunkY; chunkY < maxChunkPos.chunkY; ++chunkY) {
        for (s32 chunkX = minChunkPos.chunkX; chunkX < maxChunkPos.chunkX; ++chunkX) {
            WorldChunk *chunk = getWorldChunk(*State.World, chunkX, chunkY, origin.chunkZ);
            if (chunk) {
                for (WorldEntityBlock *block = &chunk->firstBlock; block; block = block->next) {
                    for (u32 entI = 0; entI < block->storedEntityCnt; ++entI) {
                        u32 storedInd           = block->stored_ents_inds[entI];
                        StoredEntity *storedEnt = state.storedEntities + storedInd;
                        v2 simSpacePos          = get_simSpacePos(*region, *storedEnt);
                        if (rect::isInside(Region->Bounds, simSpacePos)) {
                            addEntity(State, *region, storedInd, *storedEnt, &simSpacePos);
                            printf("rect is inside!\n");
                        }
                    }
                }
            }
        }
    }

    return region;
}

void
end_sim(GameState &state, SimRegion &region)
{
    // TODO: low entities stored in the world and not games state?
    SimEntity *entity = region.simEntities;
    for (u32 entI = 0; entI < region.simEntityCnt; ++entI, ++entity) {
        StoredEntity &storedEnt = state.storedEntities[entI];

        storedEnt.Sim = *entity;
        store_entity_ref(&storedEnt.sim.weapon_i);

        // TODO: save state back to stored entity, once high entities do state decompression
        WorldPos newPos = mapIntoChunkSpace(Region.Origin, entity->pos);
        changeEntityLocation(State.WorldArena, state.world, entity->storedInd, &storedEnt.pos,
                             &newPos);
    }
}

// ===============================================================================================
// #EXTERNAL
// ===============================================================================================

}  // namespace tom
