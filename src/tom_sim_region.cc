
namespace tom
{
function SimEntity* add_sim_entity_to_region(GameState* game, sim_region* region, u32 ent_i,
                                             Entity* source_ent, v3f* sim_pos);

function void add_hit_points(SimEntity* ent, i32 hp)
{
    ent->hp += hp;
    if (ent->hp > (i32)ent->max_hp) ent->hp = ent->max_hp;
}

function void subtract_hit_points(SimEntity* ent, i32 hp)
{
    ent->hp -= hp;
    if (ent->hp < 0) {
        ent->hp = 0;
        clear_flags(ent, sim_entity_flags::active);
        set_flags(ent, sim_entity_flags::nonspatial);
    }
}

function bool should_collide(GameState* game, SimEntity* ent_a, SimEntity* ent_b)
{
    Assert(ent_a && ent_b);

    bool result = false;

    if (ent_a->type > ent_b->type) {
        // TODO: swap func/template/macro?
        auto temp = ent_a;
        ent_a     = ent_b;
        ent_b     = temp;
    }

    // if and Entity "collides" barriers will stop them
    if ((is_flag_set(ent_a, sim_entity_flags::barrier) &&
         is_flag_set(ent_b, sim_entity_flags::collides)) ||
        (is_flag_set(ent_b, sim_entity_flags::barrier) &&
         is_flag_set(ent_a, sim_entity_flags::collides)))
        result = true;

    // DEBUG_BREAK(ent_a->type == EntityType::monster && ent_b->type == EntityType::sword);

    // TODO: better hash function (lol)
    u32 hash_bucket = ent_a->ent_i & (CountOf(game->collision_rule_hash) - 1);
    for (PairwiseCollisionRule* rule = game->collision_rule_hash[hash_bucket]; rule;
         rule                        = rule->next) {
        if (rule->ent_i_a == ent_a->ent_i && rule->ent_i_b == ent_b->ent_i) {
            result = rule->should_collide;
            break;
        }
    }

    return result;
}

function bool handle_collision(SimEntity* ent_a, SimEntity* ent_b)
{
    Assert(ent_a && ent_b);

    bool stop            = false;
    constexpr f32 hit_cd = 0.2f;

    if (ent_a->type > ent_b->type) {
        // TODO: swap func/template/macro?
        auto temp = ent_a;
        ent_a     = ent_b;
        ent_b     = temp;
    }

    // if and Entity "collides" barriers will stop them
    if ((is_flag_set(ent_a, sim_entity_flags::barrier) &&
         is_flag_set(ent_b, sim_entity_flags::collides)) ||
        (is_flag_set(ent_b, sim_entity_flags::barrier) &&
         is_flag_set(ent_a, sim_entity_flags::collides)))
        stop = true;

    if (ent_a->type == EntityType::player && ent_b->type == EntityType::monster) {
        if (ent_a->hit_cd > hit_cd) {
            ent_a->hit_cd = 0.f;
            subtract_hit_points(ent_a, 1);
        }
        stop = true;
    }

    if (ent_a->type == EntityType::player && ent_b->type == EntityType::familiar) {
        if (ent_a->hit_cd > hit_cd) {
            ent_a->hit_cd = 0.f;
            add_hit_points(ent_a, 1);
        }
        stop = true;
    }

    if (ent_a->type == EntityType::player && ent_b->type == EntityType::stair) {
        ent_a->pos.x += 3.f;
    }
    if (ent_a->type == EntityType::monster && ent_b->type == EntityType::sword) {
        if (ent_a->hit_cd > hit_cd) {
            ent_a->hit_cd = 0.f;
            subtract_hit_points(ent_a, 1);
        }
    }

    return stop;
}

function v3f calc_entity_delta(SimEntity* ent, EntityActions ent_act, EntityMoveSpec move_spec,
                               const f32 dt)
{
    v3f ent_accel = ent_act.dir;

    // NOTE: normalize vector to unit length
    f32 ent_accel_len = vec_length(ent_accel);
    // TODO: make speed specific to ent type

    if (ent_accel_len > 1.f) ent_accel *= (1.f / sqrt_f32(ent_accel_len));
    ent_accel *= move_spec.speed;
    ent_accel -= ent->vel * move_spec.drag;
    ent->vel += ent_accel * dt;

    v3f ent_delta = (.5f * ent_accel * square(dt) + ent->vel * dt);

    return ent_delta;
}

function void move_entity(GameState* game, sim_region* region, SimEntity* ent, v3f ent_delta,
                          const f32 dt)
{
    Assert(game && region && ent);

    f32 dist_remain = ent->dist_limit;
    if (dist_remain == 0.0f) {
        dist_remain = 1000.0f;
    }

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 t_min         = 1.0f;
        f32 ent_delta_len = vec_length(ent_delta);
        // REVIEW: use epsilon?
        if (!(ent_delta_len > 0.0f)) break;  // if the ent delta is 0 that means no movmement
        if (ent_delta_len > dist_remain) t_min = dist_remain / ent_delta_len;
        v2f wall_nrm       = {};
        SimEntity* hit_ent = nullptr;

        if (!is_flag_set(ent, sim_entity_flags::nonspatial)) {
            // FIXME: this is N * N bad
            for (SimEntity* test_ent = region->sim_entities;
                 test_ent != region->sim_entities + region->sim_entity_cnt; ++test_ent) {
                if (test_ent->ent_i == ent->ent_i) continue;  // don't test against self

                Assert(test_ent);  // nullptr probably means something broke
                if (!should_collide(game, ent, test_ent)) continue;
                // TODO: need this flag anymore?

                // TODO: this is redundant, leaving it for now
                if (ent->weapon_i == test_ent->ent_i || ent->parent_i == test_ent->ent_i)
                    continue;  // skip parent and weapon

                // NOTE: Minkowski sum
                f32 r_w = ent->dim.x + test_ent->dim.x;
                f32 r_h = ent->dim.y + test_ent->dim.y;

                v2f min_corner = { -.5f * v2f { r_w, r_h } };
                v2f max_corner = { .5f * v2f { r_w, r_h } };
                v3f rel        = ent->pos - test_ent->pos;

                auto test_wall = [&t_min](f32 wall_x, f32 rel_x, f32 rel_y, f32 player_delta_x,
                                          f32 player_delta_y, f32 min_y, f32 max_y) -> bool {
                    bool hit = false;

                    if (player_delta_x != 0.f) {
                        f32 t_res = (wall_x - rel_x) / player_delta_x;
                        f32 y     = rel_y + t_res * player_delta_y;

                        if (t_res >= 0.f && (t_min > t_res)) {
                            if (y >= min_y && y <= max_y) {
                                t_min = max(0.f, t_res - eps_f32);
                                hit   = true;
                            }
                        }
                    }

                    return hit;
                };

                if (test_wall(min_corner.x, rel.x, rel.y, ent_delta.x, ent_delta.y, min_corner.y,
                              max_corner.y)) {
                    wall_nrm = v2f { -1.f, 0.f };
                    hit_ent  = test_ent;
                }
                if (test_wall(max_corner.x, rel.x, rel.y, ent_delta.x, ent_delta.y, min_corner.y,
                              max_corner.y)) {
                    wall_nrm = v2f { 1.f, 0.f };
                    hit_ent  = test_ent;
                }
                if (test_wall(min_corner.y, rel.y, rel.x, ent_delta.y, ent_delta.x, min_corner.x,
                              max_corner.x)) {
                    wall_nrm = v2f { 0.f, -1.f };
                    hit_ent  = test_ent;
                }
                if (test_wall(max_corner.y, rel.y, rel.x, ent_delta.y, ent_delta.x, min_corner.x,
                              max_corner.x)) {
                    wall_nrm = v2f { 0.f, 1.f };
                    hit_ent  = test_ent;
                }
            }
        }

        if (hit_ent) {
            bool stop_on_collision = handle_collision(ent, hit_ent);
            if (stop_on_collision) {
                v3f wall_nrm_v3 = v3_init(wall_nrm);
                ent->vel -= 1.f * vec_inner(ent->vel, wall_nrm_v3) * wall_nrm_v3;
                ent_delta -= 1.f * vec_inner(ent_delta, wall_nrm_v3) * wall_nrm_v3;
            } else {
                t_min = 1.f;
            }
        }
        // REVIEW: t_min is causing collision to always happen
        ent->pos += t_min * ent_delta;
        dist_remain -= t_min * ent_delta_len;
    }
    if (ent->dist_limit != 0.f) {
        ent->dist_limit = dist_remain;
    }
}

