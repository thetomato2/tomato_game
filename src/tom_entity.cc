namespace tom
{

function Entity* get_entity(GameState* state, u32 ind)
{
    Entity* result = nullptr;

    if (ind > 0 && ind < state->ent_cnt) {
        result = state->entities + ind;
    }

    return result;
}

function void init_hit_points(SimEntity* ent, u32 hp)
{
    ent->max_hp = hp;
    ent->hp     = ent->max_hp;
}

function Entity* add_new_entity(GameState* state, f32 abs_x = 0.0f, f32 abs_y = 0.0f,
                                f32 abs_z = 0.0f)
{
    Entity* ent = nullptr;

    if (state->ent_cnt < max_ent_cnt) {
        u32 ent_i      = state->ent_cnt++;
        ent            = state->entities + ent_i;
        *ent           = {};
        ent->sim.ent_i = ent_i;
        ent->sim.type  = EntityType::null;
        ent->sim.flags = sim_entity_flags::active;
        ent->world_pos = null_world_pos();

        // NOTE: this inserts the Entity into the corresponding  World chunk
        // REVIEW: make add_entity_to_world()?
        WorldPos start_pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);
        change_entity_location(&state->world_arena, state->world, ent, start_pos);
    } else {
        InvalidCodePath;
    }

    return ent;
}

function Entity* add_wall(GameState* state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    Entity* ent = add_new_entity(state, abs_x, abs_y, abs_z);
    Assert(ent);

    ent->sim.type  = EntityType::wall;
    ent->sim.dim.y = 1.f;
    ent->sim.dim.x = 1.f;
    ent->sim.dim.z = 1.f;
    set_flags(ent, sim_entity_flags::barrier);

    return ent;
}

function Entity* add_tree(GameState* state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    Entity* ent = add_new_entity(state, abs_x, abs_y, abs_z);
    Assert(ent);

    ent->sim.type  = EntityType::tree;
    ent->sim.dim.y = 1.f;
    ent->sim.dim.x = 0.5f;
    ent->sim.dim.z = 1.f;
    set_flags(ent, sim_entity_flags::barrier);

    return ent;
}

function Entity* add_bush(GameState* state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    Entity* ent = add_new_entity(state, abs_x, abs_y, abs_z);
    Assert(ent);

    ent->sim.type  = EntityType::bush;
    ent->sim.dim.y = 0.9f;
    ent->sim.dim.x = 0.9f;
    ent->sim.dim.z = 1.f;
    set_flags(ent, sim_entity_flags::barrier);

    return ent;
}

function Entity* add_stair(GameState* state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    Entity* ent = add_new_entity(state, abs_x, abs_y, abs_z);
    Assert(ent);

    ent->sim.type        = EntityType::stair;
    ent->sim.dim.y       = 0.5f;
    ent->sim.dim.x       = 0.8f;
    ent->sim.dim.z       = 1.f;
    ent->sim.argb_offset = 12.f;
    set_flags(ent, sim_entity_flags::collides);

    return ent;
}

function Entity* add_monster(GameState* state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    Entity* ent = add_new_entity(state, abs_x, abs_y, abs_z);
    Assert(ent);

    ent->sim.type        = EntityType::monster;
    ent->sim.dim.y       = .6f;
    ent->sim.dim.x       = .6f * .6f;
    ent->sim.dim.z       = .6f * .6f;
    ent->sim.argb_offset = 16.f;
    set_flags(ent, sim_entity_flags::collides);

    init_hit_points(&ent->sim, 6);

    return ent;
}

function Entity* add_cat(GameState* state, f32 abs_x, f32 abs_y, f32 abs_z)
{
    Entity* ent = add_new_entity(state, abs_x, abs_y, abs_z);
    Assert(ent);
    ent->world_pos = abs_pos_to_world_pos(abs_x, abs_y, abs_z);

    ent->sim.type        = EntityType::familiar;
    ent->sim.dim.y       = 0.6f;
    ent->sim.dim.x       = 0.8f;
    ent->sim.dim.z       = 0.2f;
    ent->sim.argb_offset = 5.0f;
    set_flags(ent, sim_entity_flags::collides);

    return ent;
}

function Entity* add_sword(GameState* state, u32 parent_i)
{
    Entity* ent = add_new_entity(state);
    Assert(ent);

    ent->sim.type        = EntityType::sword;
    ent->sim.pos         = {};
    ent->sim.dim.y       = 0.2f;
    ent->sim.dim.x       = 0.2f;
    ent->sim.dim.z       = 0.2f;
    ent->sim.argb_offset = 5.0f;
    ent->sim.parent_i    = parent_i;
    set_flags(ent, sim_entity_flags::hurtbox | sim_entity_flags::nonspatial);

    // REVIEW: what happens if I add a second weapon?
    Entity* parent_ent       = state->entities + parent_i;
    parent_ent->sim.weapon_i = ent->sim.ent_i;

    return ent;
}

function void add_player(GameState* state, u32 player_i, f32 abs_x, f32 abs_y, f32 abs_z)
{
    // NOTE: the first 5 entities are reserved for players
    if (player_i <= state->player_cnt) {
        Entity* player = add_new_entity(state);
        if (player_i == player->sim.ent_i) {
            player->sim.type        = EntityType::player;
            player->sim.dim.y       = 0.6f;
            player->sim.dim.x       = 0.6f * player->sim.dim.y;
            player->sim.dim.z       = 0.6f * player->sim.dim.y;
            player->sim.argb_offset = 16.0f;
            set_flags(player, sim_entity_flags::collides);

            init_hit_points(&player->sim, 10);

            Entity* sword        = add_sword(state, player->sim.ent_i);
            player->sim.weapon_i = sword->sim.ent_i;

        } else {
            InvalidCodePath;
        }
    } else {
        InvalidCodePath;
    }
}

}  // namespace tom