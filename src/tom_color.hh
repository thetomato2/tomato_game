namespace tom
{

struct Color
{
    union
    {
        struct
        {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
        u32 rgba;
    };
};

struct Color_argb
{
    union
    {
        u32 argb;
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
    count
};

inline Color v3_to_color(v3f col, f32 a = 1.0f)
{
    Color res;

    res.r = (u8)(col.r * 255.0f);
    res.g = (u8)(col.g * 255.0f);
    res.b = (u8)(col.b * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}

inline Color f32_to_color(f32 col, f32 a = 1.0f)
{
    Color res;

    res.r = (u8)(col * 255.0f);
    res.g = (u8)(col * 255.0f);
    res.b = (u8)(col * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}

#if 1

inline v3f color_u32_to_v3f(u32 col)
{
    v3f res;

    res.r = (f32)((col & 0xff000000) >> 24) / 255.0f;
    res.b = (f32)((col & 0x00ff0000) >> 16) / 255.0f;
    res.g = (f32)((col & 0x0000ff00) >> 8) / 255.0f;

    return res;
}

#else
inline v3f color_u32_to_v3f(u32 col)
{
    v3f res;

    res.r = (f32)(col >> 24) / 255.0f;
    res.b = (f32)(col >> 16) / 255.0f;
    res.g = (f32)(col >> 8) / 255.0f;

    return res;
}

#endif

inline Color_argb v3_to_color_argb(v3f col, f32 a = 1.0f)
{
    Color_argb res;

    res.r = (u8)(col.r * 255.0f);
    res.g = (u8)(col.g * 255.0f);
    res.b = (u8)(col.b * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}

inline Color_argb f32_to_argb(f32 col, f32 a = 1.0f)
{
    Color_argb res;

    res.r = (u8)(col * 255.0f);
    res.g = (u8)(col * 255.0f);
    res.b = (u8)(col * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}


function constexpr Color color(TomColors col)
{
    Color res;

    switch (col){
        case TomColors::black:{
            res.rgba = 0xff000000;
        }break;
        case TomColors::white:{
            res.rgba = 0xffffffff;
        }break;
        case TomColors::red:{
            res.rgba = 0xff0000ff;
        }break;
        case TomColors::green:{
            res.rgba = 0xff00ff00;
        }break;
        case TomColors::blue:{
            res.rgba = 0xffff0000;
        }break;
        case TomColors::pink:{
            res.rgba = 0xffff00ff;
        }break;
        case TomColors::teal:{
            res.rgba = 0xffffff00;
        }break;
        case TomColors::yellow:{
            res.rgba = 0xff00ffff;
        }break;
        default:{
            res.rgba = 0xffffffff;
        }break;
    }

    return res;
}

global constexpr v4f red_v4   = { 1.0f, 0.0f, 0.0f, 1.0f };
global constexpr v4f green_v4 = { 0.0f, 1.0f, 0.0f, 1.0f };
global constexpr v4f blue_v4  = { 0.0f, 0.0f, 1.0f, 1.0f };
global constexpr v4f pink_v4  = { 1.0f, 0.0f, 1.0f, 1.0f };

global constexpr v3f red_v3   = { 1.0f, 0.0f, 0.0f };
global constexpr v3f green_v3 = { 0.0f, 1.0f, 0.0f };
global constexpr v3f blue_v3  = { 0.0f, 0.0f, 1.0f };
global constexpr v3f pink_v3  = { 1.0f, 0.0f, 1.0f };

struct bitmap_header
{
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
};

struct argb_header
{
    u32 width;
    u32 height;
    u32 size;
};

struct bitmap_img
{
    i32 width;
    i32 height;
    u32 *pixel_ptr;
};

struct argb_img
{
    const char *name;
    u32 width;
    u32 height;
    u32 size;
    u32 *pixel_ptr;
};

// note: argb
// global constexpr color red   =  0xff'00'00'ff ;
// global constexpr color green =  0x00'ff'00'ff ;
// global constexpr color blue  =  0x00'00'ff'ff ;
// global constexpr color pink  =  0xff'00'ff'ff ;
// global constexpr color white =  0xff'ff'ff'ff ;
// global constexpr color black =  0x00'00'00'ff ;

}  // namespace tom
