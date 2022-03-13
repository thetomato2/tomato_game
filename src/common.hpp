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

global_var constexpr u32 max_low_cnt            = 65536;
global_var constexpr u32 max_high_cnt           = 4096;
global_var constexpr u32 num_screens            = 10;
global_var constexpr u32 num_tiles_per_screen_y = 11;
global_var constexpr s32 chunk_safe_margin      = S32_MAX / 64;
global_var constexpr f32 chunk_size_meters      = 22.f;
global_var constexpr f32 meters_to_pixels       = 50.f;
global_var constexpr f32 screen_size_x          = chunk_size_meters;
global_var constexpr f32 screen_size_y          = chunk_size_meters * 9.f / 16.f;
global_var constexpr f32 jump_vel               = 2.f;
global_var constexpr f32 gravity                = -9.8f;

}  // namespace global

struct color
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
// note: argb
global_var constexpr color red   = { 0xff'ff'00'00 };
global_var constexpr color green = { 0xff'00'ff'00 };
global_var constexpr color blue  = { 0xff'00'00'ff };
global_var constexpr color pink  = { 0xff'ff'00'ff };
global_var constexpr color black = { 0xff'ff'ff'ff };
}  // namespace colors

struct memory_arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

inline void *
push_size(memory_arena *arena, mem_ind size)
{
    TOM_ASSERT((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;

    return result;
}

#define PUSH_STRUCT(arena, type)       (type *)push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))
}  // namespace tom
#endif  // TOMATO_COMMON_HPP_
