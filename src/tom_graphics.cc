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
            *pixel++ = v4f_to_color_u32(blend_col).rgba;
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

fn v3f sample_env_map(v2f screen_space_uv, v3f normal, f32 rough, EnviromentMap *map)
{
#if 1

    u32 lod_i = (u32)(rough * (f32)(CountOf(map->lod) - 1) + 0.5f);
    Assert(lod_i < CountOf(map->lod));

    Texture *lod = &map->lod[lod_i];
    f32 tx       = lod->width / 2 + normal.x * (f32)(lod->width / 2);
    f32 ty       = lod->height / 2 + normal.y * (f32)(lod->height / 2);

    i32 x = (i32)tx;
    i32 y = (i32)ty;

    f32 fx = tx - (f32)x;
    f32 fy = ty - (f32)y;

    Assert(x >= 0 && x < lod->width);
    Assert(y >= 0 && y < lod->height);

    BilinearSample samp = get_bilinear_sample(lod, x, y);

    v3f result = bilinear_sample_blend(samp, fx, fy).xyz;

    return result;

#else
    return normal;
#endif
}

// NOTE: don't use this for realz
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
    u32 pitch = texture_get_pitch(buffer);
    byt *row  = (byt *)buffer->buf + rc.x0 * texture_get_texel_size(buffer->type) + rc.y0 * pitch;
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
                    Assert((u >= 0.0f) && (u <= 1.0f + eps_f32));
                    Assert((v >= 0.0f) && (v <= 1.0f + eps_f32));
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

                        // derefence the pointer for the u32 color, convert to normalized v4f,
                        v4f nrm_texel_0 = color_u32_to_v4f(*(u32 *)nrm_texel_ptr);
                        v4f nrm_texel_1 = color_u32_to_v4f(*(u32 *)(nrm_texel_ptr + nrm_texel_sz));
                        v4f nrm_texel_2 = color_u32_to_v4f(*(u32 *)(nrm_texel_ptr + nrm_pitch));
                        v4f nrm_texel_3 =
                            color_u32_to_v4f(*(u32 *)(nrm_texel_ptr + nrm_pitch + nrm_texel_sz));

                        v4f nrm_texel =
                            bi_lerp(nrm_texel_0, nrm_texel_1, nrm_texel_2, nrm_texel_3, fx, fy);

                        nrm_texel     = bias_nrm(nrm_texel);
                        nrm_texel.xyz = vec_normalize(nrm_texel.xyz);

                        EnviromentMap *far_map = nullptr;
                        f32 t_far_map          = 0.0f;
                        f32 t_env_map          = nrm_texel.y;
                        if (t_env_map < -0.5f) {
                            far_map   = bottom;
                            t_far_map = -1.0f - t_env_map * 2.0f;
                        } else if (t_env_map > 0.5f) {
                            far_map   = top;
                            t_far_map = (t_env_map - 0.5f) * 2.0f;
                        }

                        v3f light_col =
                            sample_env_map(screen_space_uv, nrm_texel.rgb, nrm_texel.a, middle);
                        if (far_map) {
                            v3f far_map_col = sample_env_map(screen_space_uv, nrm_texel.rgb,
                                                             nrm_texel.a, far_map);
                            light_col       = lerp(light_col, far_map_col, t_far_map);
                        }

                        texel.rgb = vec_hadamard(texel.rgb, light_col);
                        texel     = clamp_01(texel);
                    }
                    v4f dest_col  = color_u32_to_v4f(*(u32 *)pixel_ptr);
                    v4f blend_col = lerp(dest_col, texel, texel.a);
                    blend_col.a   = 1.0f;
                    *pixel_ptr    = v4f_to_color_u32(blend_col).rgba;
                }
            }
            ++pixel_ptr;
        }
        row += pitch;
    }
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

fn void push_texture(RenderGroup *group, RenderBasis basis, Texture *texture, v2f offset)
{
    auto entry =
        (RenderGroupEntryTexture *)push_render_element(group, RenderGroupEntryType::texture);
    Assert(entry);
    entry->basis   = basis;
    entry->offset  = offset;
    entry->texture = texture;
}

