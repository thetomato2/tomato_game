#include "tom_graphics.hh"

#include "tom_time.hh"
#include "tom_app.hh"

namespace tom
{

void clear_color(Texture *buffer, Color_u32 color)
{
    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    u32 pitch = texture_get_pitch(buffer);
    byt *row  = (byt *)buffer->buf;
    for (i32 y = 0; y < buffer->height; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = 0; x < buffer->width; ++x) {
            *pixel++ = color.rgba;
        }
        row += pitch;
    }
}

void draw_rect(Texture *buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, Color_u32 color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    u32 pitch = texture_get_pitch(buffer);
    byt *row  = (byt *)buffer->buf + min_x * texture_get_texel_size(buffer->type) + min_y * pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = min_x; x < max_x; ++x) {
            v4f dest_col  = color_u32_to_v4f(*(u32 *)pixel);
            v4f src_col   = color_u32_to_v4f(color.rgba);
            v4f blend_col = lerp(dest_col, src_col, src_col.a);
            blend_col.a   = 1.0f;
            *pixel++      = v4f_to_color_u32(blend_col).rgba;
        }
        row += pitch;
    }
}

void draw_rect_outline(Texture *buffer, i32 min_x, i32 min_y, f32 max_x, f32 max_y, i32 thickness,
                       Color_u32 color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    u32 pitch = texture_get_pitch(buffer);
    byt *row  = (byt *)buffer->buf + min_x * texture_get_texel_size(buffer->type) + min_y * pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = min_x; x < max_x; ++x) {
            if (x <= min_x + thickness || x >= max_x - thickness - 1 || y <= min_y + thickness ||
                y >= max_y - thickness - 1) {
                *pixel = color.rgba;
            }
            ++pixel;
        }
        row += pitch;
    }
}

internal v4f bias_nrm(v4f nrm)
{
    v4f result;

    result.x = -1.0f + 2.0f * nrm.x;
    result.y = -1.0f + 2.0f * nrm.y;
    result.z = -1.0f + 2.0f * nrm.z;
    result.w = nrm.w;

    return result;
}

internal v3f sample_env_map(v2f screen_space_uv, v3f samp_dir, f32 rough, EnviromentMap *map,
                            f32 dist_z = 1.0f)
{
    u32 lod_i = (u32)(rough * (f32)(ARR_CNT(map->lod) - 1) + 0.5f);
    TOM_ASSERT(lod_i < ARR_CNT(map->lod));

    Texture *lod = &map->lod[lod_i];

    TOM_ASSERT(samp_dir.y > 0.0f);
    f32 uv_per_meter = 0.01f;
    f32 c            = (uv_per_meter * dist_z) / samp_dir.y;
    v2f offset       = c * v2f { samp_dir.x, samp_dir.z };
    v2f uv           = screen_space_uv + offset;
    uv               = clamp_01(uv);

    f32 tx = (uv.x * (f32)(lod->width - 2));
    f32 ty = (uv.y * (f32)(lod->height - 2));

    i32 x = (i32)tx;
    i32 y = (i32)ty;

    f32 fx = tx - (f32)x;
    f32 fy = ty - (f32)y;

    TOM_ASSERT(x >= 0 && x < lod->width);
    TOM_ASSERT(y >= 0 && y < lod->height);

    BilinearSample samp = get_bilinear_sample(lod, x, y);

    v3f result = bilinear_sample_blend(samp, fx, fy).xyz;

    return result;
}

