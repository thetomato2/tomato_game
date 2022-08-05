namespace tom
{

// NOTE: HARDCODED!!!
// maps a glyph index to a char
function char get_glyph_char(i32 i)
{
    switch (i) {
        case 0: return ' ';
        case 1: return 'a';
        case 2: return 'b';
        case 3: return 'c';
        case 4: return 'd';
        case 5: return 'e';
        case 6: return 'f';
        case 7: return 'g';
        case 8: return 'h';
        case 9: return 'i';
        case 10: return 'j';
        case 11: return 'k';
        case 12: return 'l';
        case 13: return 'm';
        case 14: return 'n';
        case 15: return 'o';
        case 16: return 'p';
        case 17: return 'q';
        case 18: return 'r';
        case 19: return 's';
        case 20: return 't';
        case 21: return 'u';
        case 22: return 'v';
        case 23: return 'w';
        case 24: return 'x';
        case 25: return 'y';
        case 26: return 'z';
        case 27: return 'A';
        case 28: return 'B';
        case 29: return 'C';
        case 30: return 'D';
        case 31: return 'E';
        case 32: return 'F';
        case 33: return 'G';
        case 34: return 'H';
        case 35: return 'I';
        case 36: return 'J';
        case 37: return 'K';
        case 38: return 'L';
        case 39: return 'M';
        case 40: return 'N';
        case 41: return 'O';
        case 42: return 'P';
        case 43: return 'Q';
        case 44: return 'R';
        case 45: return 'S';
        case 46: return 'T';
        case 47: return 'U';
        case 48: return 'V';
        case 49: return 'W';
        case 50: return 'X';
        case 51: return 'Y';
        case 52: return 'Z';
        case 53: return '0';
        case 54: return '1';
        case 55: return '2';
        case 56: return '3';
        case 57: return '4';
        case 58: return '5';
        case 59: return '6';
        case 60: return '7';
        case 61: return '8';
        case 62: return '9';
        case 63: return '!';
        case 64: return '\'';
        case 65: return '#';
        case 66: return '$';
        case 67: return ':';
        case 68: return ';';
        case 69: return ',';
        case 70: return '_';
        case 71: return '@';
        case 72: return ')';
        case 73: return '(';
        case 74: return '&';
        case 75: return '%';
        case 76: return '?';
        case 77: return '/';
        case 78: return '\"';
        case 79: return ' ';
        case 80: return '.';
        case 81: return '>';
        case 82: return '<';
        case 83: return '|';
        case 84: return '[';
        case 85: return ']';
        case 86: return '{';
        case 87: return '}';
    }

    return 0;
}

// NOTE: HARDCODED!!!
// maps a char to a glyph index
function i32 get_glyph_index(char c)
{
    switch (c) {
        case 'a': return 1;
        case 'b': return 2;
        case 'c': return 3;
        case 'd': return 4;
        case 'e': return 5;
        case 'f': return 6;
        case 'g': return 7;
        case 'h': return 8;
        case 'i': return 9;
        case 'j': return 10;
        case 'k': return 11;
        case 'l': return 12;
        case 'm': return 13;
        case 'n': return 14;
        case 'o': return 15;
        case 'p': return 16;
        case 'q': return 17;
        case 'r': return 18;
        case 's': return 19;
        case 't': return 20;
        case 'u': return 21;
        case 'v': return 22;
        case 'w': return 23;
        case 'x': return 24;
        case 'y': return 25;
        case 'z': return 26;
        case 'A': return 27;
        case 'B': return 28;
        case 'C': return 29;
        case 'D': return 30;
        case 'E': return 31;
        case 'F': return 32;
        case 'G': return 33;
        case 'H': return 34;
        case 'I': return 35;
        case 'J': return 36;
        case 'K': return 37;
        case 'L': return 38;
        case 'M': return 39;
        case 'N': return 40;
        case 'O': return 41;
        case 'P': return 42;
        case 'Q': return 43;
        case 'R': return 44;
        case 'S': return 45;
        case 'T': return 46;
        case 'U': return 47;
        case 'V': return 48;
        case 'W': return 49;
        case 'X': return 50;
        case 'Y': return 51;
        case 'Z': return 52;
        case '0': return 53;
        case '1': return 54;
        case '2': return 55;
        case '3': return 56;
        case '4': return 57;
        case '5': return 58;
        case '6': return 59;
        case '7': return 60;
        case '8': return 61;
        case '9': return 62;
        case '!': return 63;
        case '\'': return 64;
        case '#': return 65;
        case '$': return 66;
        case ':': return 67;
        case ';': return 68;
        case ',': return 69;
        case '_': return 70;
        case '@': return 71;
        case ')': return 72;
        case '(': return 73;
        case '&': return 74;
        case '%': return 75;
        case '?': return 76;
        case '/': return 77;
        case '\"': return 78;
        case ' ': return 79;
        case '.': return 80;
        case '>': return 81;
        case '<': return 82;
        case '|': return 83;
        case '[': return 84;
        case ']': return 85;
        case '{': return 86;
        case '}': return 87;
    }

    return 79;
}

fn ttf_GlyphResult load_ttf_glyph(const char* file_path, f32 point_sz, char glyph)
{
    ttf_GlyphResult result;

    auto ttf_file = read_file(file_path);
    if (ttf_file.contents == nullptr) {
        InvalidCodePath;
    }

    i32 bake_width  = 64;
    i32 bake_height = 100;

    stbtt_fontinfo font;
    stbtt_InitFont(&font, (byt*)ttf_file.contents,
                   stbtt_GetFontOffsetForIndex((byt*)ttf_file.contents, 0));
    result.bitmap = (void*)stbtt_GetCodepointBitmap(
        &font, 0, stbtt_ScaleForPixelHeight(&font, point_sz), glyph, &result.width, &result.height,
        &result.x_off, &result.y_off);

    // stbtt_FreeBitmap(font_bitmap, 0);

    return result;
}

// NOTE: hard-coded
// TODO: dynanic font-sheet creation
fn FontSheet create_font_sheet(const char* name, const char* file_path, f32 point_sz)
{
    FontSheet result {};
    result.name = name;

    bool draw_lines = false;

    i32 row_cnt     = 13;
    i32 col_cnt     = 1;
    i32 glyph_cnt   = 87;
    i32 pad         = 20;
    i32 max_glyph_x = 0;
    i32 max_glyph_y = 0;

    auto ttf_file = read_file(file_path);
    if (ttf_file.contents == nullptr) {
        InvalidCodePath;
    }

    stbtt_fontinfo font;
    stbtt_InitFont(&font, (byt*)ttf_file.contents,
                   stbtt_GetFontOffsetForIndex((byt*)ttf_file.contents, 0));

    f32 scale = stbtt_ScaleForPixelHeight(&font, point_sz);
    i32 ascent;
    stbtt_GetFontVMetrics(&font, &ascent, 0, 0);

    // load all the glyphs to get the toal size of the bitmap needed
    for (i32 i = 1; i < glyph_cnt + 1; ++i) {
        char c = get_glyph_char(i);
        r2i bounds;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &bounds.x0, &bounds.y0, &bounds.x1,
                                    &bounds.y1);
        max_glyph_x = max(bounds.x1 - bounds.x0, max_glyph_x);
        max_glyph_y = max(bounds.y1 - bounds.y0, max_glyph_y);
    }

    // allocate the needed size
    i32 r         = max(max_glyph_x, max_glyph_y) + pad;
    result.width  = r * row_cnt;
    result.height = (glyph_cnt / row_cnt) * r + r;
    szt buf_sz    = sizeof(byt) * result.width * result.height * glyph_cnt;
    auto bm_buf   = (byt*)plat_malloc(buf_sz);
    zero_size(bm_buf, buf_sz);

    i32 baseline = r - r / 4;

    // draw the glyphs to the bitmap
    i32 cur_x = 0, cur_y = 0;
    for (i32 i = 1; i < glyph_cnt + 1; ++i) {
        char c = get_glyph_char(i);
        ttf_GlyphResult glyph;
        glyph.bitmap = (void*)stbtt_GetCodepointBitmap(&font, scale, scale, c, &glyph.width,
                                                       &glyph.height, &glyph.x_off, &glyph.y_off);

        i32 y_off      = baseline + glyph.y_off;
        i32 x_off      = r / 2 - glyph.width / 2;
        byt* row       = bm_buf + (cur_x + x_off) + (cur_y + y_off) * result.width;
        byt* glyph_ptr = (byt*)glyph.bitmap;
        for (i32 y = 0; y < glyph.height; ++y) {
            byt* dest_ptr = row;
            for (i32 x = 0; x < glyph.width; ++x) {
                *dest_ptr++ = *glyph_ptr++;
            }
            row += result.width;
        }
        glyph.free();

        cur_x += r;
        if (i % row_cnt == 0) {
            cur_x = 0;
            cur_y += r;
            ++col_cnt;
        }
    }

    if (draw_lines) {
        for (i32 y = 0; y < result.height; y += r) {
            byt* row = bm_buf + y * result.width;
            for (i32 x = 0; x < result.width; ++x) {
                row[x] = 0xff;
            }
        }

        for (i32 y = 0; y < result.height; ++y) {
            byt* row = bm_buf + y * result.width;
            for (i32 x = 0; x < result.width; ++x) {
                if (x % r == 0) row[x] = 0xff;
            }
        }
    }

    result.bitmap = bm_buf;
    result.r      = r;
    result.x_cnt  = row_cnt;
    result.y_cnt  = col_cnt;

    return result;
}

fn char* write_fontsheet_png(FontSheet* fs)
{
    if (!dir_exists("./out")) create_dir("./out");
    char* path_buf = str_copy("./out/", fs->name, "_glyph_table.png");
    stbi_write_png(path_buf, fs->width, fs->height, 1, fs->bitmap, sizeof(byt) * fs->width);
    return path_buf;
}

fn v2f get_uv_offset(FontSheet* fs, i32 glyph_ind)
{
    v2f result;

    f32 constexpr border = 2.0f;

    i32 gx = glyph_ind % fs->x_cnt - 1;
    i32 gy = ((glyph_ind - 1) / fs->x_cnt);

    result.x = (f32)fs->r * (f32)gx;
    result.y = (f32)fs->height - (f32)fs->r * (f32)(gy + 1);

    return result;
}

fn void alloc_glyph_cache(GfxState* gfx, r2i dims)
{
    // Reset the buffers if they exists
    if (gfx->glyph_tex) {
        gfx->glyph_tex->Release();
        gfx->glyph_tex = nullptr;
    }
    if (gfx->glyph_tex_view) {
        gfx->glyph_tex_view->Release();
        gfx->glyph_tex_view = nullptr;
    }
    
    
}

}  // namespace tom