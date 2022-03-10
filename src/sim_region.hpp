#ifndef SIM_REGION_HPP_
#define SIM_REGION_HPP_
#include "common.hpp"
#include "entity.hpp"

namespace tom
{

struct Game_State;

struct Sim_Entity_Hash
{
    Sim_Entity *ptr;
    u32 ind;
};

struct Sim_Region
{
    World_Pos origin;
    Rect bounds;

    u32 max_sim_entity_cnt;
    u32 sim_entity_cnt;
    Sim_Entity *sim_entities;

    // TODO: change size? need hash?
    // NOTE: must be a power of 2
    Sim_Entity_Hash hash[4096];
};

Stored_Entity *
get_stored_entity(Game_State state, u32 ind);

Sim_Region *
begin_sim(Memory_Arena *sim_arena, Game_State &state, World_Pos origin, Rect bounds);

void
end_sim(Game_State &state, Sim_Region &region);

}  // namespace tom

#endif  // SIM_REGION_HPP_
