namespace tom
{

struct bitscan_result
{
    bool found;
    u32 index;
};

inline bitscan_result find_least_signifcant_set_bit(u32 val)
{
    bitscan_result result {};

#if MSVC
    result.found = _BitScanForward((ul*)&result.index, val);
#else

    for (u32 test {}; test < 32; ++test) {
        if (val & (1 << test)) {
            result.index = test;
            result.found = true;
        }
    }

#endif
    return result;
}

inline i32 round_f32_to_i32(f32 val)
{
    i32 result = (i32)roundf(val);
    return result;
}

inline u32 round_f32_to_u32(f32 val)
{
    u32 result = (u32)roundf(val);
    return result;
}

inline i32 floorf_to_i32(f32 val)
{
    i32 result = (i32)floorf(val);
    return result;
}

inline f32 tom_sin(f32 angle)
{
    f32 result = sinf(angle);
    return result;
}

inline f32 tom_cos(f32 angle)
{
    f32 result = cosf(angle);
    return result;
}

inline f32 tom_atan2(f32 x, f32 y)
{
    f32 result = atan2f(x, y);
    return result;
}

inline f32 sqrt_f32(f32 val)
{
    f32 result = sqrtf(val);
    return result;
}

inline f32 abs_f32(f32 val)
{
    f32 result = fabsf(val);
    return result;
}

inline i32 sign_of(i32 val)
{
    i32 result = (val >= 0) ? 1 : -1;
    return result;
}

inline i32 ceilf_to_i32(f32 val)
{
    i32 result = (i32)ceilf(val);
    return result;
}

}  // namespace tom
