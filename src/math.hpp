#pragma once
#include <cassert>
#include <cmath>
#include "base_types.hpp"

// TODO: convert all of these to platform-efficient versions and remove cmath

namespace tomato
{

struct v2
{
    union
    {
        struct
        {
            f32 x, y;
        };
        f32 e[2];
    };

    v2
    operator+=(v2 a_);
    v2
    operator+=(f32 a_);
    v2
    operator-=(v2 a_);
    v2
    operator-=(f32 a_);
};

inline v2
operator+(v2 a_, v2 b_)
{
    v2 result;

    result.x = a_.x + b_.x;
    result.y = a_.y + b_.y;

    return result;
}

inline v2
v2::operator+=(v2 a_)
{
    *this = *this + a_;

    return *this;
}

inline v2
v2::operator+=(f32 a_)
{
    this->x += +a_;
    this->y += +a_;

    return *this;
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

inline v2
v2::operator-=(v2 a_)
{
    *this = *this - a_;

    return *this;
}

inline v2
v2::operator-=(f32 a_)
{
    this->x = -a_;
    this->y = -a_;

    return *this;
}

inline v2
operator*(f32 a_, v2 b_)
{
    v2 result;

    result.x = a_ * b_.x;
    result.y = a_ * b_.y;

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
}  // namespace tomato
