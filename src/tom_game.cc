#include "tom_game.hh"
#include "tom_thread.hh"
#include "tom_input.hh"
#include "tom_app.hh"
#include "tom_entity.hh"
#include "tom_sim.hh"

namespace tom
{

internal void process_keyboard(Entity *player, const Keyboard &kb, EntityActions *entity_action)
{
    if (entity_action) {
        if (key_pressed(kb.t)) entity_action->start = true;
        if (key_pressed(kb.space)) entity_action->attack = true;
        if (kb.space.ended_down) entity_action->jump = true;
        if (kb.w.ended_down) entity_action->dir.y += 1.0f;
        if (kb.s.ended_down) entity_action->dir.y += -1.0f;
        if (kb.a.ended_down) entity_action->dir.x += -1.0f;
        if (kb.d.ended_down) entity_action->dir.x += 1.0f;
        entity_action->dir = vec_noz(entity_action->dir);
        if (key_down(kb.left_shift)) entity_action->sprint = true;

        if (kb.w.ended_down && kb.d.ended_down) {
            player->dir = EntityDirection::north_east;
        } else if (kb.w.ended_down && kb.a.ended_down) {
            player->dir = EntityDirection::north_west;
        } else if (kb.s.ended_down && kb.d.ended_down) {
            player->dir = EntityDirection::south_east;
        } else if (kb.s.ended_down && kb.a.ended_down) {
            player->dir = EntityDirection::south_west;
        } else if (kb.w.ended_down) {
            player->dir = EntityDirection::north;
        } else if (kb.s.ended_down) {
            player->dir = EntityDirection::south;
        } else if (kb.d.ended_down) {
            player->dir = EntityDirection::east;
        } else if (kb.a.ended_down) {
            player->dir = EntityDirection::west;
        }
    }
}

#if USE_DS5
internal void process_ds5(Entity *player, const DS5_State &ds5, EntityActions *entity_action)
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
internal r2f calc_player_sprite(Entity *player, SpriteSheet *sheet)
{
    r2f result;

    const f32 inv_x = 1.0f / (f32)sheet->x_cnt;
    const f32 inv_y = 1.0f / (f32)sheet->y_cnt;

    constexpr u32 NORTH      = 0;
    constexpr u32 NORTH_EAST = 5;
    constexpr u32 EAST       = 9;
    constexpr u32 SOUTH_EAST = 13;
    constexpr u32 SOUTH      = 17;

    bool flipped = false;

    switch (player->dir) {
        case EntityDirection::north: {
            if (sheet->cur_y != NORTH) {
                sheet->cur_y = NORTH;
                sheet->cur_x = 0;
            }
        } break;
        case EntityDirection::east: {
            if (sheet->cur_y != EAST) {
                sheet->cur_y = EAST;
                sheet->cur_x = 0;
            }
        } break;
        case EntityDirection::south: {
            if (sheet->cur_y != SOUTH) {
                sheet->cur_y = SOUTH;
                sheet->cur_x = 0;
            }
        } break;
        case EntityDirection::west: {
            // NOTE: need to flip the texture
            if (sheet->cur_y != EAST) {
                sheet->cur_y = EAST;
                sheet->cur_x = 0;
                flipped      = true;
            }
        } break;
        case EntityDirection::north_east: {
            // NOTE: need to flip the texture
            if (sheet->cur_y != NORTH_EAST) {
                sheet->cur_y = NORTH_EAST;
                sheet->cur_x = 0;
            }
        } break;
        case EntityDirection::north_west: {
            // NOTE: need to flip the texture
            if (sheet->cur_y != NORTH_EAST) {
                sheet->cur_y = NORTH_EAST;
                sheet->cur_x = 0;
                flipped      = true;
            }
        } break;
        case EntityDirection::south_east: {
            // NOTE: need to flip the texture
            if (sheet->cur_y != SOUTH_EAST) {
                sheet->cur_y = SOUTH_EAST;
                sheet->cur_x = 0;
            }
        } break;
        case EntityDirection::south_west: {
            // NOTE: need to flip the texture
            if (sheet->cur_y != SOUTH_EAST) {
                sheet->cur_y = SOUTH_EAST;
                sheet->cur_x = 0;
                flipped      = true;
            }
        } break;
            INVALID_DEFAULT_CASE;
    }

    result.x0 = (f32)sheet->cur_x * inv_x;
    result.y0 = (f32)sheet->cur_y * inv_y;
    result.x1 = result.x0 + inv_x;
    result.y1 = result.y0 + inv_y;

    if (player->next_sprite > 1.0f) {
        player->next_sprite = 0.0f;
        ++sheet->cur_x;
    }
    if (sheet->cur_x >= sheet->x_cnt) sheet->cur_x = 0;

    return result;
}

void game_init(ThreadContext *thread, AppState *app)
{
    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    // init memory
    GameState *game            = app->game;
    game->debug_draw_collision = false;
    game->debug_flag           = false;

    spawn_threads(app->threads, &game->render_queue);

    const char *bg = "uv_color_squares_960x540";

    // load textures
    game->player_sprites[EntityDirection::north] =
        // texture_load_from_file("../../assets/images/pink.png");
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
    game->player_sprite.sheet =
        texture_load_from_file("../../assets/images/hr-level1_running_gun.png");
    game->player_sprite.x_cnt = 22;
    game->player_sprite.y_cnt = 18;

    game->cat_sprites[0] = texture_load_from_file("../../assets/images/cat.png");
    game->cat_sprites[1] = texture_load_from_file("../../assets/images/cat.png");

    game->tree_sprite  = texture_load_from_file("../../assets/images/tree.png");
    game->bush_sprite  = texture_load_from_file("../../assets/images/bush.png");
    game->stair_sprite = texture_load_from_file("../../assets/images/stair.png");
    game->wall_sprite  = texture_load_from_file("../../assets/images/wall.png");
    game->bg           = texture_load_from_file("../../assets/images/bg.png");

    // set World render size

    // NOTE: Entity 0 is the null Entity
    ++game->ent_cnt;
    game->meters_to_pixels = 45.0f;

    Entity *player = add_entity(game, EntityType::player);
    // set_flags(player->flags, EntityFlags::nonspatial);
    game->player_cnt               = 1;
    game->entity_camera_follow_ind = 1;
    game->camera.pos               = {};
    game->camera.dims.w            = 45.0f;
    game->camera.dims.h            = 25.0f;
    // game->camera.dims *= 1.2f;
    // game->camera.dims *= 60.0f / game->meters_to_pixels;

#if 1
    i32 tree_cnt = 20;
    f32 x        = 0 - tree_cnt / 2;
    for (i32 i = 0; i < tree_cnt; ++i) {
        Entity *ent = add_entity(game, EntityType::tree, { x, 4.0f, 0.0f });
        add_entity(game, EntityType::tree, { x, -8.0f, 0.0f });
        if (i == 0 || i == tree_cnt - 1) {
            for (i32 j = 3; j > -8; j -= 2) {
                add_entity(game, EntityType::tree, { x, (f32)j, 0.0f });
            }
        }
        x += ent->dims.x + ent->dims.x * 0.1f;
    }
#else
    for (i32 tx = 5; tx < 100; ++tx) {
        for (i32 ty = -1; ty < 100; ++ty) {
            add_entity(game, EntityType::tree, { (f32)tx, (f32)ty, 0.0f });
        }
    }
#endif

#if 0
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
#endif
}

void game_update_and_render(ThreadContext *thread, AppState *app)
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
    process_keyboard(player, kb, &player_action);
#if USE_DS5
    for (u32 i = 0; i < DS5_MAX_CNT; ++i) {
        if (app->input.ds5_context[i].connected) {
            process_ds5(player, app->input.ds5_state[i], &player_action);
        }
    }
#endif
    game->player_acts[1] = player_action;

