#include "math.hpp"

namespace tomato::math
{

s32
round_f32_to_s32(f32 val_)
{
    s32 val = (s32)roundf(val_);
    return val;
}

u32
round_f32_to_u32(f32 val_)
{
    u32 val = (u32)roundf(val_);
    return val;
}

s32
floorf_to_s32(f32 val_)
{
    s32 result = (s32)floorf(val_);
    return result;
}

f32
sin(f32 angle_)
{
    f32 result = sinf(angle_);
    return result;
}

f32
cos(f32 angle_)
{
    f32 result = cosf(angle_);
    return result;
}

f32
atan2(f32 x_, f32 y_)
{
    f32 result = atan2f(x_, y_);
    return result;
}

}  // namespace tomato::math
