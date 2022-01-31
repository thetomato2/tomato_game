#ifndef TOMATO_MATH_HPP_
#define TOMATO_MATH_HPP_
#include "tomato_platform.h"
#include "tomato_intrinsic.hpp"

// ===============================================================================================
// #VECTOR 2
// ===============================================================================================
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
// ===============================================================================================
// #RECT_V2
// ===============================================================================================

struct rect_v2
{
    v2 min;
    v2 max;
};

inline rect_v2
rect_v2_min_max(v2 min_, v2 max_)
{
    rect_v2 result;

    result.min = min_;
    result.max = max_;

    return result;
}

inline rect_v2
rect_v2_min_dim(v2 min_, v2 dim_)
{
    rect_v2 result;

    result.min = min_;
    result.max = min_ + dim_;

    return result;
}

inline rect_v2
rect_v2_center_half_dim(v2 center_, v2 half_dim_)
{
    rect_v2 result;

    result.min = center_ - half_dim_;
    result.max = center_ + half_dim_;

    return result;
}

inline bool
is_in_rect_v2(rect_v2 rect_, v2 test_)
{
    bool result = ((test_.x >= rect_.min.x) && (test_.y >= rect_.min.y) &&
                   (test_.x <= rect_.max.x) && (test_.y <= rect_.max.y));

    return result;
}

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
#endif  // TOMATO_MATH_HPP_
