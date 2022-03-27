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

global_var constexpr u32 max_ent_cnt            = 65536;
global_var constexpr u32 num_screens            = 10;
global_var constexpr u32 num_tiles_per_screen_y = 11;
global_var constexpr s32 chunk_safe_margin      = S32_MAX / 64;
global_var constexpr f32 chunk_size_meters      = 22.f;
global_var constexpr f32 tile_size_meters       = 1.4f;
global_var constexpr f32 meters_to_pixels       = 60.f;
global_var constexpr f32 screen_size_x          = chunk_size_meters;
global_var constexpr f32 screen_size_y          = chunk_size_meters * 9.f / 16.f;
global_var constexpr f32 jump_vel               = 2.f;
global_var constexpr f32 gravity                = -9.8f;
global_var constexpr f32 epsilon                = 0.0001f;
global_var constexpr v3 chunk_dim_meters        = { chunk_size_meters, chunk_size_meters, 1.f };

}  // namespace global

struct color_argb
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
global_var constexpr color_argb red   = { 0xff'ff'00'00 };
global_var constexpr color_argb green = { 0xff'00'ff'00 };
global_var constexpr color_argb blue  = { 0xff'00'00'ff };
global_var constexpr color_argb pink  = { 0xff'ff'00'ff };
global_var constexpr color_argb black = { 0xff'ff'ff'ff };
}  // namespace colors

struct memory_arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

inline void
init_arena(memory_arena *arena, const mem_ind size, void *base)
{
    arena->size = size;
    arena->base = scast(byt *, base);
    arena->used = 0;
}

inline void *
push_size(memory_arena *arena, mem_ind size)
{
    TOM_ASSERT((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;

    return result;
}

inline void
zero_size(mem_ind size, void *ptr)
{
    // TODO: profile this for performance
    byt *byte = scast(byt *, ptr);
    while (size--) {
        *byte++ = 0;
    }
}

#define PUSH_STRUCT(arena, type)       (type *)push_size(arena, sizeof(type))
#define PUSH_ARRAY(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))
#define ZERO_STRUCT(inst)              zero_size(sizeof(inst), &(inst))

// Generic flag stuff

inline bool
is_flag_set(s32 flags, s32 flag)
{
    return flags & flag;
}

inline void
set_flag(s32 &flags, s32 flag)
{
    flags |= flag;
}

inline void
clear_flag(s32 &flags, s32 flag)
{
    flags &= ~flag;
}

}  // namespace tom
#endif  // TOMATO_COMMON_HPP_
