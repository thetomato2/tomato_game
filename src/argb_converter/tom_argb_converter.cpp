#include "platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

// NOTE: coverst PNGs to my personal image format which is just bascially
// the most basic bitch bitmap
//
#ifdef _EMACS
using wchar_t = uint16_t;
#endif
// WinHelp is deprecate
#define NOHELP
// DirectX apps don't need GDI
//#define NODRAWTEXT
//#define NOGDI
//#define NOBITMAP
//#define WIN32_LEAN_AND_MEAN

// Use the C++ standard templated min/max
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

#include "utils.hpp"

#pragma pack(push, 1)

struct bitmap_header
{
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
};

struct argb_header
{
    u32 width;
    u32 height;
    u32 size;
};
#pragma pack(pop)

struct bitmap_img
{
    i32 width;
    i32 height;
    u32 *pixel_ptr;
};

void
win32_free_memor(void *memory_)
{
    if (memory_) {
        VirtualFree(memory_, 0, MEM_RELEASE);
    }
}

debug_read_file_result
win32_read_file(const char *file_name)
{
    debug_read_file_result file = {};

    HANDLE file_handle =
        CreateFileA(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(file_handle, &fileSize)) {
            u32 fileSize32 = safe_truncate_u32_to_u64(fileSize.QuadPart);
            file.contents  = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (file.contents) {
                DWORD bytesRead;
                if (ReadFile(file_handle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
                    fileSize32 == bytesRead) {
                    // NOTE: file read successfully
                    file.content_size = fileSize32;
                } else {
                    win32_free_memor(file.contents);
                    file.contents = 0;
                }
            } else {
                printf("ERROR-> Failed to read file contents!\n");
            }
        } else {
            printf("ERROR-> Failed to open file handle!\n");
        }
        CloseHandle(file_handle);
    }
    return file;
}

bool
win32_write_file(const char *file_name, u64 memory_size_, void *memory_)
{
    b32 success = false;

    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if (WriteFile(file_handle, memory_, (DWORD)memory_size_, &bytes_written, 0)) {
            // NOTE: file wrote successfully
            success = (bytes_written == memory_size_);
        } else {
            printf("ERROR-> Failed to write file contents!\n");
        }
        CloseHandle(file_handle);
    } else {
        printf("ERROR-> Failed to oepn file handle!\n");
    }
    return success;
}
bitmap_img
load_bmp(const char *file_name)
{
    debug_read_file_result read_result = win32_read_file(file_name);
    bitmap_img result;

    if (read_result.content_size != 0) {
        bitmap_header *header = (bitmap_header *)read_result.contents;
        u32 *pixels   = rcast(u32 *, (scast(byt *, read_result.contents) + header->bitmap_offset));
        result.width  = header->width;
        result.height = header->height;
        result.pixel_ptr = pixels;
    }
    return result;
}

i32
main(i32 argc, char *argv[])
{
    if (argc != 2) {
        printf("needs a valid path argument!\n");
        return 1;
    }

    // char img_path_buf[512];
    // const char *image_dir = "T:/assets/images/";

    // tomato::util::cat_str(image_dir, argv[1], &img_path_buf[0]);

    // printf("%s\n", img_path_buf);

    i32 width, height, channels;
    byt *image = stbi_load(argv[1], &width, &height, &channels, 0);

    if (!image) {
        printf("Failed to load image!! -> \"%s\"\n", argv[1]);
        return 1;
    } else {
        printf("Loaded: \"%s\"\n", argv[1]);
    }

    argb_header argb;
    argb.width  = scast(u32, width);
    argb.height = scast(u32, height);
    argb.size =
        sizeof(argb_header) + sizeof(u32) * scast(u32, argb.width) * scast(u32, argb.height);
    auto *pixel_ptr = rcast(u32 *, image);

    for (u32 pixel {}; pixel < width * height; ++pixel) {
        u8 temp = *image;
        *image  = *(image + 2);
        image += 2;
        *image = temp;
        image += 2;
    }

    auto *argb_buf_ptr =
        scast(u32 *, VirtualAlloc(0, argb.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));

    auto *write_ptr = argb_buf_ptr;
    *write_ptr++    = argb.width;
    *write_ptr++    = argb.height;
    *write_ptr++    = argb.size;

    for (u32 pixel {}; pixel < argb.width * argb.height; ++pixel) {
        *write_ptr++ = *pixel_ptr++;
    }

    szt input_path_len = tom::get_str_len(argv[1]);
    szt path_ind       = input_path_len - 1;
    char *path_str     = argv[1];
    while (path_str[path_ind] != '\\') {
        --path_ind;
    }
    ++path_ind;

    char img_path_buf[512];
    char *img_path_buf_ptr = &img_path_buf[0];

    *img_path_buf_ptr++ = '.';
    *img_path_buf_ptr++ = '/';

    while (path_str[path_ind] != '.') {
        *img_path_buf_ptr++ = path_str[path_ind++];
    }
    *img_path_buf_ptr++ = '.';
    *img_path_buf_ptr++ = 'a';
    *img_path_buf_ptr++ = 'r';
    *img_path_buf_ptr++ = 'g';
    *img_path_buf_ptr++ = 'b';
    *img_path_buf_ptr++ = '\0';

    win32_write_file(img_path_buf, scast(u64, argb.size), argb_buf_ptr);

    printf("Wrote: %s\n", img_path_buf);

    return 0;
}
