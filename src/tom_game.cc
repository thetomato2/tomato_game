
namespace tom
{

function void process_keyboard(const Keyboard& kb, EntityActions* entity_action)
{
    if (entity_action) {
        if (key_pressed(kb.t)) entity_action->start = true;
        if (key_pressed(kb.space)) entity_action->attack = true;
        if (kb.space.ended_down) entity_action->jump = true;
        if (kb.w.ended_down) entity_action->dir.y += 1.f;
        if (kb.s.ended_down) entity_action->dir.y += -1.f;
        if (kb.a.ended_down) entity_action->dir.x += -1.f;
        if (kb.d.ended_down) entity_action->dir.x += 1.f;
        if (key_down(kb.left_shift)) entity_action->sprint = true;
    }
}

function void process_ds5(const DS5_State& ds5, EntityActions* entity_action)
{
    if (entity_action) {
        if (button_pressed(ds5.menu)) entity_action->start = true;
        if (button_pressed(ds5.square)) entity_action->attack = true;
        if (button_down(ds5.dpad_U)) entity_action->dir.y += 1.f;
        if (button_down(ds5.dpad_D)) entity_action->dir.y += -1.f;
        if (button_down(ds5.dpad_L)) entity_action->dir.x += -1.f;
        if (button_down(ds5.dpad_R)) entity_action->dir.x += 1.f;
        if (button_down(ds5.cross)) entity_action->sprint = true;
    }
}

function void clear_collision_rule(GameState* game, u32 ent_i)
{
    // TODO: make better data structure that allows removal of collision rules without searching
    // the entire table
    for (u32 hash_bucket = 0; hash_bucket < CountOf(game->collision_rule_hash); ++hash_bucket) {
        for (PairwiseCollisionRule** rule = &game->collision_rule_hash[hash_bucket]; *rule;) {
            if ((*rule)->ent_i_a == ent_i || (*rule)->ent_i_b == ent_i) {
                PairwiseCollisionRule* removed_rule = *rule;
                *rule                               = (*rule)->next;

                removed_rule->next              = game->first_free_collision_rule;
                game->first_free_collision_rule = removed_rule;
            } else {
                rule = &(*rule)->next;
            }
        }
    }
}

function void add_collsion_rule(GameState* game, u32 ent_i_a, u32 ent_i_b, bool should_collide)
{
    // TODO: collapse this with should_collide()
    auto ent_a = game->entities + ent_i_a;
    auto ent_b = game->entities + ent_i_b;
    if (ent_a->sim.type > ent_b->sim.type) {
        // TODO: swap func/template/macro
        auto temp = ent_i_a;
        ent_i_a   = ent_i_b;
        ent_i_b   = temp;
    }

    PairwiseCollisionRule* found = nullptr;

    u32 hash_bucket = ent_i_a & (CountOf(game->collision_rule_hash) - 1);
    for (PairwiseCollisionRule* rule = game->collision_rule_hash[hash_bucket]; rule;
         rule                        = rule->next) {
        if (rule->ent_i_a == ent_i_a && rule->ent_i_b == ent_i_b) {
            found = rule;
            break;
        }
    }

    if (!found) {
        found = game->first_free_collision_rule;
        if (found) {
            game->first_free_collision_rule = found->next;
        } else {
            found = push_struct<PairwiseCollisionRule>(&game->world_arena);
        }
        found->next                            = game->collision_rule_hash[hash_bucket];
        game->collision_rule_hash[hash_bucket] = found;
    }

    Assert(found);
    if (found) {
        found->ent_i_a        = ent_i_a;
        found->ent_i_b        = ent_i_b;
        found->should_collide = (b32)should_collide;
    }
}

function void game_init(ThreadContext* thread, AppState* state)
{
    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    // init memory
    GameState* game = state->game;
    init_arena(&game->world_arena, state->memory.permanent_storage_size - sizeof(*state),
               (byt*)state->memory.permanent_storage + sizeof(*state));

    game->world = push_struct<World>(&game->world_arena);

    World* world = game->world;
    Assert(world);
    init_world(world, tile_size_meters);

    game->debug_draw_collision = false;
    game->debug_flag           = false;

    const char* bg = "uv_color_squares_960x540";

    // load textures
    game->default_img = load_argb_or_default(thread, nullptr, "pink");
    game->player_sprites[EntityDirection::north] =
        load_argb_or_default(thread, &game->default_img, "player_n");
    game->player_sprites[EntityDirection::east] =
        load_argb_or_default(thread, &game->default_img, "player_e");
    game->player_sprites[EntityDirection::south] =
        load_argb_or_default(thread, &game->default_img, "player_s");
    game->player_sprites[EntityDirection::west] =
        load_argb_or_default(thread, &game->default_img, "player_w");

    game->monster_sprites[EntityDirection::north] =
        load_argb_or_default(thread, &game->default_img, "monster_n");
    game->monster_sprites[EntityDirection::east] =
        load_argb_or_default(thread, &game->default_img, "monster_e");
    game->monster_sprites[EntityDirection::south] =
        load_argb_or_default(thread, &game->default_img, "monster_s");
    game->monster_sprites[EntityDirection::west] =
        load_argb_or_default(thread, &game->default_img, "monster_w");

    game->sword_sprites[EntityDirection::north] =
        load_argb_or_default(thread, &game->default_img, "sword_n");
    game->sword_sprites[EntityDirection::east] =
        load_argb_or_default(thread, &game->default_img, "sword_e");
    game->sword_sprites[EntityDirection::south] =
        load_argb_or_default(thread, &game->default_img, "sword_s");
    game->sword_sprites[EntityDirection::west] =
        load_argb_or_default(thread, &game->default_img, "sword_w");

    game->cat_sprites[0] = load_argb_or_default(thread, &game->default_img, "cat");
    game->cat_sprites[1] = load_argb_or_default(thread, &game->default_img, "cat");

    game->bg_img        = load_argb_or_default(thread, &game->default_img, bg);
    game->crosshair_img = load_argb_or_default(thread, &game->default_img, "crosshair");
    game->tree_sprite   = load_argb_or_default(thread, &game->default_img, "tree");
    game->bush_sprite   = load_argb_or_default(thread, &game->default_img, "bush");
    game->stair_sprite  = load_argb_or_default(thread, &game->default_img, "stairs");
    game->wall_sprite   = load_argb_or_default(thread, &game->default_img, "wall");

    i32 screen_base_x {}, screen_base_y {}, screen_base_z {}, virtual_z {}, rng_ind {};
    i32 screen_x { screen_base_x }, screen_y { screen_base_y }, screen_z { screen_base_z };

    // set World render size
    game->camera.pos.chunk_x = 0;
    game->camera.pos.chunk_y = 0;
    game->camera.pos.offset.x += screen_base_x / 2.f;
    game->camera.pos.offset.y += screen_base_y / 2.f;

    // NOTE: Entity 0 is the null Entity
    // TODO: do I even need this?
    add_new_entity(game);
    // game->player_cnt               = Game_Input::s_input_cnt;
    game->player_cnt               = 1;
    game->entity_camera_follow_ind = 1;

    // add the player entites
    for (u32 player_i = 1; player_i <= game->player_cnt; ++player_i) {
        add_player(game, player_i, 0.f, 0.f, 0.f);
    }

    game->entities[2].world_pos = game->entities[1].world_pos;

#if 1
    f32 x_len = 55.f;
    for (f32 x = -20.f; x < x_len; ++x) {
        add_bush(game, x, 15, 0.f);
        add_bush(game, x, -5, 0.f);
        if ((i32)x % 17 == 0) continue;
        add_bush(game, x, 5, 0.f);
    }

    for (f32 x = -20.f; x <= x_len; x += 15.f) {
        for (f32 y = -5; y < 15; ++y) {
            if ((y == 0.f || y == 10.f) && (x != -20.f && x != 55.f)) continue;
            add_bush(game, x, y, 0.);
        }
    }
#endif

    for (f32 x = 0.f; x < 5.f; x += 0.5f) {
        add_tree(game, x, -2.f, 0.f);
    }

    {
        auto p1          = get_entity(game, 1);
        auto monster_ent = add_monster(game, 5.f, 0.f, 0.f);
        auto cat_ent     = add_cat(game, -1.f, 1.f, 0.f);
        auto sword_ent   = get_entity(game, p1->sim.weapon_i);
        auto stair_ent   = add_stair(game, 2.5f, 3.f, 0.f);
        add_collsion_rule(game, sword_ent->sim.ent_i, monster_ent->sim.ent_i, true);
        add_collsion_rule(game, p1->sim.ent_i, monster_ent->sim.ent_i, true);
        add_collsion_rule(game, p1->sim.ent_i, cat_ent->sim.ent_i, true);
        add_collsion_rule(game, p1->sim.ent_i, stair_ent->sim.ent_i, true);
    }
}

function void game_update_and_render(ThreadContext* thread, AppState* state)

{
    // ===============================================================================================
    // #START
    // ===============================================================================================

    GameState* game      = state->game;
    Keyboard& kb         = state->input.keyboard;
    BackBuffer* back_buf = &state->back_buffer;
    const f32 dt         = state->dt;
    Camera* cam          = &game->camera;
    auto p1              = get_entity(game, 1);

    // NOTE: only doing one player
    Entity* player = p1;
    Assert(player->sim.type == EntityType::player);
    EntityActions player_action = {};
    process_keyboard(kb, &player_action);
    process_ds5(state->input.ds5_state[0], &player_action);
    game->player_acts[1] = player_action;

    if (key_pressed(kb.d1)) game->debug_draw_collision = !game->debug_draw_collision;

    // sword attack
    if (game->player_acts[1].attack && p1->sim.weapon_i &&
        is_flag_set(game->entities[p1->sim.weapon_i].sim.flags, sim_entity_flags::nonspatial)) {
        Entity* sword_ent = game->entities + p1->sim.weapon_i;
        clear_flags(sword_ent->sim.flags, sim_entity_flags::nonspatial);
        sword_ent->world_pos      = p1->world_pos;
        constexpr f32 sword_vel   = 5.f;
        sword_ent->sim.dist_limit = 5.f;
        switch (p1->sim.dir) {
            case EntityDirection::north: {
                sword_ent->sim.vel.y      = sword_vel;
                sword_ent->sim.cur_sprite = 0;
            } break;
            case EntityDirection::east: {
                sword_ent->sim.vel.x      = sword_vel;
                sword_ent->sim.cur_sprite = 1;
            } break;
            case EntityDirection::south: {
                sword_ent->sim.vel.y      = -sword_vel;
                sword_ent->sim.cur_sprite = 2;
            } break;
            case EntityDirection::west: {
                sword_ent->sim.vel.x      = -sword_vel;
                sword_ent->sim.cur_sprite = 3;
            } break;
        }
    }

    Entity* cam_ent = get_entity(game, game->entity_camera_follow_ind);
    v3 entity_dif   = get_world_diff(cam_ent->world_pos, cam->pos);

    // NOTE: cam is following the player
    cam->pos         = p1->world_pos;
    cam->pos.chunk_z = cam_ent->world_pos.chunk_z;
    r3f sim_bounds   = rect_init_center_dim(
          v3f { 0.0f, 0.0f, 0.0f },
          v3f { (f32)chunk_size_meters * 2.0f, (f32)chunk_size_meters * 2.0f, 3.0f });

    Arena sim_arena;
    init_arena(&sim_arena, state->memory.transient_storage_size, state->memory.transient_storage);

    sim_region* region = begin_sim(&sim_arena, game, cam->pos, sim_bounds, dt);

    v2f screen_center { .5f * (f32)back_buf->width, .5f * (f32)back_buf->height };
    EntityVisiblePieceGroup piece_group = {};
    argb_img* sprite                    = nullptr;

    // NOTE: *not* using PatBlt in the win32 layer
    Color_argb clear_color = { 0xff1e2e1e };
    clear_buffer(back_buf, clear_color);

    for (SimEntity* sim_ent = region->sim_entities;
         sim_ent != region->sim_entities + region->sim_entity_cnt; ++sim_ent) {
        if (sim_ent->type == EntityType::null) continue;

        // TODO: active and nonspatial redundant? -NO spatial ents get updated
        if (!is_flag_set(sim_ent->flags, sim_entity_flags::active))
            continue;  // don't update inactive entities

        sim_ent->hit_cd += dt;

        EntityMoveSpec move_spec = default_move_spec();
        EntityActions ent_act {};

        // update logic
        if (sim_ent->updateable) {
            switch (sim_ent->type) {
                case EntityType::none: {
                } break;
                case EntityType::player: {
                    f32 ent_speed = game->player_acts[1].sprint ? move_spec.speed = 25.f
                                                                : move_spec.speed = 10.f;
                    ent_act       = game->player_acts[1];

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
                    SimEntity* closest_player  = nullptr;
                    f32 closest_player_dist_sq = square(10.f);

                    for (SimEntity* test_ent = region->sim_entities;
                         test_ent != region->sim_entities + region->sim_entity_cnt; ++test_ent) {
                        if (test_ent->type == EntityType::player) {
                            f32 test_dist_sq = vec_length_sq(test_ent->pos - sim_ent->pos);
                            if (closest_player_dist_sq > test_dist_sq) {
                                closest_player         = test_ent;
                                closest_player_dist_sq = test_dist_sq;
                            }
                        }
                    }

                    if (closest_player) {
                        f32 one_over_len = 1.f / sqrt_f32(closest_player_dist_sq);
                        f32 min_dist     = 2.f;
                        v3 dif           = closest_player->pos - sim_ent->pos;
                        if (abs_f32(dif.x) > min_dist || abs_f32(dif.y) > min_dist)
                            ent_act.dir = one_over_len * (dif);
                    }
                } break;
                case EntityType::monster: {
                    // TODO: update monster
                } break;
                case EntityType::sword: {
                    move_spec.drag = 0.f;
                    if (!is_flag_set(sim_ent->flags, sim_entity_flags::nonspatial) &&
                        sim_ent->dist_limit == 0.f) {
                        set_flags(sim_ent->flags, sim_entity_flags::nonspatial);
                    }
                } break;
                case EntityType::stair: {
                    // TODO: stair logic?
                } break;
                default: {
                    InvalidCodePath;
                } break;
            }
        }

        v3 ent_delta = calc_entity_delta(sim_ent, ent_act, move_spec, dt);

        if (abs_f32(sim_ent->vel.x) > 0.f + epsilon || abs_f32(sim_ent->vel.y) > 0.f + epsilon)
            move_entity(game, region, sim_ent, ent_delta, dt);

        // NOTE: changes the players direction for the sprite
        v3 pv = sim_ent->vel;

        constexpr f32 dir_eps = 0.1f;

        if (abs_f32(pv.x) > abs_f32(pv.y)) {
            if (pv.x > 0.f + dir_eps) {
                sim_ent->dir = EntityDirection::east;
            } else if (pv.x < 0.f - dir_eps) {
                sim_ent->dir = EntityDirection::west;
            }

        } else if (abs_f32(pv.y) > abs_f32(pv.x)) {
            if (pv.y > 0.f + dir_eps) {
                sim_ent->dir = EntityDirection::north;
            } else if (pv.y < 0.f - dir_eps) {
                sim_ent->dir = EntityDirection::south;
            }
        }

        sim_ent->hit_cd += dt;

        // ===============================================================================================
        // #PUSH TO RENDER
        // ===============================================================================================
        piece_group.piece_cnt = 0;
        sprite                = nullptr;
        // the regions origin should be the center of the camera so this should line up
        v3f sim_space_pos = get_sim_space_pos(*game, *region, sim_ent->ent_i);
        v2f ent_screen_pos { (screen_center.x + (sim_space_pos.x * meters_to_pixels)),
                             (screen_center.y - (sim_space_pos.y * meters_to_pixels)) };

        // TODO: pull this out in render code?
        auto push_hp = [](EntityVisiblePieceGroup* piece_group, SimEntity* ent, v2f argb_mid) {
            for (u32 i = 0; i < (u32)ent->hp; ++i) {
                push_piece(piece_group, 3.0f, 6.0f, color(red),
                           v2f { argb_mid.x - (ent->dim.x / 2.f) * meters_to_pixels - 10.0f +
                                     (f32)i * 4.0f,
                                 argb_mid.y - ent->dim.y * meters_to_pixels - 10.0f },
                           ent->z);
            }
        };

        r3f cam_bounds;
        cam_bounds.min = { 0.f, 0.0f, 0.0f };
        cam_bounds.max = { (f32)back_buf->width, (f32)back_buf->height, 0.0f };

        constexpr f32 cam_bound_delta = 50.f;
        cam_bounds                    = rect_add_radius(cam_bounds, cam_bound_delta);

        // draw only inside window
        if (rect_is_inside(cam_bounds, v3_init(ent_screen_pos, 0.f))) {
            switch (sim_ent->type) {
                case EntityType::none: {
                    draw_rect(
                        back_buf, ent_screen_pos.x - (sim_ent->dim.x * meters_to_pixels) / 2.f,
                        ent_screen_pos.y - (sim_ent->dim.y * meters_to_pixels) / 2.f,
                        ent_screen_pos.x + (sim_ent->dim.x * meters_to_pixels) / 2.f,
                        ent_screen_pos.y + (sim_ent->dim.x * meters_to_pixels) / 2.f, pink_v3);
                } break;
                case EntityType::player: {
                    // TODO: get player index from Entity?
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    // TODO: this can probably be extracted out for general use
                    sim_ent->cur_sprite = sim_ent->dir;
                    sprite              = game->player_sprites + sim_ent->cur_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                    push_hp(&piece_group, sim_ent, argb_mid);
                } break;
                case EntityType::wall: {
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite       = &game->wall_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case EntityType::tree: {
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite       = &game->tree_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case EntityType::bush: {
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite       = &game->bush_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case EntityType::stairs: {
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite       = &game->stair_sprite;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case EntityType::familiar: {
                    if (sim_ent->dir == EntityDirection::east)
                        sim_ent->cur_sprite = 0;
                    else if (sim_ent->dir == EntityDirection::west)
                        sim_ent->cur_sprite = 1;
                    sprite       = game->cat_sprites + sim_ent->cur_sprite;
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case EntityType::monster: {
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    sprite       = game->monster_sprites;
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                    push_hp(&piece_group, sim_ent, argb_mid);
                } break;
                case EntityType::sword: {
                    sim_ent->cur_sprite = sim_ent->dir;
                    sprite              = game->sword_sprites + sim_ent->cur_sprite;
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                case EntityType::stair: {
                    sprite       = &game->stair_sprite;
                    v2f argb_mid = { ent_screen_pos.x, ent_screen_pos.y - sim_ent->argb_offset };
                    push_piece(&piece_group, sprite, argb_mid, sim_ent->z);
                } break;
                default: {
                    InvalidCodePath;
                } break;
            }
        }

        // ===============================================================================================
        // #DRAW
        // ===============================================================================================

        for (u32 piece_i = 0; piece_i < piece_group.piece_cnt; ++piece_i) {
            EntityVisiblePiece* piece = &piece_group.pieces[piece_i];
            if (piece->img) {
                draw_argb(back_buf, *piece->img, piece->mid_p);
            } else {
                draw_rect(back_buf, piece->rect, color(pink));
            }
        }

        // NOTE:collision box
        if (game->debug_draw_collision) {
            draw_rect_outline(
                back_buf, ent_screen_pos.x - (sim_ent->dim.x * meters_to_pixels) / 2.f,
                ent_screen_pos.y - (sim_ent->dim.y * meters_to_pixels) / 2.f,
                ent_screen_pos.x + (sim_ent->dim.x * meters_to_pixels) / 2.f,
                ent_screen_pos.y + (sim_ent->dim.y * meters_to_pixels) / 2.f, 1, color(red));
        }
    }

    end_sim(game, region);
}
}  // namespace tom