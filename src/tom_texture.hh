#ifndef TOM_TEXTURE_HH
#define TOM_TEXTURE_HH

#include "tom_core.hh"
#include "tom_math.hh"
#include "tom_color.hh"

namespace tom
{

////////////////////////////////////////////////////////////////////////////////////////////////
// Texture Types
struct Texture
{
    enum class Type
    {

        none,
        R8,
        R8G8B8A8,
    };

    i32 width, height;
    Type type;
    void *buf;
};

struct BilinearSample
{
    u32 a, b, c, d;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Texture Functions
inline constexpr u32 texture_get_texel_size(Texture::Type type)
{
    switch (type) {
        case Texture::Type::none: return 0;
        case Texture::Type::R8: return 1;
        case Texture::Type::R8G8B8A8: return 4;
    }

    return -1;
};

inline constexpr i32 texture_get_num_channels(Texture::Type type)
{
    switch (type) {
        case Texture::Type::none: return 0;
        case Texture::Type::R8: return 1;
        case Texture::Type::R8G8B8A8: return 4;
    }

    return -1;
};

inline Texture::Type texture_type_from_channels(i32 n_channels)
{
    switch (n_channels) {
        case 0: return Texture::Type::none;
        case 1: return Texture::Type::R8;
        case 4: return Texture::Type::R8G8B8A8;
    }

    return Texture::Type::none;
};

inline u32 texture_get_size(Texture tex)
{
    return tex.width * tex.height * texture_get_texel_size(tex.type);
}

inline u32 texture_get_size(Texture *tex)
{
    TOM_ASSERT(tex);
    if (tex) return tex->width * tex->height * texture_get_texel_size(tex->type);
    return 0;
}

inline u32 texture_get_pitch(Texture tex)
{
    return texture_get_texel_size(tex.type) * tex.width;
}

inline u32 texture_get_pitch(Texture *tex)
{
    TOM_ASSERT(tex);
    if (tex) return texture_get_texel_size(tex->type) * tex->width;
    return 0;
}

Texture texture_load_from_file(const char *path);
void make_sphere_nrm_map(Texture *tex, f32 rough, f32 cx = 1.0f, f32 cy = 1.0f);
void make_pyramid_nrm_map(Texture *tex, f32 rough);
BilinearSample get_bilinear_sample(Texture *texture, i32 x, i32 y);
v4f bilinear_sample_blend(BilinearSample samp, f32 fx, f32 fy, bool pre_mult_a = false);

}  // namespace tom
#endif
