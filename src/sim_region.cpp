#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{

namespace
{

struct Stored_Entity
{
    World_Pos pos;
};

struct Sim_Entity
{
    World_Pos pos;
};

struct Sim_Region
{
    World_Pos origin;
    Rect bounds;

    u32 max_sim_entity_cnt;
    u32 sim_entity_cnt;
    Sim_Entity *sim_entities;
};

}  // namespace

static v2
get_cam_space_pos(const Game_State &state, Entity_Low *low_ent)
{
    World_Dif diff { get_world_diff(low_ent->pos, state.camera.pos) };
    v2 result { diff.dif_xy };

    return result;
}

static v2
get_region_space_pos(const Sim_Region &region, const Stored_Entity &stored)
{
    World_Dif dif = get_world_diff(stored.pos, region.origin);
    return dif.dif_xy;
}

static bool
validate_entity_pairs(const Game_State &state)
{
    bool valid { true };
    for (u32 high_i { 1 }; high_i < state.high_cnt; ++high_i) {
        valid = valid && state.low_entities[state.high_entities[high_i].low_i].high_i == high_i;
    }

    return valid;
}

static Sim_Entity *
add_entity(Sim_Region &sim_region)
{
    Sim_Entity *entity {};

    if (sim_region.sim_entity_cnt < sim_region.max_sim_entity_cnt) {
        entity = &sim_region.sim_entities[sim_region.sim_entity_cnt++];
    } else {
        INVALID_CODE_PATH;
    }

    return entity;
}

static Sim_Entity *
add_entity(Sim_Region &sim_region, Stored_Entity *source, v2 *sim_pos = nullptr)
{
    Sim_Entity *dest { add_entity(sim_region) };
    if (dest) {
        if (sim_pos) {
            dest->pos;
        } else {
            // build wolrd position
            printf("poop\n");
        }
    }
}

static Sim_Region
start_sim(Game_State &state, World_Pos origin, Rect bounds)
{
    Sim_Region sim_region;
    sim_region.bounds = bounds;
    sim_region.origin = origin;

    World_Pos min_chunk_pos { map_into_chunk_space(origin, rect::min_corner(bounds)) };
    World_Pos max_chunk_pos { map_into_chunk_space(origin, rect::max_corner(bounds)) };

    // make all entities outside camera space low
    for (s32 chunk_y { min_chunk_pos.chunk_y }; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x { min_chunk_pos.chunk_x }; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            World_Chunk *chunk { get_world_chunk(*state.world, chunk_x, chunk_y, origin.chunk_z) };
            if (chunk) {
                for (World_Entity_Block *block { &chunk->first_block }; block;
                     block = block->next) {
                    for (u32 ent_i {}; ent_i < block->low_entity_cnt; ++ent_i) {
                        u32 low_i { block->low_ent_inds[ent_i] };
                        Entity_Low *low_ent { state.low_entities + low_i };
                        v2 cam_space_pos { get_cam_space_pos(state, low_ent) };
                        if (rect::is_inside(bounds, cam_space_pos)) {
                            // add_entity();
                            printf("rect is inside!\n");
                        }
                    }
                }
            }
        }
    }

    return {};

    TomAssert(validate_entity_pairs(state));
}

static void
end_sim(Sim_Region *region)
{
    Sim_Entity *entity { region->sim_entities };
    for (u32 ent_i {}; ent_i < region->sim_entity_cnt; ++ent_i, ++entity) {
        // TODO: store entity logic here
    }
}

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
simulate_region(Game_State &state, World_Pos origin, Rect bounds)
{
    start_sim(state, origin, bounds);
}

}  // namespace tom