    if (key_pressed(kb.d1)) game->debug_draw_collision = !game->debug_draw_collision;

    local constexpr f32 cam_inc = 0.1f;
    if (key_down(kb.add)) {
        cam->dims += cam_inc;
        printf("Cam dims - (%f, %f)\n", cam->dims.x, cam->dims.y);
    }
    if (key_down(kb.subtract)) {
        printf("Cam dims - (%f, %f)\n", cam->dims.x, cam->dims.y);
        cam->dims -= cam_inc;
    }

    local constexpr f32 zoom_inc = 0.3f;
    f32 old_zoom                 = game->meters_to_pixels;
    if (key_down(kb.down)) {
        game->meters_to_pixels -= zoom_inc;
        cam->dims *= old_zoom / game->meters_to_pixels;
        printf("Zoom: %f\n", game->meters_to_pixels);
    }
    if (key_down(kb.up)) {
        game->meters_to_pixels += zoom_inc;
        cam->dims *= old_zoom / game->meters_to_pixels;
        printf("Zoom: %f\n", game->meters_to_pixels);
    }

    RenderGroup *render_group =
        alloc_render_group(&trans_arena, Megabytes(4), game->meters_to_pixels, *cam);
    render_group->meters_to_pixels = game->meters_to_pixels;

    r2f cam_rc         = rect_init_dims(cam->pos, cam->dims);
    Color_u32 clear_co = color_u32(black);
    // push_clear(render_group, clear_color);
    clear_color(&app->back_buffer, clear_co);

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
                    // push_rect(render_group, cam->pos, cam->dims, { 0xff2c2c2c });
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
                default: INVALID_CODE_PATH;
            }

            v3f ent_delta = calc_entity_delta(ent, ent_act, move_spec, dt);

            if (abs_f32(ent->vel.x) > 0.0f + EPS_F32 || abs_f32(ent->vel.y) > 0.0f + EPS_F32)
                move_entity(game, ent, ent_delta, dt);

            // NOTE: changes the players direction for the sprite
            v3 pv = ent->vel;

            constexpr f32 dir_eps = 0.1f;

            // if (abs_f32(pv.x) > abs_f32(pv.y)) {
            //     if (pv.x > 0.f + dir_eps) {
            //         ent->dir = EntityDirection::east;
            //     } else if (pv.x < 0.f - dir_eps) {
            //         ent->dir = EntityDirection::west;
            //     }
            //
            // } else if (abs_f32(pv.y) > abs_f32(pv.x)) {
            //     if (pv.y > 0.f + dir_eps) {
            //         ent->dir = EntityDirection::north;
            //     } else if (pv.y < 0.f - dir_eps) {
            //         ent->dir = EntityDirection::south;
            //     }
            // }

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

                push_texture(render_group, tex, model);
                if (game->debug_draw_collision) {
                    push_rect_outline(render_group, ent->pos.xy, ent->dims.xy, 2, color_u32(red),
                                      model);
                }
            } else
                push_rect(render_group, ent->pos.xy, ent->dims.xy, color_u32(pink), {});
        };

        switch (ent->type) {
            case EntityType::none: {
            } break;
            case EntityType::player: {
                {
                    m3 model = m3_identity();
                    // model    = m3_set_trans(model, ent->pos.xy);
                    model = m3_sca_x(model, 80.0f);
                    model = m3_sca_y(model, 80.0f);
                    push_texture(render_group, &game->bg, model);
                }
                if (game->debug_draw_collision) {
                    m3 model = m3_identity();
                    model    = m3_set_trans(model, ent->pos.xy);
                    model    = m3_sca_x(model, cam->dims.x);
                    model    = m3_sca_y(model, cam->dims.y);
                    push_rect_outline(render_group, cam->pos, cam->dims, 4, color_u32(light_blue),
                                      model);
                }
                {
                    m3 model = m3_identity();
                    model    = m3_set_trans(model, ent->pos.xy);
                    model    = m3_sca_x(model, ent->dims.x);
                    model    = m3_sca_y(model, ent->dims.y);
                    player->next_sprite += abs_f32(vec_length(player->vel)) * dt * 50.0f;
                    r2f offset = calc_player_sprite(ent, &game->player_sprite);
                    if (ent->dir == EntityDirection::west ||
                        ent->dir == EntityDirection::north_west ||
                        ent->dir == EntityDirection::south_west) {
                        model.r[0].xy = -model.r[0].xy;
                    }
                    push_atlas(render_group, &game->player_sprite.sheet, model, offset);
                }

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
            default: INVALID_CODE_PATH;
        }
    }

    render_group->tile_r = 128;
    draw_render_group_tiled(&game->render_queue, render_group, back_buffer);
}

}  // namespace tom