void draw_rect_128(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo, v4f color)
{
    TOM_ASSERT(albedo);
    BEGIN_TIMED_BLOCK(DrawRect);

    f32 x_inv_len_sq = 1.0f / vec_length_sq(x_axis);
    f32 y_inv_len_sq = 1.0f / vec_length_sq(y_axis);

    color = premultiply_alpha(color);

    i32 max_width  = buffer->width - 1;
    i32 max_height = buffer->height - 1;

    f32 inv_max_width  = 1.0f / max_width;
    f32 inv_max_height = 1.0f / max_height;

    f32 x_axis_len = vec_length(x_axis);
    f32 y_axis_len = vec_length(y_axis);

    r2i rc = { max_width, max_height, 0, 0 };

    v2f points[4] = { origin, origin + x_axis, origin + y_axis, origin + x_axis + y_axis };
    for (auto p : points) {
        i32 x_floor = floorf_to_i32(p.x);
        i32 y_floor = floorf_to_i32(p.y);
        i32 x_ceil  = ceilf_to_i32(p.x);
        i32 y_ceil  = ceilf_to_i32(p.y);

        rc.x0 = min(rc.x0, x_floor);
        rc.y0 = min(rc.y0, y_floor);
        rc.x1 = max(rc.x1, x_ceil);
        rc.y1 = max(rc.y1, y_ceil);
    }

    rc.x0 = max(rc.x0, 0);
    rc.y0 = max(rc.y0, 0);
    rc.x1 = min(rc.x1, max_width);
    rc.y1 = min(rc.y1, max_height);

    v2f nx_axis = x_inv_len_sq * x_axis;
    v2f ny_axis = y_inv_len_sq * y_axis;

    f32 one_255         = 255.0f;
    f32 inv_255         = 1.0f / 255.0f;
    __m128 inv_255_m128 = _mm_set1_ps(inv_255);

    __m128 one_m128       = _mm_set1_ps(1.0f);
    __m128 four_m128      = _mm_set1_ps(4.0f);
    __m128 one_255_m128   = _mm_set1_ps(255.0f);
    __m128 zero_m128      = _mm_set1_ps(0.0f);
    __m128 mask_ff        = _mm_set1_epi32(0xff);
    __m128 nx_axis_x_m128 = _mm_set1_ps(nx_axis.x);
    __m128 nx_axis_y_m128 = _mm_set1_ps(nx_axis.y);
    __m128 ny_axis_x_m128 = _mm_set1_ps(ny_axis.x);
    __m128 ny_axis_y_m128 = _mm_set1_ps(ny_axis.y);
    __m128 origin_x_m128  = _mm_set1_ps(origin.x);
    __m128 origin_y_m128  = _mm_set1_ps(origin.y);
    __m128 width_m2       = _mm_set1_ps((f32)(albedo->width - 2));
    __m128 height_m2      = _mm_set1_ps((f32)(albedo->height - 2));

    u32 texel_sz        = texture_get_texel_size(albedo->type);
    u32 alb_pitch       = texture_get_pitch(albedo);
    u32 alb_pitch_texel = alb_pitch / texel_sz;

    u32 buf_pitch = texture_get_pitch(buffer);
    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    byt *row =
        (byt *)buffer->buf + rc.x0 * texture_get_texel_size(buffer->type) + rc.y0 * buf_pitch;

    BEGIN_TIMED_BLOCK(ProcessPixel);
    for (i32 y = rc.y0; y < rc.y1; ++y) {
        __m128 pixel_py = _mm_set1_ps((f32)y);
        pixel_py        = _mm_sub_ps(pixel_py, origin_y_m128);
        __m128 pixel_px =
            _mm_set_ps((f32)(rc.x0 + 3), (f32)(rc.x0 + 2), (f32)(rc.x0 + 1), (f32)(rc.x0 + 0));
        pixel_px = _mm_sub_ps(pixel_px, origin_x_m128);

        u32 *pixel_ptr = (u32 *)row;

        for (i32 x = rc.x0; x < rc.x1; x += 4) {
#define mmSquare(a) _mm_mul_ps(a, a)
#define M(a, i)     ((f32 *)&(a))[i]
#define MI(a, i)    ((u32 *)&(a))[i]

            __m128i original_dest = _mm_loadu_si128((__m128i *)pixel_ptr);

            __m128 u = _mm_add_ps(_mm_mul_ps(pixel_px, nx_axis_x_m128),
                                  _mm_mul_ps(pixel_py, nx_axis_y_m128));
            __m128 v = _mm_add_ps(_mm_mul_ps(pixel_px, ny_axis_x_m128),
                                  _mm_mul_ps(pixel_py, ny_axis_y_m128));

            __m128i write_msk = _mm_castps_si128(
                _mm_and_ps(_mm_and_ps(_mm_cmpge_ps(u, zero_m128), _mm_cmple_ps(u, one_m128)),
                           _mm_and_ps(_mm_cmpge_ps(v, zero_m128), _mm_cmple_ps(v, one_m128))));

            {
                u = _mm_min_ps(_mm_max_ps(u, zero_m128), one_m128);
                v = _mm_min_ps(_mm_max_ps(v, zero_m128), one_m128);

                __m128 tx = _mm_mul_ps(u, width_m2);
                __m128 ty = _mm_mul_ps(v, height_m2);

                __m128i fetch_x_128 = _mm_cvttps_epi32(tx);
                __m128i fetch_y_128 = _mm_cvttps_epi32(ty);

                __m128 fx = _mm_sub_ps(tx, _mm_cvtepi32_ps(fetch_x_128));
                __m128 fy = _mm_sub_ps(ty, _mm_cvtepi32_ps(fetch_y_128));
                __m128i samp_a;
                __m128i samp_b;
                __m128i samp_c;
                __m128i samp_d;

                for (i32 i = 0; i < 4; ++i) {
                    i32 fetch_x = MI(fetch_x_128, i);
                    i32 fetch_y = MI(fetch_y_128, i);

                    TOM_ASSERT((fetch_x >= 0) && (fetch_x < albedo->width));
                    TOM_ASSERT((fetch_y >= 0) && (fetch_y < albedo->height));

                    auto texel_ptr = (u32 *)albedo->buf + fetch_y * alb_pitch_texel + fetch_x;
                    MI(samp_a, i)  = *texel_ptr;
                    MI(samp_b, i)  = *(texel_ptr + 1);
                    MI(samp_c, i)  = *(texel_ptr + alb_pitch_texel);
                    MI(samp_d, i)  = *(texel_ptr + alb_pitch_texel + 1);
                }

                // NOTE(casey): Unpack bilinear samples
                __m128 t0_r = _mm_cvtepi32_ps(_mm_and_si128(samp_a, mask_ff));
                __m128 t0_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_a, 8), mask_ff));
                __m128 t0_b = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_a, 16), mask_ff));
                __m128 t0_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_a, 24), mask_ff));

                __m128 t1_r = _mm_cvtepi32_ps(_mm_and_si128(samp_b, mask_ff));
                __m128 t1_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_b, 8), mask_ff));
                __m128 t1_b = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_b, 16), mask_ff));
                __m128 t1_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_b, 24), mask_ff));

                __m128 t2_r = _mm_cvtepi32_ps(_mm_and_si128(samp_c, mask_ff));
                __m128 t2_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_c, 8), mask_ff));
                __m128 t2_b = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_c, 16), mask_ff));
                __m128 t2_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_c, 24), mask_ff));

                __m128 t3_r = _mm_cvtepi32_ps(_mm_and_si128(samp_d, mask_ff));
                __m128 t3_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_d, 8), mask_ff));
                __m128 t3_b = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_d, 16), mask_ff));
                __m128 t3_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(samp_d, 24), mask_ff));

                // load destination
                __m128 dest_r = _mm_cvtepi32_ps(_mm_and_si128(original_dest, mask_ff));
                __m128 dest_g =
                    _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(original_dest, 8), mask_ff));
                __m128 dest_b =
                    _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(original_dest, 16), mask_ff));
                __m128 dest_a =
                    _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(original_dest, 24), mask_ff));

                // normalize
                t0_r = _mm_mul_ps(inv_255_m128, t0_r);
                t0_g = _mm_mul_ps(inv_255_m128, t0_g);
                t0_b = _mm_mul_ps(inv_255_m128, t0_b);
                t0_a = _mm_mul_ps(inv_255_m128, t0_a);

                t1_r = _mm_mul_ps(inv_255_m128, t1_r);
                t1_g = _mm_mul_ps(inv_255_m128, t1_g);
                t1_b = _mm_mul_ps(inv_255_m128, t1_b);
                t1_a = _mm_mul_ps(inv_255_m128, t1_a);

                t2_r = _mm_mul_ps(inv_255_m128, t2_r);
                t2_g = _mm_mul_ps(inv_255_m128, t2_g);
                t2_b = _mm_mul_ps(inv_255_m128, t2_b);
                t2_a = _mm_mul_ps(inv_255_m128, t2_a);

                t3_r = _mm_mul_ps(inv_255_m128, t3_r);
                t3_g = _mm_mul_ps(inv_255_m128, t3_g);
                t3_b = _mm_mul_ps(inv_255_m128, t3_b);
                t3_a = _mm_mul_ps(inv_255_m128, t3_a);

                // for (i32 i = 0; i < 4; ++i) {
                // }
                // bi-linear texture blen
                __m128 ifx = _mm_sub_ps(one_m128, fx);
                __m128 ify = _mm_sub_ps(one_m128, fy);

                __m128 l0 = _mm_mul_ps(ify, ifx);
                __m128 l1 = _mm_mul_ps(ify, fx);
                __m128 l2 = _mm_mul_ps(fy, ifx);
                __m128 l3 = _mm_mul_ps(fy, fx);

                __m128 texel_r =
                    _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, t0_r), _mm_mul_ps(l1, t1_r)),
                                          _mm_mul_ps(l2, t2_r)),
                               _mm_mul_ps(l3, t3_r));
                __m128 texel_g =
                    _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, t0_g), _mm_mul_ps(l1, t1_g)),
                                          _mm_mul_ps(l2, t2_g)),
                               _mm_mul_ps(l3, t3_g));
                __m128 texel_b =
                    _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, t0_b), _mm_mul_ps(l1, t1_b)),
                                          _mm_mul_ps(l2, t2_b)),
                               _mm_mul_ps(l3, t3_b));
                __m128 texel_a =
                    _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, t0_a), _mm_mul_ps(l1, t1_a)),
                                          _mm_mul_ps(l2, t2_a)),
                               _mm_mul_ps(l3, t3_a));

                // TODO: do I need this???
                texel_r = _mm_mul_ps(texel_r, texel_a);
                texel_g = _mm_mul_ps(texel_g, texel_a);
                texel_b = _mm_mul_ps(texel_b, texel_a);

                dest_r = _mm_mul_ps(inv_255_m128, dest_r);
                dest_g = _mm_mul_ps(inv_255_m128, dest_g);
                dest_b = _mm_mul_ps(inv_255_m128, dest_b);
                dest_a = _mm_mul_ps(inv_255_m128, dest_a);

                // linear blend the texel and the back buffer pixel
                __m128 ita     = _mm_sub_ps(one_m128, texel_a);
                __m128 blend_r = _mm_add_ps(_mm_mul_ps(ita, dest_r), texel_r);
                __m128 blend_g = _mm_add_ps(_mm_mul_ps(ita, dest_g), texel_g);
                __m128 blend_b = _mm_add_ps(_mm_mul_ps(ita, dest_b), texel_b);
                __m128 blend_a = _mm_add_ps(_mm_mul_ps(ita, dest_a), texel_a);

                // to easily convert to u8 later
                blend_r = _mm_mul_ps(one_255_m128, blend_r);
                blend_g = _mm_mul_ps(one_255_m128, blend_g);
                blend_b = _mm_mul_ps(one_255_m128, blend_b);
                blend_a = _mm_mul_ps(one_255_m128, blend_a);

                __m128i int_r = _mm_cvtps_epi32(blend_r);
                __m128i int_g = _mm_cvtps_epi32(blend_g);
                __m128i int_b = _mm_cvtps_epi32(blend_b);
                __m128i int_a = _mm_cvtps_epi32(blend_a);

                __m128i sr = int_r;
                __m128i sg = _mm_slli_epi32(int_g, 8);
                __m128i sb = _mm_slli_epi32(int_b, 16);
                __m128i sa = _mm_slli_epi32(int_a, 24);

                __m128i out = _mm_or_si128(_mm_or_si128(sr, sg), _mm_or_si128(sb, sa));

                __m128i msk_out = _mm_or_si128(_mm_and_si128(write_msk, out),
                                               _mm_andnot_si128(write_msk, original_dest));
                _mm_storeu_si128((__m128i *)pixel_ptr, msk_out);
            }
            pixel_px = _mm_add_ps(pixel_px, four_m128);
            pixel_ptr += 4;
        }
        row += buf_pitch;
    }
    END_TIMED_BLOCK_COUNTED(ProcessPixel, (rc.x1 - rc.x0 + 1) * (rc.y1 - rc.y0 + 1));
    END_TIMED_BLOCK(DrawRect);
}

