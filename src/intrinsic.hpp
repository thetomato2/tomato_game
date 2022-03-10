#ifndef TOMATO_INTRINSIC_HPP_
#define TOMATO_INTRINSIC_HPP_
#include "Platform.h"

namespace tom
{
namespace math
{
struct BitscanResult
{
    bool found;
    u32 index;
};

inline BitscanResult
find_least_signifcant_set_bit(u32 val)
{
    BitscanResult result {};

#if MSVC
    result.found = _BitScanForward((ul *)&result.index, val);
#else

    for (u32 test {}; test < 32; ++test) {
        if (val & (1 << test)) {
            result.index = test;
            result.found = true;
        }
    }

#endif
    return result;
}

inline s32
round_f32_s32(f32 val)
{
    s32 result = (s32)roundf(val);
    return result;
}

inline u32
round_f32_u32(f32 val)
{
    u32 result = (u32)roundf(val);
    return result;
}

inline s32
floorf_s32(f32 val)
{
    s32 result = (s32)floorf(val);
    return result;
}

inline f32
sin(f32 angle)
{
    f32 result = sinf(angle);
    return result;
}

inline f32
cos(f32 angle)
{
    f32 result = cosf(angle);
    return result;
}

inline f32
atan2(f32 x, f32 y)
{
    f32 result = atan2f(x, y);
    return result;
}

inline f32
sqrt_f32(f32 val)
{
    f32 result = sqrtf(val);
    return result;
}

inline f32
abs_f32(f32 val)
{
    f32 result = fabsf(val);
    return result;
}

inline s32
sign_of(s32 val)
{
    s32 result = (val >= 0) ? 1 : -1;
    return result;
}

inline s32
ceil_f32_s32(f32 val)
{
    s32 result = (s32)ceilf(val);
    return result;
}
}  // namespace math

}  // namespace tom

#endif  // TOMATO_INTRINSIC_HPP_
