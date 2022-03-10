#ifndef GAME_HPP_
#define GAME_HPP_

#include "Platform.h"
#include "Common.hpp"
#include "Image.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "SimRegion.hpp"

namespace tom
{

struct Camera
{
    WorldPos pos;
};

struct GameState
{
    MemoryArena worldArena;
    World *world;

    u32 entityCameraFollowInd;
    Camera camera;
    BitmapImg bitmap;
    u32 playerControllerInd[INPUT_CNT];
    u32 player_cnt;
    EntityActions player_acts[INPUT_CNT];

    u32 storedCnt;
    StoredEntity storedEntities[global::max_ent_cnt];

    ArgbImg bgImg;
    ArgbImg crosshairImg;
    ArgbImg playerSprites[4];
    ArgbImg monsterSprites[4];
    ArgbImg swordSprites[4];
    ArgbImg catSprites[2];
    ArgbImg treeSprite;
    ArgbImg stairSprite;

    WorldPos testPos;

    b32 debug_drawCollision;
};

inline bool
isKeyUp(const GameButtonState &key)
{
    return key.halfTransitionCount > 0 && key.endedDown == 0;
}

inline bool
isButtonUp(const GameButtonState &button)
{
    return isKeyUp(button);
}
}  // namespace tom
#endif
