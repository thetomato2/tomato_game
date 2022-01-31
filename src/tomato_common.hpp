#ifndef TOMATO_COMMON_HPP_
#define TOMATO_COMMON_HPP_
#include "tomato_platform.h"
/*
** Dump stuff here that doesn't go in to tomato_platfrom.h so IDE's don't complain and I can use
** intellsense,  has nothing to do with the build itself
*/

struct Mem_Arena
{
    mem_ind size;
    u8 *base;
    mem_ind used;
};

inline void *
push_size(Mem_Arena *arena_, mem_ind size_)
{
    assert((arena_->used + size_) <= arena_->size);
    void *result = arena_->base + arena_->used;
    arena_->used += size_;

    return result;
}

#define PushStruct(arena, type)       (type *)push_size(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)push_size(arena, (count * sizeof(type)))

#endif  // TOMATO_COMMON_HPP_
