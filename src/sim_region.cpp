#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{
namespace
{

v2
get_cam_space_pos(const Game_State &state, Entity_Low *low_ent)
{
    World_Dif diff { get_world_diff(low_ent->pos, state.camera.pos) };
    v2 result { diff.dif_xy };

    return result;
}

bool
validate_entity_pairs(const Game_State &state)
{
    bool valid { true };
    for (u32 high_i { 1 }; high_i < state.high_cnt; ++high_i) {
        valid = valid && state.low_entities[state.high_entities[high_i].low_i].high_i == high_i;
    }

    return valid;
}

Sim_Region
start_sim(Game_State &state, World_Pos region_center, Rect region_bounds)
{
    World_Pos min_chunk_pos { map_into_chunk_space(region_center,
                                                   rect::min_corner(region_bounds)) };
    World_Pos max_chunk_pos { map_into_chunk_space(region_center,
                                                   rect::max_corner(region_bounds)) };

    // make all entities outside camera space low
    for (s32 chunk_y { min_chunk_pos.chunk_y }; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x { min_chunk_pos.chunk_x }; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            World_Chunk *chunk { get_world_chunk(*state.world, chunk_x, chunk_y,
                                                 region_center.chunk_z) };
            if (chunk) {
                for (World_Entity_Block *block { &chunk->first_block }; block;
                     block = block->next) {
                    for (u32 ent_i {}; ent_i < block->low_entity_cnt; ++ent_i) {
                        u32 low_i { block->low_ent_inds[ent_i] };
                        Entity_Low *low_ent { state.low_entities + low_i };
                        if (low_ent->high_i == 0) {
                            v2 cam_space_pos { get_cam_space_pos(state, low_ent) };
                            if (rect::is_inside(region_bounds, cam_space_pos)) {
                                make_entity_high(state, low_i);
                            }
                        }
                    }
                }
            }
        }
    }

    return {};

    TomAssert(validate_entity_pairs(state));
}

void
end_sim(Sim_Region *region)
{
    Entity *entity { region->entities };
    for (u32 ent_i {}; ent_i < region->entity_cnt; ++ent_i, ++entity) {
        // TODO: store entity logic here
    }
}
}  // namespace

Entity_High *
make_entity_high(Game_State &state, Entity_Low *low_ent, u32 low_i, v2 cam_space_pos)
{
    Entity_High *high_ent {};
    TomAssert(low_i < global::max_low_cnt);
    TomAssert(!low_ent->high_i);

    if (state.high_cnt < global::max_high_cnt) {
        u32 high_i { state.high_cnt++ };
        TomAssert(high_i < global::max_high_cnt);
        high_ent = state.high_entities + high_i;

        high_ent->pos   = cam_space_pos;
        high_ent->vel   = { 0.f, 0.f };
        high_ent->dir   = Entity_Direction::north;
        high_ent->low_i = low_i;

        low_ent->high_i = high_i;
    } else {
        INVALID_CODE_PATH;
    }

    return high_ent;
}

Entity_High *
make_entity_high(Game_State &state, u32 low_i)
{
    Entity_High *high_ent {};

    Entity_Low *low_ent { state.low_entities + low_i };

    if (low_ent->high_i) {
        high_ent = state.high_entities + low_ent->high_i;
    } else {
        v2 cam_space_pos { get_cam_space_pos(state, low_ent) };
        high_ent = make_entity_high(state, low_ent, low_i, cam_space_pos);
    }

    return high_ent;
}

void
simulate_region(Game_State &state, World_Pos region_center, Rect region_bounds)
{
    start_sim(state, region_center, region_bounds);
}

}  // namespace tom
