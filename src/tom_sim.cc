
namespace tom
{

fn void add_hit_points(Entity* ent, i32 hp)
{
    ent->hp += hp;
    if (ent->hp > (i32)ent->max_hp) ent->hp = ent->max_hp;
}

fn void subtract_hit_points(Entity* ent, i32 hp)
{
    ent->hp -= hp;
    if (ent->hp < 0) {
        ent->hp = 0;
        clear_flags(ent->flags, EntityFlags::active);
        set_flags(ent->flags, EntityFlags::nonspatial);
    }
}

fn v3f calc_entity_delta(Entity* ent, EntityActions ent_act, EntityMoveSpec move_spec,
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

fn void move_entity(GameState* game, Entity* ent, v3f ent_delta, const f32 dt)
{
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
        v2f wall_nrm    = {};
        Entity* hit_ent = nullptr;

        if (!is_flag_set(ent->flags, EntityFlags::nonspatial)) {
            // FIXME: this is N * N bad
            for (Entity* test_ent = game->entities + 1; test_ent != game->entities + game->ent_cnt;
                 ++test_ent) {
                if (test_ent->ind == ent->ind) continue;  // don't test against self

                // Assert(test_ent);  // nullptr probably means something broke
                // if (!should_collide(game, ent, test_ent)) continue;
                // TODO: need this flag anymore?

                // // TODO: this is redundant, leaving it for now
                // if (ent->weapon_i == test_ent->ent_i || ent->parent_i == test_ent->ent_i)
                //     continue;  // skip parent and weapon

                // NOTE: Minkowski sum
                f32 r_w = ent->dims.x + test_ent->dims.x;
                f32 r_h = ent->dims.y + test_ent->dims.y;

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
                    wall_nrm = v2f { -1.0f, 0.0f };
                    hit_ent  = test_ent;
                }
                if (test_wall(max_corner.x, rel.x, rel.y, ent_delta.x, ent_delta.y, min_corner.y,
                              max_corner.y)) {
                    wall_nrm = v2f { 1.0f, 0.0f };
                    hit_ent  = test_ent;
                }
                if (test_wall(min_corner.y, rel.y, rel.x, ent_delta.y, ent_delta.x, min_corner.x,
                              max_corner.x)) {
                    wall_nrm = v2f { 0.0f, -1.0f };
                    hit_ent  = test_ent;
                }
                if (test_wall(max_corner.y, rel.y, rel.x, ent_delta.y, ent_delta.x, min_corner.x,
                              max_corner.x)) {
                    wall_nrm = v2f { 0.0f, 1.0f };
                    hit_ent  = test_ent;
                }
            }
        }

        if (hit_ent) {
            v3f wall_nrm_v3 = v3_init(wall_nrm);
            ent->vel -= 1.0f * vec_inner(ent->vel, wall_nrm_v3) * wall_nrm_v3;
            ent_delta -= 1.0f * vec_inner(ent_delta, wall_nrm_v3) * wall_nrm_v3;
        }
        // REVIEW: t_min is causing collision to always happen
        ent->pos += t_min * ent_delta;
        dist_remain -= t_min * ent_delta_len;
    }
    if (ent->dist_limit != 0.0f) {
        ent->dist_limit = dist_remain;
    }
}

fn bool entity_overlap_rect(const r3f rect, Entity* ent)
{
    r3f grown   = rect_add_dim(rect, ent->dims);
    bool result = rect_is_inside(rect, ent->pos);

    return result;
}

}  // namespace tom