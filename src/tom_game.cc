#include "tom_graphics.cc"
// #include "tom_world.cc"
#include "tom_entity.cc"
#include "tom_sim.cc"

namespace tom
{

fn void process_keyboard(const Keyboard &kb, EntityActions *entity_action)
{
    if (entity_action) {
        if (key_pressed(kb.t)) entity_action->start = true;
        if (key_pressed(kb.space)) entity_action->attack = true;
        if (kb.space.ended_down) entity_action->jump = true;
        if (kb.w.ended_down) entity_action->dir.y += 1.0f;
        if (kb.s.ended_down) entity_action->dir.y += -1.0f;
        if (kb.a.ended_down) entity_action->dir.x += -1.0f;
        if (kb.d.ended_down) entity_action->dir.x += 1.0f;
        if (key_down(kb.left_shift)) entity_action->sprint = true;
    }
}

#if USE_DS5
fn void process_ds5(const DS5_State &ds5, EntityActions *entity_action)
{
    if (entity_action) {
        if (button_pressed(ds5.menu)) entity_action->start = true;
        if (button_pressed(ds5.square)) entity_action->attack = true;
        if (button_down(ds5.dpad_U)) entity_action->dir.y += 1.0f;
        if (button_down(ds5.dpad_D)) entity_action->dir.y += -1.0f;
        if (button_down(ds5.dpad_L)) entity_action->dir.x += -1.0f;
        if (button_down(ds5.dpad_R)) entity_action->dir.x += 1.0f;
        if (button_down(ds5.cross)) entity_action->sprint = true;
        if (ds5.stick_L.x > 10 || ds5.stick_L.x < -10)
            entity_action->dir.x = (f32)ds5.stick_L.x / 128;
        if (ds5.stick_L.y > 10 || ds5.stick_L.y < -10)
            entity_action->dir.y = (f32)ds5.stick_L.y / 128;
    }
}
#endif

fn void game_init(ThreadContext *thread, AppState *app)
{
    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    // init memory
    GameState *game            = app->game;
    game->debug_draw_collision = false;
    game->debug_flag           = false;

    const char *bg = "uv_color_squares_960x540";

    // load textures
    game->player_sprites[EntityDirection::north] =
        texture_load_from_file("../../assets/images/player_n.png");
    game->player_sprites[EntityDirection::east] =
        texture_load_from_file("../../assets/images/player_e.png");
    game->player_sprites[EntityDirection::south] =
        texture_load_from_file("../../assets/images/player_s.png");
    game->player_sprites[EntityDirection::west] =
        texture_load_from_file("../../assets/images/player_w.png");

    game->sword_sprites[EntityDirection::north] =
        texture_load_from_file("../../assets/images/sword_n.png");
    game->sword_sprites[EntityDirection::east] =
        texture_load_from_file("../../assets/images/sword_e.png");
    game->sword_sprites[EntityDirection::south] =
        texture_load_from_file("../../assets/images/sword_s.png");
    game->sword_sprites[EntityDirection::west] =
        texture_load_from_file("../../assets/images/sword_w.png");

    game->cat_sprites[0] = texture_load_from_file("../../assets/images/cat.png");
    game->cat_sprites[1] = texture_load_from_file("../../assets/images/cat.png");

    game->tree_sprite  = texture_load_from_file("../../assets/images/tree.png");
    game->bush_sprite  = texture_load_from_file("../../assets/images/bush.png");
    game->stair_sprite = texture_load_from_file("../../assets/images/stair.png");
    game->wall_sprite  = texture_load_from_file("../../assets/images/wall.png");

    // set World render size

    // NOTE: Entity 0 is the null Entity
    ++game->ent_cnt;

    add_entity(game, EntityType::player);
    game->player_cnt               = 1;
    game->entity_camera_follow_ind = 1;
    game->camera.pos               = {};
    game->camera.dims.w            = 16.0f;
    game->camera.dims.h            = 9.0f;
    game->camera.dims *= 1.2f;
    game->camera.dims *= 60.0f / g_meters_to_pixels;

    i32 tree_cnt = 100;
    f32 x        = 0 - tree_cnt / 2;
    for (i32 i = 0; i < tree_cnt; ++i) {
        Entity *ent = add_entity(game, EntityType::tree, { x, 4.0f, 0.0f });
        x += ent->dims.x + ent->dims.x * 0.1f;
    }

    game->env_map_dims = { 512, 256 };
    for (auto &map : game->env_maps) {
        v2i dims = game->env_map_dims;
        for (auto &lod : map.lod) {
            lod.width  = dims.w;
            lod.height = dims.h;
            lod.type   = Texture::Type::R8G8B8A8;
            lod.buf    = plat_malloc(dims.w * dims.h * texture_get_texel_size(lod.type));
            // make_sphere_nrm_map(&lod, 1.0f);
            dims.w >>= 1;
            dims.h >>= 1;
        }
    }
    clear_color(game->env_maps[0].lod, { 0xff0000ff });
    clear_color(game->env_maps[1].lod, { 0xff00ff00 });
    clear_color(game->env_maps[2].lod, { 0xffff0000 });

    u32 n_checkers        = 32;
    Color_u32 checker_col = { 0xf8000000 };
    u32 checker_x         = (u32)((f32)game->env_maps[0].lod[0].width / (f32)n_checkers);
    u32 checker_y         = checker_x;
    for (u32 y = 0; y < n_checkers; ++y) {
        for (u32 x = 0; x < n_checkers; x += 2) {
            r2u rc;
            if (y % 2 == 0) {
                rc = { x * checker_x, y * checker_y, x * checker_x + checker_x,
                       y * checker_y + checker_y };
            } else {
                rc = { x * checker_x + checker_x, y * checker_y, x * checker_x + checker_x * 2,
                       y * checker_y + checker_y };
            }
            draw_rect(game->env_maps[0].lod, rc, checker_col);
            draw_rect(game->env_maps[1].lod, rc, checker_col);
            draw_rect(game->env_maps[2].lod, rc, checker_col);
        }
    }

    game->test_alb.width  = 128;
    game->test_alb.height = 128;
    game->test_alb.type   = Texture::Type::R8G8B8A8;
    game->test_alb.buf    = plat_malloc(texture_get_size(&game->test_alb));
    clear_color(&game->test_alb, { 0xff1e1e1e });

    game->test_nrm.width  = game->test_alb.width;
    game->test_nrm.height = game->test_alb.height;
    game->test_nrm.type   = game->test_alb.type;
    game->test_nrm.buf    = plat_malloc(texture_get_size(&game->test_nrm));
    // make_sphere_nrm_map(&game->test_nrm, 0.0f, 0.0f, 1.0f);
    make_sphere_nrm_map(&game->test_nrm, 0.0f);
    // make_pyramid_nrm_map(&game->test_nrm, 0.0f);
}

fn void game_update_and_render(ThreadContext *thread, AppState *app)
{
    // ==============================================================================================
    // #START
    // ==============================================================================================

    const f32 dt = app->dt;

    GameState *game      = app->game;
    Keyboard &kb         = app->input.keyboard;
    Texture *back_buffer = &app->back_buffer;
    Camera *cam          = &game->camera;
    Entity *player       = &game->entities[1];
    // NOTE: overwriting the old arena
    Arena trans_arena =
        init_arena(app->memory.transient_storage, app->memory.transient_storage_size);

    // NOTE: only doing one player
    EntityActions player_action = {};
    process_keyboard(kb, &player_action);
#if USE_DS5
    for (u32 i = 0; i < DS5_MAX_CNT; ++i) {
        if (app->input.ds5_context[i].connected) {
            process_ds5(app->input.ds5_state[i], &player_action);
        }
    }
#endif
    game->player_acts[1] = player_action;

    if (key_pressed(kb.d1)) game->debug_draw_collision = !game->debug_draw_collision;

    local constexpr f32 cam_inc = 0.1f;
    if (key_down(kb.add)) cam->dims += cam_inc;
    if (key_down(kb.subtract)) cam->dims -= cam_inc;

    local constexpr f32 zoom_inc = 0.3f;
    f32 old_zoom                 = g_meters_to_pixels;
    if (key_down(kb.down)) {
        g_meters_to_pixels -= zoom_inc;
        cam->dims *= old_zoom / g_meters_to_pixels;
    }
    if (key_down(kb.up)) {
        g_meters_to_pixels += zoom_inc;
        cam->dims *= old_zoom / g_meters_to_pixels;
    }

    RenderGroup *render_group =
        alloc_render_group(&trans_arena, Megabytes(4), g_meters_to_pixels, *cam);

    r2f cam_rc            = rect_init_dims(cam->pos, cam->dims);
    Color_u32 clear_color = color_u32(black);
    push_clear(render_group, clear_color);

    for (u32 i = 0; i < game->ent_cnt; ++i) {
        Entity *ent = &game->entities[i];
        if (ent->type == EntityType::null) continue;

        if (!is_flag_set(ent->flags, EntityFlags::active))
            continue;  // don't update inactive entities

        ent->hit_cd += dt;

        EntityMoveSpec move_spec = default_move_spec();
        EntityActions ent_act    = {};

        // update logic
        if (is_flag_set(ent->flags, EntityFlags::updateable)) {
            switch (ent->type) {
                case EntityType::none: {
                } break;
                case EntityType::player: {
                    f32 ent_speed = game->player_acts[1].sprint ? move_spec.speed = 25.f
                                                                : move_spec.speed = 10.f;
                    ent_act       = game->player_acts[1];
                    cam->pos      = player->pos.xy;
                    cam_rc        = rect_init_dims(cam->pos, cam->dims);
                    push_rect(render_group, cam->pos, cam->dims, { 0xff2c2c2c });
                } break;
                case EntityType::wall: {
                    // do nothing
                } break;
                case EntityType::tree: {
                    // do nothing
                } break;
                case EntityType::bush: {
                    // do nothing
                } break;
                case EntityType::stairs: {
                    // do nothing
                } break;
                case EntityType::familiar: {
                    f32 dist_sq      = vec_length_sq(player->pos - ent->pos);
                    f32 one_over_len = 1.0f / sqrt_f32(dist_sq);
                    f32 min_dist     = 2.0f;
                    v3 dif           = player->pos - ent->pos;
                    if (abs_f32(dif.x) > min_dist || abs_f32(dif.y) > min_dist)
                        ent_act.dir = one_over_len * (dif);
                } break;
                case EntityType::monster: {
                    // TODO: update monster
                } break;
                case EntityType::sword: {
                } break;

                case EntityType::stair: {
                    // TODO: stair logic?
                } break;
                default: InvalidCodePath;
            }

            v3f ent_delta = calc_entity_delta(ent, ent_act, move_spec, dt);

            if (abs_f32(ent->vel.x) > 0.0f + EPS_F32 || abs_f32(ent->vel.y) > 0.0f + EPS_F32)
                move_entity(game, ent, ent_delta, dt);

            // NOTE: changes the players direction for the sprite
            v3 pv = ent->vel;

            constexpr f32 dir_eps = 0.1f;

            if (abs_f32(pv.x) > abs_f32(pv.y)) {
                if (pv.x > 0.f + dir_eps) {
                    ent->dir = EntityDirection::east;
                } else if (pv.x < 0.f - dir_eps) {
                    ent->dir = EntityDirection::west;
                }

            } else if (abs_f32(pv.y) > abs_f32(pv.x)) {
                if (pv.y > 0.f + dir_eps) {
                    ent->dir = EntityDirection::north;
                } else if (pv.y < 0.f - dir_eps) {
                    ent->dir = EntityDirection::south;
                }
            }

            ent->hit_cd += dt;
        }

        // NOTE: cam is following the player
        if (!rect_is_inside(cam_rc, ent->pos.xy)) continue;

        // r2f screen_space_cam_rc = {
        //     (f32)buf_mid.x - abs_f32(cam_rc.x0 - cam->pos.x) * g_meters_to_pixels,
        //     (f32)buf_mid.y - abs_f32(cam_rc.y0 - cam->pos.y) * g_meters_to_pixels,
        //     (f32)buf_mid.x + abs_f32(cam->pos.x - cam_rc.x1) * g_meters_to_pixels,
        //     (f32)buf_mid.y + abs_f32(cam->pos.y - cam_rc.y1) * g_meters_to_pixels
        // };

        // v2f ent_screen_space_pos = {
        //     ((ent->pos.x - cam_rc.x0) * g_meters_to_pixels) + screen_space_cam_rc.x0,
        //     screen_space_cam_rc.y1 - ((ent->pos.y - cam_rc.y0) * g_meters_to_pixels)
        // };

        // ===========================================================================================
        // #RENDER
        // ===========================================================================================

        auto push_texture_if = [&](Texture *tex) {
            if (tex != nullptr) {
                m3 model = m3_identity();
                model    = m3_set_trans(model, ent->pos.xy);
                model    = m3_sca_x(model, ent->dims.x);
                model    = m3_sca_y(model, ent->dims.y);
                // model    = m3_rot(model, app->time);

                push_texture(render_group, { model }, tex, ent->sprite_off, model);
            } else
                push_rect(render_group, ent->pos.xy, ent->dims.xy, color_u32(pink));
        };

        switch (ent->type) {
            case EntityType::none: {
            } break;
            case EntityType::player: {
                push_texture_if(&game->player_sprites[ent->dir]);

            } break;
            case EntityType::wall: {
            } break;
            case EntityType::tree: {
                push_texture_if(&game->tree_sprite);
            } break;
            case EntityType::bush: {
            } break;
            case EntityType::stairs: {
            } break;
            case EntityType::familiar: {
            } break;
            case EntityType::monster: {
            } break;
            case EntityType::sword: {
            } break;
            case EntityType::stair: {
            } break;
            default: InvalidCodePath;
        }

        if (game->debug_draw_collision) {
            push_rect_outline(render_group, ent->pos.xy, ent->dims.xy, 2, color_u32(red));
            push_rect_outline(render_group, cam->pos, cam->dims, 2, color_u32(light_blue));
        }
    }

    draw_render_group(render_group, back_buffer);
}

}  // namespace tom
