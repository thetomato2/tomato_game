#ifndef TOMATO_COMMON_HPP_
#define TOMATO_COMMON_HPP_
#include "platform.h"
#include "math.hpp"
#include "utils.hpp"
/*
 * Dump stuff here that doesn't go in to tomato_platfrom.h so IDE's don't complain and I can use
 * my preciouis intellsense
 */

namespace tom
{
namespace global
{
static constexpr u32 max_low_cnt { 65536 };
static constexpr u32 max_high_cnt { 4096 };
static constexpr u32 num_screens { 10 };
static constexpr u32 num_tiles_per_screen_y { 11 };
static constexpr s32 chunk_safe_margin { INT32_MAX / 64 };
static constexpr f32 chunk_size_meters { 22.f };
static constexpr f32 meters_to_pixels { 50.f };
static constexpr f32 screen_size_x { chunk_size_meters };
static constexpr f32 screen_size_y { chunk_size_meters * 9.f / 16.f };
static constexpr f32 jump_vel { 2.f };
static constexpr f32 gravitl { -9.8f };
}  // namespace global

struct Color
{
    union
    {
        u32 argb;
        struct
        {
            u8 b;
            u8 g;
            u8 r;
            u8 a;
        };
    };
};

namespace colors
{
// NOTE: ARGB
static constexpr Color red { 0xFF'FF'00'00 };
static constexpr Color green { 0xFF'00'FF'00 };
static constexpr Color blue { 0xFF'00'00'FF };
static constexpr Color pink { 0xFF'FF'00'FF };
static constexpr Color black { 0xFF'FF'FF'FF };
}  // namespace colors

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
