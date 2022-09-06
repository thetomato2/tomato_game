namespace tom
{

fn void clear_color(Texture *buffer, Color_u32 color)
{
    Assert(buffer->type == Texture::Type::R8G8B8A8);
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

fn void draw_rect(Texture *buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, Color_u32 color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    Assert(buffer->type == Texture::Type::R8G8B8A8);
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

fn void draw_rect(Texture *buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, v3f color)
{
    draw_rect(buffer, min_x, min_y, max_x, max_y, v3f_to_color(color));
}

fn void draw_rect(Texture *buffer, r2i rect, Color_u32 color)
{
    draw_rect(buffer, rect.x0, rect.y0, rect.x1, rect.y1, color);
}
fn void draw_rect(Texture *buffer, r2f rect, Color_u32 color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, color);
}

fn void draw_rect(Texture *buffer, r2i rect, v3f color)
{
    draw_rect(buffer, rect.x0, rect.y0, rect.x1, rect.y1, v3f_to_color(color));
}

fn void draw_rect(Texture *buffer, r2u rect, v3f color)
{
    draw_rect(buffer, (i32)rect.x0, (i32)rect.y0, (i32)rect.x1, (i32)rect.y1, v3f_to_color(color));
}

fn void draw_rect(Texture *buffer, r2u rect, Color_u32 color)
{
    draw_rect(buffer, (i32)rect.x0, (i32)rect.y0, (i32)rect.x1, (i32)rect.y1, color);
}

fn void draw_rect(Texture *buffer, r2f rect, v3f color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, v3f_to_color(color));
}

fn void draw_square(Texture *buffer, v2f pos, f32 radius, Color_u32 color)
{
    r2i rc = rect_f32_to_i32(rect_init_square(pos, radius));
    draw_rect(buffer, rc.x0, rc.y0, rc.x1, rc.y1, color);
}

fn void draw_rect_outline(Texture *buffer, i32 min_x, i32 min_y, f32 max_x, f32 max_y,
                          i32 thickness, Color_u32 color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    Assert(buffer->type == Texture::Type::R8G8B8A8);
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

fn void draw_rect_outline(Texture *buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y,
                          i32 thickness, v3f color)
{
    draw_rect_outline(buffer, min_x, min_y, max_x, max_y, thickness, v3f_to_color(color));
}

fn void draw_rect_outline(Texture *buffer, r2i rect, i32 thickness, Color_u32 color)
{
    draw_rect_outline(buffer, rect.x0, rect.y0, rect.x1, rect.y1, thickness, color);
}
fn void draw_rect_outline(Texture *buffer, r2f rect, i32 thickness, Color_u32 color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect_outline(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, thickness, color);
}

fn void draw_rect_outline(Texture *buffer, r2i rect, i32 thickness, v3f color)
{
    draw_rect_outline(buffer, rect.x0, rect.y0, rect.x1, rect.y1, thickness, v3f_to_color(color));
}

fn void draw_rect_outline(Texture *buffer, r2f rect, i32 thickness, v3f color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect_outline(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, thickness,
                      v3f_to_color(color));
}

fn v4f bias_nrm(v4f nrm)
{
    v4f result;

    result.x = -1.0f + 2.0f * nrm.x;
    result.y = -1.0f + 2.0f * nrm.y;
    result.z = -1.0f + 2.0f * nrm.z;
    result.w = nrm.w;

    return result;
}

fn v3f sample_env_map(v2f screen_space_uv, v3f samp_dir, f32 rough, EnviromentMap *map,
                      f32 dist_z = 1.0f)
{
    u32 lod_i = (u32)(rough * (f32)(CountOf(map->lod) - 1) + 0.5f);
    Assert(lod_i < CountOf(map->lod));

    Texture *lod = &map->lod[lod_i];

    Assert(samp_dir.y > 0.0f);
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

    Assert(x >= 0 && x < lod->width);
    Assert(y >= 0 && y < lod->height);

    BilinearSample samp = get_bilinear_sample(lod, x, y);

    v3f result = bilinear_sample_blend(samp, fx, fy).xyz;

    return result;
}

fn void draw_rect_quickly(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                          v4f color = { 1.0f, 1.0f, 1.0f, 1.0f })
{
    Assert(albedo);
    BeginTimedBlock(DrawRect);

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
    __m128 One_255_m128   = _mm_set1_ps(255.0f);
    __m128 zero           = _mm_set1_ps(0.0f);
    __m128 nx_axis_x_m128 = _mm_set1_ps(nx_axis.x);
    __m128 nx_axis_y_m128 = _mm_set1_ps(nx_axis.y);
    __m128 ny_axis_x_m128 = _mm_set1_ps(ny_axis.x);
    __m128 ny_axis_y_m128 = _mm_set1_ps(ny_axis.y);
    __m128 origin_x_m128  = _mm_set1_ps(origin.x);
    __m128 origin_y_m128  = _mm_set1_ps(origin.y);

    u32 texel_sz        = texture_get_texel_size(albedo->type);
    u32 alb_pitch       = texture_get_pitch(albedo);
    u32 alb_pitch_texel = alb_pitch / texel_sz;

    u32 buf_pitch = texture_get_pitch(buffer);
    Assert(buffer->type == Texture::Type::R8G8B8A8);
    byt *row =
        (byt *)buffer->buf + rc.x0 * texture_get_texel_size(buffer->type) + rc.y0 * buf_pitch;

    BeginTimedBlock(ProcessPixel);
    for (i32 y = rc.y0; y < rc.y1; ++y) {
        u32 *pixel_ptr = (u32 *)row;
        for (i32 x = rc.x0; x < rc.x1; x += 4) {
            __m128 t0_r = _mm_set1_ps(0.0f);
            __m128 t0_g = _mm_set1_ps(0.0f);
            __m128 t0_b = _mm_set1_ps(0.0f);
            __m128 t0_a = _mm_set1_ps(0.0f);

            __m128 t1_r = _mm_set1_ps(0.0f);
            __m128 t1_g = _mm_set1_ps(0.0f);
            __m128 t1_b = _mm_set1_ps(0.0f);
            __m128 t1_a = _mm_set1_ps(0.0f);

            __m128 t2_r = _mm_set1_ps(0.0f);
            __m128 t2_g = _mm_set1_ps(0.0f);
            __m128 t2_b = _mm_set1_ps(0.0f);
            __m128 t2_a = _mm_set1_ps(0.0f);

            __m128 t3_r = _mm_set1_ps(0.0f);
            __m128 t3_g = _mm_set1_ps(0.0f);
            __m128 t3_b = _mm_set1_ps(0.0f);
            __m128 t3_a = _mm_set1_ps(0.0f);

            __m128 fx = _mm_set1_ps(0.0f);
            __m128 fy = _mm_set1_ps(0.0f);

            __m128 dest_r = _mm_set1_ps(0.0f);
            __m128 dest_g = _mm_set1_ps(0.0f);
            __m128 dest_b = _mm_set1_ps(0.0f);
            __m128 dest_a = _mm_set1_ps(0.0f);

            __m128 blend_r = _mm_set1_ps(0.0f);
            __m128 blend_g = _mm_set1_ps(0.0f);
            __m128 blend_b = _mm_set1_ps(0.0f);
            __m128 blend_a = _mm_set1_ps(0.0f);

            bool should_fill[4];

#define mmSquare(a) _mm_mul_ps(a, a)
#define M(a, i)     ((float *)&(a))[i]

            __m128 pixel_px = _mm_set_ps((f32)(x + 3), (f32)(x + 2), (f32)(x + 1), (f32)(x + 0));
            __m128 pixel_py = _mm_set1_ps((f32)y);

            __m128 dx = _mm_sub_ps(pixel_px, origin_x_m128);
            __m128 dy = _mm_sub_ps(pixel_py, origin_y_m128);

            __m128 u = _mm_add_ps(_mm_mul_ps(dx, nx_axis_x_m128), _mm_mul_ps(dy, nx_axis_y_m128));
            __m128 v = _mm_add_ps(_mm_mul_ps(dx, ny_axis_x_m128), _mm_mul_ps(dy, ny_axis_y_m128));

            for (i32 i = 0; i < 4; ++i) {
                should_fill[i] =
                    M(u, i) >= 0.0f && M(u, i) <= 1.0f && M(v, i) >= 0.0f && M(v, i) <= 1.0f;

                if (should_fill[i]) {
                    // use the uv coords to get a texel out of the texture
                    f32 tx = M(u, i) * (f32)(albedo->width - 2);
                    f32 ty = M(v, i) * (f32)(albedo->height - 2);

                    i32 tex_x = (i32)tx;
                    i32 tex_y = (i32)ty;

                    // this used for blending
                    M(fx, i) = tx - (f32)tex_x;
                    M(fy, i) = ty - (f32)tex_y;

                    Assert((tex_x >= 0) && (tex_x < albedo->width));
                    Assert((tex_y >= 0) && (tex_y < albedo->height));

                    // get 4 texels in a square to use for blending
                    auto texel_ptr = (u32 *)albedo->buf + tex_y * alb_pitch_texel + tex_x * 1;
                    u32 samp_a     = *texel_ptr;
                    u32 samp_b     = *(texel_ptr + 1);
                    u32 samp_c     = *(texel_ptr + alb_pitch_texel);
                    u32 samp_d     = *(texel_ptr + alb_pitch_texel + 1);

                    // unpack
                    M(t0_r, i) = (f32)((samp_a >> 0) & 0xff) / 255.0f;
                    M(t0_g, i) = (f32)((samp_a >> 8) & 0xff) / 255.0f;
                    M(t0_b, i) = (f32)((samp_a >> 16) & 0xff) / 255.0f;
                    M(t0_a, i) = (f32)((samp_a >> 24) & 0xff) / 255.0f;

                    M(t1_r, i) = (f32)((samp_b >> 0) & 0xff) / 255.0f;
                    M(t1_g, i) = (f32)((samp_b >> 8) & 0xff) / 255.0f;
                    M(t1_b, i) = (f32)((samp_b >> 16) & 0xff) / 255.0f;
                    M(t1_a, i) = (f32)((samp_b >> 24) & 0xff) / 255.0f;

                    M(t2_r, i) = (f32)((samp_c >> 0) & 0xff) / 255.0f;
                    M(t2_g, i) = (f32)((samp_c >> 8) & 0xff) / 255.0f;
                    M(t2_b, i) = (f32)((samp_c >> 16) & 0xff) / 255.0f;
                    M(t2_a, i) = (f32)((samp_c >> 24) & 0xff) / 255.0f;

                    M(t3_r, i) = (f32)((samp_d >> 0) & 0xff) / 255.0f;
                    M(t3_g, i) = (f32)((samp_d >> 8) & 0xff) / 255.0f;
                    M(t3_b, i) = (f32)((samp_d >> 16) & 0xff) / 255.0f;
                    M(t3_a, i) = (f32)((samp_d >> 24) & 0xff) / 255.0f;

                    // unpack the pixel to write to in the back buffer
                    M(dest_r, i) = (f32)((*pixel_ptr >> 0) & 0xff) / 255.0f;
                    M(dest_g, i) = (f32)((*pixel_ptr >> 8) & 0xff) / 255.0f;
                    M(dest_b, i) = (f32)((*pixel_ptr >> 16) & 0xff) / 255.0f;
                    M(dest_a, i) = (f32)((*pixel_ptr >> 24) & 0xff) / 255.0f;
                }
            }

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

            // linear blend the texel and the back buffer pixel
            __m128 ita = _mm_sub_ps(one_m128, texel_a);

            blend_r = _mm_add_ps(_mm_mul_ps(ita, dest_r), texel_r);
            blend_g = _mm_add_ps(_mm_mul_ps(ita, dest_g), texel_g);
            blend_b = _mm_add_ps(_mm_mul_ps(ita, dest_b), texel_b);
            blend_a    = _mm_add_ps(_mm_mul_ps(ita, dest_a), texel_a);
            // blend_a = _mm_set1_ps(1.0f);

            for (i32 i = 0; i < 4; ++i) {
                // pack the final color and write it to the back buffer
                if (should_fill[i]) {
                    *(pixel_ptr + i) = { (u32)(M(blend_r, i) * 255.0f) << 0 |
                                         (u32)(M(blend_g, i) * 255.0f) << 8 |
                                         (u32)(M(blend_b, i) * 255.0f) << 16 |
                                         (u32)(M(blend_a, i) * 255.0f) << 24 };

                    // *(pixel_ptr + i) = { (u32)(M(blend_r, i) + 0.5f) << 0 |
                    //                      (u32)(M(blend_g, i) + 0.5f) << 8 |
                    //                      (u32)(M(blend_b, i) + 0.5f) << 16 |
                    //                      (u32)(M(blend_a, i) + 0.5f) << 24 };
                }
            }
            pixel_ptr += 4;
        }
        row += buf_pitch;
    }
    EndTimedBlockCounted(ProcessPixel, (rc.x1 - rc.x0 + 1) * (rc.y1 - rc.y0 + 1));
    EndTimedBlock(DrawRect);
}

fn void draw_rect_slowly(Texture *buffer, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                         Texture *normal, EnviromentMap *top, EnviromentMap *middle,
                         EnviromentMap *bottom, v4f color = { 1.0f, 1.0f, 1.0f, 1.0f })
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

    Assert(buffer->type == Texture::Type::R8G8B8A8);
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
                    Assert((u >= 0.0f) && (u <= 1.0f + EPS_F32));
                    Assert((v >= 0.0f) && (v <= 1.0f + EPS_F32));
                    f32 tx = 1.0f + (u * (f32)(albedo->width - 3));
                    f32 ty = 1.0f + (v * (f32)(albedo->height - 3));

                    i32 tex_x = (i32)tx;
                    i32 tex_y = (i32)ty;

                    f32 fx = tx - (f32)tex_x;
                    f32 fy = ty - (f32)tex_y;

                    Assert((tex_x >= 0) && (tex_x < albedo->width));
                    Assert((tex_y >= 0) && (tex_y < albedo->height));

                    // set the texel to the albedo map sample
                    BilinearSample alb_bi_samp = get_bilinear_sample(albedo, tex_x, tex_y);
                    v4f texel                  = bilinear_sample_blend(alb_bi_samp, fx, fy, true);

                    // blending with input color
                    texel = vec_hadamard(texel, color);

                    if (normal) {
                        // NOTE: just to make it simpler for now
                        Assert(normal->height == albedo->height);
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

fn void draw_rect_slowly(Texture *buffer, m3 model, Texture *albedo, Texture *normal,
                         EnviromentMap *top, EnviromentMap *middle, EnviromentMap *bottom,
                         v4f color = { 1.0f, 1.0f, 1.0f, 1.0f })
{
    m3 world   = m3_identity(g_meters_to_pixels);
    m3 view    = world * model;
    v2f x_axis = view.r[0].xy;
    v2f y_axis = view.r[1].xy;
    // v2f origin = m3_get_p(view);
    v2f origin = m3_get_p(model) - 0.5f * x_axis - 0.5f * y_axis;

    draw_rect_slowly(buffer, origin, x_axis, y_axis, albedo, normal, top, middle, bottom, color);
}

fn void draw_texture(Texture *buffer, Texture *tex, v2f pos)
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

    Assert(buffer->type == Texture::Type::R8G8B8A8);
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

fn RenderGroup *alloc_render_group(Arena *arena, u32 max_pushbuffer_sz, f32 meters_to_pixels,
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
fn u32 get_entry_size(RenderGroupEntryType type)
{
    switch (type) {
        case RenderGroupEntryType::clear: return sizeof(RenderGroupEntryClear);
        case RenderGroupEntryType::rect: return sizeof(RenderGroupEntryRect);
        case RenderGroupEntryType::rect_outline: return sizeof(RenderGroupEntryRectOutline);
        case RenderGroupEntryType::texture: return sizeof(RenderGroupEntryTexture);
        case RenderGroupEntryType::coord_system: return sizeof(RenderGroupEntryCoordSystem);
        default: InvalidCodePath;
    }

    return U32_MAX;
}

fn void *push_render_element(RenderGroup *group, RenderGroupEntryType type)
{
    void *result = nullptr;
    u32 entry_sz = sizeof(RenderGroupEntryHeader) + get_entry_size(type);

    if (entry_sz != U32_MAX && group->pushbuffer_sz + entry_sz < group->max_pushbuffer_sz) {
        auto header  = (RenderGroupEntryHeader *)(group->pushbuffer_base + group->pushbuffer_sz);
        header->type = type;
        result       = header + 1;
        group->pushbuffer_sz += entry_sz;
    } else {
        InvalidCodePath;
    }

    return result;
}

fn void push_clear(RenderGroup *group, Color_u32 color)
{
    auto entry = (RenderGroupEntryClear *)push_render_element(group, RenderGroupEntryType::clear);
    Assert(entry);
    entry->clear_color = color;
}

fn void push_texture(RenderGroup *group, RenderBasis basis, Texture *texture, v2f offset, m3 model)
{
    auto entry =
        (RenderGroupEntryTexture *)push_render_element(group, RenderGroupEntryType::texture);
    Assert(entry);
    entry->basis   = basis;
    entry->offset  = offset;
    entry->texture = texture;
    entry->model   = model;
}

fn void push_rect(RenderGroup *group, v2f pos, v2f dims, Color_u32 color)
{
    auto entry = (RenderGroupEntryRect *)push_render_element(group, RenderGroupEntryType::rect);
    Assert(entry);
    entry->pos   = pos;
    entry->dims  = dims;
    entry->color = color;
}

fn void push_rect_outline(RenderGroup *group, v2f pos, v2f dims, i32 thickness, Color_u32 color)
{
    auto entry = (RenderGroupEntryRectOutline *)push_render_element(
        group, RenderGroupEntryType::rect_outline);
    Assert(entry);
    entry->pos       = pos;
    entry->dims      = dims;
    entry->thickness = thickness;
    entry->color     = color;
}

fn void push_coord_system(RenderGroup *group, m3 model, Texture *albedo, Texture *normal,
                          EnviromentMap *top, EnviromentMap *middle, EnviromentMap *bottom,
                          Color_u32 color = { 0xffffffff })
{
    auto entry = (RenderGroupEntryCoordSystem *)push_render_element(
        group, RenderGroupEntryType::coord_system);
    Assert(entry);
    entry->model  = model;
    entry->color  = color;
    entry->albedo = albedo;
    entry->normal = normal;
    entry->top    = top;
    entry->middle = middle;
    entry->bottom = bottom;
}

fn v2f get_screen_space_coord(RenderGroup *group, Texture *back_buffer, v2f pos)
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

fn void draw_render_group(RenderGroup *group, Texture *back_buffer)
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
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->pos);
                r2f rc     = rect_init_dims(coord, entry->dims * group->meters_to_pixels);
                draw_rect(back_buffer, rc, entry->color);
                inc_base(entry);
            } break;
            case RenderGroupEntryType::rect_outline: {
                auto entry = (RenderGroupEntryRectOutline *)data;
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->pos);
                r2f rc     = rect_init_dims(coord, entry->dims * group->meters_to_pixels);
                draw_rect_outline(back_buffer, rc, entry->thickness, entry->color);
                inc_base(entry);
            } break;
            case RenderGroupEntryType::texture: {
                auto entry = (RenderGroupEntryTexture *)data;
                Assert(entry->texture);
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

                draw_rect_quickly(back_buffer, origin, x_axis, y_axis, entry->texture);

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

                draw_square(back_buffer, origin, 3, color_u32(yellow));
                draw_square(back_buffer, origin + x_axis, 3, color_u32(yellow));
                draw_square(back_buffer, origin + y_axis, 3, color_u32(yellow));
                draw_square(back_buffer, origin + x_axis + y_axis, 3, color_u32(yellow));
                draw_square(back_buffer, origin + 0.5f * x_axis + 0.5f * y_axis, 3,
                            color_u32(orange));
                inc_base(entry);
            } break;
            default: InvalidCodePath;
        }
    }
}

}  // namespace tom