// AVX2
void draw_texture_256(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                      r2i clip, bool even, v4f color)
{
    TOM_ASSERT(albedo);
    BEGIN_TIMED_BLOCK(DrawRect);

    f32 x_inv_len_sq = 1.0f / vec_length_sq(x_axis);
    f32 y_inv_len_sq = 1.0f / vec_length_sq(y_axis);

    color = premultiply_alpha(color);

    i32 max_width  = buffer->width - 1;
    i32 max_height = buffer->height - 1;

    f32 inv_max_width  = 1.0f / max_width;
    f32 inv_max_height = 1.0f / max_height;

    f32 x_axis_len = vec_length(x_axis);
    f32 y_axis_len = vec_length(y_axis);

    r2i rc = { max_width, max_height, 0, 0 };

    v2f points[4] = { origin, origin + x_axis, origin + y_axis, origin + x_axis + y_axis };
    for (auto p : points) {
        i32 x_floor = floorf_to_i32(p.x);
        i32 y_floor = floorf_to_i32(p.y);
        i32 x_ceil  = ceilf_to_i32(p.x);
        i32 y_ceil  = ceilf_to_i32(p.y);

        rc.x0 = min(rc.x0, x_floor);
        rc.y0 = min(rc.y0, y_floor);
        rc.x1 = max(rc.x1, x_ceil);
        rc.y1 = max(rc.y1, y_ceil);
    }

    rc = rect_clip(rc, clip);

    // rc.x0 = max(rc.x0, 0);
    // rc.y0 = max(rc.y0, 0);
    // rc.x1 = min(rc.x1, max_width);
    // rc.y1 = min(rc.y1, max_height);

    if (!even == (rc.y0 % 2 == 0)) ++rc.y0;

    v2f nx_axis = x_inv_len_sq * x_axis;
    v2f ny_axis = y_inv_len_sq * y_axis;

    f32 one_255         = 255.0f;
    f32 inv_255         = 1.0f / 255.0f;
    __m256 inv_255_m256 = _mm256_set1_ps(inv_255);

    __m256 one_m256       = _mm256_set1_ps(1.0f);
    __m256 four_m256      = _mm256_set1_ps(4.0f);
    __m256 eight_m256     = _mm256_set1_ps(8.0f);
    __m256 one_255_m256   = _mm256_set1_ps(255.0f);
    __m256 zero_m256      = _mm256_set1_ps(0.0f);
    __m256 mask_ff        = _mm256_set1_epi32(0xff);
    __m256 nx_axis_x_m256 = _mm256_set1_ps(nx_axis.x);
    __m256 nx_axis_y_m256 = _mm256_set1_ps(nx_axis.y);
    __m256 ny_axis_x_m256 = _mm256_set1_ps(ny_axis.x);
    __m256 ny_axis_y_m256 = _mm256_set1_ps(ny_axis.y);
    __m256 origin_x_m256  = _mm256_set1_ps(origin.x);
    __m256 origin_y_m256  = _mm256_set1_ps(origin.y);
    __m256 width_m2       = _mm256_set1_ps((f32)(albedo->width - 2));
    __m256 height_m2      = _mm256_set1_ps((f32)(albedo->height - 2));

    u32 texel_sz        = texture_get_texel_size(albedo->type);
    u32 alb_pitch       = texture_get_pitch(albedo);
    u32 alb_pitch_texel = alb_pitch / texel_sz;

    u32 buf_pitch = texture_get_pitch(buffer);
    u32 row_adv   = buf_pitch * 2;
    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    byt *row =
        (byt *)buffer->buf + rc.x0 * texture_get_texel_size(buffer->type) + rc.y0 * buf_pitch;

    BEGIN_TIMED_BLOCK(ProcessPixel);
    for (i32 y = rc.y0; y < rc.y1; y += 2) {
        __m256 pixel_py = _mm256_set1_ps((f32)y);
        pixel_py        = _mm256_sub_ps(pixel_py, origin_y_m256);
        __m256 pixel_px =
            _mm256_set_ps((f32)(rc.x0 + 7), (f32)(rc.x0 + 6), (f32)(rc.x0 + 5), (f32)(rc.x0 + 4),
                          (f32)(rc.x0 + 3), (f32)(rc.x0 + 2), (f32)(rc.x0 + 1), (f32)(rc.x0 + 0));
        pixel_px = _mm256_sub_ps(pixel_px, origin_x_m256);

        u32 *pixel_ptr = (u32 *)row;

        for (i32 x = rc.x0; x < rc.x1; x += 8) {
#define mmSquare(a) _mm_mul_ps(a, a)
#define M(a, i)     ((f32 *)&(a))[i]
#define MI(a, i)    ((u32 *)&(a))[i]

            __m256i original_dest = _mm256_loadu_si256((__m256i *)pixel_ptr);

            __m256 u = _mm256_add_ps(_mm256_mul_ps(pixel_px, nx_axis_x_m256),
                                     _mm256_mul_ps(pixel_py, nx_axis_y_m256));
            __m256 v = _mm256_add_ps(_mm256_mul_ps(pixel_px, ny_axis_x_m256),
                                     _mm256_mul_ps(pixel_py, ny_axis_y_m256));

            __m256i write_msk = _mm256_castps_si256(_mm256_and_ps(
                _mm256_and_ps(_mm256_cmp_ps(u, zero_m256, 0x0d), _mm256_cmp_ps(u, one_m256, 0x02)),
                _mm256_and_ps(_mm256_cmp_ps(v, zero_m256, 0x0d),
                              _mm256_cmp_ps(v, one_m256, 0x02))));

            {
                u = _mm256_min_ps(_mm256_max_ps(u, zero_m256), one_m256);
                v = _mm256_min_ps(_mm256_max_ps(v, zero_m256), one_m256);

                __m256 tx = _mm256_mul_ps(u, width_m2);
                __m256 ty = _mm256_mul_ps(v, height_m2);

                __m256i fetch_x_256 = _mm256_cvttps_epi32(tx);
                __m256i fetch_y_256 = _mm256_cvttps_epi32(ty);

                __m256 fx = _mm256_sub_ps(tx, _mm256_cvtepi32_ps(fetch_x_256));
                __m256 fy = _mm256_sub_ps(ty, _mm256_cvtepi32_ps(fetch_y_256));
                __m256i samp_a;
                __m256i samp_b;
                __m256i samp_c;
                __m256i samp_d;

                for (i32 i = 0; i < 8; ++i) {
                    i32 fetch_x = MI(fetch_x_256, i);
                    i32 fetch_y = MI(fetch_y_256, i);

                    TOM_ASSERT((fetch_x >= 0) && (fetch_x < albedo->width));
                    TOM_ASSERT((fetch_y >= 0) && (fetch_y < albedo->height));

                    auto texel_ptr = (u32 *)albedo->buf + fetch_y * alb_pitch_texel + fetch_x;
                    MI(samp_a, i)  = *texel_ptr;
                    MI(samp_b, i)  = *(texel_ptr + 1);
                    MI(samp_c, i)  = *(texel_ptr + alb_pitch_texel);
                    MI(samp_d, i)  = *(texel_ptr + alb_pitch_texel + 1);
                }

                // NOTE(casey): Unpack bilinear samples
                __m256 t0_r = _mm256_cvtepi32_ps(_mm256_and_si256(samp_a, mask_ff));
                __m256 t0_g =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_a, 8), mask_ff));
                __m256 t0_b =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_a, 16), mask_ff));
                __m256 t0_a =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_a, 24), mask_ff));

                __m256 t1_r = _mm256_cvtepi32_ps(_mm256_and_si256(samp_b, mask_ff));
                __m256 t1_g =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_b, 8), mask_ff));
                __m256 t1_b =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_b, 16), mask_ff));
                __m256 t1_a =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_b, 24), mask_ff));

                __m256 t2_r = _mm256_cvtepi32_ps(_mm256_and_si256(samp_c, mask_ff));
                __m256 t2_g =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_c, 8), mask_ff));
                __m256 t2_b =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_c, 16), mask_ff));
                __m256 t2_a =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_c, 24), mask_ff));

                __m256 t3_r = _mm256_cvtepi32_ps(_mm256_and_si256(samp_d, mask_ff));
                __m256 t3_g =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_d, 8), mask_ff));
                __m256 t3_b =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_d, 16), mask_ff));
                __m256 t3_a =
                    _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(samp_d, 24), mask_ff));

                // load destination
                __m256 dest_r = _mm256_cvtepi32_ps(_mm256_and_si256(original_dest, mask_ff));
                __m256 dest_g = _mm256_cvtepi32_ps(
                    _mm256_and_si256(_mm256_srli_epi32(original_dest, 8), mask_ff));
                __m256 dest_b = _mm256_cvtepi32_ps(
                    _mm256_and_si256(_mm256_srli_epi32(original_dest, 16), mask_ff));
                __m256 dest_a = _mm256_cvtepi32_ps(
                    _mm256_and_si256(_mm256_srli_epi32(original_dest, 24), mask_ff));

                // normalize
                t0_r = _mm256_mul_ps(inv_255_m256, t0_r);
                t0_g = _mm256_mul_ps(inv_255_m256, t0_g);
                t0_b = _mm256_mul_ps(inv_255_m256, t0_b);
                t0_a = _mm256_mul_ps(inv_255_m256, t0_a);

                t1_r = _mm256_mul_ps(inv_255_m256, t1_r);
                t1_g = _mm256_mul_ps(inv_255_m256, t1_g);
                t1_b = _mm256_mul_ps(inv_255_m256, t1_b);
                t1_a = _mm256_mul_ps(inv_255_m256, t1_a);

                t2_r = _mm256_mul_ps(inv_255_m256, t2_r);
                t2_g = _mm256_mul_ps(inv_255_m256, t2_g);
                t2_b = _mm256_mul_ps(inv_255_m256, t2_b);
                t2_a = _mm256_mul_ps(inv_255_m256, t2_a);

                t3_r = _mm256_mul_ps(inv_255_m256, t3_r);
                t3_g = _mm256_mul_ps(inv_255_m256, t3_g);
                t3_b = _mm256_mul_ps(inv_255_m256, t3_b);
                t3_a = _mm256_mul_ps(inv_255_m256, t3_a);

                // for (i32 i = 0; i < 4; ++i) {
                // }
                // bi-linear texture blen
                __m256 ifx = _mm256_sub_ps(one_m256, fx);
                __m256 ify = _mm256_sub_ps(one_m256, fy);

                __m256 l0 = _mm256_mul_ps(ify, ifx);
                __m256 l1 = _mm256_mul_ps(ify, fx);
                __m256 l2 = _mm256_mul_ps(fy, ifx);
                __m256 l3 = _mm256_mul_ps(fy, fx);

                __m256 texel_r = _mm256_add_ps(
                    _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, t0_r), _mm256_mul_ps(l1, t1_r)),
                                  _mm256_mul_ps(l2, t2_r)),
                    _mm256_mul_ps(l3, t3_r));
                __m256 texel_g = _mm256_add_ps(
                    _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, t0_g), _mm256_mul_ps(l1, t1_g)),
                                  _mm256_mul_ps(l2, t2_g)),
                    _mm256_mul_ps(l3, t3_g));
                __m256 texel_b = _mm256_add_ps(
                    _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, t0_b), _mm256_mul_ps(l1, t1_b)),
                                  _mm256_mul_ps(l2, t2_b)),
                    _mm256_mul_ps(l3, t3_b));
                __m256 texel_a = _mm256_add_ps(
                    _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, t0_a), _mm256_mul_ps(l1, t1_a)),
                                  _mm256_mul_ps(l2, t2_a)),
                    _mm256_mul_ps(l3, t3_a));

                // TODO: do I need this???
                texel_r = _mm256_mul_ps(texel_r, texel_a);
                texel_g = _mm256_mul_ps(texel_g, texel_a);
                texel_b = _mm256_mul_ps(texel_b, texel_a);

                dest_r = _mm256_mul_ps(inv_255_m256, dest_r);
                dest_g = _mm256_mul_ps(inv_255_m256, dest_g);
                dest_b = _mm256_mul_ps(inv_255_m256, dest_b);
                dest_a = _mm256_mul_ps(inv_255_m256, dest_a);

                // linear blend the texel and the back buffer pixel
                __m256 ita     = _mm256_sub_ps(one_m256, texel_a);
                __m256 blend_r = _mm256_add_ps(_mm256_mul_ps(ita, dest_r), texel_r);
                __m256 blend_g = _mm256_add_ps(_mm256_mul_ps(ita, dest_g), texel_g);
                __m256 blend_b = _mm256_add_ps(_mm256_mul_ps(ita, dest_b), texel_b);
                __m256 blend_a = _mm256_add_ps(_mm256_mul_ps(ita, dest_a), texel_a);

                // to easily convert to u8 later
                blend_r = _mm256_mul_ps(one_255_m256, blend_r);
                blend_g = _mm256_mul_ps(one_255_m256, blend_g);
                blend_b = _mm256_mul_ps(one_255_m256, blend_b);
                blend_a = _mm256_mul_ps(one_255_m256, blend_a);

                __m256i int_r = _mm256_cvtps_epi32(blend_r);
                __m256i int_g = _mm256_cvtps_epi32(blend_g);
                __m256i int_b = _mm256_cvtps_epi32(blend_b);
                __m256i int_a = _mm256_cvtps_epi32(blend_a);

                __m256i sr = int_r;
                __m256i sg = _mm256_slli_epi32(int_g, 8);
                __m256i sb = _mm256_slli_epi32(int_b, 16);
                __m256i sa = _mm256_slli_epi32(int_a, 24);

                __m256i out = _mm256_or_si256(_mm256_or_si256(sr, sg), _mm256_or_si256(sb, sa));

                __m256i msk_out = _mm256_or_si256(_mm256_and_si256(write_msk, out),
                                                  _mm256_andnot_si256(write_msk, original_dest));
                _mm256_storeu_si256((__m256i *)pixel_ptr, msk_out);
            }
            pixel_px = _mm256_add_ps(pixel_px, eight_m256);
            pixel_ptr += 8;
        }
        row += row_adv;
    }
    END_TIMED_BLOCK_COUNTED(ProcessPixel, (rc.x1 - rc.x0 + 1) * (rc.y1 - rc.y0 + 1));
    END_TIMED_BLOCK(DrawRect);
}

