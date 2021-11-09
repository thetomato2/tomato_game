#include "TomatoGame.h"
namespace tomato
{
namespace
{
void RenderWeirdGradient(GameOffscreenBuffer* buffer, i32 xOffset, i32 yOffset)
{
	i32 width  = buffer->width;
	i32 height = buffer->height;

	u8* row = (u8*)buffer->memory;
	for (i32 y = 0; y < height; y++) {
		u32* pixel = (u32*)row;
		for (i32 x = 0; x < width; x++) {
			/*					  1  2  3  4
					pixel in memory: 00 00 00 00
					 BB GG RR xx
					0x xxRRGGBB
			*/

			u8 blue	 = x + xOffset;
			u8 green = y + yOffset;
			u8 red	 = 0;
			// u8 red	 = ((x + xOffset) / 2) + ((y + yOffset) / 2);
#ifdef _DEBUG
			*pixel++ = ((red << 16) | (green << 8) | blue);
#else
			*pixel++ = ((red << 16) | (green << 8) | blue);
#endif
		}
		row += buffer->pitch;
	}
}

void GameOuputSound(GameSoundOutputBuffer* soundBuffer, i32 toneHz, f32 tSine)
{
	static f32 tempSine {};
	i16 toneVolume = 3000;
	i32 wavePeriod = soundBuffer->samplesPerSecond / toneHz;

	i16* sampleOut = soundBuffer->samples;
	for (szt sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex) {
		f32 sineValue	= sinf(tempSine * 0.75);
		i16 sampleValue = (i16)(sineValue * toneVolume);
		*sampleOut++	= sampleValue;
		*sampleOut++	= sampleValue;

		tempSine += 2.0f * pi_32 * 1.0f / (f32)wavePeriod;
		/*if (tSine > 2.f * util::pi32) {
			tSine -= 2.f * util::pi32;
		}*/
	}
}
}  // namespace

extern "C" TOM_DLL_EXPORT GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	auto* gameState = (GameState*)mem->permanentStorage;
	GameOuputSound(soudBuf, gameState->toneHz, gameState->tSine);
}

extern "C" TOM_DLL_EXPORT GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	assert(sizeof(GameState) <= mem->permanentStorageSize);

	auto* gameState = (GameState*)mem->permanentStorage;
	if (!mem->isInitialized) {
		const char* fileName = __FILE__;

#ifdef _DEBUG
		debug_ReadFileResult file = mem->debug_platfromReadEntireFile(fileName);
		if (file.contents) {
			mem->debug_platformWriteEntireFile("test.txt", file.contentSize, file.contents);
			mem->debug_platformFreeFileMem(file.contents);
		}
#endif
		gameState->toneHz  = 256;
		gameState->tSine   = 0.f;
		gameState->xOffset = 0;
		gameState->yOffset = 0;

		// TODO: this might be more appropriate in the platform layer
		mem->isInitialized = true;
	}

	f32 speed = 10.0f;
	if (kbd->shift == true) speed = 20.0f;

	GameControllerInput* input0 = &input->controllers[0];
	if (kbd->a) {
		gameState->xOffset -= (i32)speed;
	} else if (kbd->d) {
		gameState->xOffset += (i32)speed;
	}
	if (kbd->w) {
		gameState->yOffset -= (i32)speed;
	} else if (kbd->s) {
		gameState->yOffset += (i32)speed;
	}
	if (input0->button_A.endedDown) gameState->toneHz *= (3 / 4);

	if (input0->isAnalog) {
		gameState->xOffset += i32(speed * (input0->endX));
		gameState->yOffset += i32(speed * (input0->endY));
		gameState->toneHz = 256 + (i32)(64.0f * (input0->endY)) + (i32)(64.0f * (input0->endX));
	} else {
		// TODO: handle digital input
	}

	// TODO: Allow sample offsets here for more robust platform options
	RenderWeirdGradient(videoBuf, gameState->xOffset, gameState->yOffset);
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
