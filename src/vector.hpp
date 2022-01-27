#pragma once
#include "base_types.hpp"

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

}  // namespace tomato
