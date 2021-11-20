#pragma once
#include "TomatoFramework.h"

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

	#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) \
		bool32 name(const char* fileName, u64 memorySize, void* mem)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_Platform_Write_Entire_File);
#endif

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#if TOM_INTERNAL
	#define Assert(expression) \
		if (!(expression)) {   \
			*(int*)0 = 0;      \
		}
#elif
	#define Assert(expression)
#endif

struct GameOffscreenBuffer
{
	void* mem;
	i32 width;
	i32 height;
	i32 pitch;
	i32 bytesPerPix;
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
	bool isConnnected;
	bool isAnalog;

	f32 startLX;
	f32 startLY;
	f32 startRX;
	f32 startRY;

	f32 minX;
	f32 minY;

	f32 maxX;
	f32 maxY;

	f32 endLX;
	f32 endLY;
	f32 endRX;
	f32 endRY;

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

struct GameKeyboard
{
	union
	{
		GameButtonState keys[4];
		struct
		{
			GameButtonState key_W;
			GameButtonState key_S;
			GameButtonState key_A;
			GameButtonState key_D;
			GameButtonState key_Space;
			GameButtonState key_LShift;
			GameButtonState key_P;
			GameButtonState key_1;
			GameButtonState key_2;
			GameButtonState key_3;
			GameButtonState key_4;
		};
	};
};

struct GameInput
{
	GameButtonState mouseButtons[3];
	i32 mouseX, mouseY, mouseZ;

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

struct Color
{
	Color() : bgra(0xFFFFFFFF) {}
	union
	{
		u32 bgra;
		struct
		{
			u8 b;
			u8 g;
			u8 r;
			u8 a;
		};
	};
};

struct iVector2
{
	iVector2() : x(0), y(0) {}
	iVector2(i32 x, i32 y)
	{
		this->x = x;
		this->y = y;
	}

	i32 x;
	i32 y;
};

struct Player
{
	iVector2 pos;
	Color color;
	i32 size;
	bool isJump;
	f32 velocity;
};

struct GameState
{
	i32 toneHz;
	i32 xOffset;
	i32 yOffset;

	f32 fader;
	f32 tSine;

	Player player1;
	Player playerLast;
	i32 floorY;
	f32 gravity;
};

// TODO: implement this
struct ThreadContext
{
	i32 placeHolder;
};

// NOTE: helpers
inline u32
SafeTruncateUint64(u64 value)
{
	// TODO: defines for max values
	assert(value <= 0xFFFFFFFF);
	return (u32)value;
}

#define GAME_UPDATE_AND_RENDER(name)                                   \
	void name(ThreadContext thread, GameMemory* mem, GameInput* input, \
			  GameOffscreenBuffer* videoBuf, GameSoundOutputBuffer* soundBuffer)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);

#define GAME_GET_SOUND_SAMPLES(name) \
	void name(ThreadContext thread, GameMemory* mem, GameSoundOutputBuffer* soudBuf)
typedef GAME_GET_SOUND_SAMPLES(Game_Get_Sound_Samples);
}  // namespace tomato
