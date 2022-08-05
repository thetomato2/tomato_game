namespace tom
{

fn void clear_buffer(BackBuffer* buffer, Color color)
{
    byt* row = (byt*)buffer->buf;
    for (i32 y = 0; y < buffer->height; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = 0; x < buffer->width; ++x) {
            *pixel++ = color.rgba;
        }
        row += buffer->pitch;
    }
}

fn void draw_rect(BackBuffer* buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, Color color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    byt* row = (byt*)buffer->buf + min_x * buffer->byt_per_pix + min_y * buffer->pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = min_x; x < max_x; ++x) {
            Color dest_col, blended_col;
            dest_col.rgba = (u32)*pixel;
            blended_col.a = 0xff;

            f32 alpha = (f32)color.a / 255.0f;

            blended_col.r = (u8)((1.0f - alpha) * (f32)dest_col.r + alpha * (f32)color.r);
            blended_col.g = (u8)((1.0f - alpha) * (f32)dest_col.g + alpha * (f32)color.g);
            blended_col.b = (u8)((1.0f - alpha) * (f32)dest_col.b + alpha * (f32)color.b);

            *pixel++ = blended_col.rgba;
        }
        row += buffer->pitch;
    }
}

fn void draw_rect(BackBuffer* buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y, v3f color)
{
    draw_rect(buffer, min_x, min_y, max_x, max_y, v3_to_color(color));
}

fn void draw_rect(BackBuffer* buffer, r2i rect, Color color)
{
    draw_rect(buffer, rect.x0, rect.y0, rect.x1, rect.y1, color);
}
fn void draw_rect(BackBuffer* buffer, r2f rect, Color color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, color);
}

fn void draw_rect(BackBuffer* buffer, r2i rect, v3f color)
{
    draw_rect(buffer, rect.x0, rect.y0, rect.x1, rect.y1, v3_to_color(color));
}

fn void draw_rect(BackBuffer* buffer, r2f rect, v3f color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, v3_to_color(color));
}

fn void draw_square(BackBuffer* buffer, v2f pos, f32 radius, Color color)
{
    r2i rc = rect_f32_to_i32(rect_init_square(pos, radius));
    draw_rect(buffer, rc.x0, rc.y0, rc.x1, rc.y1, color);
}

fn void draw_rect_outline(BackBuffer* buffer, i32 min_x, i32 min_y, f32 max_x, f32 max_y,
                          i32 thickness, Color color)
{
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > buffer->width) max_x = buffer->width;
    if (max_y > buffer->height) max_y = buffer->height;

    byt* row = (byt*)buffer->buf + min_x * buffer->byt_per_pix + min_y * buffer->pitch;

    for (i32 y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)row;
        for (i32 x = min_x; x < max_x; ++x) {
            if (x <= min_x + thickness || x >= max_x - thickness - 1 || y <= min_y + thickness ||
                y >= max_y - thickness - 1) {
                *pixel = color.rgba;
            }
            ++pixel;
        }
        row += buffer->pitch;
    }
}

fn void draw_rect_outline(BackBuffer* buffer, i32 min_x, i32 min_y, i32 max_x, i32 max_y,
                          i32 thickness, v3f color)
{
    draw_rect_outline(buffer, min_x, min_y, max_x, max_y, thickness, v3_to_color(color));
}

fn void draw_rect_outline(BackBuffer* buffer, r2i rect, i32 thickness, Color color)
{
    draw_rect_outline(buffer, rect.x0, rect.y0, rect.x1, rect.y1, thickness, color);
}
fn void draw_rect_outline(BackBuffer* buffer, r2f rect, i32 thickness, Color color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect_outline(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, thickness, color);
}

fn void draw_rect_outline(BackBuffer* buffer, r2i rect, i32 thickness, v3f color)
{
    draw_rect_outline(buffer, rect.x0, rect.y0, rect.x1, rect.y1, thickness, v3_to_color(color));
}

fn void draw_rect_outline(BackBuffer* buffer, r2f rect, i32 thickness, v3f color)
{
    r2i rect_i32 = rect_f32_to_i32(rect);
    draw_rect_outline(buffer, rect_i32.x0, rect_i32.y0, rect_i32.x1, rect_i32.y1, thickness,
                      v3_to_color(color));
}

fn void draw_texture(BackBuffer* buffer, Texture* tex, v2f pos)
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

    u32* source = (u32*)tex->buf + (y_offset * tex->width);
    byt* row    = (byt*)buffer->buf + rect.x0 * buffer->byt_per_pix + rect.y0 * buffer->pitch;

    for (i32 y = rect.y0; y < rect.y1; ++y) {
        u32* dest = (u32*)row;
        source += x_offset_left;
        for (i32 x = rect.x0; x < rect.x1; ++x) {
            Color dest_col, src_col, blend_col;
            dest_col.rgba = (u32)*dest;
            src_col.rgba  = (u32)*source;
            blend_col.a   = 0xff;

            f32 alpha = (f32)src_col.a / 255.0f;

            blend_col.r = (u8)((1.0f - alpha) * (f32)dest_col.r + alpha * (f32)src_col.r);
            blend_col.g = (u8)((1.0f - alpha) * (f32)dest_col.g + alpha * (f32)src_col.g);
            blend_col.b = (u8)((1.0f - alpha) * (f32)dest_col.b + alpha * (f32)src_col.b);

            *dest = blend_col.rgba;

            ++dest, ++source;
        }
        source += x_offset_right;
        row += buffer->pitch;
    }
}

