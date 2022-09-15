#ifndef TOM_COLOR_HH
#define TOM_COLOR_HH

#include "tom_core.hh"
#include "tom_math.hh"

namespace tom
{

struct Color_u32
{
    union
    {
        u32 rgba;
        struct
        {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
    };
};

enum TomColors
{
    black,
    white,
    red,
    green,
    blue,
    pink,
    teal,
    yellow,
    orange,
    light_blue,
    count,
};

inline Color_u32 v3f_to_color(v3f col, f32 a = 1.0f)
{
    TOM_ASSERT(col.r >= 0.0f && col.r <= 1.0f);
    TOM_ASSERT(col.g >= 0.0f && col.b <= 1.0f);
    TOM_ASSERT(col.b >= 0.0f && col.g <= 1.0f);
    TOM_ASSERT(a >= 0.0f && a <= 1.0f);

    Color_u32 res;

    res.r = (u8)(col.r * 255.0f);
    res.g = (u8)(col.g * 255.0f);
    res.b = (u8)(col.b * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}

inline Color_u32 v4f_to_color_u32(v4f col)
{
    TOM_ASSERT(col.r >= 0.0f && col.r <= 1.0f);
    TOM_ASSERT(col.g >= 0.0f && col.b <= 1.0f);
    TOM_ASSERT(col.b >= 0.0f && col.g <= 1.0f);
    TOM_ASSERT(col.a >= 0.0f && col.a <= 1.0f);

    Color_u32 res;

    res.r = (u8)(col.r * 255.0f);
    res.g = (u8)(col.g * 255.0f);
    res.b = (u8)(col.b * 255.0f);
    res.a = (u8)(col.a * 255.0f);

    return res;
}

inline v3f color_u32_to_v3f(u32 col)
{
    v3f res;

    res.r = (f32)((col & 0x000000ff)) / 255.0f;
    res.g = (f32)((col & 0x0000ff00) >> 8) / 255.0f;
    res.b = (f32)((col & 0x00ff0000) >> 16) / 255.0f;

    return res;
}

inline v4f color_u32_to_v4f(u32 col)
{
    v4f res;

    res.r = (f32)((col >> 0) & 0xff) / 255.0f;
    res.g = (f32)((col >> 8) & 0xff) / 255.0f;
    res.b = (f32)((col >> 16) & 0xff) / 255.0f;
    res.a = (f32)((col >> 24) & 0xff) / 255.0f;

    return res;
}

fn constexpr Color_u32 color_u32(TomColors col)
{
    Color_u32 res;

    switch (col) {
        case TomColors::black: {
            res.rgba = 0xff000000;
        } break;
        case TomColors::white: {
            res.rgba = 0xffffffff;
        } break;
        case TomColors::red: {
            res.rgba = 0xff0000ff;
        } break;
        case TomColors::green: {
            res.rgba = 0xff00ff00;
        } break;
        case TomColors::blue: {
            res.rgba = 0xffff0000;
        } break;
        case TomColors::pink: {
            res.rgba = 0xffff00ff;
        } break;
        case TomColors::teal: {
            res.rgba = 0xffffff00;
        } break;
        case TomColors::yellow: {
            res.rgba = 0xff00ffff;
        } break;
        case TomColors::orange: {
            res.rgba = 0xff0080ff;
        } break;
        case TomColors::light_blue: {
            res.rgba = 0xffff8000;
        } break;
        default: {
            res.rgba = 0xffffffff;
        } break;
    }

    return res;
}

fn constexpr v4f color_v4f(TomColors col)
{
    v4f res;

    switch (col) {
        case TomColors::black: {
            res = { 0.0f, 0.0f, 0.0f, 1.0f };
        } break;
        case TomColors::white: {
            res = { 1.0f, 1.0f, 1.0f, 1.0f };
        } break;
        case TomColors::red: {
            res = { 1.0f, 0.0f, 0.0f, 1.0f };
        } break;
        case TomColors::green: {
            res = { 0.0f, 1.0f, 0.0f, 1.0f };
        } break;
        case TomColors::blue: {
            res = { 0.0f, 0.0f, 1.0f, 1.0f };
        } break;
        case TomColors::pink: {
            res = { 1.0f, 0.0f, 1.0f, 1.0f };
        } break;
        case TomColors::teal: {
            res = { 0.0f, 1.0f, 1.0f, 1.0f };
        } break;
        case TomColors::yellow: {
            res = { 1.0f, 1.0f, 0.0f, 1.0f };
        } break;
        case TomColors::orange: {
            res = { 1.0f, 0.5f, 0.0f, 1.0f };
        } break;
        case TomColors::light_blue: {
            res = { 0.0f, 0.5f, 1.0f, 1.0f };
        } break;
        default: {
            res = { 0.0f, 0.0f, 0.0f, 1.0f };
        } break;
    }

    return res;
}

inline v4f sRGB_to_linear(v4f col)
{
    return { square(col.r), square(col.g), square(col.b), square(col.a) };
}

inline v4f linear_to_sRGB(v4f col)
{
    return { sqrt_f32(col.r), sqrt_f32(col.g), sqrt_f32(col.b), sqrt_f32(col.a) };
}

inline v4f premultiply_alpha(v4f col)
{
    return { col.r * col.a, col.g * col.a, col.b * col.a, col.a };
}

global constexpr v4f red_v4   = { 1.0f, 0.0f, 0.0f, 1.0f };
global constexpr v4f green_v4 = { 0.0f, 1.0f, 0.0f, 1.0f };
global constexpr v4f blue_v4  = { 0.0f, 0.0f, 1.0f, 1.0f };
global constexpr v4f pink_v4  = { 1.0f, 0.0f, 1.0f, 1.0f };

global constexpr v3f red_v3   = { 1.0f, 0.0f, 0.0f };
global constexpr v3f green_v3 = { 0.0f, 1.0f, 0.0f };
global constexpr v3f blue_v3  = { 0.0f, 0.0f, 1.0f };
global constexpr v3f pink_v3  = { 1.0f, 0.0f, 1.0f };

}  // namespace tom

#endif
