#include "sim_region.hpp"
#include "entity.hpp"
#include "game.hpp"
#include "world.hpp"

namespace tom
{
// ===============================================================================================
// #INTERNAL
// ===============================================================================================
static Sim_Entity *
add_entity(Game_State &state, Sim_Region &region, u32 stored_i, Stored_Entity &source_ent,
           v2 *sim_pos);

static Sim_Entity_Hash *
get_hash_from_ind(Sim_Region &sim_region, u32 stored_i)
{
    TomAssert(stored_i);
    Sim_Entity_Hash *result = nullptr;
    u32 hash_val            = stored_i;
    for (u32 offset = 0; offset < ArrayCount(sim_region.hash); ++offset) {
        Sim_Entity_Hash *entry =
            sim_region.hash + ((hash_val + offset) & (ArrayCount(sim_region.hash) - 1));
        if (entry->ind == 0 || entry->ind == stored_i) {
            result = entry;
            break;
        }
    }

    return result;
}

static void
map_storage_ind_to_ent(Sim_Region &sim_region, u32 stored_i, Sim_Entity &entity)
{
    Sim_Entity_Hash *entry = get_hash_from_ind(sim_region, stored_i);
    TomAssert(entry->ind == 0 || entry->ind == stored_i);
    entry->ind = stored_i;
    entry->ptr = &entity;
}

static void
store_entity_ref(Entity_Ref *ref)
{
    if (ref->ptr != 0) {
        ref->ind = ref->ptr->stored_i;
    }
}
static Sim_Entity *
get_entity_by_ind(Sim_Region &region, u32 stored_i)
{
    Sim_Entity_Hash *entry = get_hash_from_ind(region, stored_i);
    return entry->ptr;
}

static void
load_entity_ref(Game_State &state, Sim_Region sim_region, Entity_Ref &ref)
{
    if (ref.ind) {
        Sim_Entity_Hash *entry = get_hash_from_ind(sim_region, ref.ind);
        if (entry->ptr == nullptr) {
        }
        ref.ptr = entry->ptr;
    }
}

static v2
get_cam_space_pos(const Game_State &state, Stored_Entity *stored_ent)
{
    World_Dif diff = get_world_diff(stored_ent->world_pos, state.camera.pos);
    v2 result      = { diff.dif_xy };

    return result;
}

static v2
get_sim_space_pos(const Sim_Region &region, const Stored_Entity &stored_ent)
{
    World_Dif dif = get_world_diff(stored_ent.world_pos, region.origin);
    return dif.dif_xy;
}

static Sim_Entity *
add_entity(Game_State &state, Sim_Region &region, u32 stored_i, Stored_Entity *source_ent)
{
    TomAssert(stored_i);
    Sim_Entity *entity = nullptr;

    if (region.sim_entity_cnt < region.max_sim_entity_cnt) {
        // TODO: should be a decrompression step, not a copy!
        entity = region.sim_entities + region.sim_entity_cnt++;
        map_storage_ind_to_ent(region, stored_i, *entity);

        if (source_ent) {
            *entity = source_ent->sim;
            load_entity_ref(state, region, entity->weapon_i);
        }

        entity->stored_i = stored_i;

    } else {
        INVALID_CODE_PATH;
    }

    return entity;
}

static Sim_Entity *
add_entity(Game_State &state, Sim_Region &region, u32 stored_i, Stored_Entity &source_ent,
           v2 *sim_pos)
{
    Sim_Entity *dest_ent = add_entity(region, stored_i, &source_ent);
    if (dest_ent) {
        if (sim_pos) {
            dest_ent->pos = *sim_pos;
        } else {
            dest_ent->pos = get_sim_space_pos(region, source_ent);
        }
    }
}

// ===============================================================================================
// #EXTERNAL
// ===============================================================================================

Stored_Entity *
get_stored_entity(Game_State &state, u32 ind)
{
    Stored_Entity *result = nullptr;

    if (ind > 0 && ind < state.stored_cnt) {
        result = state.stored_entities + ind;
    }

    return result;
}

Sim_Region *
begin_sim(Memory_Arena *sim_arena, Game_State &state, World_Pos origin, Rect bounds)
{
    Sim_Region *region         = PushStruct(sim_arena, Sim_Region);
    region->origin             = origin;
    region->bounds             = bounds;
    region->max_sim_entity_cnt = 4096;  // TODO: how many max entities?
    region->sim_entity_cnt     = 0;
    region->sim_entities       = PushArray(sim_arena, region->max_sim_entity_cnt, Sim_Entity);

    World_Pos min_chunk_pos = map_into_chunk_space(origin, rect::min_corner(bounds));
    World_Pos max_chunk_pos = map_into_chunk_space(origin, rect::max_corner(bounds));

    // make all entities outside camera space low
    for (s32 chunk_y = min_chunk_pos.chunk_y; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (s32 chunk_x = min_chunk_pos.chunk_x; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            World_Chunk *chunk = get_world_chunk(*state.world, chunk_x, chunk_y, origin.chunk_z);
            if (chunk) {
                for (World_Entity_Block *block = &chunk->first_block; block; block = block->next) {
                    for (u32 ent_i = 0; ent_i < block->stored_entity_cnt; ++ent_i) {
                        u32 stored_i              = block->stored_ents_inds[ent_i];
                        Stored_Entity *stored_ent = state.stored_entities + stored_i;
                        v2 sim_space_pos          = get_sim_space_pos(*region, *stored_ent);
                        if (rect::is_inside(region->bounds, sim_space_pos)) {
                            add_entity(state, *region, stored_i, *stored_ent, &sim_space_pos);
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
end_sim(Game_State &state, Sim_Region &region)
{
    // TODO: low entities stored in the world and not games state?
    Sim_Entity *entity = region.sim_entities;
    for (u32 ent_i = 0; ent_i < region.sim_entity_cnt; ++ent_i, ++entity) {
        Stored_Entity &stored_ent = state.stored_entities[ent_i];

        stored_ent.sim = *entity;
        store_entity_ref(&stored_ent.sim.weapon_i);

        // TODO: save state back to stored entity, once high entities do state decompression
        World_Pos new_pos = map_into_chunk_space(region.origin, entity->pos);
        change_entity_location(state.world_arena, state.world, entity->stored_i, &stored_ent.pos,
                               &new_pos);
    }
}

// ===============================================================================================
// #EXTERNAL
// ===============================================================================================

}  // namespace tom
