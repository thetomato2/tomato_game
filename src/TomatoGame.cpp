#include "TomatoGame.h"

namespace tomato
{
namespace
{
inline i32
Rndf32toi23(f32 value)
{
	return i32(value + 0.5f);
}

void
Rainbow(Color& color, f32 frequency, f32 time)
{
	float f = (float)sin(time * frequency) / 2.0f + 0.5f;

	float a = (1.0f - f) / 0.2f;
	float x = floorf(a);
	float y = floorf(255.0f * (a - x));
	f32 red, green, blue;

	if (x == 0.0f) {
		red	  = 255.0f;
		green = y;
		blue  = 0;
	} else if (x == 1.0f) {
		red	  = 255.0f - y;
		green = 255.0f;
		blue  = 0;
	} else if (x == 2.0f) {
		red	  = 0.0f;
		green = 255.0f;
		blue  = y;
	} else if (x == 3.0f) {
		red	  = 0.0f;
		green = 255.0f - y;
		blue  = 255.0f;
	} else if (x == 4.0f) {
		red	  = y;
		green = 0.0f;
		blue  = 255.0f;
	} else if (x == 5.0f) {
		red	  = 255.0f;
		green = 0.0f;
		blue  = 255.0f;
	}

	color.argb = (0xFF << 24) | ((u8)red << 16) | ((u8)green << 8) | ((u8)blue);
}

void
ClearBuffer(GameOffscreenBuffer& buf, Color color)
{
	i32 width  = buf.width;
	i32 height = buf.height;

	u8* row = (u8*)buf.mem;
	for (i32 y = 0; y < height; ++y) {
		u32* pixel = (u32*)row;
		for (i32 x = 0; x < width; ++x) {
			*pixel++ = color.argb;
		}
		row += buf.pitch;
	}
}

void
DrawRect(GameOffscreenBuffer& buf, f32 fMinX, f32 fMinY, f32 fMaxX, f32 fMaxY,
		 Color color = { 0xFFFF00FF })
{
	i32 minX = Rndf32toi23(fMinX);
	i32 minY = Rndf32toi23(fMinY);
	i32 maxX = Rndf32toi23(fMaxX);
	i32 maxY = Rndf32toi23(fMaxY);

	if (minX < 0) minX = 0;
	if (minY < 0) minY = 0;
	if (maxX > buf.width) maxX = buf.width;
	if (maxY > buf.height) maxX = buf.height;

	u8* row = ((u8*)buf.mem + minX * buf.bytesPerPix + minY * buf.pitch);

	for (i32 y = minY; y < maxY; ++y) {
		u32* pixel = (u32*)row;
		for (i32 x = minX; x < maxX; ++x) {
			*pixel++ = color.argb;
		}
		row += buf.pitch;
	}
}

void
GameOuputSound(GameSoundOutputBuffer& soundBuffer, i32 toneHz, f32 tSine)
{
	static f32 tempSine {};
	i16 toneVolume = 3000;
	i32 wavePeriod = soundBuffer.samplesPerSecond / toneHz;

	i16* sampleOut = soundBuffer.samples;
	for (szt sampleIndex = 0; sampleIndex < soundBuffer.sampleCount; ++sampleIndex) {
		f32 sineValue = sinf(tempSine * 0.75f);
		// i16 sampleValue = (i16)(sineValue * toneVolume);
		i16 sampleValue = 0;
		*sampleOut++	= sampleValue;
		*sampleOut++	= sampleValue;

		tempSine += 2.0f * 3.14f * 1.0f / (f32)wavePeriod;
		/*if (tSine > 2.f * util::pi32) {
			tSine -= 2.f * util::pi32;
		}*/
	}
}
}  // namespace

extern "C" TOM_DLL_EXPORT
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	auto* gameState = (GameState*)mem.permanentStorage;
	GameOuputSound(soudBuf, gameState->toneHz, gameState->tSine);
}

extern "C" TOM_DLL_EXPORT
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	assert(sizeof(GameState) <= mem.permanentStorageSize);

	// NOTE: cast to GameState ptr, dereference and cast to GameState reference
	auto& gameState = (GameState&)(*(GameState*)mem.permanentStorage);

	if (!mem.isInitialized) {
		const char* fileName = __FILE__;

#ifdef TOM_INTERNAL
		debug_ReadFileResult file = mem.debug_platfromReadEntireFile(fileName);
		if (file.contents) {
			mem.debug_platformWriteEntireFile("test.txt", file.contentSize, file.contents);
			mem.debug_platformFreeFileMem(file.contents);
		}
#endif
		gameState.toneHz  = 256;
		gameState.tSine	  = 0.f;
		gameState.xOffset = 0;
		gameState.yOffset = 0;
		gameState.fader	  = 0;

		// TODO: this might be more appropriate in the platform layer
		mem.isInitialized = true;
	}

	f32 offsetYMul	= 0.2f;
	f32 offsetXMul	= 1.f;
	f32 playerSpeed = 8.f;

	GameControllerInput& controller0 = input.controllers[0];
	if (controller0.isAnalog) {
		// gameState.xOffset += i32(speed * (controller0.endLX));
		// gameState.yOffset += i32(speed * (controller0.endLY));
		gameState.toneHz =
			256 + (i32)(64.0f * (controller0.endLY)) + (i32)(64.0f * (controller0.endLX));

	} else {
		// TODO: handle digital input
	}

	local_persist f32 time {};
	time += input.secondsPerFrame * 0.2f;

	Color clearColor;
	Rainbow(clearColor, 1.f, time);

	ClearBuffer(videoBuf, clearColor);

	DrawRect(videoBuf, 10.0f, 10.0f, 30.0f, 30.0f);
}

#if 0
	#ifdef TOM_WIN32
namespace win32
{
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN	 // Exclude rarely-used stuff from Windows headers
		#endif
		#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH: break;
	}
	return TRUE;
}
}  // namespace win32

	#endif
#endif
}  // namespace tomato
