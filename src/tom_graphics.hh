namespace tom
{
#define MAX_PIECE_CNT 4096

struct BackBuffer
{
    void* buf;
    i32 width, height, pitch, byt_per_pix;
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
    RenderGroupEntryHeader header;
    Color clear_color;
};

struct RenderGroupEntryTexture
{
    RenderGroupEntryHeader header;
    RenderBasis basis;
    Texture* texture;
    v2f offset;
};

struct RenderGroupEntryRect
{
    RenderGroupEntryHeader header;
    RenderBasis basis;
    v2f dims;
    Color color;
};

struct RenderGroupEntryRectOutline
{
    RenderGroupEntryHeader header;
    RenderBasis basis;
    v2f dims;
    i32 thickness;
    Color color;
};

struct RenderGroupEntryCoordSystem
{
    RenderGroupEntryType type;
    v2f origin;
    v2f x_axis;
    v2f y_axis;
    i32 num_points;
    Color color;
};

struct RenderGroup
{
    f32 meters_to_pixels;
    u32 max_pushbuffer_sz;
    u32 pushbuffer_sz;
    byt* pushbuffer_base;
    Camera cam;
};

}  // namespace tom