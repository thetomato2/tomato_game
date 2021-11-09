#pragma once

namespace tomato::win32
{
struct WindowDimensions
{
	i32 width;
	i32 height;
};

struct OffScreenBuffer
{
	BITMAPINFO info;
	void* mem;
	i32 width;
	i32 height;
	i32 pitch;
	i32 bytPerPix;
};

struct SoundOutput
{
	i32 samplesPerSec;
	u32 runningSampleInd;
	i32 bytPerSample;
	DWORD secondaryBufSz;
	f32 tSine;
	i32 latencySampleCnt;
	// TODO: add bytes per second
};

#ifdef TOM_DEBUG
struct debug_SoundTimeMarker
{
	DWORD playCursor;
	DWORD writeCursor;
};
#endif

template<typename T>
consteval T debug_nTimeMarkers(const T size)
{
	return size / 2;
}
}  // namespace tomato::win32
