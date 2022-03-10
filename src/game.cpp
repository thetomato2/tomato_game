#include "Game.hpp"
#include "rng_nums.h"

namespace tom
{

internal void
clearBuffer(GameOffscreenBuffer &buffer, const Color color = colors::pink)
{
    const s32 width  = buffer.width;
    const s32 height = buffer.height;

    byt *row = scast(byt *, buffer.memory);
    for (s32 y = 0; y < height; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = 0; x < width; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

internal void
drawRect(GameOffscreenBuffer &buffer, const f32 minX_f32, const f32 minY_f32, const f32 maxX_f32,
         const f32 maxY_f32, const Color color = colors::pink)
{
    s32 minX = math::round_f32_s32(minX_f32);
    s32 minY = math::round_f32_s32(minY_f32);
    s32 maxX = math::round_f32_s32(maxX_f32);
    s32 maxY = math::round_f32_s32(maxY_f32);

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX > buffer.width) maxX = buffer.width;
    if (maxY > buffer.height) maxY = buffer.height;

    byt *row = scast(byt *, buffer.memory) + minX * buffer.bytesPerPixel + minY * buffer.pitch;

    for (s32 y = minY; y < maxY; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = minX; x < maxX; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

internal void
drawRect(GameOffscreenBuffer &buffer, const Rect rect, const Color color = colors::pink)
{
    s32 minX = math::round_f32_s32(rect.min.x);
    s32 minY = math::round_f32_s32(rect.min.y);
    s32 maxX = math::round_f32_s32(rect.max.x);
    s32 maxY = math::round_f32_s32(rect.max.y);

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX > buffer.width) maxX = buffer.width;
    if (maxY > buffer.height) maxY = buffer.height;

    byt *row = scast(byt *, buffer.memory) + minX * buffer.bytesPerPixel + minY * buffer.pitch;

    for (s32 y = minY; y < maxY; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = minX; x < maxX; ++x) {
            *pixel++ = color.argb;
        }
        row += buffer.pitch;
    }
}

internal void
drawRectOutline(GameOffscreenBuffer &buffer, const f32 minX_f32, const f32 minY_f32,
                const f32 maxX_f32, const f32 maxY_f32, const s32 thickness,
                const Color color = colors::pink)
{
    s32 minX = math::round_f32_s32(minX_f32);
    s32 minY = math::round_f32_s32(minY_f32);
    s32 maxX = math::round_f32_s32(maxX_f32);
    s32 maxY = math::round_f32_s32(maxY_f32);

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX > buffer.width) maxX = buffer.width;
    if (maxY > buffer.height) maxY = buffer.height;

    byt *row = scast(byt *, buffer.memory) + minX * buffer.bytesPerPixel + minY * buffer.pitch;

    for (s32 y = minY; y < maxY; ++y) {
        u32 *pixel = rcast(u32 *, row);
        for (s32 x = minX; x < maxX; ++x) {
            if (x <= minX + thickness || x >= maxX - thickness - 1 || y <= minY + thickness ||
                y >= maxY - thickness - 1) {
                *pixel = color.argb;
            }
            ++pixel;
        }
        row += buffer.pitch;
    }
}

internal void
drawArgb(GameOffscreenBuffer &buffer, const ArgbImg &img, const v2 pos)
{
    s32 minY = math::round_f32_s32(pos.y - (scast(f32, img.height) / 2.f));
    s32 minX = math::round_f32_s32(pos.x - (scast(f32, img.width) / 2.f));
    s32 maxY = math::round_f32_s32(pos.y + (scast(f32, img.height) / 2.f));
    s32 maxX = math::round_f32_s32(pos.x + (scast(f32, img.width) / 2.f));

    s32 xOffsetLeft = 0, xOffsetRight = 0, yOffset = 0;

    if (minY < 0) {
        yOffset = minY * -1;
        minY    = 0;
    }
    if (minX < 0) {
        xOffsetLeft = minX * -1;
        minX        = 0;
    }
    if (maxX > buffer.width) {
        xOffsetRight = maxX - buffer.width;
        maxX         = buffer.width;
    }
    if (maxY > buffer.height) maxY = buffer.height;

    u32 *source = img.pixelPtr + (yOffset * img.width);
    byt *row    = scast(byt *, buffer.memory) + minX * buffer.bytesPerPixel + minY * buffer.pitch;

    for (s32 y = minY; y < maxY; ++y) {
        u32 *dest = rcast(u32 *, row);
        source += xOffsetLeft;
        for (s32 x = minX; x < maxX; ++x) {
            Color destCol   = { *dest };
            Color sourceCol = { *source };
            Color blendedCol;
            blendedCol.a = 0xff;

            f32 alpha = scast(f32, sourceCol.a) / 255.f;

            blendedCol.r =
                scast(u8, (1.f - alpha) * scast(f32, destCol.r) + alpha * scast(f32, sourceCol.r));
            blendedCol.g =
                scast(u8, (1.f - alpha) * scast(f32, destCol.g) + alpha * scast(f32, sourceCol.g));
            blendedCol.b =
                scast(u8, (1.f - alpha) * scast(f32, destCol.b) + alpha * scast(f32, sourceCol.b));

            *dest = blendedCol.argb;

            ++dest, ++source;
        }
        source += xOffsetRight;
        row += buffer.pitch;
    }
}

internal void
pushPiece(EntityVisblePieceGroup *group, ArgbImg *img, const v2 midP, const f32 zOffset,
          const f32 alpha = 1.0f)
{
    TOM_ASSERT(group->pieceCnt < ARRAY_COUNT(group->pieces));
    EntityVisiblePiece *piece = group->pieces + group->pieceCnt++;
    piece->img                = img;
    piece->midP               = midP;
    piece->z                  = zOffset;
    piece->alpha              = alpha;
}

internal void
pushPiece(EntityVisblePieceGroup *group, const f32 width, const f32 height, const Color color,
          const v2 midP, const f32 zOffset, const f32 alpha = 1.0f)
{
    TOM_ASSERT(group->pieceCnt < ARRAY_COUNT(group->pieces));
    EntityVisiblePiece *piece = group->pieces + group->pieceCnt++;
    piece->img                = nullptr;
    piece->midP               = midP;
    piece->z                  = zOffset;
    piece->alpha              = alpha;
    piece->rect.min.x         = midP.x - width / 2;
    piece->rect.min.y         = midP.y - height / 2;
    piece->rect.max.x         = midP.x + width / 2;
    piece->rect.max.y         = midP.y + height / 2;
    piece->color              = color;
}

internal void
gameOutputSound(GameSoundOutputBuffer &soundBuffer)
{
    // NOTE: outputs nothing atm
    s16 sampleValue = 0;
    s16 *sampleOut  = soundBuffer.samples;
    for (szt sampleIndex = 0; sampleIndex < soundBuffer.sampleCount; ++sampleIndex) {
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
    }
}

internal void
initArena(MemoryArena *arena, const mem_ind size, byt *base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

internal BitmapImg
loadBmp(ThreadContext *thread, debug_platformReadEntireFile *readEntireFile, const char *fileName)
{
    debug_ReadFileResult readResult = readEntireFile(thread, fileName);
    BitmapImg result;

    if (readResult.contentSz != 0) {
        auto *header    = scast(BitmapHeader *, readResult.contents);
        u32 *pixels     = rcast(u32 *, (scast(byt *, readResult.contents) + header->bitmapOffset));
        result.width    = header->width;
        result.height   = header->height;
        result.pixelPtr = pixels;
    }

    return result;
}

internal ArgbImg
loadArgb(ThreadContext *thread, debug_platformReadEntireFile *readEntireFile, const char *fileName,
         const char *name = nullptr)
{
    const char *argbDir = "T:/assets/argbs/";
    char imgPathBuf[512];
    szt imgBufLen;
    catStr(argbDir, fileName, &imgPathBuf[0], &imgBufLen);
    imgPathBuf[imgBufLen++] = '.';
    imgPathBuf[imgBufLen++] = 'a';
    imgPathBuf[imgBufLen++] = 'r';
    imgPathBuf[imgBufLen++] = 'g';
    imgPathBuf[imgBufLen++] = 'b';
    imgPathBuf[imgBufLen++] = '\0';

    debug_ReadFileResult readResult = readEntireFile(thread, imgPathBuf);
    ArgbImg result;

    TOM_ASSERT(readResult.contentSz != 0);
    if (readResult.contentSz != 0) {
        if (name)
            result.name = name;
        else
            result.name = fileName;

        u32 *filePtr    = scast(u32 *, readResult.contents);
        result.width    = *filePtr++;
        result.height   = *filePtr++;
        result.size     = *filePtr++;
        result.pixelPtr = filePtr;
    }

    return result;
}

internal void
processKeyboard(const GameKeyboardInput &keyboard, EntityActions *entityAction)
{
    if (entityAction) {
        if (isKeyUp(keyboard.t)) entityAction->start = true;
        if (keyboard.space.endedDown) entityAction->jump = true;
        if (keyboard.w.endedDown) entityAction->dir.y += 1.f;
        if (keyboard.s.endedDown) entityAction->dir.y += -1.f;
        if (keyboard.a.endedDown) entityAction->dir.x += -1.f;
        if (keyboard.d.endedDown) entityAction->dir.x += 1.f;
        if (keyboard.leftShift.endedDown) entityAction->sprint = true;
    }
}

internal void
processController(Const GameControllerInput &controller, EntityActions *entityAction)
{
    if (entityAction) {
        if (isButtonUp(Controller.ButtonStart)) entityAction->Start = true;
        if (isButtonUp(Controller.ButtonA)) entityAction->Attack = true;
        if (controller.buttonB.EndedDown) entityAction->Sprint = true;

        constexpr f32 stickDeadzone = 0.1f;
        if (controller.isAnalog && (math::absF32(Controller.EndLeftStickX) > stickDeadzone ||
                                    math::absF32(Controller.EndLeftStickY) > stickDeadzone)) {
            entityAction->Dir = { controller.endLeftStickX, controller.endLeftStickY };
        }
    }
}

#if 0
internal void
addHitPoints(StoredEntity &entity, s32 hp)
{
    entity.hitPoints += hp;
    if (entity.hitPoints > entity.maxHitPoints) entity.hitPoints = entity.maxHitPoints;
}

internal void
subtractHitPoints(StoredEntity entity, s32 hp)
{
    entity.hitPoints -= hp;
    if (entity.hitPoints < 0) {
        entity.hitPoints = 0;
        entity.active     = false;
    }
}

internal void
initHitPoints(StoredEntity *entLow, const u32 hp)
{
    entLow->MaxHitPoints = hp;
    entLow->HitPoints     = entLow->MaxHitPoints;
}

#endif

internal EntityRef
addWall(GameState &state, const f32 absX, const f32 absY, const f32 absZ)
{
    auto wall    = addEntity(State, EntityType::Wall);
    WorldPos pos = abs_pos_to_world_pos(absX, absY, absZ);

    wall.low->pos      = pos;
    wall.low->height   = 1.f;
    wall.low->width    = 1.f;
    wall.low->color    = { 0xff'dd'dd'dd };
    wall.low->collides = true;
    wall.low->barrier  = true;
    wall.low->sprite   = &state.treeSprite;

    return wall;
}

internal EntityRef
addStairs(GameState &state, const f32 absX, const f32 absY, const f32 absZ)
{
    auto stairs  = addLowEntity(State, EntityType::Stairs);
    WorldPos pos = abs_pos_to_world_pos(absX, absY, absZ);

    stairs.low->height     = 1.f;
    stairs.low->width      = 1.f;
    stairs.low->pos        = pos;
    stairs.low->color      = { 0xff'1e'1e'1e };
    stairs.low->argbOffset = 16.f;
    stairs.low->collides   = true;
    stairs.low->barrier    = false;
    stairs.low->sprite     = &state.stairSprite;

    return stairs;
}

internal EntityRef
addMonster(GameState &state, const f32 absX, const f32 absY, const f32 absZ)
{
    auto monster = addLowEntity(State, EntityType::Monster);
    WorldPos pos = abs_pos_to_world_pos(absX, absY, absZ);

    monster.low->pos        = pos;
    monster.low->height     = .6f;
    monster.low->width      = .6f * .6f;
    monster.low->color      = { 0xff'dd'dd'dd };
    monster.low->argbOffset = 16.f;
    monster.low->collides   = true;
    monster.low->barrier    = true;
    monster.low->sprite     = &state.monsterSprites[0];

    initHitPoints(Monster.Low, 6);

    return monster;
}

internal EntityRef
addCat(GameState &state, const f32 absX, const f32 absY, const f32 absZ)
{
    auto cat     = addLowEntity(State, EntityType::Familiar);
    WorldPos pos = abs_pos_to_world_pos(absX, absY, absZ);

    cat.low->pos        = pos;
    cat.low->height     = .6f;
    cat.low->width      = .8f;
    cat.low->color      = { 0xff'dd'dd'dd };
    cat.low->argbOffset = 5.f;
    cat.low->collides   = true;
    cat.low->barrier    = true;
    cat.low->sprite     = &state.catSprites[0];

    return cat;
}

internal EntityRef
addSword(GameState &state, ARGBImg *sprite = nullptr)
{
    auto sword = addLowEntity(State, EntityType::Sword, {}, false);

    sword.low->pos        = {};
    sword.low->height     = .6f;
    sword.low->width      = .8f;
    sword.low->color      = { 0xff'dd'dd'dd };
    sword.low->argbOffset = 5.f;
    sword.low->collides   = false;
    sword.low->barrier    = false;
    sword.low->hurtbox    = true;

    return sword;
}

internal void
addPlayer(GameState &state, const u32 playerI, const f32 x, const f32 y, const f32 z,
          ARGBImg *sprite)
{
    // NOTE: the first 5 entities are reserved for players
    TOM_ASSERT(playerI <= state.playerCnt);
    if (playerI <= state.playerCnt) {
        auto player = addLowEntity(State, EntityType::Player);
        TOM_ASSERT(playerI == player.lowI);
        if (playerI == player.lowI) {
            auto pos               = absPosToWorldPos(X, y, z);
            player.low->height     = .6f;
            player.low->width      = 0.6f * player.low->height;
            player.low->pos        = pos;
            player.low->color      = { 0xff'00'00'ff };
            player.low->argbOffset = 16.f;
            player.low->collides   = true;
            player.low->barrier    = true;
            player.low->sprite     = sprite;

            initHitPoints(Player.Low, 10);
            forceEntityIntoHigh(State, playerI);

            auto sword          = addSword(State);
            player.low->weaponI = sword.lowI;
            sword.low->parentI  = player.lowI;
        }
    }
}

struct EntityMoveSpec
{
    f32 speed;
    f32 drag;
};

internal EntityMoveSpec
getDefaultMoveSpec()
{
    return { 10.0f, 10.0f };
}

internal void
moveEntity(GameState &state, Entity entity, const EntityActions &entityActions,
           EntityMoveSpec moveSpec, const f32 dt)
{
    v2 entityAcc = entityActions.Dir;

    // NOTE: normalize vector to unit length
    f32 entAccLength = vec::length_sq(entityAcc);
    // TODO: make speed spefific to entity type

    if (entAccLength > 1.f) entityAcc *= (1.f / sqrt_f32(entAccLength));
    entityAcc *= moveSpec.Speed;
    entityAcc -= entity.high->vel * moveSpec.Drag;

    v2 playerDelta  = (.5f * entityAcc * square(dt) + entity.high->vel * dt);
    v2 newPlayerPos = entity.high->pos + playerDelta;

    entity.high->vel += entityAcc * dt;

    // NOTE: how many iterations/time resolution
    for (u32 i = 0; i < 4; ++i) {
        f32 tMin           = 1.0f;
        u32 hitEntInd      = 0;  // 0 is the null entity
        v2 wallNrm         = {};
        v2 desiredPosition = entity.high->pos + playerDelta;

        // FIXME: this is N * N bad
        for (u32 testHighI = 1; testHighI < state.highCnt; ++testHighI) {
            if (testHighI == entity.low->highI) continue;  // don't test against self

            Entity testEnt = getEntityFromHighI(State, testHighI);

            if (!testEnt.low->active || !testEnt.low->collides)
                continue;  // skip inactive and non-collision entities
            if (entity.low->weaponI == testEnt.low_i) continue;  // skip entity's weapon
            if (entity.low->parentI == testEnt.low_i) continue;  // skip parent entity

            // NOTE: Minkowski sum
            f32 radiusW = entity.low->width + testEnt.low->width;
            f32 radiusH = entity.low->height + testEnt.low->height;

            v2 minCorner = { -.5f * v2 { radiusW, radiusH } };
            v2 maxCorner = { .5f * v2 { radiusW, radiusH } };
            v2 rel       = entity.high->pos - testEnt.high->pos;

            // TODO: maybe pull this out into a free function (but why?)
            auto testWall = [&tMin](f32 wallX, f32 relX, f32 relY, f32 playerDelta_x,
                                    f32 playerDelta_y, f32 minY, f32 maxY) -> bool {
                bool hit = false;

                f32 tEsp = .001f;
                if (playerDelta_x != 0.f) {
                    f32 tRes = (wallX - relX) / playerDelta_x;
                    f32 y    = relY + tRes * playerDelta_y;

                    if (tRes >= 0.f && (tMin > tRes)) {
                        if (y >= minY && y <= maxY) {
                            tMin = max(0.f, tRes - tEsp);
                            hit  = true;
                        }
                    }
                }

                return hit;
            };

            if (testWall(minCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y,
                         maxCorner.y)) {
                wallNrm   = v2 { -1.f, 0.f };
                hitEntInd = testHighI;
            }
            if (testWall(maxCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y,
                         maxCorner.y)) {
                wallNrm   = v2 { 1.f, 0.f };
                hitEntInd = testHighI;
            }
            if (testWall(minCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x,
                         maxCorner.x)) {
                wallNrm   = v2 { 0.f, -1.f };
                hitEntInd = testHighI;
            }
            if (testWall(maxCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x,
                         maxCorner.x)) {
                wallNrm   = v2 { 0.f, 1.f };
                hitEntInd = testHighI;
            }
        }

        EntityHigh *hitHigh = state.highEntities + hitEntInd;

        if (!state.lowEntities[HitHigh->LowI].Barrier) {
            wallNrm = v2 { 0.f, 0.f };
        }

        entity.high->pos += tMin * playerDelta;
        if (hitEntInd) {
            entity.high->vel -= 1.f * vec::inner(entity.high->vel, wallNrm) * wallNrm;
            playerDelta -= 1.f * vec::inner(playerDelta, wallNrm) * wallNrm;

            // printf("%d hit %d!\n", entity.low->highI, hitEntInd);

            // TODO: temp player hitting monster logic
            if (entity.high->hitCd > .5f) {
                switch (entity.low->type) {
                    case EntityType::Player: {
                        if (state.lowEntities[HitHigh->LowI].Type == EntityType::Monster) {
                            entity.high->hitCd = 0.f;
                            subtractHitPoints(Entity, 1);
                        } else if (state.lowEntities[HitHigh->LowI].Type == EntityType::Familiar) {
                            entity.high->hitCd = 0.f;
                            addHitPoints(Entity, 1);
                        }
                    } break;
                    case EntityType::Sword: {
                        if (state.lowEntities[HitHigh->LowI].Type == EntityType::Monster) {
                            Entity hitMonster = getEntityFromLowI(State, hitHigh->LowI);
                            if (hitMonster.high->hit_cd <= 0.0f) {
                                hitMonster.high->hit_cd = 0.5f;
                                subtract_hit_points(hitMonster, 1);
                            } else {
                                hitMonster.high->hit_cd -= dt;
                            }
                        }
                    } break;
                }

                if (entity.high->stairCd > .5f &&
                    state.lowEntities[HitHigh->LowI].Type == EntityType::Stairs) {
                    entity.low->virtualZ == 0  ? ++entity.low->pos.chunkZ,
                        ++entity.low->virtualZ : --entity.low->pos.chunkZ, --entity.low->virtualZ;
                    entity.high->stairCd = 0.f;
                }
            } else {
                break;
            }
        }
    }

    // jump code
    // TODO: implement this
    if (entityActions.Jump && !entity.high->isJumping) {
        entity.high->isJumping = true;
        entity.high->velZ      = global::jumpVel;
    }

    entity.high->stairCd += dt;
    entity.high->hitCd += dt;

    // NOTE: changes the players direction for the sprite
    v2 pv = { entity.high->vel };
    if (absF32(Pv.X) > absF32(Pv.Y)) {
        pv.x > 0.f ? entity.high->dir = EntityDirection::East
                   : entity.high->dir = EntityDirection::West;
    } else if (absF32(Pv.Y) > absF32(Pv.X)) {
        pv.y > 0.f ? entity.high->dir = EntityDirection::North
                   : entity.high->dir = EntityDirection::South;
    }

    // TODO:
    WorldPos newPos = mapIntoChunkSpace(State.Camera.Pos, entity.high->pos);
    changeEntityLocation(&State.WorldArena, *state.world, entity.lowI, &entity.low->pos, &newPos);
    entity.low->pos = newPos;
}

internal void
updateFamiliar(GameState &state, Entity fam, const f32 dt)
{
    Entity closestPlayer      = 0;
    f32 closestPlayer_dist_sq = square(10.f);
    for (u32 highI = 1; highI < state.highCnt; ++highI) {
        Entity testEnt = getEntityFromHighI(State, highI);
        if (testEnt.low->type == EntityType::Player) {
            f32 testDistSq = vec::length_sq(testEnt.high->pos - fam.high->pos);
            if (closestPlayer_dist_sq > testDistSq) {
                closestPlayer         = testEnt;
                closestPlayer_dist_sq = testDistSq;
            }
        }
    }

    if (closestPlayer.high) {
        EntityActions famActs = {};
        f32 oneOverLen        = 1.f / sqrt_f32(closestPlayer_dist_sq);
        f32 minDist           = 2.f;
        v2 dif                = closestPlayer.high->pos - fam.high->pos;
        if (absF32(Dif.X) > minDist || absF32(Dif.Y) > minDist) famActs.dir = oneOverLen * (dif);

        auto moveSpec = getDefaultMoveSpec();
        moveEntity(State, fam, famActs, moveSpec, dt);
        if (fam.high->dir == EntityDirection::East)
            fam.low->sprite = &state.catSprites[0];
        else if (fam.high->dir == EntityDirection::West)
            fam.low->sprite = &state.catSprites[1];
    }
}

internal void
updateSword(GameState &state, Entity sword, const f32 dt)
{
    EntityActions swordActs = {};
    constexpr f32 swordVel  = 5.0f;

    switch (sword.high->dir) {
        case EntityDirection::North: {
            sword.high->vel.y = swordVel;
            sword.low->sprite = &state.sword_sprites[EntityDirection::North];
        } break;
        case EntityDirection::East: {
            sword.high->vel.x = swordVel;
            sword.low->sprite = &state.sword_sprites[EntityDirection::East];
        } break;
        case EntityDirection::South: {
            sword.high->vel.y = -swordVel;
            sword.low->sprite = &state.swordSprites[EntityDirection::South];
        } break;
        case EntityDirection::West: {
            sword.high->vel.x = -swordVel;
            sword.low->sprite = &state.swordSprites[EntityDirection::West];
        } break;
    }
    auto moveSpec = getDefaultMoveSpec();
    moveSpec.drag = 0.0f;
    moveEntity(State, sword, swordActs, moveSpec, dt);
}

internal void
updateMonster(GameState &state, Entity monster, f32 dt)
{
}

internal void
updatePlayer(GameState &state, Entity player, f32 dt)
{
    auto moveSpec = getDefaultMoveSpec();

    f32 entSpeed = state.playerActs[1].Sprint ? moveSpec.speed = 25.f : moveSpec.drag = 10.f;
    moveEntity(State, player, state.playerActs[1], moveSpec, dt);
    switch (player.high->dir) {
        case EntityDirection::North: {
            player.low->sprite = &state.player_sprites[EntityDirection::North];
        } break;
        case EntityDirection::East: {
            player.low->sprite = &state.player_sprites[EntityDirection::East];
        } break;
        case EntityDirection::South: {
            player.low->sprite = &state.playerSprites[EntityDirection::South];
        } break;
        case EntityDirection::West: {
            player.low->sprite = &state.playerSprites[EntityDirection::West];
        } break;
    }
}

// ===============================================================================================
// #EXPORT
// ===============================================================================================

extern "C" TOMDllExport
GAMEGetSoundSamples(GameGetSoundSamples)
{
    auto *state = (GameState *)memory.permanentStorage;
    gameOutputSound(SoundBuffer);
}

extern "C" TOMDllExport
GAMEUpdateAndRender(GameUpdateAndRender)
{
    TOM_ASSERT(sizeof(GameState) <= memory.permanentStorageSize);

    // NOTE: cast to GameState ptr, dereference and cast to GameState reference
    // TODO: just fucking use a pointer?
    auto &state = (GameState &)(*(GameState *)memory.permanentStorage);

    // ===============================================================================================
    // #Initialization
    // ===============================================================================================
    if (!memory.isInitialized) {
        // init memory
        initArena(&State.WorldArena, memory.permanentStorageSize - sizeof(state),
                  (u8 *)memory.permanentStorage + sizeof(state));

        state.world = PushStruct(&state.worldArena, World);

        World *world = state.world;
        initWorld(*World, 1.4f);

        state.debugDrawCollision = false;

        const char *bg = "uvColorSquares960X540";

        // load textures
        state.player_sprites[EntityDirection::North] =
            loadArgb(Thread, memory.platfromReadEntireFile, "playerN");
        state.player_sprites[EntityDirection::East] =
            loadArgb(Thread, memory.platfromReadEntireFile, "playerE");
        state.playerSprites[EntityDirection::South] =
            loadArgb(Thread, memory.platfromReadEntireFile, "playerS");
        state.playerSprites[EntityDirection::West] =
            loadArgb(Thread, memory.platfromReadEntireFile, "playerW");

        state.monster_sprites[EntityDirection::North] =
            loadArgb(Thread, memory.platfromReadEntireFile, "monsterN");
        state.monster_sprites[EntityDirection::East] =
            loadArgb(Thread, memory.platfromReadEntireFile, "monsterE");
        state.monsterSprites[EntityDirection::South] =
            loadArgb(Thread, memory.platfromReadEntireFile, "monsterS");
        state.monsterSprites[EntityDirection::West] =
            loadArgb(Thread, memory.platfromReadEntireFile, "monsterW");

        state.sword_sprites[EntityDirection::North] =
            loadArgb(Thread, memory.platfromReadEntireFile, "swordN");
        state.sword_sprites[EntityDirection::East] =
            loadArgb(Thread, memory.platfromReadEntireFile, "swordE");
        state.swordSprites[EntityDirection::South] =
            loadArgb(Thread, memory.platfromReadEntireFile, "swordS");
        state.swordSprites[EntityDirection::West] =
            loadArgb(Thread, memory.platfromReadEntireFile, "swordW");

        state.catSprites[0] = loadArgb(Thread, memory.platfromReadEntireFile, "catE");
        state.catSprites[1] = loadArgb(Thread, memory.platfromReadEntireFile, "catW");

        state.bgImg        = loadArgb(Thread, memory.platfromReadEntireFile, bg);
        state.crosshairImg = loadArgb(Thread, memory.platfromReadEntireFile, "crosshair");
        state.treeSprite   = loadArgb(Thread, memory.platfromReadEntireFile, "shittyTree");
        state.stairSprite  = loadArgb(Thread, memory.platfromReadEntireFile, "stairs");

        s32 screenBaseX {}, screenBaseY {}, screenBaseZ {}, virtualZ {}, rngInd {};
        s32 screenX { screenBaseX }, screenY { screenBaseY }, screenZ { screenBaseZ };

        // set world render size
        state.camera.pos.chunkX = 0;
        state.camera.pos.chunkY = 0;
        state.camera.pos.offset.x += screenBaseX / 2.f;
        state.camera.pos.offset.y += screenBaseY / 2.f;

        // NOTE: entity 0 is the null entity
        addLowEntity(State, EntityType::Null);
        state.highEntities[0] = {};
        ++state.highCnt;
        // state.playerCnt               = GameInput::SInputCnt;
        state.playerCnt             = 1;
        state.entityCameraFollowInd = 1;

        // add the player entites
        for (u32 playerI = 1; playerI <= state.playerCnt; ++playerI) {
            addPlayer(State, playerI, 0.f, 0.f, 0.f, &state.playerSprites[0]);
        }

        f32 xLen = 55.f;
        for (f32 x = -20.f; x < xLen; ++x) {
            addWall(State, x, 15, 0.0f);
            addWall(State, x, -5, 0.0f);
            if (scast(s32, x) % 17 == 0) continue;
            addWall(State, x, 5, 0.0f);
        }

        for (f32 x = -20.f; x <= xLen; x += 15.f) {
            for (f32 y = -5; y < 15; ++y) {
                if ((y == 0.f || y == 10.f) && (x != -20.f && x != 55.f)) continue;
                addWall(State, x, y, 0.0);
            }
        }

        addMonster(State, 5.f, 0.f, 0.f);
        addCat(State, -1.f, 1.f, 0.f);

        // TODO: this might be more appropriate in the platform layer
        memory.isInitialized = true;
    }

    // ===============================================================================================
    // #START
    // ===============================================================================================

    // REVIEW: if you put GameWorld & it won't convert??????
    auto &world    = state.world;
    Camera &camera = state.camera;

    auto p1 = getEntityFromLowI(State, 1);

    // get input
    // NOTE: only doing one player
    StoredEntity *player = getLowEntity(State, 1);
    assert(player->type == EntityType::Player);
    EntityActions playerAction = {};
    processKeyboard(Input.Keyboard, &playerAction);
    processController(Input.Controllers[0], &playerAction);
    state.playerActs[1] = playerAction;

    // player sword attack
    if (state.playerActs[1].attack) {
        if (!p1.high->isAttacking) {
            p1.high->isAttacking = true;
            Entity sword         = forceEntityIntoHigh(State, p1.low->weaponI);
            sword.high->dir      = p1.high->dir;
            sword.low->active    = true;
            // TODO: player weapon pos offset
            sword.high->pos   = p1.high->pos;
            p1.high->attackCd = 0.5f;
        }
    }
    if (p1.high->isAttacking) {
        p1.high->attackCd -= input.deltaTime;
        Entity sword = forceEntityIntoHigh(State, p1.low->weaponI);
        updateSword(State, sword, input.deltaTime);
        if (p1.high->attackCd <= 0.f) {
            p1.high->isAttacking = false;
            sword.low->active    = false;
        }
    }

    if (isKeyUp(Input.Keyboard.D1)) state.debugDrawCollision = !state.debugDrawCollision;

    Entity camEnt      = getEntityFromLowI(State, state.entityCameraFollowInd);
    WorldDif entityDif = get_world_diff(camEnt.low->pos, camera.pos);

    camera.pos.chunkZ = camEnt.low->pos.chunk_z;

    // NOTE: camera is following the player
    WorldPos newCamPos     = p1.low->pos;
    f32 screenTestSizeMult = 2.f;
    v2 testScreenSize      = { (global::screenSizeX * screenTestSizeMult),
                          global::screenSizeY * screenTestSizeMult };
    Rect camBounds         = rect::centerHalfDim({ 0.f, 0.f }, testScreenSize);

    SimRegion *simRegion = beginSim(State, origin, bounds);

    v2 screenCenter = { .5f * (f32)videoBuffer.Width, .5f * (f32)videoBuffer.Height };
    EntityVisblePieceGroup pieceGroup = {};

    // NOTE: *not* using PatBlt in the win32 layer
    Color clearColor { 0xff'4e'4e'4e };
    clearBuffer(VideoBuffer, clearColor);

    for (u32 entI = 0; entI < simRegion->SimEntityCnt; ++entI) {
        pieceGroup.piece_cnt = 0;
        Entity ent           = getEntityFromHighI(State, entI);
        if (!ent.low->active) continue;  // don't draw inactive entities

        auto entDif = getWorldDiff(Ent.Low->Pos, camera.pos);
        v2 entMid   = { (screenCenter.x + (entDif.dif_xy.x * global::metersToPixels)),
                      (screenCenter.y - (entDif.dif_xy.y * global::metersToPixels)) };

        // TODO: pull this out?
        auto pushHp = [](EntityVisblePieceGroup &pieceGroup, Entity ent, v2 argbMid) {
            for (u32 i {}; i < ent.low->hitPoints; ++i) {
                push_piece(&pieceGroup, 3.f, 6.f, { colors::red },
                           v2 { argbMid.X - (ent.low->width / 2.f) * global::metersToPixels - 10.f +
                                    scast(f32, i) * 4.f,
                                argbMid.Y - ent.low->height * global::metersToPixels - 10.f },
                           ent.high->z);
            }
        };

        switch (ent.low->type) {
            case EntityType::None: {
                drawRect(videoBuffer, entMid.x - (ent.low->width * global::metersToPixels) / 2.f,
                         entMid.y - (ent.low->height * global::metersToPixels) / 2.f,
                         entMid.x + (ent.low->width * global::metersToPixels) / 2.f,
                         entMid.y + (ent.low->height * global::metersToPixels) / 2.f,
                         { 0xffff00ff });
            } break;
            case EntityType::Player: {
                // TODO: get player index from entity?
                updatePlayer(State, ent, input.deltaTime);
                v2 argbMid = { entMid.x, entMid.y - ent.low->argbOffset };
                push_piece(&pieceGroup, ent.low->sprite, argbMid, ent.high->z);
                pushHp(pieceGroup, ent, argbMid);

            } break;
            case EntityType::Wall: {
                v2 argbMid = { entMid.x, entMid.y - ent.low->argbOffset };
                push_piece(&pieceGroup, ent.low->sprite, argbMid, ent.high->z);
            } break;
            case EntityType::Stairs: {
                v2 argbMid = { entMid.x, entMid.y - ent.low->argbOffset };
                push_piece(&pieceGroup, ent.low->sprite, argbMid, ent.high->z);
            } break;
            case EntityType::Familiar: {
                updateFamiliar(State, ent, input.deltaTime);
                v2 argbMid = { entMid.x, entMid.y - ent.low->argbOffset };
                push_piece(&pieceGroup, ent.low->sprite, argbMid, ent.high->z);
            } break;
            case EntityType::Monster: {
                updateMonster(State, ent, input.deltaTime);
                v2 argbMid = { entMid.x, entMid.y - ent.low->argbOffset };
                push_piece(&pieceGroup, ent.low->sprite, argbMid, ent.high->z);
                pushHp(pieceGroup, ent, argbMid);
            } break;
            case EntityType::Sword: {
                updateMonster(State, ent, input.deltaTime);
                v2 argbMid = { entMid.x, entMid.y - ent.low->argbOffset };
                push_piece(&pieceGroup, ent.low->sprite, argbMid, ent.high->z);
            } break;
            default: {
                INVALIDCodePath;
            } break;
        }

        // ===============================================================================================
        // #DRAW
        // ===============================================================================================

        for (u32 pieceI = 0; pieceI < pieceGroup.piece_cnt; ++pieceI) {
            EntityVisiblePiece *piece = &pieceGroup.pieces[pieceI];
            if (piece->img) {
                draw_ARGB(videoBuffer, *piece->img, piece->midP);
            } else {
                drawRect(videoBuffer, piece->rect, { colors::red });
            }
        }

        // NOTE:collision box
        if (state.debugDrawCollision) {
            drawRectOutline(videoBuffer, entMid.x - (ent.low->width * global::metersToPixels) / 2.f,
                            entMid.y - (ent.low->height * global::metersToPixels) / 2.f,
                            entMid.x + (ent.low->width * global::metersToPixels) / 2.f,
                            entMid.y + (ent.low->height * global::metersToPixels) / 2.f, 1,
                            { 0xffff0000 });
        }
    }

#if 0
    // HACK: hacky way to draw a debug postion
    auto testDif = getWorldDiff(State.TestPos, camera.pos);
    v2 testMid   = { (screenCenter.x + (testDif.dif_xy.x * global::gMetersToPixels)),
                    (screenCenter.y - (testDif.dif_xy.y * global::gMetersToPixels)) };
    draw_ARGB(videoBuffer, state.crosshairImg, testMid);
#endif

    endSim(State, *simRegion);
}

}  // namespace tom
