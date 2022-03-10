#ifndef SIM_REGION_HPP_
#define SIM_REGION_HPP_
#include "Common.hpp"
#include "Entity.hpp"

namespace tom
{

struct GameState;

struct SimEntityHash
{
    SimEntity *ptr;
    u32 ind;
};

struct SimRegion
{
    WorldPos origin;
    Rect bounds;

    u32 maxSimEntityCnt;
    u32 simEntityCnt;
    SimEntity *simEntities;

    // TODO: change size? need hash?
    // NOTE: must be a power of 2
    SimEntityHash hash[4096];
};

StoredEntity *
get_stored_entity(GameState state, u32 ind);

SimRegion *
begin_sim(MemoryArena *simArena, GameState &state, WorldPos origin, Rect bounds);

void
end_sim(GameState &state, SimRegion &region);

}  // namespace tom

#endif  // SIM_REGION_HPP_