function SimEntityHash* get_hash_from_ind(sim_region* region, u32 stored_i)
{
    Assert(stored_i);
    SimEntityHash* result = nullptr;
    u32 hash_val          = stored_i;
    for (u32 offset = 0; offset < CountOf(region->hash); ++offset) {
        SimEntityHash* entry = region->hash + ((hash_val + offset) & (CountOf(region->hash) - 1));
        if (entry->ind == 0 || entry->ind == stored_i) {
            result = entry;
            break;
        }
    }

    return result;
}

function void map_storage_ind_to_ent(sim_region* sim_region, u32 stored_i, SimEntity& Entity)
{
    SimEntityHash* entry = get_hash_from_ind(sim_region, stored_i);
    Assert(entry->ind == 0 || entry->ind == stored_i);
    entry->ind = stored_i;
    entry->ptr = &Entity;
}

function SimEntity* get_entity_from_ind(sim_region* region, u32 stored_i)
{
    SimEntityHash* entry = get_hash_from_ind(region, stored_i);
    return entry->ptr;
}

function void store_entity_ref(EntityRef* ref)
{
    if (ref->ptr != 0) {
        ref->ind = ref->ptr->ent_i;
    }
}

function void load_entity_ref(GameState* game, sim_region* region, EntityRef* ref)
{
    Assert(game && region && ref);

    if (ref->ind) {
        SimEntityHash* entry = get_hash_from_ind(region, ref->ind);
        if (entry->ptr == nullptr) {
            entry->ind = ref->ind;
            entry->ptr = add_sim_entity_to_region(game, region, ref->ind,
                                                  get_entity(game, ref->ind), nullptr);
        }
        ref->ptr = entry->ptr;
    }
}

