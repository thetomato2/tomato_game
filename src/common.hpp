#ifndef TOMATO_COMMON_HPP_
#define TOMATO_COMMON_HPP_
#include "Platform.h"
#include "Math.hpp"
#include "Utils.hpp"
/*
 * Dump stuff here that doesn't go in to tomato_platfrom.h so IDE's don't complain and I can use
 * my preciouis intellsense
 */

namespace tom
{
namespace global
{
global_var constexpr u32 max_ent_cnt { 65536 };
global_var constexpr u32 max_high_cnt { 4096 };
global_var constexpr u32 num_screens { 10 };
global_var constexpr u32 num_tiles_per_screen_y { 11 };
global_var constexpr s32 chunk_safe_margin { INT32_MAX / 64 };
global_var constexpr f32 chunk_size_meters { 22.f };
global_var constexpr f32 meters_to_pixels { 50.f };
global_var constexpr f32 screen_size_x { chunk_size_meters };
global_var constexpr f32 screen_size_y { chunk_size_meters * 9.f / 16.f };
global_var constexpr f32 jump_vel { 2.f };
global_var constexpr f32 gravitl { -9.8f };
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
global_var constexpr Color red { 0xFF'FF'00'00 };
global_var constexpr Color green { 0xFF'00'FF'00 };
global_var constexpr Color blue { 0xFF'00'00'FF };
global_var constexpr Color pink { 0xFF'FF'00'FF };
global_var constexpr Color black { 0xFF'FF'FF'FF };
}  // namespace colors

struct MemoryArena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

inline void *
pushSize(MemoryArena *arena, mem_ind size)
{
    assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;

    return result;
}

#define PUSH_STRUCT(arena, type)       (type *)pushSize(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)pushSize(arena, (count * sizeof(type)))
}  // namespace tom
#endif  // TOMATO_COMMON_HPP_
