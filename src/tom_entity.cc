#include "tom_entity.hh"
#include "tom_game.hh"

namespace tom
{

internal void int_hit_points(Entity *ent, u32 hp)
{
    ent->max_hp = hp;
    ent->hp     = ent->max_hp;
}

Entity *get_entity(GameState *state, u32 ind)
{
    Entity *result = nullptr;

    if (ind > 0 && ind < state->ent_cnt) {
        result = state->entities + ind;
    }

    return result;
}

Entity *add_entity(GameState *game, EntityType type, v3f pos)
{
    TOM_ASSERT(game->ent_cnt < g_max_ent_cnt);
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
            ent->dims.y = 1.3f * ent->dims.x;
        } break;
        case EntityType::wall: {
        } break;
        case EntityType::tree: {
            set_flags(ent->flags, EntityFlags::active);
            set_flags(ent->flags, EntityFlags::barrier);
            ent->dims.y = 2.0f;
            ent->dims.x = 0.8f * ent->dims.y;
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
            INVALID_CODE_PATH;
        } break;
    }

    return ent;
}

}  // namespace tom