// TODO: do I need this? its not called anywhere but it might be useful idk
function v3f get_cam_space_pos(const GameState& game, Entity* ent)
{
    v3f result = get_world_diff(ent->world_pos, game.camera.pos);

    return result;
}

function v3f get_sim_space_pos(const sim_region& region, const Entity& ent)
{
    // TODO: what is the null/invalid value here?
    v3f result = { 100'000'000.0f, 100'000'000.0f, 100'000'000.0f };
    if (!is_flag_set(ent.sim.flags, sim_entity_flags::nonspatial)) {
        result = get_world_diff(ent.world_pos, region.origin);
    }

    return result;
}

function v3f get_sim_space_pos(const GameState& game, const sim_region& region, u32 ent_i)
{
    // TODO: what is the null/invalid value here?
    v3f result        = { 100'000'000.0f, 100'000'000.0f, 100'000'000.0f };
    const Entity& ent = game.entities[ent_i];
    if (!is_flag_set(ent.sim.flags, sim_entity_flags::nonspatial)) {
        result = get_world_diff(ent.world_pos, region.origin);
    }

    return result;
}

function SimEntity* add_sim_entity_to_region_raw(GameState* game, sim_region* region, u32 ent_i,
                                                 Entity* source_ent)
{
    // because why use a const reference?
    Assert(game && region && ent_i);
    SimEntity* Entity = nullptr;

    if (region->sim_entity_cnt < region->max_sim_entity_cnt) {
        // TODO: should be a decrompression step, not a copy!
        Entity = region->sim_entities + region->sim_entity_cnt++;
        map_storage_ind_to_ent(region, ent_i, *Entity);
        if (source_ent) {
            *Entity = source_ent->sim;
            Assert(!is_flag_set(Entity->flags, sim_entity_flags::simming));
            set_flags(Entity->flags, sim_entity_flags::simming);
            // load_entity_ref(game, region, Entity->weapon_i);
        }
        Entity->ent_i      = ent_i;
        Entity->updateable = false;
    } else {
        InvalidCodePath;
    }

    return Entity;
}

