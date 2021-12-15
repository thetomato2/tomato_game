#ifndef TOMATO_MATH_H_
#define TOMATO_MATH_H_
#include <cassert>
#include <cmath>
#include "base_types.h"

// TODO: convert all of these to platform-efficient versions and remove cmath

namespace tomato::math
{
inline u32
safe_truncate_u32_to_u64(u64 value)
{
	// TODO: defines for max values
	assert(value <= 0xFFFFFFFF);
	return (u32)value;
}

inline i32
round_f32_to_i32(f32 value)
{
	return i32(value + 0.5f);
}

inline u32
rnd_f32_to_u32(f32 value)
{
	return u32(value + 0.5f);
}

inline i32
floorf_to_i32(f32 val)
{
	return (i32)floorf(val);
}

inline f32
sin(f32 angle)
{
	f32 res = sinf(angle);
	return res;
}

inline f32
cos(f32 angle)
{
	f32 res = cosf(angle);
	return res;
}

inline f32
atan2(f32 x, f32 y)
{
	f32 res = atan2f(x, y);
	return res;
}

}  // namespace tomato::math

#endif	// TOMATO_MATH_H_
