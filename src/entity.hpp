#ifndef ENTITY_HPP_
#define ENTITY_HPP_
#include "Common.hpp"
#include "World.hpp"
#include "Image.hpp"

namespace tom
{
struct EntityActions
{
    bool start;
    bool jump;
    bool attack;

    v2 dir;

    bool sprint;
};

enum EntityDirection : s32
{
    north = 0,
    east,
    south,
    west
};

enum class EntityType
{
    null = 0,
    none,
    player,
    wall,
    stairs,
    familiar,
    monster,
    sword
};

struct EntityLowChunkRef
{
    WorldChunk *tileChunk;
    u32 chunkInd;
};

struct EntityVisiblePiece
{
    ArgbImg *img;
    v2 midP;
    f32 z;
    f32 alpha;
    Rect rect;
    Color color;
};

struct EntityVisblePieceGroup
{
    u32 pieceCnt;
    EntityVisiblePiece pieces[64];
};

struct SimEntity;

union EntityRef
{
    SimEntity *ptr;
    u32 ind;
};

struct SimEntity
{
    v2 pos;
    v2 vel;
    u32 chunkZ;
    f32 z;
    f32 velZ;

    b32 active;
    b32 collides;
    b32 barrier;
    b32 hurtbox;
    s32 hitPoints;
    u32 maxHitPoints;
    s32 virtualZ;
    f32 width, height;
    f32 argbOffset;

    u32 storedInd;

    EntityRef weaponInd;
    EntityRef parentInd;

    Color color;
    EntityType type;
    EntityDirection direction;
    ArgbImg *sprite;
};

struct StoredEntity
{
    SimEntity sim;
    WorldPos worldPos;
};

}  // namespace tom
#endif  // ENTITY_HPP_