function bool entity_overlap_rect(const r3f rect, SimEntity* ent)
{
    r3f grown   = rect_add_dim(rect, ent->dim);
    bool result = rect_is_inside(rect, ent->pos);

    return result;
}

function SimEntity* add_sim_entity_to_region(GameState* game, sim_region* region, u32 ent_i,
                                             Entity* source_ent, v3f* sim_pos)
{
    Assert(game && region && ent_i);

    SimEntity* dest_ent = add_sim_entity_to_region_raw(game, region, ent_i, source_ent);
    if (dest_ent) {
        if (sim_pos) {
            dest_ent->pos        = *sim_pos;
            dest_ent->updateable = entity_overlap_rect(region->update_bounds, dest_ent);
        } else {
            dest_ent->pos = get_sim_space_pos(*region, *source_ent);
        }
    }

    return dest_ent;
}

function sim_region* begin_sim(Arena* arena, GameState* game, WorldPos origin, r3f bounds, f32 dt)
{
    Assert(arena && game);

    // NOTE: clear the hash table!
    sim_region* region = push_struct<sim_region>(arena);
    zero_struct(region->hash);

    // TODO: IMPORTANT-> calc this from the max value of all entities radius + speed
    f32 update_safety_margin = max_entity_r + max_entity_vel * dt + update_margin;

    region->origin        = origin;
    region->update_bounds = bounds;
    region->bounds        = rect_add_radius(bounds, update_safety_margin);

    region->max_sim_entity_cnt = 4096;  // TODO: how many max entities?
    region->sim_entity_cnt     = 0;
    region->sim_entities       = push_array<SimEntity>(arena, region->max_sim_entity_cnt);

    WorldPos min_chunk_pos = map_into_chunk_space(origin, region->bounds.min);
    WorldPos max_chunk_pos = map_into_chunk_space(origin, region->bounds.max);

    // make all entities outside camera space stored
    for (i32 chunk_y = min_chunk_pos.chunk_y; chunk_y < max_chunk_pos.chunk_y; ++chunk_y) {
        for (i32 chunk_x = min_chunk_pos.chunk_x; chunk_x < max_chunk_pos.chunk_x; ++chunk_x) {
            WorldChunk* chunk =
                get_world_chunk(game->world, chunk_x, chunk_y, origin.chunk_z, &game->world_arena);
            if (chunk) {
                for (WorldEntityBlock* block = &chunk->first_block; block; block = block->next) {
                    for (u32 ent_i = 0; ent_i < block->ent_cnt; ++ent_i) {
                        u32 block_ent_i = block->ent_inds[ent_i];
                        Entity* ent     = game->entities + block_ent_i;
                        if (!is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)) {
                            v3f sim_space_pos = get_sim_space_pos(*region, *ent);
                            if (entity_overlap_rect(region->bounds, &ent->sim)) {
                                add_sim_entity_to_region(game, region, block_ent_i, ent,
                                                         &sim_space_pos);
                            }
                        }
                    }
                }
            }
        }
    }

    return region;
}

function void end_sim(GameState* game, sim_region* region)
{
    // TODO: low entities stored in the World and not games state?
    for (SimEntity* sim_ent = region->sim_entities;
         sim_ent != region->sim_entities + region->sim_entity_cnt; ++sim_ent) {
        Entity* ent = game->entities + sim_ent->ent_i;
        ent->sim    = *sim_ent;
        // if (ent->type == EntityType::player && state->debug_flag) __debugbreak();

        // store_entity_ref(&ent->sim.weapon_i);

        // TODO: save game back to stored sim_entity, once high entities do state decompression
        WorldPos new_pos = !is_flag_set(ent->sim.flags, sim_entity_flags::nonspatial)
                               ? map_into_chunk_space(region->origin, sim_ent->pos)
                               : null_world_pos();
        if (ent->world_pos != new_pos)
            change_entity_location(&game->world_arena, game->world, ent, new_pos);
        Assert(is_flag_set(ent->sim.flags, sim_entity_flags::simming));
        clear_flags(ent->sim.flags, sim_entity_flags::simming);
    }
}
}  // namespace tom