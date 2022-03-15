#ifndef TOMATO_MATH_HPP_
#define TOMATO_MATH_HPP_
#include "platform.h"
#include "intrinsic.hpp"
namespace tom
{
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
operator+(v2 lhs, v2 rhs)
{
    v2 result;

    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;

    return result;
}

inline v2 &
operator+=(v2 &lhs, v2 rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

inline v2 &
operator+=(v2 &lhs, f32 rhs)
{
    lhs.x += rhs;
    lhs.y += rhs;

    return lhs;
}

inline v2
operator-(v2 lhs)
{
    v2 result;

    result.x = -lhs.x;
    result.y = -lhs.y;

    return result;
}
inline v2
operator-(v2 lhs, v2 rhs)
{
    v2 result;

    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;

    return result;
}

inline v2 &
operator-=(v2 &lhs, v2 rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

inline v2 &
operator-=(v2 &lhs, f32 rhs)
{
    lhs.x = rhs;
    lhs.y = rhs;

    return lhs;
}

inline v2
operator*(f32 lhs, v2 rhs)
{
    v2 result;

    result.x = lhs * rhs.x;
    result.y = lhs * rhs.y;

    return result;
}

inline v2
operator*(v2 lhs, f32 rhs)
{
    v2 result;

    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;

    return result;
}

inline v2 &
operator*=(v2 &lhs, f32 rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;

    return lhs;
}

inline v2
operator/(v2 lhs, f32 rhs)
{
    v2 result;

    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;

    return result;
}

inline v2 &
operator/=(v2 &lhs, f32 rhs)
{
    lhs.x /= rhs;
    lhs.y /= rhs;

    return lhs;
}

inline bool
operator==(v2 &lhs, v2 &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

// ===============================================================================================
// #VECTOR 3
// ===============================================================================================
union v3
{
    struct
    {
        f32 x, y, z;
    };
    f32 e[3];
};

// ===============================================================================================
// #VECTOR 4
// ===============================================================================================
union v4
{
    struct
    {
        f32 x, y, z, w;
    };
    f32 e[4];
};

// ===============================================================================================
// #VECTOR FUNCS
// ===============================================================================================
namespace vec
{
// NOTE: inner product or dot product
inline f32
inner(v2 a, v2 b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
};

inline f32
length_sq(const v2 a)
{
    f32 result = inner(a, a);

    return result;
}
}  // namespace vec

// ===============================================================================================
// #RECT_V2
// ===============================================================================================

struct rect
{
    v2 min;
    v2 max;
};

namespace rec
{

inline v2
max_corner(rect rect)
{
    v2 result = rect.max;
    return result;
}

inline v2
min_corner(rect rect)
{
    v2 result = rect.min;
    return result;
}

inline rect
min_max(v2 min, v2 max)
{
    rect result;

    result.min = min;
    result.max = max;

    return result;
}

inline rect
min_dim(v2 min, v2 dim)
{
    rect result;

    result.min = min;
    result.max = min + dim;

    return result;
}

inline rect
center_half_dim(v2 center, v2 half_dim)
{
    rect result;

    result.min = center - half_dim;
    result.max = center + half_dim;

    return result;
}

inline v2
center(rect rect)
{
    v2 result = 0.5f * (rect.min + rect.max);

    return result;
}

inline bool
is_inside(rect rect, v2 test)
{
    bool result = ((test.x >= rect.min.x) && (test.y >= rect.min.y) && (test.x <= rect.max.x) &&
                   (test.y <= rect.max.y));
    return result;
}
}  // namespace rec

// ===============================================================================================
// #FREE_FUNCS
// ===============================================================================================
namespace math
{
template<typename T>
T
square(const T val)
{
    return val * val;
}

// Returns min or max if input is not in between
template<typename T>
T
check_bounds(const T in, const T min, const T max)
{
    T result = in;

    if (in < min)
        result = min;
    else if (in > max)
        result = max;

    return in;
}

template<typename T>
T
max(const T a, const T b)
{
    T result;
    a > b ? result = a : result = b;
    return result;
}

template<typename T>
T
min(const T a, const T b)
{
    T result;
    a < b ? result = a : result = b;
    return result;
}
}  // namespace math

}  // namespace tom
#endif  // TOMATO_MATH_HPP_
