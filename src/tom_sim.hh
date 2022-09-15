#ifndef TOM_SIM_HH
#define TOM_SIM_HH

#include "tom_core.hh"

namespace tom
{

struct GameState;


////////////////////////////////////////////////////////////////////////////////////////////////
// #DECLARES
v3f calc_entity_delta(Entity *ent, EntityActions ent_act, EntityMoveSpec move_spec,  f32 dt);
void move_entity(GameState *game, Entity *ent, v3f ent_delta,  f32 dt);

}  // namespace tom

#endif

