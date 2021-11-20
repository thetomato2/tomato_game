#pragma once
#include "TomatoGame.h"

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
};

struct Win32State
{
	szt totalSz;
	void* gameMemoryBlock;

	HANDLE recordingHandle;
	i32 inputRecordingInd;

	HANDLE playBackHandle;
	i32 inputPlayBackInd;

	char exePath[MAX_PATH];
};

#ifdef TOM_INTERNAL
struct debug_SoundTimeMarker
{
	DWORD playCursor;
	DWORD writeCursor;
};
#endif

template<typename T>
consteval T
debug_nTimeMarkers(const T size)
{
	return size / 2;
}
}  // namespace tomato::win32
