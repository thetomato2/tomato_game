namespace tom
{

// ===============================================================================================
// #FREE_FUNCS
// ===============================================================================================

global constexpr f32 eps_f32    = 0.00001f;
global constexpr f32 xm_pi      = 3.141592654f;
global constexpr f32 xm_2pi     = 6.283185307f;
global constexpr f32 xm_1divpi  = 0.318309886f;
global constexpr f32 xm_1div2pi = 0.159154943f;
global constexpr f32 xm_pidiv2  = 1.570796327f;
global constexpr f32 xm_pidiv4  = 0.785398163f;

inline f32 to_radian(f32 val)
{
    return (val * xm_pi) / 180.0f;
}

inline f32 to_degree(f32 val)
{
    return (val * 180.0f) / xm_pi;
}

// inline f32 lerp(f32 v0, f32 v1, f32 a)
// {
//     return (1 - a) * v0 + a * v1;
// }

template<typename T>
inline T lerp(T v0, T v1, f32 a)
{
    return (1.0f - a) * v0 + a * v1;
}

template<typename T>
inline T bi_lerp(T v0, T v1, T v2, T v3, f32 a, f32 b)
{
    f32 a1 = 1.0f - a;
    return (1.0f - b) * (v0 * a1 + v1 * a) + b * (v2 * a1 + v3 * a);
}

inline i32 fast_floor(f32 fp)
{
    i32 i = (i32)fp;

    return (fp < i) ? (i - 1) : (i);
}

inline f32 safe_ratio(f32 numerator, f32 divisor, f32 def)
{
    f32 res = def;

    if (divisor != 0.0f) {
        res = numerator / divisor;
    }

    return res;
}

inline f32 safe_ratio_0(f32 numerator, f32 divisor)
{
    return safe_ratio(numerator, divisor, 0.0f);
}

inline f32 safe_ratio_1(f32 numerator, f32 divisor)
{
    return safe_ratio(numerator, divisor, 1.0f);
}

inline bool equals_f32(f32 a, f32 b)
{
    bool res = (fabsf(a - b)) <= (eps_f32 * fmaxf(1.0f, fmaxf(fabsf(a), fabsf(b))));

    return res;
}

inline void scalar_sin_cos(f32* p_sin, f32* p_cos, f32 val)
{
    Assert(p_sin);
    Assert(p_cos);
    f32 quo = xm_1div2pi * val;

    if (val >= 0.0f)
        quo = (f32)(i32)(quo + 0.5f);
    else
        quo = (f32)(i32)(quo - 0.5f);

    f32 y = val - xm_2pi * quo;
    f32 sign;
    if (y > xm_pidiv2) {
        y    = xm_pi - y;
        sign = -1.0f;
    } else if (y < -xm_pidiv2) {
        y    = -xm_pi - y;
        sign = -1.0f;
    } else {
        sign = 1.0f;
    }
    f32 y2 = y * y;

    // 11-degree minimax approximation
    *p_sin =
        (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) *
              y2 -
          0.16666667f) *
             y2 +
         1.0f) *
        y;

    // 10-degree minimax approximation
    f32 p =
        ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 -
         0.5f) *
            y2 +
        1.0f;
    *p_cos = sign * p;
}

inline bool near_equal_f32(f32 a, f32 b, f32 eps = 0.00001f)
{
    f32 delta = a - b;
    return fabsf(delta) <= eps;
}

template<typename T>
T square(T val)
{
    return val * val;
}

template<typename T>
T cube(T val)
{
    return val * val * val;
}

// Returns min or max if input is not in between
template<typename T>
T clamp(T val, T min, T max)
{
    T res = (val < min) ? min : val;
    if (res > max) res = max;

    return res;
}


template<typename T>
T max(T a, T b)
{
    T res;
    a > b ? res = a : res = b;
    return res;
}

template<typename T>
T min(T a, T b)
{
    T res;
    a < b ? res = a : res = b;
    return res;
}

template<typename T>
inline f32 normalize_coord(T min, T max, T a)
{
    Assert(a >= min && a <= max);
    return ((f32)a - f32(min)) / ((f32)max - (f32)min);
}

inline f32 clamp_01(f32 val)
{
    return  max(min(val, 1.0f), 0.0f);
}

inline v2f clamp_01(v2f val)
{
    v2f res;
    res.x = clamp_01(val.x);
    res.y = clamp_01(val.y);

    return res;
}

inline v3f clamp_01(v3f val)
{
    v3f res;
    res.x = clamp_01(val.x);
    res.y = clamp_01(val.y);
    res.z = clamp_01(val.z);

    return res;
}

inline v4f clamp_01(v4f val)
{
    v4f res;
    res.x = clamp_01(val.x);
    res.y = clamp_01(val.y);
    res.z = clamp_01(val.z);
    res.w = clamp_01(val.w);

    return res;
}

// ===============================================================================================
// #VECTOR 2
// ===============================================================================================

template<typename T>
inline v2<T> v2_init(T a)
{
    v2<T> res;

    res.x = a;
    res.y = a;

    return res;
}

template<typename T>
inline v2<T> v2_init(T x, T y)
{
    v2<T> res;

    res.x = x;
    res.y = y;

    return res;
}

