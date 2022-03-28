#ifndef TOMATO_SIM_REGION_HPP_
#define TOMATO_SIM_REGION_HPP_

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
    rect3 bounds;
    rect3 update_bounds;

    u32 max_sim_entity_cnt;
    u32 sim_entity_cnt;
    sim_entity *sim_entities;

    // todo: change size? need hash?
    // note: must be a power of 2
    sim_entity_hash hash[4096];
};

void
move_entity(game_state *state, sim_region *region, sim_entity *ent, v3 ent_delta, f32 dt);

v3
get_sim_space_pos(const game_state &state, const sim_region &region, u32 ent_i);

sim_region *
begin_sim(memory_arena *arena, game_state *state, world_pos origin, rect3 bounds);

void
end_sim(game_state *state, sim_region *region);

}  // namespace tom

#endif  // SIM_REGION_HPP_
