namespace tom
{

fn Entity *get_entity(GameState *state, u32 ind)
{
    Entity *result = nullptr;

    if (ind > 0 && ind < state->ent_cnt) {
        result = state->entities + ind;
    }

    return result;
}

fn void init_hit_points(Entity *ent, u32 hp)
{
    ent->max_hp = hp;
    ent->hp     = ent->max_hp;
}

fn void add_entity(GameState *game, EntityType type, v3f pos = {})
{
    Assert(game->ent_cnt < g_max_ent_cnt);
    Entity *ent = &game->entities[game->ent_cnt];
    ent->ind    = game->ent_cnt++;
    ent->type   = type;
    ent->pos    = pos;

    switch (ent->type) {
        case EntityType::none: {
        } break;
        case EntityType::player: {
            set_flags(ent->flags, EntityFlags::active);
            set_flags(ent->flags, EntityFlags::updateable);
            ent->dims.x = 2.0f;
            ent->dims.y = 1.6f * ent->dims.x;
        } break;
        case EntityType::wall: {
        } break;
        case EntityType::tree: {
            set_flags(ent->flags, EntityFlags::active);
            set_flags(ent->flags, EntityFlags::barrier);
            ent->dims.x = 0.8f;
            ent->dims.y = 2.0f;
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
        default: {
            InvalidCodePath;
        } break;
    }
}

}  // namespace tom