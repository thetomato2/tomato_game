#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <string>
#include <vector>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using szt = size_t;
using byt = unsigned char;

using bool32 = i32;

constexpr f32 pi_32 = 3.14159265359f;

#define internal		static
#define local_persist	static
#define global_variable static

#ifdef _DEBUG
	#define TOM_INTERNAL
#endif

#define TOM_WIN32
#ifdef TOM_WIN32
	#define TOM_DLL_EXPORT __declspec(dllexport)
#else
	#define TOM_DLL_EXPORT
#endif
namespace tomato
{
#define Kilobytes(val) ((val)*1024)
#define Megabytes(val) (Kilobytes(val) * 1024)
#define Gigabytes(val) (Megabytes(val) * 1024)
#define Terabytes(val) (Gigabytes(val) * 1024)

// NOTE: services that the platform provides for the game
#ifdef TOM_INTERNAL
struct debug_ReadFileResult
{
	u32 contentSize;
	void* contents;
};

	//! all these C shenanigans...
	// TODO:  C++-ify this
	#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void* mem)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_Platform_Free_File_Memory);

	#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_ReadFileResult name(const char* fileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_Platform_Read_Entire_File);

	#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(const char* fileName, u64 memorySize, void* mem)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_Platform_Write_Entire_File);
#endif

struct GameOffscreenBuffer
{
	void* memory;
	i32 width;
	i32 height;
	i32 pitch;
	i32 bytesPerPixel;
};

struct GameSoundOutputBuffer
{
	i32 samplesPerSecond;
	i32 sampleCount;
	i16* samples;
	i32 toneHz;
};

struct GameButtonState
{
	i32 halfTransitionCount;
	bool endedDown;
};

struct GameControllerInput
{
	bool isAnalog;

	f32 startX;
	f32 startY;

	f32 minX;
	f32 minY;

	f32 maxX;
	f32 maxY;

	f32 endX;
	f32 endY;

	union
	{
		GameButtonState buttons[12];
		struct
		{
			GameButtonState dpad_up;
			GameButtonState dpad_right;
			GameButtonState dpad_down;
			GameButtonState dpad_left;
			GameButtonState button_A;
			GameButtonState button_B;
			GameButtonState button_X;
			GameButtonState button_Y;
			GameButtonState button_RB;
			GameButtonState button_LB;
			GameButtonState button_back;
			GameButtonState button_start;
		};
	};
};

struct GameInput
{
	GameControllerInput controllers[4];
};

struct GameMemory
{
	bool isInitialized;
	u64 permanentStorageSize;
	void* permanentStorage;	 //! required to be cleared to 0!

	u64 transientStorageSize;
	void* transientStorage;	 //! required to be cleared to 0!

#ifdef TOM_INTERNAL
	debug_Platform_Free_File_Memory* debug_platformFreeFileMem;
	debug_Platform_Read_Entire_File* debug_platfromReadEntireFile;
	debug_Platform_Write_Entire_File* debug_platformWriteEntireFile;
#endif
};

// TODO: temp
struct Keyboard
{
	bool w;
	bool s;
	bool a;
	bool d;
	bool u;
	bool shift;
};

//! part of the main loop
// static void GameUpdateAndRender(GameMemory* memory, GameInput* input, Keyboard* keyboard,
//								GameOffscreenBuffer* videoBuffer, GameSoundOutputBuffer* soundBuffer);
// static void GameGetSoundSamples(GameMemory* mem, GameSoundOutputBuffer* soudBuf);

struct GameState
{
	i32 toneHz;
	i32 xOffset;
	i32 yOffset;

	f32 tSine;
};

// NOTE: helpers
inline u32 SafeTruncateUint64(u64 value)
{
	// TODO: defines for max values
	assert(value <= 0xFFFFFFFF);
	return (u32)value;
}

#define GAME_UPDATE_AND_RENDER(name)                                                           \
	void name(GameMemory* mem, GameInput* input, Keyboard* kbd, GameOffscreenBuffer* videoBuf, \
			  GameSoundOutputBuffer* soundBuffer)

typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
}

#define GAME_GET_SOUND_SAMPLES(name) void name(GameMemory* mem, GameSoundOutputBuffer* soudBuf)
typedef GAME_GET_SOUND_SAMPLES(Game_Get_Sound_Samples);
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub)
{
}
}  // namespace tomato