void draw_rect_slowly(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                      Texture *normal, EnviromentMap *top, EnviromentMap *middle,
                      EnviromentMap *bottom, v4f color)
{
    f32 x_inv_len_sq = 1.0f / vec_length_sq(x_axis);
    f32 y_inv_len_sq = 1.0f / vec_length_sq(y_axis);

    color = premultiply_alpha(color);

    i32 max_width  = buffer->width - 1;
    i32 max_height = buffer->height - 1;

    f32 inv_max_width  = 1.0f / max_width;
    f32 inv_max_height = 1.0f / max_height;

    f32 x_axis_len = vec_length(x_axis);
    f32 y_axis_len = vec_length(y_axis);

    v2f nx_axis = (y_axis_len / x_axis_len) * x_axis;
    v2f ny_axis = (x_axis_len / y_axis_len) * y_axis;
    f32 nz_sca  = 0.5f * (x_axis_len + y_axis_len);

    r2i rc = { max_width, max_height, 0, 0 };

    v2f points[4] = { origin, origin + x_axis, origin + y_axis, origin + x_axis + y_axis };
    for (auto p : points) {
        i32 x_floor = floorf_to_i32(p.x);
        i32 y_floor = floorf_to_i32(p.y);
        i32 x_ceil  = ceilf_to_i32(p.x);
        i32 y_ceil  = ceilf_to_i32(p.y);

        rc.x0 = min(rc.x0, x_floor);
        rc.y0 = min(rc.y0, y_floor);
        rc.x1 = max(rc.x1, x_ceil);
        rc.y1 = max(rc.y1, y_ceil);
    }

    rc.x0 = max(rc.x0, 0);
    rc.y0 = max(rc.y0, 0);
    rc.x1 = min(rc.x1, max_width);
    rc.y1 = min(rc.y1, max_height);

    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    u32 buf_pitch = texture_get_pitch(buffer);
    byt *row =
        (byt *)buffer->buf + rc.x0 * texture_get_texel_size(buffer->type) + rc.y0 * buf_pitch;
    for (i32 y = rc.y0; y < rc.y1; ++y) {
        u32 *pixel_ptr = (u32 *)row;
        for (i32 x = rc.x0; x < rc.x1; ++x) {
            v2f pixel_v2f = { (f32)x, (f32)y };
            v2f d         = pixel_v2f - origin;

            f32 edge_0 = vec_dot(d, -vec_perp(x_axis));
            f32 edge_1 = vec_dot(d - x_axis, -vec_perp(y_axis));
            f32 edge_2 = vec_dot(d - y_axis, vec_perp(y_axis));
            f32 edge_3 = vec_dot(d - x_axis - y_axis, vec_perp(x_axis));

            if (edge_0 < 0 && edge_1 < 0 && edge_2 < 0 && edge_3 < 0) {
                // start with the color
                if (albedo) {
                    v2f screen_space_uv = { inv_max_width * (f32)x, inv_max_height * (f32)y };

                    f32 u = x_inv_len_sq * vec_dot(d, x_axis);
                    f32 v = y_inv_len_sq * vec_dot(d, y_axis);
                    TOM_ASSERT((u >= 0.0f) && (u <= 1.0f + EPS_F32));
                    TOM_ASSERT((v >= 0.0f) && (v <= 1.0f + EPS_F32));
                    f32 tx = 1.0f + (u * (f32)(albedo->width - 3));
                    f32 ty = 1.0f + (v * (f32)(albedo->height - 3));

                    i32 tex_x = (i32)tx;
                    i32 tex_y = (i32)ty;

                    f32 fx = tx - (f32)tex_x;
                    f32 fy = ty - (f32)tex_y;

                    TOM_ASSERT((tex_x >= 0) && (tex_x < albedo->width));
                    TOM_ASSERT((tex_y >= 0) && (tex_y < albedo->height));

                    // set the texel to the albedo map sample
                    BilinearSample alb_bi_samp = get_bilinear_sample(albedo, tex_x, tex_y);
                    v4f texel                  = bilinear_sample_blend(alb_bi_samp, fx, fy, true);

                    // blending with input color
                    texel = vec_hadamard(texel, color);

                    if (normal) {
                        // NOTE: just to make it simpler for now
                        TOM_ASSERT(normal->height == albedo->height);
                        u32 nrm_texel_sz = texture_get_texel_size(normal->type);
                        u32 nrm_pitch =
                            texture_get_pitch(albedo);  // TODO: different sized normal maps
                        byt *nrm_texel_ptr =
                            (byt *)normal->buf + tex_y * nrm_pitch + tex_x * nrm_texel_sz;

                        // dereference the pointer for the u32 color, convert to normalized v4f,
                        v4f nrm_texel_0 = color_u32_to_v4f(*(u32 *)nrm_texel_ptr);
                        v4f nrm_texel_1 = color_u32_to_v4f(*(u32 *)(nrm_texel_ptr + nrm_texel_sz));
                        v4f nrm_texel_2 = color_u32_to_v4f(*(u32 *)(nrm_texel_ptr + nrm_pitch));
                        v4f nrm_texel_3 =
                            color_u32_to_v4f(*(u32 *)(nrm_texel_ptr + nrm_pitch + nrm_texel_sz));

                        v4f nrm_texel =
                            bi_lerp(nrm_texel_0, nrm_texel_1, nrm_texel_2, nrm_texel_3, fx, fy);

                        nrm_texel = bias_nrm(nrm_texel);

                        nrm_texel.xy = nrm_texel.x * nx_axis + nrm_texel.y * ny_axis;
                        nrm_texel.z *= nz_sca;

                        nrm_texel.xyz  = vec_normalize(nrm_texel.xyz);
                        v3f eye_v      = { 0.0f, 0.0f, 1.0f };
                        v3f bounce_dir = 2.0f * nrm_texel.z * nrm_texel.xyz;
                        bounce_dir.z -= 1.0f;

                        EnviromentMap *far_map = nullptr;
                        f32 dist_map_z         = 1.0f;
                        f32 t_far_map          = 0.0f;
                        f32 t_env_map          = bounce_dir.y;
                        if (t_env_map < -0.5f) {
                            far_map      = bottom;
                            t_far_map    = -1.0f - t_env_map * 2.0f;
                            dist_map_z   = -dist_map_z;
                            bounce_dir.y = -bounce_dir.y;
                        } else if (t_env_map > 0.5f) {
                            far_map   = top;
                            t_far_map = (t_env_map - 0.5f) * 2.0f;
                        }

                        v3f light_col = {};
                        // sample_env_map(screen_space_uv, nrm_texel.rgb, nrm_texel.a, middle);
                        if (far_map) {
                            v3f far_map_col = sample_env_map(screen_space_uv, bounce_dir,
                                                             nrm_texel.a, far_map, dist_map_z);
                            light_col       = lerp(light_col, far_map_col, t_far_map);
                        }

                        // texel.rgb = vec_hadamard(texel.rgb, light_col);
                        texel.rgb += texel.a * light_col;
                        texel = clamp_01(texel);
                    }

                    v4f dest_col  = color_u32_to_v4f(*(u32 *)pixel_ptr);
                    v4f blend_col = lerp(dest_col, texel, texel.a);
                    blend_col.a   = 1.0f;
                    *pixel_ptr    = v4f_to_color_u32(blend_col).rgba;
                }
            }
            ++pixel_ptr;
        }
        row += buf_pitch;
    }
}

