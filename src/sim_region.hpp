#ifndef SIM_REGION_HPP_
#define SIM_REGION_HPP_

#include "common.hpp"

namespace tom
{

struct Game_State;
struct Entity;
struct Entity_Low;
struct Entity_High;
struct World_Pos;

struct Sim_Region
{
    u32 max_entity_cnt;
    u32 entity_cnt;
    Entity *entities;
};

Entity_High *
make_entity_high(Game_State &state, Entity_Low *low_ent, u32 low_i, v2 cam_space_pos);

Entity_High *
make_entity_high(Game_State &state, u32 low_i);

void
simulate_region(Game_State &state, World_Pos region_center, Rect region_bounds);

}  // namespace tom

#endif  // SIM_REGION_HPP_
