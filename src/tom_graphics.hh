#ifndef TOM_GRAPHICS_HH
#define TOM_GRAPHICS_HH

#include "tom_core.hh"
#include "tom_math.hh"
#include "tom_memory.hh"
#include "tom_color.hh"
#include "tom_thread.hh"
#include "tom_texture.hh"

namespace tom
{

////////////////////////////////////////////////////////////////////////////////////////////////
// #Graphics Types

struct EnviromentMap
{
    // // NOTE: lod[0] is width_pow_2 x height_pow_2;
    // u32 width_pow_2, height_pow_2;
    Texture lod[4];
};

struct RenderBasis
{
    m3 model;
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
    m3 model;
};

struct RenderGroupEntryRect
{
    v2f pos;
    v2f dims;
    Color_u32 color;
    m3 model;
};

struct RenderGroupEntryRectOutline
{
    v2f pos;
    v2f dims;
    i32 thickness;
    Color_u32 color;
    m3 model;
};

struct RenderGroupEntryCoordSystem
{
    RenderGroupEntryType type;
    m3 model;
    Color_u32 color;
    Texture *albedo, *normal;
    EnviromentMap *top, *middle, *bottom;
};

struct RenderGroup
{
    i32 tile_r;
    f32 meters_to_pixels;
    u32 max_pushbuffer_sz;
    u32 pushbuffer_sz;
    byt *pushbuffer_base;
    Camera cam;
};

struct TileRenderWork
{
    RenderGroup* render_group;
    Texture* target;
    r2i clip;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// #Graphics Functions
void clear_color(Texture *buffer, Color_u32 color);
void draw_rect(Texture *buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, Color_u32 color);
void draw_square(Texture *buffer, v2f pos, f32 radius, Color_u32 color);

void draw_rect_outline(Texture *buffer, i32 min_x, i32 min_y, f32 max_x, f32 max_y, i32 thickness,
                       Color_u32 color);


// SIMD
void draw_rect_256(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, r2i clip, bool even,
                   Color_u32 color);
void draw_rect_128(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                   v4f color = { 1.0f, 1.0f, 1.0f, 1.0f });
void draw_texture_256(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                      r2i clip, bool even);
// normals and stuff
void draw_rect_slowly(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                      Texture *normal, EnviromentMap *top, EnviromentMap *middle,
                      EnviromentMap *bottom, v4f color = { 1.0f, 1.0f, 1.0f, 1.0f });
void draw_rect_slowly(Texture *buffer, m3 model, Texture *albedo, Texture *normal,
                      EnviromentMap *top, EnviromentMap *middle, EnviromentMap *bottom,
                      v4f color = { 1.0f, 1.0f, 1.0f, 1.0f });

void draw_texture(Texture *buffer, Texture *tex, v2f pos);
void clear_color(Texture *buffer, Color_u32 color);

void push_clear(RenderGroup *group, Color_u32 color);
void push_texture(RenderGroup *group, RenderBasis basis, Texture *texture, v2f offset, m3 model);
void push_rect(RenderGroup *group, v2f pos, v2f dims, Color_u32 color, m3 model);
void push_rect_outline(RenderGroup *group, v2f pos, v2f dims, i32 thickness, Color_u32 color, m3 model);
void push_coord_system(RenderGroup *group, m3 model, Texture *albedo, Texture *normal,
                       EnviromentMap *top, EnviromentMap *middle, EnviromentMap *bottom,
                       Color_u32 color = { 0xffffffff });
void draw_render_group(RenderGroup *group, Texture *back_buffer, bool even);
void draw_render_group_tiled(WorkQueue *queue, RenderGroup *group, Texture *back_buffer);
RenderGroup *alloc_render_group(Arena *arena, u32 max_pushbuffer_sz, f32 meters_to_pixels,
                                Camera cam);
}  // namespace tom

#endif
