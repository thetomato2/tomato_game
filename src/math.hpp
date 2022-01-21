#pragma once
#include <cassert>
#include <cmath>
#include "base_types.hpp"

// TODO: convert all of these to platform-efficient versions and remove cmath

namespace tomato::math
{

struct Bit_Scan_Result
{
    bool found;
    u32 index;
};

template<typename T>
T
square(T val_)
{
    return val_ * val_;
}

// NOTE: win32_layer uses this
inline u32
safe_truncate_u32_to_u64(u64 val_)
{
    // TODO: defines for max values
    assert(val_ <= 0xFFFFFFFF);
    u32 result = (u32)val_;
    return result;
}

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

s32
round_f32_to_s32(f32 val_);

u32
round_f32_to_u32(f32 val_);

s32
floorf_to_s32(f32 value_);

f32
sin(f32 angle_);

f32
cos(f32 angle_);

f32
atan2(f32 x_, f32 y_);

}  // namespace tomato::math
