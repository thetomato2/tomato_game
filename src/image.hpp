#ifndef IMAGE_HPP_
#define IMAGE_HPP_
#include "Common.hpp"

namespace tom
{

#pragma pack(push, 1)
struct BitmapHeader
{
    u16 fileType;
    u32 fileSize;
    u16 reserved_1;
    u16 reserved_2;
    u32 bitmapOffset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel;
};

struct ArgbHeader
{
    u32 width;
    u32 height;
    u32 size;
};
#pragma pack(pop)

struct BitmapImg
{
    s32 width;
    s32 height;
    u32 *pixelPtr;
};

struct ArgbImg
{
    const char *name;
    u32 width;
    u32 height;
    u32 size;
    u32 *pixelPtr;
};

}  // namespace tom

#endif  // IMAGE_HPP_