fn void push_rect(RenderGroup *group, RenderBasis basis, v2f dims, Color_u32 color)
{
    auto entry = (RenderGroupEntryRect *)push_render_element(group, RenderGroupEntryType::rect);
    Assert(entry);
    entry->basis = basis;
    entry->dims  = dims;
    entry->color = color;
}

fn void push_rect_outline(RenderGroup *group, RenderBasis basis, v2f dims, i32 thickness,
                          Color_u32 color)
{
    auto entry = (RenderGroupEntryRectOutline *)push_render_element(
        group, RenderGroupEntryType::rect_outline);
    Assert(entry);
    entry->basis     = basis;
    entry->dims      = dims;
    entry->thickness = thickness;
    entry->color     = color;
}

fn void push_coord_system(RenderGroup *group, v2f origin, v2f x_axis, v2f y_axis, Texture *albedo,
                          Texture *normal, EnviromentMap *top, EnviromentMap *middle,
                          EnviromentMap *bottom, Color_u32 color = { 0xffffffff })
{
    auto entry = (RenderGroupEntryCoordSystem *)push_render_element(
        group, RenderGroupEntryType::coord_system);
    Assert(entry);
    entry->origin = origin;
    entry->x_axis = x_axis;
    entry->y_axis = y_axis;
    entry->color  = color;
    entry->albedo = albedo;
    entry->normal = normal;
    entry->top    = top;
    entry->middle = middle;
    entry->bottom = bottom;
}

fn v2f get_screen_space_coord(RenderGroup *group, Texture *back_buffer, RenderBasis basis)
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
    result.x = ((basis.pos.x - cam_rc.x0) * g_meters_to_pixels) + screen_space_cam_rc.x0;
    result.y = screen_space_cam_rc.y1 - ((basis.pos.y - cam_rc.y0) * g_meters_to_pixels);
    result.x = ((basis.pos.x - cam_rc.x0) * g_meters_to_pixels) + screen_space_cam_rc.x0;
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
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->basis);
                r2f rc     = rect_init_dims(coord, entry->dims * group->meters_to_pixels);
                draw_rect(back_buffer, rc, entry->color);
                inc_base(entry);
            } break;
            case RenderGroupEntryType::rect_outline: {
                auto entry = (RenderGroupEntryRectOutline *)data;
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->basis);
                r2f rc     = rect_init_dims(coord, entry->dims * group->meters_to_pixels);
                draw_rect_outline(back_buffer, rc, entry->thickness, entry->color);
                inc_base(entry);
            } break;
            case RenderGroupEntryType::texture: {
                auto entry = (RenderGroupEntryTexture *)data;
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->basis);
                coord += entry->offset;
                Assert(entry->texture);
                draw_texture(back_buffer, entry->texture, coord);
                inc_base(entry);
            } break;
            case RenderGroupEntryType::coord_system: {
                auto entry = (RenderGroupEntryCoordSystem *)data;
                draw_rect_slowly(back_buffer, entry->origin, entry->x_axis, entry->y_axis,
                                 entry->albedo, entry->normal, entry->top, entry->middle,
                                 entry->bottom);
                entry->albedo, entry->normal, entry->top, entry->middle,
                    draw_square(back_buffer, entry->origin, 3, color_u32(yellow));
                draw_square(back_buffer, entry->origin + entry->x_axis, 3, color_u32(yellow));
                draw_square(back_buffer, entry->origin + entry->y_axis, 3, color_u32(yellow));
                draw_square(back_buffer, entry->origin + entry->x_axis + entry->y_axis, 3,
                            color_u32(yellow));
                draw_square(back_buffer,
                            entry->origin + 0.5f * entry->x_axis + 0.5f * entry->y_axis, 3,
                            color_u32(orange));
                entry->origin + 0.5f * entry->x_axis + 0.5f * entry->y_axis, 3, inc_base(entry);
            } break;
            default: InvalidCodePath;
        }
    }
}
}  // namespace tom