void draw_rect_slowly(Texture *buffer, m3 model, Texture *albedo, Texture *normal,
                      EnviromentMap *top, EnviromentMap *middle, EnviromentMap *bottom, v4f color)
{
    m3 world   = m3_identity(g_meters_to_pixels);
    m3 view    = world * model;
    v2f x_axis = view.r[0].xy;
    v2f y_axis = view.r[1].xy;
    // v2f origin = m3_get_p(view);
    v2f origin = m3_get_p(model) - 0.5f * x_axis - 0.5f * y_axis;

    draw_rect_slowly(buffer, origin, x_axis, y_axis, albedo, normal, top, middle, bottom, color);
}

void draw_texture(Texture *buffer, Texture *tex, v2f pos)
{
    r2i rect;
    rect.x0 = round_f32_to_i32(pos.x - ((f32)tex->width) / 2.0f);
    rect.y0 = round_f32_to_i32(pos.y - ((f32)tex->height) / 2.0f);
    rect.x1 = round_f32_to_i32(pos.x + ((f32)tex->width) / 2.0f);
    rect.y1 = round_f32_to_i32(pos.y + ((f32)tex->height) / 2.0f);

    i32 x_offset_left = 0, x_offset_right = 0, y_offset = 0;

    if (rect.y0 < 0) {
        y_offset = rect.y0 * -1;
        rect.y0  = 0;
    }
    if (rect.x0 < 0) {
        x_offset_left = rect.x0 * -1;
        rect.x0       = 0;
    }
    if (rect.x1 > buffer->width) {
        x_offset_right = rect.x1 - buffer->width;
        rect.x1        = buffer->width;
    }
    if (rect.y1 > buffer->height) {
        rect.y1 = buffer->height;
    }

    TOM_ASSERT(buffer->type == Texture::Type::R8G8B8A8);
    u32 pitch   = texture_get_pitch(buffer);
    u32 *source = (u32 *)tex->buf + (y_offset * tex->width);
    byt *row =
        (byt *)buffer->buf + rect.x0 * texture_get_texel_size(buffer->type) + rect.y0 * pitch;

    for (i32 y = rect.y0; y < rect.y1; ++y) {
        u32 *dest = (u32 *)row;
        source += x_offset_left;
        for (i32 x = rect.x0; x < rect.x1; ++x) {
            v4f dest_col  = color_u32_to_v4f(*(u32 *)dest);
            v4f src_col   = color_u32_to_v4f(*(u32 *)source);
            v4f blend_col = lerp(dest_col, src_col, src_col.a);
            blend_col.a   = 1.0f;
            *dest         = v4f_to_color_u32(blend_col).rgba;

            ++dest, ++source;
        }
        source += x_offset_right;
        row += pitch;
    }
}

