#include "math.hpp"

namespace tomato::math
{

i32
round_f32_to_i32(f32 val_)
{
    i32 val = (i32)roundf(val_);
    return val;
}

u32
round_f32_to_u32(f32 val_)
{
    u32 val = (u32)roundf(val_);
    return val;
}

i32
floorf_to_i32(f32 val_)
{
    i32 result = (i32)floorf(val_);
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
