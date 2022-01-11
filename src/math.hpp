#pragma once
#include <cassert>
#include <cmath>
#include "base_types.hpp"

// TODO: convert all of these to platform-efficient versions and remove cmath

namespace tomato::math
{
inline u32
safe_truncate_u32_to_u64(u64 value_)
{
    // TODO: defines for max values
    assert(value_ <= 0xFFFFFFFF);
    u32 result = (u32)value_;
    return result;
}

inline i32
round_f32_to_i32(f32 val_)
{
    i32 val = (i32)roundf(val_);
    return val;
}

inline u32
round_f32_to_u32(f32 val_)
{
    u32 val = (u32)roundf(val_);
    return val;
}

inline i32
floorf_to_i32(f32 value_)
{
    i32 result = (i32)floorf(value_);
    return result;
}

inline f32
sin(f32 angle_)
{
    f32 result = sinf(angle_);
    return result;
}

inline f32
cos(f32 angle_)
{
    f32 result = cosf(angle_);
    return result;
}

inline f32
atan2(f32 x_, f32 y_)
{
    f32 result = atan2f(x_, y_);
    return result;
}

template<typename T>
T
square(T val_)
{
    return val_ * val_;
}

}  // namespace tomato::math