RenderGroup *alloc_render_group(Arena *arena, u32 max_pushbuffer_sz, f32 meters_to_pixels,
                                Camera cam)
{
    auto result               = push_struct<RenderGroup>(arena);
    result->pushbuffer_base   = (byt *)push_size(arena, max_pushbuffer_sz);
    result->max_pushbuffer_sz = max_pushbuffer_sz;
    result->meters_to_pixels  = meters_to_pixels;
    result->pushbuffer_sz     = 0;
    result->cam               = cam;

    return result;
}

// NOTE: fucking stupid but I just wanted to get it done quick
internal u32 get_entry_size(RenderGroupEntryType type)
{
    switch (type) {
        case RenderGroupEntryType::clear: return sizeof(RenderGroupEntryClear);
        case RenderGroupEntryType::rect: return sizeof(RenderGroupEntryRect);
        case RenderGroupEntryType::rect_outline: return sizeof(RenderGroupEntryRectOutline);
        case RenderGroupEntryType::texture: return sizeof(RenderGroupEntryTexture);
        case RenderGroupEntryType::coord_system: return sizeof(RenderGroupEntryCoordSystem);
        default: INVALID_CODE_PATH;
    }

    return U32_MAX;
}

internal void *push_render_element(RenderGroup *group, RenderGroupEntryType type)
{
    void *result = nullptr;
    u32 entry_sz = sizeof(RenderGroupEntryHeader) + get_entry_size(type);

    if (entry_sz != U32_MAX && group->pushbuffer_sz + entry_sz < group->max_pushbuffer_sz) {
        auto header  = (RenderGroupEntryHeader *)(group->pushbuffer_base + group->pushbuffer_sz);
        header->type = type;
        result       = header + 1;
        group->pushbuffer_sz += entry_sz;
    } else {
        INVALID_CODE_PATH;
    }

    return result;
}