fn RenderGroup* alloc_render_group(Arena* arena, u32 max_pushbuffer_sz, f32 meters_to_pixels,
                                   Camera cam)
{
    auto result               = push_struct<RenderGroup>(arena);
    result->pushbuffer_base   = (byt*)push_size(arena, max_pushbuffer_sz);
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

fn RenderGroupEntryHeader* push_render_element(RenderGroup* group, RenderGroupEntryType type)
{
    RenderGroupEntryHeader* result = nullptr;
    u32 entry_sz                   = get_entry_size(type);

    if (entry_sz != U32_MAX && group->pushbuffer_sz + entry_sz < group->max_pushbuffer_sz) {
        result       = (RenderGroupEntryHeader*)(group->pushbuffer_base + group->pushbuffer_sz);
        result->type = type;
        group->pushbuffer_sz += entry_sz;
    } else {
        InvalidCodePath;
    }

    return result;
}

fn void push_clear(RenderGroup* group, Color color)
{
    auto entry = (RenderGroupEntryClear*)push_render_element(group, RenderGroupEntryType::clear);
    Assert(entry);
    entry->clear_color = color;
}

fn void push_texture(RenderGroup* group, RenderBasis basis, Texture* texture, v2f offset)
{
    auto entry =
        (RenderGroupEntryTexture*)push_render_element(group, RenderGroupEntryType::texture);
    Assert(entry);
    entry->basis   = basis;
    entry->offset  = offset;
    entry->texture = texture;
}

fn void push_rect(RenderGroup* group, RenderBasis basis, v2f dims, Color color)
{
    auto entry = (RenderGroupEntryRect*)push_render_element(group, RenderGroupEntryType::rect);
    Assert(entry);
    entry->basis = basis;
    entry->dims  = dims;
    entry->color = color;
}

fn void push_rect_outline(RenderGroup* group, RenderBasis basis, v2f dims, i32 thickness,
                          Color color)
{
    auto entry = (RenderGroupEntryRectOutline*)push_render_element(
        group, RenderGroupEntryType::rect_outline);
    Assert(entry);
    entry->basis     = basis;
    entry->dims      = dims;
    entry->thickness = thickness;
    entry->color     = color;
}

fn void push_coord_system(RenderGroup* group, v2f origin, v2f x_axis, v2f y_axis, Color color,
                          i32 num_points = 0)
{
    auto entry = (RenderGroupEntryCoordSystem*)push_render_element(
        group, RenderGroupEntryType::coord_system);

    Assert(entry);
    entry->origin     = origin;
    entry->x_axis     = x_axis;
    entry->y_axis     = y_axis;
    entry->color      = color;
    entry->num_points = num_points;
}

fn v2f get_screen_space_coord(RenderGroup* group, BackBuffer* back_buffer, RenderBasis basis)
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

    return result;
}

fn void draw_render_group(RenderGroup* group, BackBuffer* back_buffer)
{
    for (u32 base_address = 0; base_address < group->pushbuffer_sz;) {
        auto header = (RenderGroupEntryHeader*)(group->pushbuffer_base + base_address);

        switch (header->type) {
            case RenderGroupEntryType::clear: {
                auto entry = (RenderGroupEntryClear*)header;
                clear_buffer(back_buffer, entry->clear_color);
                base_address += sizeof(*entry);
            } break;
            case RenderGroupEntryType::rect: {
                auto entry = (RenderGroupEntryRect*)header;
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->basis);
                r2f rc     = rect_init_dims(coord, entry->dims * group->meters_to_pixels);
                draw_rect(back_buffer, rc, entry->color);
                base_address += sizeof(*entry);
            } break;
            case RenderGroupEntryType::rect_outline: {
                auto entry = (RenderGroupEntryRectOutline*)header;
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->basis);
                r2f rc     = rect_init_dims(coord, entry->dims * group->meters_to_pixels);
                draw_rect_outline(back_buffer, rc, entry->thickness, entry->color);
                base_address += sizeof(*entry);
            } break;
            case RenderGroupEntryType::texture: {
                auto entry = (RenderGroupEntryTexture*)header;
                v2f coord  = get_screen_space_coord(group, back_buffer, entry->basis);
                coord += entry->offset;
                Assert(entry->texture);
                draw_texture(back_buffer, entry->texture, coord);
                base_address += sizeof(*entry);
            } break;
            case RenderGroupEntryType::coord_system: {
                auto entry = (RenderGroupEntryCoordSystem*)header;
                f32 inc = 1.0f / (f32)entry->num_points;
                for (f32 x = 0.0f; x < 1.0f; x += inc) {
                    for (f32 y = 0.0f; y < 1.0f; y += inc) {
                        Color col = v3_to_color({x, y, 0.5f});
                        draw_square(back_buffer,
                                    entry->origin + entry->x_axis * x + entry->y_axis * y, 3.0f,
                                    col);
                    }
                }
                base_address += sizeof(*entry);
            } break;
            default: InvalidCodePath;
        }
    }
}

}  // namespace tom