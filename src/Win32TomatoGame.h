#pragma once
#include "TomatoGame.h"

namespace tomato::win32
{
struct WindowDimensions
{
	s32 width;
	s32 height;
};

struct OffScreenBuffer
{
	BITMAPINFO info;
	void* mem;
	s32 width;
	s32 height;
	s32 pitch;
	s32 bytPerPix;
};

struct SoundOutput
{
	s32 samplesPerSec;
	u32 runningSampleInd;
	s32 bytPerSample;
	DWORD secondaryBufSz;
	f32 tSine;
	s32 latencySampleCnt;
};

struct ReplayBuffer
{
	_TCHAR fileName[512];
	HANDLE fileHandle;
	HANDLE memMap;
	void* memBlock;
};

struct Win32State
{
	szt totalSz;

	HANDLE recordingHandle;
	s32 inputRecordingInd;

	HANDLE playBackHandle;
	s32 inputPlayBackInd;

	char exePath[MAX_PATH];

	void* gameMemoryBlock;
	ReplayBuffer replayBuffers[4];
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