void push_clear(RenderGroup *group, Color_u32 color)
{
    auto entry = (RenderGroupEntryClear *)push_render_element(group, RenderGroupEntryType::clear);
    TOM_ASSERT(entry);
    entry->clear_color = color;
}

void push_texture(RenderGroup *group, RenderBasis basis, Texture *texture, v2f offset, m3 model)
{
    auto entry =
        (RenderGroupEntryTexture *)push_render_element(group, RenderGroupEntryType::texture);
    TOM_ASSERT(entry);
    entry->basis   = basis;
    entry->offset  = offset;
    entry->texture = texture;
    entry->model   = model;
}

void push_rect(RenderGroup *group, v2f pos, v2f dims, Color_u32 color)
{
    auto entry = (RenderGroupEntryRect *)push_render_element(group, RenderGroupEntryType::rect);
    TOM_ASSERT(entry);
    entry->pos   = pos;
    entry->dims  = dims;
    entry->color = color;
}

void push_rect_outline(RenderGroup *group, v2f pos, v2f dims, i32 thickness, Color_u32 color)
{
    auto entry = (RenderGroupEntryRectOutline *)push_render_element(
        group, RenderGroupEntryType::rect_outline);
    TOM_ASSERT(entry);
    entry->pos       = pos;
    entry->dims      = dims;
    entry->thickness = thickness;
    entry->color     = color;
}

void push_coord_system(RenderGroup *group, m3 model, Texture *albedo, Texture *normal,
                       EnviromentMap *top, EnviromentMap *middle, EnviromentMap *bottom,
                       Color_u32 color)
{
    auto entry = (RenderGroupEntryCoordSystem *)push_render_element(
        group, RenderGroupEntryType::coord_system);
    TOM_ASSERT(entry);
    entry->model  = model;
    entry->color  = color;
    entry->albedo = albedo;
    entry->normal = normal;
    entry->top    = top;
    entry->middle = middle;
    entry->bottom = bottom;
}