template<typename T>
inline v2<T> operator+(v2<T> lhs, v2<T> rhs)
{
    v2<T> res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;

    return res;
}

template<typename T>
inline v2<T> operator+(v2<T> lhs, T rhs)
{
    v2<T> res;

    res.x = lhs.x + rhs;
    res.y = lhs.y + rhs;

    return res;
}

template<typename T>
inline v2<T>& operator+=(v2<T>& lhs, v2<T> rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

template<typename T>
inline v2<T>& operator+=(v2<T>& lhs, T rhs)
{
    lhs.x += rhs;
    lhs.y += rhs;

    return lhs;
}

template<typename T>
inline v2<T> operator-(v2<T> lhs)
{
    v2<T> res;

    res.x = -lhs.x;
    res.y = -lhs.y;

    return res;
}

template<typename T>
inline v2<T> operator-(v2<T> lhs, v2<T> rhs)
{
    v2<T> res;

    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;

    return res;
}

template<typename T>
inline v2<T> operator-(v2<T> lhs, T rhs)
{
    v2<T> res;

    res.x = lhs.x - rhs;
    res.y = lhs.y - rhs;

    return res;
}

template<typename T>
inline v2<T>& operator-=(v2<T>& lhs, v2<T> rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

template<typename T>
inline v2<T>& operator-=(v2<T>& lhs, T rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

template<typename T>
inline v2<T> operator*(T lhs, v2<T> rhs)
{
    v2<T> res;

    res.x = lhs * rhs.x;
    res.y = lhs * rhs.y;

    return res;
}

template<typename T>
inline v2<T> operator*(v2<T> lhs, T rhs)
{
    v2<T> res;

    res.x = lhs.x * rhs;
    res.y = lhs.y * rhs;

    return res;
}

template<typename T>
inline v2<T>& operator*=(v2<T>& lhs, T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;

    return lhs;
}

template<typename T>
inline v2<T> operator/(v2<T> lhs, T rhs)
{
    v2<T> res;

    res.x = lhs.x / rhs;
    res.y = lhs.y / rhs;

    return res;
}

template<typename T>
inline v2<T>& operator/=(v2<T>& lhs, T rhs)
{
    lhs.x /= rhs;
    lhs.y /= rhs;

    return lhs;
}

template<typename T>
inline bool operator==(v2<T>& lhs, v2<T>& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template<typename T>
inline bool operator!=(v2<T>& lhs, v2<T>& rhs)
{
    return !(lhs == rhs);
}

// ===============================================================================================
// #VECTOR 3
// ===============================================================================================

template<typename T>
inline v3<T> v3_init(T a)
{
    v3f res;

    res.x = a;
    res.y = a;
    res.z = a;

    return res;
}

template<typename T>
inline v3<T> v3_init(T x, T y, T z)
{
    v3<T> res;

    res.x = x;
    res.y = y;
    res.z = z;

    return res;
}

template<typename T>
inline v3<T> v3_init(v2<T> a, T z = 0.0f)
{
    v3<T> res;

    res.x = a.x;
    res.y = a.y;
    res.z = z;

    return res;
}

template<typename T>
inline v3<T> v3_zero()
{
    return {};
}

template<typename T>
inline v3<T> operator+(v3<T> lhs, v3<T> rhs)
{
    v3<T> res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;

    return res;
}

template<typename T>
inline v3<T> operator+(v3<T> lhs, T rhs)
{
    v3<T> res;

    res.x = lhs.x + rhs;
    res.y = lhs.y + rhs;
    res.z = lhs.z + rhs;

    return res;
}

template<typename T>
inline v3<T>& operator+=(v3<T>& lhs, v3<T> rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

template<typename T>
inline v3<T>& operator+=(v3<T>& lhs, T rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

template<typename T>
inline v3<T> operator-(v3<T> lhs)
{
    v3<T> res;

    res.x = -lhs.x;
    res.y = -lhs.y;
    res.z = -lhs.z;

    return res;
}
template<typename T>
inline v3<T> operator-(v3<T> lhs, v3<T> rhs)
{
    v3<T> res;

    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;

    return res;
}

template<typename T>
inline v3<T> operator-(v3<T> lhs, T rhs)
{
    v3<T> res;

    res.x = lhs.x - rhs;
    res.y = lhs.y - rhs;
    res.z = lhs.z - rhs;

    return res;
}

template<typename T>
inline v3<T>& operator-=(v3<T>& lhs, v3<T> rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

template<typename T>
inline v3<T>& operator-=(v3<T>& lhs, T rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

template<typename T>
inline v3<T> operator*(T lhs, v3<T> rhs)
{
    v3<T> res;

    res.x = lhs * rhs.x;
    res.y = lhs * rhs.y;
    res.z = lhs * rhs.z;

    return res;
}

template<typename T>
inline v3<T> operator*(v3<T> lhs, T rhs)
{
    v3<T> res;

    res.x = lhs.x * rhs;
    res.y = lhs.y * rhs;
    res.z = lhs.z * rhs;

    return res;
}

template<typename T>
inline v3<T>& operator*=(v3<T>& lhs, T rhs)
{
    lhs = lhs * rhs;

    return lhs;
}

template<typename T>
inline v3<T> operator/(v3<T> lhs, T rhs)
{
    v3<T> res;

    res.x = lhs.x / rhs;
    res.y = lhs.y / rhs;
    res.z = lhs.z / rhs;

    return res;
}

template<typename T>
inline v3<T>& operator/=(v3<T>& lhs, T rhs)
{
    lhs = lhs / rhs;

    return lhs;
}

template<typename T>
inline bool operator==(v3<T>& lhs, v3<T> rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template<typename T>
inline bool operator!=(v3<T>& lhs, v3<T> rhs)
{
    return !(lhs == rhs);
}

// ===============================================================================================
// #VECTOR 4
// ===============================================================================================

template<typename T>
inline v4<T> v4_init(T x, T y, T z, T w)
{
    v4<T> res;

    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;

    return res;
}

template<typename T>
inline v4<T> v4_init(v2<T> a, T z = 0.0f, T w = 0.0f)
{
    v4<T> res;
    res.x = a.x;
    res.y = a.y;
    res.z = z;
    res.w = w;

    return res;
}

template<typename T>
inline v4<T> v4_init(v3<T> a, T w = 0.0f)
{
    v4<T> res;
    res.x = a.x;
    res.y = a.y;
    res.z = a.z;
    res.w = w;

    return res;
}

template<typename T>
inline v4<T> v4_zero()
{
    return {};
}

template<typename T>
inline v4<T> operator+(v4<T> lhs, v4<T> rhs)
{
    v4<T> res;

    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;
    res.w = lhs.w + rhs.w;

    return res;
}

template<typename T>
inline v4<T> operator+(v4<T> lhs, T rhs)
{
    v4<T> res;

    res.x = lhs.x + rhs;
    res.y = lhs.y + rhs;
    res.z = lhs.z + rhs;
    res.w = lhs.w + rhs;

    return res;
}

template<typename T>
inline v4<T> operator+=(v4<T> lhs, v4<T> rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

template<typename T>
inline v4<T> operator+=(v4<T> lhs, T rhs)
{
    lhs = lhs + rhs;

    return lhs;
}

template<typename T>
inline v4<T> operator-(v4<T> lhs)
{
    v4<T> res;

    res.x = -lhs.x;
    res.y = -lhs.y;
    res.z = -lhs.z;
    res.w = -lhs.w;

    return res;
}
template<typename T>
inline v4<T> operator-(v4<T> lhs, v4<T> rhs)
{
    v4<T> res;

    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;
    res.w = lhs.w - rhs.w;

    return res;
}

template<typename T>
inline v4<T> operator-(v4<T> lhs, T rhs)
{
    v4<T> res;

    res.x = lhs.x - rhs;
    res.y = lhs.y - rhs;
    res.z = lhs.z - rhs;
    res.w = lhs.w - rhs;

    return res;
}

template<typename T>
inline v4<T> operator-=(v4<T> lhs, v4<T> rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

template<typename T>
inline v4<T> operator-=(v4<T> lhs, T rhs)
{
    lhs = lhs - rhs;

    return lhs;
}

template<typename T>
inline v4<T> operator*(T lhs, v4<T> rhs)
{
    v4<T> res;

    res.x = lhs * rhs.x;
    res.y = lhs * rhs.y;
    res.z = lhs * rhs.z;
    res.w = lhs * rhs.w;

    return res;
}

template<typename T>
inline v4<T> operator*(v4<T> lhs, T rhs)
{
    v4<T> res;

    res.x = lhs.x * rhs;
    res.y = lhs.y * rhs;
    res.z = lhs.z * rhs;
    res.w = lhs.w * rhs;

    return res;
}

template<typename T>
inline v4<T> operator*=(v4<T> lhs, T rhs)
{
    lhs = lhs * rhs;

    return lhs;
}

template<typename T>
inline v4<T> operator*(v4<T> lhs, v3<T> rhs)
{
    v4<T> res;

    res.w = (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z);
    res.x = (lhs.w * rhs.x) + (lhs.y * rhs.z) - (lhs.z * rhs.y);
    res.y = (lhs.w * rhs.y) + (lhs.z * rhs.x) - (lhs.x * rhs.z);
    res.z = (lhs.w * rhs.z) + (lhs.x * rhs.y) - (lhs.y * rhs.x);

    return res;
}

template<typename T>
inline v4<T> operator*=(v4<T> lhs, v3<T> rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

template<typename T>
inline v4<T> operator*(v4<T> lhs, v4<T> rhs)
{
    v4<T> res;

    res.w = (lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z);
    res.x = (lhs.x * rhs.w) + (lhs.w * rhs.x) + (lhs.y * rhs.z) - (lhs.z * rhs.y);
    res.y = (lhs.y * rhs.w) + (lhs.w * rhs.y) + (lhs.z * rhs.x) - (lhs.x * rhs.z);
    res.z = (lhs.z * rhs.w) + (lhs.w * rhs.z) + (lhs.x * rhs.y) - (lhs.y * rhs.x);

    return res;
}

template<typename T>
inline v4<T> operator*=(v4<T> lhs, v4<T> rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

template<typename T>
inline v4<T> operator/(v4<T> lhs, T rhs)
{
    v4<T> res;

    res.x = lhs.x / rhs;
    res.y = lhs.y / rhs;
    res.z = lhs.z / rhs;
    res.w = lhs.w / rhs;

    return res;
}

template<typename T>
inline v4<T> operator/=(v4<T> lhs, T rhs)
{
    lhs = lhs / rhs;

    return lhs;
}

template<typename T>
inline bool operator==(v4<T> lhs, v4<T> rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template<typename T>
inline bool operator!=(v4<T> lhs, v4<T>& rhs)
{
    return !(lhs == rhs);
}

// ===============================================================================================
// #VECTOR FUNCS
// ===============================================================================================

template<typename T>
inline v2<T> vec_perp(v2<T> a)
{
    return { -a.y, a.x };
}

inline f32 vec_dot(v2f a, v2f b)
{
    return a.x * b.x + a.y * b.y;
}

inline f32 vec_dot(v3f a, v3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline f32 vec_dot(v4f a, v4f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline v2f vec_lerp(v2f a, v2f b, f32 t)
{
    return (1.0f - t) * a + t * b;
}

inline v3f vec_lerp(v3f a, v3f b, f32 t)
{
    return (1.0f - t) * a + t * b;
}

inline v4f vec_lerp(v4f a, v4f b, f32 t)
{
    return (1.0f - t) * a + t * b;
}

inline v3f vec_cross(v3f a, v3f b)
{
    v3f res;

    res.x = a.y * b.z - a.z * b.y;
    res.y = a.z * b.x - a.x * b.z;
    res.z = a.x * b.y - a.y * b.x;

    return res;
}

inline v2f vec_hadamard(v2f a, v2f b)
{
    v2f res;

    res.x = a.x * b.x;
    res.y = a.y * b.y;

    return res;
}

inline v3f vec_hadamard(v3f a, v3f b)
{
    v3f res;

    res.x = a.x * b.x;
    res.y = a.y * b.y;
    res.z = a.z * b.z;

    return res;
}

inline v4f vec_hadamard(v4f a, v4f b)
{
    v4f res;

    res.x = a.x * b.x;
    res.y = a.y * b.y;
    res.z = a.z * b.z;
    res.w = a.w * b.w;

    return res;
}

inline f32 vec_length_sq(v2f a)
{
    f32 res = vec_dot(a, a);
    return res;
}

inline f32 vec_length_sq(v3f a)
{
    f32 res = vec_dot(a, a);
    return res;
}

inline f32 vec_length_sq(v4f a)
{
    f32 res = vec_dot(a, a);
    return res;
}

inline f32 vec_length(v2f a)
{
    f32 res = sqrt_f32(vec_length_sq(a));
    return res;
}

inline f32 vec_length(v3f a)
{
    f32 res = sqrt_f32(vec_length_sq(a));
    return res;
}

inline f32 vec_length(v4f a)
{
    f32 res = sqrt_f32(vec_length_sq(a));
    return res;
}

inline v3f vec_normalize(v3f a)
{
    // TODO: get rid of assert/ normalize or zero
    f32 len = vec_length(a);
    Assert(len != 0.0f);
    v3f res = a / len;

    return res;
}

inline f32 vec_distance(v2f a, v2f b)
{
    v2f dis = a - b;
    dis.x   = abs(dis.x);
    dis.y   = abs(dis.y);

    f32 res = vec_length(dis);

    return res;
}

inline f32 vec_distance(v3f a, v3f b)
{
    v3f dis = a - b;
    dis.x   = abs(dis.x);
    dis.y   = abs(dis.y);
    dis.z   = abs(dis.z);

    f32 res = vec_length(dis);

    return res;
}

inline v2f vec_reflect(v2f a, v2f b)
{
    v2f res;

    f32 dot = vec_dot(a, b);

    res.x = a.x - (2.0f * b.x) * dot;
    res.y = a.y - (2.0f * b.y) * dot;

    return res;
}

inline v3f vec_reflect(v3f a, v3f b)
{
    v3f res;

    f32 dot = vec_dot(a, b);

    res.x = a.x - (2.0f * b.x) * dot;
    res.y = a.y - (2.0f * b.y) * dot;
    res.z = a.z - (2.0f * b.z) * dot;

    return res;
}

// Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c)
// NOTE: Assumes P is on the plane of the triangle
inline v3f vec_barycenter(v3f p, v3f a, v3f b, v3f c)
{
    v3f res;

    v3f v0  = { b.x - a.x, b.y - a.y, b.z - a.z };
    v3f v1  = { c.x - a.x, c.y - a.y, c.z - a.z };
    v3f v2  = { p.x - a.x, p.y - a.y, p.z - a.z };
    f32 d00 = v0.x * v0.x + v0.y * v0.y + v0.z * v0.z;
    f32 d01 = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    f32 d11 = v1.x * v1.x + v1.y * v1.y + v1.z * v1.z;
    f32 d20 = v2.x * v0.x + v2.y * v0.y + v2.z * v0.z;
    f32 d21 = v2.x * v1.x + v2.y * v1.y + v2.z * v1.z;

    f32 denom = d00 * d11 - d01 * d01;

    res.y = (d11 * d20 - d01 * d21) / denom;
    res.z = (d00 * d21 - d01 * d20) / denom;
    res.x = 1.0f - (res.z + res.y);

    return res;
}

// ===============================================================================================
// #Quaternion
// ===============================================================================================

using quat = v4f;  // Quaternion

inline quat quat_to_quat(v3f v, f32 a)
{
    f32 half_angle_rad = to_radian(a / 2.0f);
    f32 sin_half_angle = sinf(half_angle_rad);
    f32 cos_half_angle = cosf(half_angle_rad);

    quat res;

    res.x = v.x * sin_half_angle;
    res.y = v.y * sin_half_angle;
    res.z = v.z * sin_half_angle;
    res.w = cos_half_angle;

    return res;
}

inline f32 quat_norm(quat a)
{
    f32 i   = vec_dot(a.xyz, a.xyz);
    f32 s   = square(a.w);
    f32 res = sqrt_f32(i + s);

    return res;
}

inline quat quat_unit_norm(quat a)
{
    f32 half_angle_rad = to_radian(a.w / 2.0f);
    f32 sin_half_angle = sinf(half_angle_rad);
    f32 cos_half_angle = cosf(half_angle_rad);

    a.w   = cos_half_angle;
    a.xyz = vec_normalize(a.xyz);
    a.xyz = a.xyz * sin_half_angle;

    return a;
}

inline quat quat_conjuate(quat q)
{
    quat res;
    res.x = -q.x;
    res.y = -q.y;
    res.z = -q.z;
    res.w = q.w;

    return res;
}

inline quat quat_inverse(quat a)
{
    f32 abs   = 1 / (square(quat_norm(a)));
    quat conj = quat_conjuate(a);
    f32 s     = conj.w * abs;
    v3f i     = conj.xyz * abs;

    quat res;
    res.xyz = i;
    res.w   = s;

    return res;
}

// rotate v3 around arbitrary axis
inline v3f quat_rotate(v3f v, v3f u, f32 a)
{
    quat p   = { v.x, v.y, v.z, 0.0f };
    u        = vec_normalize(u);
    quat q   = quat_unit_norm({ u.x, u.y, u.z, a });
    quat q_i = quat_inverse(q);

    quat res = q * p * q_i;

    return res.xyz;
}

// transform a v3 by quat
inline v3f quat_rotate(v3f v, quat q)
{
    v3f res;

    res.x = v.x * (q.x * q.x + q.w * q.w - q.y * q.y - q.z * q.z) +
            v.y * (2 * q.x * q.y - 2 * q.w * q.z) + v.z * (2 * q.x * q.z + 2 * q.w * q.y);
    res.y = v.x * (2 * q.w * q.z + 2 * q.x * q.y) +
            v.y * (q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z) +
            v.z * (-2 * q.w * q.x + 2 * q.y * q.z);
    res.z = v.x * (-2 * q.w * q.y + 2 * q.x * q.z) + v.y * (2 * q.w * q.x + 2 * q.y * q.z) +
            v.z * (q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);

    return res;
}

// ===============================================================================================
// #Matrix 4x4
// ===============================================================================================

inline m4 operator*(m4 a, m4 b)
{
    m4 res = {};

    res.e[0]  = a.e[0] * b.e[0] + a.e[1] * b.e[4] + a.e[2] * b.e[8] + a.e[3] * b.e[12];
    res.e[1]  = a.e[0] * b.e[1] + a.e[1] * b.e[5] + a.e[2] * b.e[9] + a.e[3] * b.e[13];
    res.e[2]  = a.e[0] * b.e[2] + a.e[1] * b.e[6] + a.e[2] * b.e[10] + a.e[3] * b.e[14];
    res.e[3]  = a.e[0] * b.e[3] + a.e[1] * b.e[7] + a.e[2] * b.e[11] + a.e[3] * b.e[15];
    res.e[4]  = a.e[4] * b.e[0] + a.e[5] * b.e[4] + a.e[6] * b.e[8] + a.e[7] * b.e[12];
    res.e[5]  = a.e[4] * b.e[1] + a.e[5] * b.e[5] + a.e[6] * b.e[9] + a.e[7] * b.e[13];
    res.e[6]  = a.e[4] * b.e[2] + a.e[5] * b.e[6] + a.e[6] * b.e[10] + a.e[7] * b.e[14];
    res.e[7]  = a.e[4] * b.e[3] + a.e[5] * b.e[7] + a.e[6] * b.e[11] + a.e[7] * b.e[15];
    res.e[8]  = a.e[8] * b.e[0] + a.e[9] * b.e[4] + a.e[10] * b.e[8] + a.e[11] * b.e[12];
    res.e[9]  = a.e[8] * b.e[1] + a.e[9] * b.e[5] + a.e[10] * b.e[9] + a.e[11] * b.e[13];
    res.e[10] = a.e[8] * b.e[2] + a.e[9] * b.e[6] + a.e[10] * b.e[10] + a.e[11] * b.e[14];
    res.e[11] = a.e[8] * b.e[3] + a.e[9] * b.e[7] + a.e[10] * b.e[11] + a.e[11] * b.e[15];
    res.e[12] = a.e[12] * b.e[0] + a.e[13] * b.e[4] + a.e[14] * b.e[8] + a.e[15] * b.e[12];
    res.e[13] = a.e[12] * b.e[1] + a.e[13] * b.e[5] + a.e[14] * b.e[9] + a.e[15] * b.e[13];
    res.e[14] = a.e[12] * b.e[2] + a.e[13] * b.e[6] + a.e[14] * b.e[10] + a.e[15] * b.e[14];
    res.e[15] = a.e[12] * b.e[3] + a.e[13] * b.e[7] + a.e[14] * b.e[11] + a.e[15] * b.e[15];

    return res;
}

inline v3f transform(m4 a, v3f p, f32 Pw = 1.0f)
{
    v3f res = {};

    res.x = p.x * a.m[0][0] + p.y * a.m[0][1] + p.z * a.m[0][2] + Pw * a.m[0][3];
    res.y = p.x * a.m[1][0] + p.y * a.m[1][1] + p.z * a.m[1][2] + Pw * a.m[1][3];
    res.z = p.x * a.m[2][0] + p.y * a.m[2][1] + p.z * a.m[2][2] + Pw * a.m[2][3];

    return res;
}

inline v3f operator*(m4 a, v3f p)
{
    v3f res = transform(a, p, 1.0f);

    return res;
}

inline m4 mat_y_up_to_z_up()
{
    m4 res = { 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
               0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    return res;
}

inline m4 mat_identity(f32 a = 1.0f)
{
    m4 res = {
        a, 0.0f, 0.0f, 0.0f, 0.0f, a, 0.0f, 0.0f, 0.0f, 0.0f, a, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    return res;
}

inline m4 mat_rot_x(f32 a)
{
    f32 c = cos(a);
    f32 s = sin(a);

    // clang-format off
    m4 res = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, c, s, 0.0f,
        0.0f, -s, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    return res;
}

inline m4 mat_rot_x(m4 a, f32 b)
{
    return a * mat_rot_x(b);
}

inline m4 mat_rot_y(f32 a)
{
    f32 c = cos(a);
    f32 s = sin(a);

    // clang-format off
    m4 res = {
        c, 0.0f, -s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        s, 0.0f, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    return res;
}

inline m4 mat_rot_y(m4 a, f32 b)
{
    return a * mat_rot_y(b);
}

inline m4 mat_rot_z(f32 a)
{
    f32 c = cos(a);
    f32 s = sin(a);

    m4 res = {
        c, s, 0.0f, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    return res;
}

inline m4 mat_rot_z(m4 a, f32 b)
{
    return a * mat_rot_z(b);
}

// u = arbitrary axis, a = angle
inline m4 mat_rotate(v3f u, f32 a)
{
    a       = to_radian(a);
    f32 c   = cos(a);
    f32 l_c = 1 - c;
    f32 s   = sin(a);

    m4 res = { u.x * u.x + (1 - u.x * u.x) * c,
               u.x * u.y * l_c + u.z * s,
               u.x * u.z * l_c - u.y * s,
               0,
               u.x * u.y * l_c - u.z * s,
               u.y * u.y + (1 - u.y * u.y) * c,
               u.y * u.z * l_c + u.x * s,
               0,
               u.x * u.z * l_c + u.y * s,
               u.y * u.z * l_c - u.x * s,
               u.z * u.z + (1 - u.z * u.z) * c,
               0,
               0,
               0,
               0,
               1 };
    return res;
}

// u = arbitrary axis, a = angle
inline m4 mat_rotate(m4 m, v3f u, f32 a)
{
    m4 res = m * mat_rotate(u, a);

    return res;
}

inline m4 mat_scale(m4 a, f32 b)
{
    m4 res = mat_identity(b);

    res = a * res;

    return res;
}

inline m4 mat_transpose(m4 a)
{
    m4 res;

#if 1
    res.e[0]  = a.e[0];
    res.e[1]  = a.e[4];
    res.e[2]  = a.e[8];
    res.e[3]  = a.e[12];
    res.e[4]  = a.e[1];
    res.e[5]  = a.e[5];
    res.e[6]  = a.e[9];
    res.e[7]  = a.e[13];
    res.e[8]  = a.e[2];
    res.e[9]  = a.e[6];
    res.e[10] = a.e[10];
    res.e[11] = a.e[14];
    res.e[12] = a.e[3];
    res.e[13] = a.e[7];
    res.e[14] = a.e[11];
    res.e[15] = a.e[15];
#else

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            res.m[i][j] = a.m[j][i];
        }
    }
#endif

    return res;
}

inline m4 mat_proj_persp(f32 aspect_ratio, f32 fov_y, f32 near_z, f32 far_z)
{
    Assert(near_z > 0.0f && far_z > 0.0f);
    Assert(!near_equal_f32(fov_y, 0.0f, 0.00001f * 2.0f));
    Assert(!near_equal_f32(aspect_ratio, 0.0f));
    Assert(!near_equal_f32(near_z, far_z));

    f32 sin_fov, cos_fov;
    scalar_sin_cos(&sin_fov, &cos_fov, 0.5f * fov_y);

    f32 height  = cos_fov / sin_fov;
    f32 width   = height / aspect_ratio;
    f32 f_range = far_z / (far_z - near_z);

    m4 res      = {};
    res.m[0][0] = width;
    res.m[1][1] = height;
    res.m[2][2] = f_range;
    res.m[2][3] = 1.0f;
    res.m[3][2] = -f_range * near_z;

    return res;
}

inline m4 mat_proj_ortho(f32 aspect_ratio)
{
    f32 a = 1.0f;
    f32 b = aspect_ratio;

    m4 res = { a,    0.0f, 0.0f, 0.0f, 0.0f, b,    0.0f, 0.0f,
               0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    return res;
}

inline m4 mat_col_3x3(v3f x, v3f y, v3f z)
{
    // clang-format off
    m4 res = {
        x.x, y.x, z.x, 0.0f,
        x.y, y.y, z.y, 0.0f,
        x.z, y.z, z.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    return res;
}

inline m4 mat_row_3x3(v3f x, v3f y, v3f z)
{
    // clang-format off
    m4 res = {
        x.x, x.y, x.z, 0.0f,
        y.x, y.y, y.z, 0.0f,
        z.x, z.y, z.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    return res;
}

inline m4 mat_translate(v3f t)
{
    m4 res = mat_identity();

    res.m[3][0] = t.x;
    res.m[3][1] = t.y;
    res.m[3][2] = t.z;

    return res;
}

inline m4 mat_translate_x(m4 a, f32 tx)
{
    a.m[3][0] += tx;

    return a;
}

inline m4 mat_translate_y(m4 a, f32 ty)
{
    a.m[3][1] += ty;

    return a;
}

inline m4 mat_translate_z(m4 a, f32 tz)
{
    a.m[3][2] += tz;

    return a;
}

inline m4 mat_translate(m4 a, v3f t)
{
    a.m[3][0] += t.x;
    a.m[3][1] += t.y;
    a.m[3][2] += t.z;

    return a;
}

inline m4 mat_set_translation(m4 a, v3f t)
{
    a.m[3][0] = t.x;
    a.m[3][1] = t.y;
    a.m[3][2] = t.z;

    return a;
}

inline m4 mat_set_translation_x(m4 a, f32 tx)
{
    a.m[3][0] = tx;

    return a;
}

inline m4 mat_set_translation_y(m4 a, f32 ty)
{
    a.m[3][1] = ty;

    return a;
}

inline m4 mat_set_translation_z(m4 a, f32 tz)
{
    a.m[3][2] = tz;

    return a;
}

inline v3f mat_get_col(m4 a, u32 c)
{
    v3f res = { a.m[0][c], a.m[1][c], a.m[2][c] };

    return res;
}

inline v3f mat_get_row(m4 a, u32 r)
{
    v3f res = { a.m[r][0], a.m[r][1], a.m[r][2] };

    return res;
}

inline m4 mat_uvn_to_m4(v3f pos, v3f u, v3f v, v3f n)
{
    m4 res = { u.x, u.y, u.z, -pos.x, v.x,  v.y,  v.z,  -pos.y,
               n.x, n.y, n.z, -pos.z, 0.0f, 0.0f, 0.0f, 1.0f };

    return res;
}

inline m4 mat_look_to(v3f eye_pos, v3f eye_dir, v3f up_dir)
{
    Assert(eye_dir != v3_zero<f32>());
    Assert(up_dir != v3_zero<f32>());

    v3f r2 = vec_normalize(eye_dir);
    v3f r0 = vec_normalize(vec_cross(up_dir, r2));
    v3f r1 = vec_cross(r2, r0);

    f32 d0 = vec_dot(r0, -eye_pos);
    f32 d1 = vec_dot(r1, -eye_pos);
    f32 d2 = vec_dot(r2, -eye_pos);

    m4 res;
    res.r[0] = v4_init(r0, d0);
    res.r[1] = v4_init(r1, d1);
    res.r[2] = v4_init(r2, d2);
    res.r[3] = mat_identity().r[3];

    return res;
}

inline m4 mat_look_at(v3f eye_pos, v3f target_pos, v3f up_dir)
{
    v3f eye_dir = target_pos - eye_pos;
    return mat_look_to(eye_pos, eye_dir, up_dir);
}

inline m4 mat_get_uvn(v3f forward, v3f up, v3f pos)
{
    v3f n = vec_normalize(forward);
    v3f u = vec_normalize(vec_cross(up, n));
    v3f v = vec_cross(n, u);

    m4 res = mat_row_3x3(u, v, n) * mat_translate(-pos);

    return res;
}

inline m4 mat_inverse(m4 a)
{
}

// ===============================================================================================
// #RECTANGLE FUNCS
// ===============================================================================================

template<typename T>
inline r2<T> operator*(r2<T>& lhs, T rhs)
{
    r2<T> res;

    res.x0 = lhs.x0 * rhs;
    res.y0 = lhs.y0 * rhs;
    res.x1 = lhs.x1 * rhs;
    res.y1 = lhs.y1 * rhs;

    return res;
}

template<typename T>
inline r2<T> operator+=(r2<T>& lhs, T rhs)
{
    lhs.min.x += rhs;
    lhs.min.y += rhs;
    lhs.max.x += rhs;
    lhs.max.y += rhs;

    return lhs;
}

template<typename T>
inline r2<T> rect_init_min_max(v2<T> min, v2<T> max)
{
    r2<T> res;

    res.min = min;
    res.max = max;

    return res;
}

template<typename T>
inline r3<T> rect_init_min_max(v3<T> min, v3<T> max)
{
    r3<T> res;

    res.min = min;
    res.max = max;

    return res;
}

template<typename T>
inline r2<T> rect_init_min_dim(v2<T> min, v2<T> dim)
{
    r2<T> res;

    res.min = min;
    res.max = min + dim;

    return res;
}

template<typename T>
inline r3<T> rect_init_min_dim(v3<T> min, v3<T> dim)
{
    r3<T> res;

    res.min = min;
    res.max = min + dim;

    return res;
}

template<typename T>
inline r2<T> rect_init_half_dim(v2<T> center, v2<T> half_dim)
{
    r2<T> res;

    res.min = center - half_dim;
    res.max = center + half_dim;

    return res;
}

template<typename T>
inline r2<T> rect_init_square(v2<T> center, T radius)
{
    v2<T> half_dim = v2_init(radius);

    return rect_init_half_dim(center, half_dim);
}

template<typename T>
inline r3<T> rect_init_half_dim(v3<T> center, v3<T> half_dim)
{
    r3<T> res;

    res.min = center - half_dim;
    res.max = center + half_dim;

    return res;
}

template<typename T>
inline r3<T> rect_init_square(v3<T> center, T radius)
{
    v3<T> half_dim = v3_init(radius);

    return rect_init_half_dim(center, half_dim);
}

template<typename T>
inline r2<T> rect_init_dims(v2<T> center, v2<T> dim)
{
    r2<T> res = rect_init_half_dim(center, dim / (T)2);

    return res;
}

template<typename T>
inline r3<T> rect_init_dims(v3<T> center, v3<T> dim)
{
    r3<T> res = rect_init_half_dim(center, dim / (T)2);

    return res;
}

template<typename T>
inline v2<T> rect_get_center(r2<T> a)
{
    v2<T> res = (a.min + a.max) / (T)2;

    return res;
}

template<typename T>
inline v3<T> rect_get_center(r3<T> a)
{
    v3<T> res = (a.min + a.max) / (T)2;

    return res;
}

template<typename T>
inline bool rect_is_inside(r2<T> a, v2<T> b)
{
    bool res = b.x >= a.min.x && b.y >= a.min.y && b.x <= a.max.x && b.y <= a.max.y;
    return res;
}

template<typename T>
inline bool rect_is_inside(r3<T> a, v3<T> b)
{
    bool res = b.x >= a.min.x && b.y >= a.min.y && b.x <= a.max.x && b.y <= a.max.y &&
               b.z >= a.min.z && b.z <= a.max.z;
    return res;
}

template<typename T>
inline r2f rect_add_dim(r2<T> a, v2<T> dim)
{
    r2f res;

    res.min = a.min - dim;
    res.max = a.max + dim;

    return res;
}

template<typename T>
inline r3<T> rect_add_dim(r3<T> a, v3<T> dim)
{
    r3f res;

    res.min = a.min - dim;
    res.max = a.max + dim;

    return res;
}

template<typename T>
inline r2<T> rect_add_radius(r2<T> a, T r)
{
    v2<T> r_ = { r, r };
    return rect_add_dim(a, r_);
}

template<typename T>
inline r3<T> rect_add_radius(r3<T> a, T r)
{
    v3<T> r_ = { r, r, r };
    return rect_add_dim(a, r_);
}

template<typename T>
inline bool rect_intersect(r2<T> a, r2<T> b)
{
    bool res = !(b.max.x < a.min.x || b.min.x > a.max.x || b.max.y < a.min.y || b.min.y > a.max.y);

    return res;
}

template<typename T>
inline bool rect_intersect(r3<T> a, r3<T> b)
{
    bool res = !(b.max.x < a.min.x || b.min.x > a.max.x || b.max.y < a.min.y || b.min.y > a.max.y ||
                 b.max.z < a.min.z || b.min.z > a.max.z);

    return res;
}

template<typename T>
inline v2f rect_barycenter(r2f a, v3f p)
{
    v2f res;

    res.x = safe_ratio_0(p.x - a.min.x, a.max.x - a.min.x);
    res.y = safe_ratio_0(p.y - a.min.y, a.max.y - a.min.y);

    return res;
}

template<typename T>
inline v3f rect_barycenter(r3f a, v3f p)
{
    v3f res;

    res.x = safe_ratio_0(p.x - a.min.x, a.max.x - a.min.x);
    res.y = safe_ratio_0(p.y - a.min.y, a.max.y - a.min.y);
    res.z = safe_ratio_0(p.z - a.min.z, a.max.z - a.min.z);

    return res;
}

inline r2i rect_f32_to_i32(r2f a)
{
    r2i res;

    res.x0 = round_f32_to_i32(a.x0);
    res.x1 = round_f32_to_i32(a.x1);
    res.y0 = round_f32_to_i32(a.y0);
    res.y1 = round_f32_to_i32(a.y1);

    return res;
}

inline r3i rect_f32_to_i32(r3f a)
{
    r3i res;

    res.x0 = round_f32_to_i32(a.x0);
    res.x1 = round_f32_to_i32(a.x1);
    res.y0 = round_f32_to_i32(a.y0);
    res.y1 = round_f32_to_i32(a.y1);
    res.z0 = round_f32_to_i32(a.z0);
    res.z1 = round_f32_to_i32(a.z1);

    return res;
}

}  // namespace tom
