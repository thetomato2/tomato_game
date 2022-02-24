#ifndef TOMATO_COMMON_HPP_
#define TOMATO_COMMON_HPP_
#include "platform.h"
/*
 * Dump stuff here that doesn't go in to tomato_platfrom.h so IDE's don't complain and I can use
 * intellsense,  has nothing to do with the build itself
 */

namespace tom
{
namespace global
{
static constexpr u32 max_low_cnt            = 65536;
static constexpr u32 max_high_cnt           = 4096;
static constexpr u32 num_screens            = 10;
static constexpr u32 num_tiles_per_screen_y = 11;
static constexpr s32 chunk_safe_margin      = INT32_MAX / 64;
static constexpr f32 chunk_size_meters      = 22.f;
static constexpr f32 meters_to_pixels       = 50.f;
static constexpr f32 screen_size_x          = chunk_size_meters;
static constexpr f32 screen_size_y          = chunk_size_meters * 9.f / 16.f;
static constexpr f32 jump_vel               = 2.f;
static constexpr f32 gravitl                = -9.8f;

}  // namespace global

struct Memory_Arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

inline void *
push_size(Memory_Arena *arena, mem_ind size)
{
    assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;

    return result;
}

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))
}  // namespace tom
#endif  // TOMATO_COMMON_HPP_
