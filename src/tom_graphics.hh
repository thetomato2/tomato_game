namespace tom
{
#define MAX_PIECE_CNT 4096

// struct BackBuffer
// {
//     void *buf;
//     i32 width, height, pitch, byt_per_pix;
// };

struct EnviromentMap
{
    // // NOTE: lod[0] is width_pow_2 x height_pow_2;
    // u32 width_pow_2, height_pow_2;
    Texture lod[4];
};

struct RenderBasis
{
    v3f pos;
};

enum class RenderGroupEntryType
{
    clear,
    texture,
    rect,
    rect_outline,
    coord_system,
};

// TODO: remove the header
struct RenderGroupEntryHeader
{
    RenderGroupEntryType type;
};

struct RenderGroupEntryClear
{
    Color_u32 clear_color;
};

struct RenderGroupEntryTexture
{
    RenderBasis basis;
    Texture *texture;
    v2f offset;
};

struct RenderGroupEntryRect
{
    RenderBasis basis;
    v2f dims;
    Color_u32 color;
};

struct RenderGroupEntryRectOutline
{
    RenderBasis basis;
    v2f dims;
    i32 thickness;
    Color_u32 color;
};

struct RenderGroupEntryCoordSystem
{
    RenderGroupEntryType type;
    v2f origin, x_axis, y_axis;
    Color_u32 color;
    Texture *albedo, *normal;
    EnviromentMap *top, *middle, *bottom;
};

struct RenderGroup
{
    f32 meters_to_pixels;
    u32 max_pushbuffer_sz;
    u32 pushbuffer_sz;
    byt *pushbuffer_base;
    Camera cam;
};

}  // namespace tom