
struct Bit_Scan_Result
{
    bool found;
    u32 index;
};

inline Bit_Scan_Result
find_least_signifcant_set_bit(u32 val_)
{
    Bit_Scan_Result result {};

#if MSVC
    result.found = _BitScanForward((ul *)&result.index, val_);
#else

    for (u32 test {}; test < 32; ++test) {
        if (val_ & (1 << test)) {
            result.index = test;
            result.found = true;
        }
    }

#endif
    return result;
}

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
tom_sin(f32 angle_)
{
    f32 result = sinf(angle_);
    return result;
}

f32
tom_cos(f32 angle_)
{
    f32 result = cosf(angle_);
    return result;
}

f32
tom_atan2(f32 x_, f32 y_)
{
    f32 result = atan2f(x_, y_);
    return result;
}

inline f32
sqrt_f32(f32 val_)
{
    f32 res = sqrtf(val_);
    return res;
}

inline f32
abs_f32(f32 val_)
{
    f32 res = fabsf(val_);
    return res;
}

inline s32
sign_of(s32 val_)
{
    s32 res = (val_ >= 0) ? 1 : -1;
    return res;
}

inline s32
ceil_f32_to_s32(f32 val_)
{
    s32 res = (s32)ceilf(val_);
    return res;
}
