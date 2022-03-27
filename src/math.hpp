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
    v2 res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;

    return res;
}

inline v2
operator+(v2 lhs, f32 rhs)
{
    v2 res;

    res.x = lhs.x + rhs;
    res.y = lhs.y + rhs;

    return res;
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
    v2 res;

    res.x = -lhs.x;
    res.y = -lhs.y;

    return res;
}
inline v2
operator-(v2 lhs, v2 rhs)
{
    v2 res;

    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;

    return res;
}

inline v2
operator-(v2 lhs, f32 rhs)
{
    v2 res;

    res.x = lhs.x - rhs;
    res.y = lhs.y - rhs;

    return res;
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
    lhs = lhs - rhs;

    return lhs;
}

inline v2
operator*(f32 lhs, v2 rhs)
{
    v2 res;

    res.x = lhs * rhs.x;
    res.y = lhs * rhs.y;

    return res;
}

inline v2
operator*(v2 lhs, f32 rhs)
{
    v2 res;

    res.x = lhs.x * rhs;
    res.y = lhs.y * rhs;

    return res;
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
    v2 res;

    res.x = lhs.x / rhs;
    res.y = lhs.y / rhs;

    return res;
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

inline bool
operator!=(v2 &lhs, v2 &rhs)
{
    return !(lhs == rhs);
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

inline v3
operator+(v3 lhs, v3 rhs)
{
    v3 res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;

    return res;
}

inline v3
operator+(v3 lhs, f32 rhs)
{
    v3 res;

    res.x = lhs.x + rhs;
    res.y = lhs.y + rhs;
    res.z = lhs.z + rhs;

    return res;
}

inline v3 &
operator+=(v3 &lhs, v3 rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

inline v3 &
operator+=(v3 &lhs, f32 rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

inline v3
operator-(v3 lhs)
{
    v3 res;

    res.x = -lhs.x;
    res.y = -lhs.y;
    res.z = -lhs.z;

    return res;
}
inline v3
operator-(v3 lhs, v3 rhs)
{
    v3 res;

    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;

    return res;
}

inline v3
operator-(v3 lhs, f32 rhs)
{
    v3 res;

    res.x = lhs.x - rhs;
    res.y = lhs.y - rhs;
    res.z = lhs.z - rhs;

    return res;
}

inline v3 &
operator-=(v3 &lhs, v3 rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

inline v3 &
operator-=(v3 &lhs, f32 rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

inline v3
operator*(f32 lhs, v3 rhs)
{
    v3 res;

    res.x = lhs * rhs.x;
    res.y = lhs * rhs.y;
    res.z = lhs * rhs.z;

    return res;
}

inline v3
operator*(v3 lhs, f32 rhs)
{
    v3 res;

    res.x = lhs.x * rhs;
    res.y = lhs.y * rhs;
    res.z = lhs.z * rhs;

    return res;
}

inline v3 &
operator*=(v3 &lhs, f32 rhs)
{
    lhs = lhs * rhs;

    return lhs;
}

inline v3
operator/(v3 lhs, f32 rhs)
{
    v3 res;

    res.x = lhs.x / rhs;
    res.y = lhs.y / rhs;
    res.z = lhs.z / rhs;

    return res;
}

inline v3 &
operator/=(v3 &lhs, f32 rhs)
{
    lhs = lhs / rhs;

    return lhs;
}

inline bool
operator==(v3 &lhs, v3 &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool
operator!=(v3 &lhs, v3 &rhs)
{
    return !(lhs == rhs);
}

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

inline v4
operator+(v4 lhs, v4 rhs)
{
    v4 res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;
    res.w = lhs.w + rhs.w;

    return res;
}

inline v4
operator+(v4 lhs, f32 rhs)
{
    v4 res;

    res.x = lhs.x + rhs;
    res.y = lhs.y + rhs;
    res.z = lhs.z + rhs;
    res.w = lhs.w + rhs;

    return res;
}

inline v4 &
operator+=(v4 &lhs, v4 rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

inline v4 &
operator+=(v4 &lhs, f32 rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

inline v4
operator-(v4 lhs)
{
    v4 res;

    res.x = -lhs.x;
    res.y = -lhs.y;
    res.z = -lhs.z;
    res.w = -lhs.w;

    return res;
}
inline v4
operator-(v4 lhs, v4 rhs)
{
    v4 res;

    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;
    res.w = lhs.w - rhs.w;

    return res;
}

inline v4
operator-(v4 lhs, f32 rhs)
{
    v4 res;

    res.x = lhs.x - rhs;
    res.y = lhs.y - rhs;
    res.z = lhs.z - rhs;
    res.w = lhs.w - rhs;

    return res;
}

inline v4 &
operator-=(v4 &lhs, v4 rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

inline v4 &
operator-=(v4 &lhs, f32 rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

inline v4
operator*(f32 lhs, v4 rhs)
{
    v4 res;

    res.x = lhs * rhs.x;
    res.y = lhs * rhs.y;
    res.z = lhs * rhs.z;
    res.w = lhs * rhs.w;

    return res;
}

inline v4
operator*(v4 lhs, f32 rhs)
{
    v4 res;

    res.x = lhs.x * rhs;
    res.y = lhs.y * rhs;
    res.z = lhs.z * rhs;
    res.w = lhs.w * rhs;

    return res;
}

inline v4 &
operator*=(v4 &lhs, f32 rhs)
{
    lhs = lhs * rhs;

    return lhs;
}

inline v4
operator/(v4 lhs, f32 rhs)
{
    v4 res;

    res.x = lhs.x / rhs;
    res.y = lhs.y / rhs;
    res.z = lhs.z / rhs;
    res.w = lhs.w / rhs;

    return res;
}

inline v4 &
operator/=(v4 &lhs, f32 rhs)
{
    lhs = lhs / rhs;

    return lhs;
}

inline bool
operator==(v4 &lhs, v4 &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

inline bool
operator!=(v4 &lhs, v4 &rhs)
{
    return !(lhs == rhs);
}
// ===============================================================================================
// #VECTOR FUNCS
// ===============================================================================================
namespace vec
{
// NOTE: inner product or dot product
inline f32
inner(const v2 a, const v2 b)
{
    f32 res = a.x * b.x + a.y * b.y;
    return res;
};

inline f32
inner(const v3 a, const v3 b)
{
    f32 res = a.x * b.x + a.y * b.y + a.z * b.z;
    return res;
};

inline f32
inner(const v4 a, const v4 b)
{
    f32 res = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return res;
};

inline v2
hadamard(const v2 a, const v2 b)
{
    v2 res = { .x = a.x * b.x, .y = a.y * b.y };
    return res;
}

inline v3
hadamard(const v3 a, const v3 b)
{
    v3 res = { .x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z };
    return res;
}

inline v4
hadamard(const v4 a, const v4 b)
{
    v4 res = { .x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z, .w = a.w * b.w };
    return res;
}

inline f32
length_sq(const v2 a)
{
    f32 res = inner(a, a);
    return res;
}

inline f32
length_sq(const v3 a)
{
    f32 res = inner(a, a);
    return res;
}

inline f32
length_sq(const v4 a)
{
    f32 res = inner(a, a);
    return res;
}

inline f32
length(const v2 a)
{
    f32 res = math::sqrt_f32(length_sq(a));
    return res;
}

inline f32
length(const v3 a)
{
    f32 res = math::sqrt_f32(length_sq(a));
    return res;
}

inline f32
length(const v4 a)
{
    f32 res = math::sqrt_f32(length_sq(a));
    return res;
}
inline v2
get_xy(v3 a)
{
    v2 res = { .x = a.x, .y = a.y };
    return res;
}

inline v3
v2_to_v3(v2 a, f32 z = 0.f)
{
    v3 res = { .x = a.x, .y = a.y, .z = z };
    return res;
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
    v2 res = rect.max;
    return res;
}

inline v2
min_corner(rect rect)
{
    v2 res = rect.min;
    return res;
}

inline rect
min_max(v2 min, v2 max)
{
    rect res;

    res.min = min;
    res.max = max;

    return res;
}

inline rect
min_dim(v2 min, v2 dim)
{
    rect res;

    res.min = min;
    res.max = min + dim;

    return res;
}

inline rect
center_dim(v2 center, v2 dim)
{
    rect res;

    res.min = center - dim;
    res.max = center + dim;

    return res;
}

inline rect
center_half_dim(v2 center, v2 half_dim)
{
    half_dim /= 2.0f;
    rect res = center_dim(center, half_dim);

    return res;
}

inline v2
center(rect rect)
{
    v2 res = 0.5f * (rect.min + rect.max);

    return res;
}

inline bool
is_inside(rect rect, v2 test)
{
    bool res = ((test.x >= rect.min.x) && (test.y >= rect.min.y) && (test.x <= rect.max.x) &&
                (test.y <= rect.max.y));
    return res;
}
inline rect
add_radius(rect a, f32 r_w, f32 r_h)
{
    rect res;

    res.min = a.min - v2 { r_w, r_h };
    res.max = a.max + v2 { r_w, r_h };

    return res;
}
inline rect
add_radius(rect a, f32 r)
{
    return add_radius(a, r, r);
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
    T res = in;

    if (in < min)
        res = min;
    else if (in > max)
        res = max;

    return in;
}

template<typename T>
T
max(const T a, const T b)
{
    T res;
    a > b ? res = a : res = b;
    return res;
}

template<typename T>
T
min(const T a, const T b)
{
    T res;
    a < b ? res = a : res = b;
    return res;
}
}  // namespace math

}  // namespace tom
#endif  // TOMATO_MATH_HPP_
