#ifndef TOMATO_IMAGE_HPP_
#define TOMATO_IMAGE_HPP_

#include "common.hpp"

namespace tom
{

#pragma pack(push, 1)
struct bitmap_header
{
    u16 file_type;
    u32 file_size;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmap_offset;
    u32 size;
    s32 width;
    s32 height;
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
    s32 width;
    s32 height;
    u32 *pixel_ptr;
};

struct argb_img
{
    const char *name;
    u32 width;
    u32 height;
    u32 size;
    u32 *pixel_ptr;
};

}  // namespace tom

#endif  // IMAGE_HPP_