internal v2f get_screen_space_coord(RenderGroup *group, Texture *back_buffer, v2f pos)
{
    r2f cam_rc              = rect_init_dims(group->cam.pos, group->cam.dims);
    v2i buf_mid             = { back_buffer->width / 2, back_buffer->height / 2 };
    r2f screen_space_cam_rc = {
        (f32)buf_mid.x - abs_f32(cam_rc.x0 - group->cam.pos.x) * g_meters_to_pixels,
        (f32)buf_mid.y - abs_f32(cam_rc.y0 - group->cam.pos.y) * g_meters_to_pixels,
        (f32)buf_mid.x + abs_f32(group->cam.pos.x - cam_rc.x1) * g_meters_to_pixels,
        (f32)buf_mid.y + abs_f32(group->cam.pos.y - cam_rc.y1) * g_meters_to_pixels
    };
    v2f result;
    result.x = ((pos.x - cam_rc.x0) * g_meters_to_pixels) + screen_space_cam_rc.x0;
    result.y = screen_space_cam_rc.y1 - ((pos.y - cam_rc.y0) * g_meters_to_pixels);
    result.x = ((pos.x - cam_rc.x0) * g_meters_to_pixels) + screen_space_cam_rc.x0;

    return result;
}

void draw_render_group(RenderGroup *group, Texture *back_buffer, r2i clip, bool even)
{
    for (u32 base_address = 0; base_address < group->pushbuffer_sz;) {
        auto header   = (RenderGroupEntryHeader *)(group->pushbuffer_base + base_address);
        auto data     = (void *)(header + 1);
        auto inc_base = [&](auto *entry) { base_address += sizeof(*header) + sizeof(*entry); };

        // NOTE: don't forget to increment the base address!!!
        switch (header->type) {
            case RenderGroupEntryType::clear: {
                auto entry = (RenderGroupEntryClear *)data;
                clear_color(back_buffer, entry->clear_color);
                inc_base(entry);
            } break;
            case RenderGroupEntryType::rect: {
                auto entry = (RenderGroupEntryRect *)data;
                NOT_IMPLEMENTED;
                inc_base(entry);
            } break;
            case RenderGroupEntryType::rect_outline: {
                auto entry = (RenderGroupEntryRectOutline *)data;
                NOT_IMPLEMENTED;
                inc_base(entry);
            } break;
            case RenderGroupEntryType::texture: {
                auto entry = (RenderGroupEntryTexture *)data;
                TOM_ASSERT(entry->texture);
                m3 world   = m3_identity(g_meters_to_pixels);
                m3 view    = entry->model * world;
                v2f x_axis = view.r[0].xy;
                v2f y_axis = view.r[1].xy;
                // v2f y_axis = vec_perp(x_axis);
                // v2f y_axis = vec_normalize(vec_perp(x_axis)) * vec_length(view.r[1].xy);

                // // v2f x_axis     = (200.0f + 20.0f * cos(angle)) * v2f { cos(angle),
                // sin(angle)
                // }; v2f x_axis = 200.0f * v2f { 1.0f, 0.0f }; v2f y_axis = vec_perp(x_axis);
                // // game->debug_origin.x = sin(app->

                v2f pos    = m3_get_p(entry->model);
                v2f coord  = get_screen_space_coord(group, back_buffer, pos);
                v2f origin = coord - 0.5f * x_axis - 0.5f * y_axis;

                // draw_rect_128(back_buffer, origin, x_axis, y_axis, entry->texture);
                draw_texture_256(back_buffer, origin, x_axis, y_axis, entry->texture, clip, even);

#if 0
                draw_square(back_buffer, origin, 3, color_u32(yellow));
                draw_square(back_buffer, origin + x_axis, 3, color_u32(yellow));
                draw_square(back_buffer, origin + y_axis, 3, color_u32(yellow));
                draw_square(back_buffer, origin + x_axis + y_axis, 3, color_u32(yellow));
                draw_square(back_buffer, origin + 0.5f * x_axis + 0.5f * y_axis, 3,
                            color_u32(orange));
#endif
                inc_base(entry);
            } break;
            case RenderGroupEntryType::coord_system: {
                auto entry = (RenderGroupEntryCoordSystem *)data;
                m3 world   = m3_identity(g_meters_to_pixels);
                // m3 view    = world * entry->model;
                m3 view    = entry->model * world;
                v2f x_axis = view.r[0].xy;
                v2f y_axis = view.r[1].xy;
                // v2f origin = m3_get_p(view);
                v2f origin = m3_get_p(view) - 0.5f * x_axis - 0.5f * y_axis;
                draw_rect_slowly(back_buffer, origin, x_axis, y_axis, entry->albedo, entry->normal,
                                 entry->top, entry->middle, entry->bottom);

                // draw_square(back_buffer, origin, 3, color_u32(yellow));
                // draw_square(back_buffer, origin + x_axis, 3, color_u32(yellow));
                // draw_square(back_buffer, origin + y_axis, 3, color_u32(yellow));
                // draw_square(back_buffer, origin + x_axis + y_axis, 3, color_u32(yellow));
                // draw_square(back_buffer, origin + 0.5f * x_axis + 0.5f * y_axis, 3,
                //             color_u32(orange));
                inc_base(entry);
            } break;
            default: INVALID_CODE_PATH;
        }
    }
}

void do_tile_render_work(void *data)
{
    auto work = (TileRenderWork *)data;
    draw_render_group(work->render_group, work->target, work->clip, true);
    draw_render_group(work->render_group, work->target, work->clip, false);
}

void draw_render_group_tiled(WorkQueue *queue, RenderGroup *group, Texture *back_buffer)
{
    i32 tile_cnt_x = (back_buffer->width + group->tile_r - 1) / group->tile_r;
    i32 tile_cnt_y = (back_buffer->height + group->tile_r - 1) / group->tile_r;

    // TODO: arena/bump allocator?
    auto work_array = plat_malloc<TileRenderWork>(tile_cnt_x * tile_cnt_y);

    u32 work_cnt = 0;
    for (i32 tile_x = 0; tile_x < tile_cnt_x; ++tile_x) {
        for (i32 tile_y = 0; tile_y < tile_cnt_y; ++tile_y) {
            TileRenderWork *work = work_array + work_cnt++;
            r2i clip;
            clip.x0 = tile_x * group->tile_r;
            clip.y0 = tile_y * group->tile_r;
            clip.x1 = min(back_buffer->width, clip.x0 + group->tile_r);
            clip.y1 = min(back_buffer->height, clip.y0 + group->tile_r);
            // clip = rect_shrink(clip, 5);

            work->render_group = group;
            work->target       = back_buffer;
            work->clip         = clip;

            work_queue_add_entry(queue, do_tile_render_work, work);
        }
    }

    work_queue_complete_all_work(queue);
}

}  // namespace tom
