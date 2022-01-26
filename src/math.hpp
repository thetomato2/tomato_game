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

inline v2 &
operator*=(v2 &a_, f32 b_)
{
    a_.x *= b_;
    a_.y *= b_;

    return a_;
}

namespace math
{
// NOTE: inner product or dot product
inline f32
inner(v2 a_, v2 b_)
{
    f32 result = a_.x * b_.x + a_.y * b_.y;
    return result;
};

inline f32
length_sq(const v2 a_)
{
    f32 result = inner(a_, a_);

    return result;
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
    T result = in_;

    if (in_ < min_)
        result = min_;
    else if (in_ > max_)
        result = max_;

    return in_;
}

template<typename T>
T
max(const T a_, const T b_)
{
    T result;
    a_ > b_ ? result = a_ : result = b_;
    return result;
}

template<typename T>
T
min(const T a_, const T b_)
{
    T result;
    a_ < b_ ? result = a_ : result = b_;
    return result;
}

inline f32
sqrt_f32(f32 val_)
{
    f32 result = sqrtf(val_);
    return result;
}

inline f32
abs_f32(f32 val_)
{
    f32 result = fabsf(val_);
    return result;
}

}  // namespace math
}  // namespace tomato
