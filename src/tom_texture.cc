#include "tom_texture.hh"

namespace tom
{

// FIXME:this leaks but I don't care atm
// TODO: use arena?
Texture texture_load_from_file(const char *path)
{
    Texture result;
    i32 n_channels;

    // stbi_set_flip_vertically_on_load(true);
    result.buf  = (void *)stbi_load(path, &result.width, &result.height, &n_channels, 0);
    result.type = texture_type_from_channels(n_channels);

    return result;
}

void make_sphere_nrm_map(Texture *tex, f32 rough, f32 cx, f32 cy)
{
    f32 inv_width  = 1.0f / (tex->width - 1.0f);
    f32 inv_height = 1.0f / (tex->height - 1.0f);
    f32 seven      = 0.707106781188f;
    u32 pitch      = texture_get_pitch(*tex);

    auto row = (byt *)tex->buf;
    for (i32 y = 0; y < tex->height; ++y) {
        auto texel = (u32 *)row;
        for (i32 x = 0; x < tex->width; ++x) {
            v2f tex_uv    = { inv_width * (f32)x, inv_height * (f32)y };
            f32 nx        = cx * (2.0f * tex_uv.x - 1.0f);
            f32 ny        = cy * (2.0f * tex_uv.y - 1.0f);
            f32 root_term = 1.0f - square(nx) - square(ny);
            v3f nrm       = { 0, seven, seven };  // default
            if (root_term >= 0.0f) nrm = { nx, ny, sqrt_f32(root_term) };
            nrm      = vec_normalize(nrm);
            v4f col  = { 0.5f * (nrm.x + 1.0f), 0.5f * (nrm.y + 1.0f), 0.5f * (nrm.z + 1.0f),
                         rough };
            *texel++ = v4f_to_color_u32(col).rgba;
        }
        row += pitch;
    }
}

void make_pyramid_nrm_map(Texture *tex, f32 rough)
{
    f32 inv_width  = 1.0f / (tex->width - 1.0f);
    f32 inv_height = 1.0f / (tex->height - 1.0f);
    f32 seven      = 0.707106781188f;
    u32 pitch      = texture_get_pitch(*tex);

    auto row = (byt *)tex->buf;
    for (i32 y = 0; y < tex->height; ++y) {
        auto texel = (u32 *)row;
        for (i32 x = 0; x < tex->width; ++x) {
            v2f tex_uv = { inv_width * (f32)x, inv_height * (f32)y };
            f32 inv_x  = tex->width - 1 - x;
            v3f nrm    = { 0, 0, seven };

            if (x < y) {
                if (inv_x < y)
                    nrm.x = -seven;
                else
                    nrm.y = seven;
            } else {
                if (inv_x < y)
                    nrm.y = -seven;
                else
                    nrm.x = seven;
            }

            v4f col  = { 0.5f * (nrm.x + 1.0f), 0.5f * (nrm.y + 1.0f), 0.5f * (nrm.z + 1.0f),
                         rough };
            *texel++ = v4f_to_color_u32(col).rgba;
        }
        row += pitch;
    }
}

BilinearSample get_bilinear_sample(Texture *texture, i32 x, i32 y)
{
    u32 texel_sz   = texture_get_texel_size(texture->type);
    u32 tex_pitch  = texture_get_pitch(texture);
    byt *texel_ptr = (byt *)texture->buf + y * tex_pitch + x * texel_sz;

    BilinearSample result;
    result.a = *(u32 *)texel_ptr;
    result.b = *(u32 *)(texel_ptr + texel_sz);
    result.c = *(u32 *)(texel_ptr + tex_pitch);
    result.d = *(u32 *)(texel_ptr + tex_pitch + texel_sz);

    return result;
}

v4f bilinear_sample_blend(BilinearSample samp, f32 fx, f32 fy, bool pre_mult_a)
{
    v4f t0 = color_u32_to_v4f(samp.a);
    v4f t1 = color_u32_to_v4f(samp.b);
    v4f t2 = color_u32_to_v4f(samp.c);
    v4f t3 = color_u32_to_v4f(samp.d);

    if (pre_mult_a) {
        t0 = premultiply_alpha(t0);
        t1 = premultiply_alpha(t1);
        t2 = premultiply_alpha(t2);
        t3 = premultiply_alpha(t3);
    }

    v4f result = bi_lerp(t0, t1, t2, t3, fx, fy);

    return result;
}

}  // namespace tom
