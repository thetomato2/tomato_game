#pragma once
#include <cassert>
#include <cmath>
#include "base_types.hpp"
#include "vector.hpp"

// TODO: convert all of these to platform-efficient versions and remove cmath

namespace tom
{

namespace math
{
// NOTE: inner product or dot product
inline f32
inner(v2 a_, v2 b_)
{
    f32 res = a_.x * b_.x + a_.y * b_.y;
    return res;
};

inline f32
length_sq(const v2 a_)
{
    f32 res = inner(a_, a_);

    return res;
}

template<typename T>
T
square(const T val_)
{
    return val_ * val_;
}

// Returns min or max if input is not in between
template<typename T>
T
check_bounds(const T in_, const T min_, const T max_)
{
    T res = in_;

    if (in_ < min_)
        res = min_;
    else if (in_ > max_)
        res = max_;

    return in_;
}

template<typename T>
T
max(const T a_, const T b_)
{
    T res;
    a_ > b_ ? res = a_ : res = b_;
    return res;
}

template<typename T>
T
min(const T a_, const T b_)
{
    T res;
    a_ < b_ ? res = a_ : res = b_;
    return res;
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

inline s32
sign_of(s32 val_)
{
    s32 res = (val_ >= 0) ? 1 : -1;
    return res;
}

inline s32
ceil_f32_to_s32(f32 val_)
{
    s32 res = (s32)ceilf(val_);
    return res;
}

}  // namespace math
}  // namespace tom
