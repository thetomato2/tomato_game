#ifndef SIM_REGION_HPP_
#define SIM_REGION_HPP_

#include "common.hpp"
#include "entity.hpp"

namespace tom
{

struct game_state;

struct sim_entity_hash
{
    sim_entity *ptr;
    u32 ind;
};

struct sim_region
{
    world_pos origin;
    rect bounds;

    u32 max_sim_entity_cnt;
    u32 sim_entity_cnt;
    sim_entity *sim_entities;

    // todo: change size? need hash?
    // note: must be a power of 2
    sim_entity_hash hash[4096];
};

struct entity_move_spec
{
    f32 speed;
    f32 drag;
};

inline entity_move_spec
get_default_move_spec()
{
    return { 10.0f, 10.0f };
}

stored_entity *
get_stored_entity(game_state state, u32 ind);

sim_region *
begin_sim(memory_arena *sim_arena, game_state &state, world_pos origin, rect bounds);

void
end_sim(game_state &state, sim_region &region);

}  // namespace tom

#endif  // SIM_REGION_HPP_
