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
    struct
    {
        v2 xy;
        f32 _ignored0;
    };
    f32 e[3];
};

inline v3
v3_init(v2 a, f32 z = 0.f)
{
    v3 res = { .x = a.x, .y = a.y, .z = z };
    return res;
}

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

}  // namespace vec

// ===============================================================================================
// #RECTANGLE 2
// ===============================================================================================

struct rect2
{
    v2 min;
    v2 max;
};

// ===============================================================================================
// #RECTANGLE 3
// ===============================================================================================

struct rect3
{
    v3 min;
    v3 max;
};

inline rect3
rect3_init(rect2 a)
{
    rect3 res;

    res.min = v3_init(a.min);
    res.max = v3_init(a.max);

    return res;
}

// ===============================================================================================
// #RECTANGLE FUNCS
// ===============================================================================================

namespace rec
{

inline v2
max_corner(rect2 a)
{
    v2 res = a.max;
    return res;
}

inline v3
max_corner(rect3 a)
{
    v3 res = a.max;
    return res;
}

inline v2
min_corner(rect2 a)
{
    v2 res = a.min;
    return res;
}

inline v3
min_corner(rect3 a)
{
    v3 res = a.min;
    return res;
}

inline rect2
min_max(v2 min, v2 max)
{
    rect2 res;

    res.min = min;
    res.max = max;

    return res;
}

inline rect3
min_max(v3 min, v3 max)
{
    rect3 res;

    res.min = min;
    res.max = max;

    return res;
}

inline rect2
min_dim(v2 min, v2 dim)
{
    rect2 res;

    res.min = min;
    res.max = min + dim;

    return res;
}

inline rect3
min_dim(v3 min, v3 dim)
{
    rect3 res;

    res.min = min;
    res.max = min + dim;

    return res;
}

inline rect2
center_half_dim(v2 center, v2 half_dim)
{
    rect2 res;

    res.min = center - half_dim;
    res.max = center + half_dim;

    return res;
}

inline rect3
center_half_dim(v3 center, v3 half_dim)
{
    rect3 res;

    res.min = center - half_dim;
    res.max = center + half_dim;

    return res;
}

inline rect2
center_dim(v2 center, v2 dim)
{
    rect2 res = center_half_dim(center, dim * 0.5f);

    return res;
}

inline rect3
center_dim(v3 center, v3 dim)
{
    rect3 res = center_half_dim(center, dim * 0.5f);

    return res;
}

inline v2
center(rect2 a)
{
    v2 res = 0.5f * (a.min + a.max);

    return res;
}

inline v3
center(rect3 a)
{
    v3 res = 0.5f * (a.min + a.max);

    return res;
}

inline bool
is_inside(rect2 a, v2 test)
{
    bool res = test.x >= a.min.x && test.y >= a.min.y && test.x <= a.max.x && test.y <= a.max.y;
    return res;
}

inline bool
is_inside(rect3 a, v3 test)
{
    bool res = test.x >= a.min.x && test.y >= a.min.y && test.x <= a.max.x && test.y <= a.max.y &&
               test.z >= a.min.z && test.z <= a.max.z;
    return res;
}

inline rect2
add_radius(rect2 a, v2 r)
{
    rect2 res;

    res.min = a.min - r;
    res.max = a.max + r;

    return res;
}

inline rect3
add_radius(rect3 a, v3 r)
{
    rect3 res;

    res.min = a.min - r;
    res.max = a.max + r;

    return res;
}

inline rect2
add_radius(rect2 a, f32 r)
{
    v2 r_ = { r, r };
    return add_radius(a, r_);
}

inline rect3
add_radius(rect3 a, f32 r)
{
    v3 r_ = { r, r, r };
    return add_radius(a, r_);
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
