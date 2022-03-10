#include "Platform.h"

#define STBImageImplementation
#include "../external/stb_image.h"

// NOTE: coverst PNGs to my personal image format which is just bascially
// the most basic bitch bitmap
//
#ifdef Emacs
using wcharT = uint16T;
#endif
// WinHelp is deprecate
#define NOHELP
// DirectX apps don't need GDI
// NOTE: I am using GDI to slowly blit to the screen
//#define NODRAWTEXT
//#define NOGDI
//#define NOBITMAP
//#define WIN32LeanAndMean

// Use the C++ standard templated min/max
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

#include "Utils.hpp"

#pragma pack(push, 1)

struct BitmapHeader
{
    u16 fileType;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel;
};

struct ARGBHeader
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

void
win32FreeMemor(Void *memory)
{
    if (memory) {
        VirtualFree(memory, 0, MEMRelease);
    }
}

debugReadFileResult
win32ReadFile(Const char *fileName)
{
    DebugReadFileResult file = {};

    HANDLE fileHandle =
        CreateFileA(fileName, GENERICRead, 0, 0, OPENExisting, FILEAttributeNormal, 0);
    if (fileHandle != INVALIDHandleValue) {
        LARGEInteger fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize)) {
            u32 fileSize32 = safeTruncateU32ToU64(Filesize.Quadpart);
            file.contents  = VirtualAlloc(0, fileSize32, MEMReserve | MEMCommit, PAGEReadwrite);
            if (file.contents) {
                DWORD bytesRead;
                if (ReadFile(fileHandle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
                    fileSize32 == bytesRead) {
                    // NOTE: file read successfully
                    file.contentSize = fileSize32;
                } else {
                    win32FreeMemor(File.Contents);
                    file.contents = 0;
                }
            } else {
                printf("ERROR-> Failed to read file contents!\n");
            }
        } else {
            printf("ERROR-> Failed to open file handle!\n");
        }
        CloseHandle(fileHandle);
    }
    return file;
}

bool
win32WriteFile(Const char *fileName, u64 memorySize, void *memory)
{
    b32 success = false;

    HANDLE fileHandle = CreateFileA(fileName, GENERICWrite, 0, 0, CREATEAlways, 0, 0);
    if (fileHandle != INVALIDHandleValue) {
        DWORD bytesWritten;
        if (WriteFile(fileHandle, memory, (DWORD)memorySize, &bytesWritten, 0)) {
            // NOTE: file wrote successfully
            success = (bytesWritten == memorySize);
        } else {
            printf("ERROR-> Failed to write file contents!\n");
        }
        CloseHandle(fileHandle);
    } else {
        printf("ERROR-> Failed to oepn file handle!\n");
    }
    return success;
}
BitmapImg
loadBmp(Const char *fileName)
{
    debugReadFileResult readResult = win32ReadFile(FileName);
    BitmapImg result;

    if (readResult.content_size != 0) {
        BitmapHeader *header = (BitmapHeader *)readResult.contents;
        u32 *pixels          = (u32 *)((byt *)readResult.contents + header->bitmapOffset);
        result.width         = header->width;
        result.height        = header->height;
        result.pixelPtr      = pixels;
    }
    return result;
}

s32
main(s32 argc, char *argv[])
{
    if (argc != 2) {
        printf("needs a valid path argument!\n");
        return 1;
    }

    // char imgPathBuf[512];
    // const char *imageDir = "T:/assets/images/";

    // tomato::util::catStr(ImageDir, argv[1], &imgPathBuf[0]);

    // printf("%s\n", imgPathBuf);

    s32 width, height, channels;
    byt *image = stbiLoad(Argv[1], &width, &height, &channels, 0);

    if (!image) {
        printf("Failed to load image!! -> \"%s\"\n", argv[1]);
        return 1;
    } else {
        printf("Loaded: \"%s\"\n", argv[1]);
    }

    ARGBHeader argb;
    argb.width     = (u32)width;
    argb.height    = (u32)height;
    argb.size      = sizeof(ARGBHeader) + sizeof(u32) * (u32)argb.width * (u32)argb.height;
    auto *pixelPtr = (u32 *)image;

    for (u32 pixel {}; pixel < width * height; ++pixel) {
        u8 temp = *image;
        *image  = *(image + 2);
        image += 2;
        *image = temp;
        image += 2;
    }

    auto *argbBufPtr = (u32 *)VirtualAlloc(0, argb.size, MEMReserve | MEMCommit, PAGEReadwrite);

    auto *writePtr = argbBufPtr;
    *writePtr++    = argb.width;
    *writePtr++    = argb.height;
    *writePtr++    = argb.size;

    for (u32 pixel {}; pixel < argb.width * argb.height; ++pixel) {
        *writePtr++ = *pixelPtr++;
    }

    szt inputPathLen = tom::getStrLen(Argv[1]);
    szt pathInd      = inputPathLen - 1;
    char *pathStr    = argv[1];
    while (path_str[pathInd] != '\\') {
        --pathInd;
    }
    ++pathInd;

    char imgPathBuf[512];
    char *imgPathBufPtr = &imgPathBuf[0];

    *imgPathBufPtr++ = '.';
    *imgPathBufPtr++ = '/';

    while (path_str[pathInd] != '.') {
        *imgPathBufPtr++ = path_str[pathInd++];
    }
    *imgPathBufPtr++ = '.';
    *imgPathBufPtr++ = 'a';
    *imgPathBufPtr++ = 'r';
    *imgPathBufPtr++ = 'g';
    *imgPathBufPtr++ = 'b';
    *imgPathBufPtr++ = '\0';

    win32WriteFile(ImgPathBuf, (u64)argb.size, argbBufPtr);

    printf("Wrote: %s\n", imgPathBuf);

    return 0;
}
