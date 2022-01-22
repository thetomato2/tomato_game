#pragma once
#include <cassert>
#include <cmath>
#include "base_types.hpp"

// TODO: convert all of these to platform-efficient versions and remove cmath

namespace tomato
{

union v2
{
    struct
    {
        f32 x, y;
    };
    f32 e[2];
};

inline v2
operator+(v2 a_, v2 b_)
{
    v2 result;

    result.x = a_.x + b_.x;
    result.y = a_.y + b_.y;

    return result;
}

inline v2 &
operator+=(v2 &a_, v2 b_)
{
    a_ = a_ + b_;

    return a_;
}

inline v2 &
operator+=(v2 &a_, f32 b_)
{
    a_.x += b_;
    a_.y += b_;

    return a_;
}

inline v2
operator-(v2 a_)
{
    v2 result;

    result.x = -a_.x;
    result.y = -a_.y;

    return result;
}
inline v2
operator-(v2 a_, v2 b_)
{
    v2 result;

    result.x = a_.x - b_.x;
    result.y = a_.y - b_.y;

    return result;
}

inline v2 &
operator-=(v2 &a_, v2 b_)
{
    a_ = a_ - b_;

    return a_;
}

inline v2 &
operator-=(v2 &a_, f32 b_)
{
    a_.x = b_;
    a_.y = b_;

    return a_;
}

inline v2
operator*(f32 a_, v2 b_)
{
    v2 result;

    result.x = a_ * b_.x;
    result.y = a_ * b_.y;

    return result;
}

inline v2
operator*(v2 a_, f32 b_)
{
    v2 result;

    result.x = a_.x * b_;
    result.y = a_.y * b_;

    return result;
}

namespace math
{
template<typename T>
T
square(T val_)
{
    return val_ * val_;
}
}  // namespace math

// Returns min or max if input is not in between
template<typename T>
T
check_bounds(T in_, T min_, T max_) noexcept
{
    if (in_ < min_)
        in_ = min_;
    else if (in_ > max_)
        in_ = max_;

    return in_;
}
}  // namespace tomato
