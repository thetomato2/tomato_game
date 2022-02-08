#ifndef TOMATO_INTRINSIC_HPP_
#define TOMATO_INTRINSIC_HPP_
#include "tomato_platform.h"

struct Bit_Scan_Result
{
    bool found;
    u32 index;
};

inline Bit_Scan_Result
find_least_signifcant_set_bit(u32 val_)
{
    Bit_Scan_Result result {};

#if MSVC
    result.found = _BitScanForward((ul *)&result.index, val_);
#else

    for (u32 test {}; test < 32; ++test) {
        if (val_ & (1 << test)) {
            result.index = test;
            result.found = true;
        }
    }

#endif
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
floorf_to_i32(f32 val_)
{
    i32 result = (i32)floorf(val_);
    return result;
}

inline f32
tom_sin(f32 angle_)
{
    f32 result = sinf(angle_);
    return result;
}

inline f32
tom_cos(f32 angle_)
{
    f32 result = cosf(angle_);
    return result;
}

inline f32
tom_atan2(f32 x_, f32 y_)
{
    f32 result = atan2f(x_, y_);
    return result;
}

inline f32
sqrt_f32(f32 val_)
{
    f32 res = sqrtf(val_);
    return res;
}

inline f32
abs_f32(f32 val_)
{
    f32 res = fabsf(val_);
    return res;
}

inline i32
sign_of(i32 val_)
{
    i32 res = (val_ >= 0) ? 1 : -1;
    return res;
}

inline i32
ceil_f32_to_i32(f32 val_)
{
    i32 res = (i32)ceilf(val_);
    return res;
}

#endif  // TOMATO_INTRINSIC_HPP_
