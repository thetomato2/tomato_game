#include "TomatoGame.h"

namespace tomato
{
namespace
{
void
RenderWeirdGradient(GameOffscreenBuffer& buf, i32 xOffset, i32 yOffset, f32 fader)
{
	i32 width  = buf.width;
	i32 height = buf.height;

	u8* row = (u8*)buf.mem;
	for (i32 y = 0; y < height; ++y) {
		u32* pixel = (u32*)row;
		for (i32 x = 0; x < width; ++x) {
			/*					  1  2  3  4
					pixel in mem: 00 00 00 00 BB GG RR xx
					0x xxRRGGBB
			*/

			u8 blue	 = x + xOffset;
			u8 green = y + yOffset;
			u8 red	 = 0;

			*pixel++ = (green << 8) | blue;
		}
		row += buf.pitch;
	}
}

void
DrawFloor(GameOffscreenBuffer& buf, i32 floorY)
{
	u8* row = (u8*)buf.mem + (floorY * buf.pitch) + (buf.bytesPerPix * buf.width);
	for (i32 y = floorY; y < buf.height - 1; ++y) {
		u32* pixel = (u32*)row;
		for (i32 x = 0; x < buf.width; ++x) {
			*pixel++ = 0xFF000000;
		}
		row += buf.pitch;
	}
}

void
DrawSquare(GameOffscreenBuffer& buf, i32 posX, i32 posY, i32 size, Color color)
{
	u8* endOfBuf = (u8*)buf.mem + buf.bytesPerPix * buf.width + buf.pitch * buf.height;
	for (i32 x = posX > 0 ? posX : 0; x < posX + size && x < buf.width; ++x) {
		u8* pixel = ((u8*)buf.mem + x * buf.bytesPerPix + posY * buf.pitch);
		for (i32 y = posY > 0 ? posY : size; y < posY + size && y < buf.height; ++y) {
			*(u32*)pixel = color.bgra;
			pixel += buf.pitch;
		}
	}
}

void
RenderPlayer(GameOffscreenBuffer& buf, Player& player)
{
	DrawSquare(buf, player.pos.x, player.pos.y, player.size, player.color);
}

void
GameOuputSound(GameSoundOutputBuffer& soundBuffer, i32 toneHz, f32 tSine)
{
	static f32 tempSine {};
	i16 toneVolume = 3000;
	i32 wavePeriod = soundBuffer.samplesPerSecond / toneHz;

	i16* sampleOut = soundBuffer.samples;
	for (szt sampleIndex = 0; sampleIndex < soundBuffer.sampleCount; ++sampleIndex) {
		f32 sineValue	= sinf(tempSine * 0.75f);
		i16 sampleValue = (i16)(sineValue * toneVolume);
		*sampleOut++	= sampleValue;
		*sampleOut++	= sampleValue;

		tempSine += 2.0f * 3.14f * 1.0f / (f32)wavePeriod;
		/*if (tSine > 2.f * util::pi32) {
			tSine -= 2.f * util::pi32;
		}*/
	}
}
}  // namespace

extern "C" TOM_DLL_EXPORT GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	auto* gameState = (GameState*)mem.permanentStorage;
	GameOuputSound(soudBuf, gameState->toneHz, gameState->tSine);
}

extern "C" TOM_DLL_EXPORT GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
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

		gameState.gravity			 = -1.f;
		gameState.player1.pos.x		 = 100;
		gameState.player1.size		 = 20;
		gameState.player1.isJump	 = false;
		gameState.player1.velocity	 = 0.f;
		gameState.floorY			 = videoBuf.height - 150;
		gameState.player1.pos.y		 = gameState.floorY - gameState.player1.size;
		gameState.player1.color.bgra = 0xffffffff;

		gameState.playerLast = gameState.player1;

		gameState.mouseTrails[0].color.bgra = 0xffff0000;
		gameState.mouseTrails[1].color.bgra = 0xff00ff00;
		gameState.mouseTrails[2].color.bgra = 0xff0000ff;

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

		gameState.player1.pos.x += (i32)(playerSpeed * controller0.endLX);
	} else {
		// TODO: handle digital input
	}

	auto& player = gameState.player1;

	if (player.pos.y > gameState.floorY - player.size) {
		player.pos.y	  = gameState.floorY - player.size;
		gameState.yOffset = 0;
		player.isJump	  = false;
	}

	if (player.isJump) {
		player.pos.y -= (i32)player.velocity;
		gameState.yOffset -= i32(player.velocity * offsetYMul);
		player.velocity += gameState.gravity;
	}

	if (controller0.button_A.endedDown && !player.isJump) {
		player.velocity = 20.f;
		player.isJump	= true;
	}

	if (player.pos.x < i32((f32)videoBuf.width * 0.33f)) {
		i32 xMove = i32((player.pos.x - gameState.playerLast.pos.x) * offsetXMul);
		if (xMove < 0) {
			gameState.xOffset += xMove;
			player.pos.x = gameState.playerLast.pos.x;
		}
	}
	if (player.pos.x > i32((f32)videoBuf.width * 0.66f)) {
		i32 xMove = i32((player.pos.x - gameState.playerLast.pos.x) * offsetXMul);
		if (xMove > 0) {
			gameState.xOffset += xMove;
			player.pos.x = gameState.playerLast.pos.x;
		}
	}

	if (player.pos.y > gameState.floorY - player.size) {
		player.pos.y	  = gameState.floorY - player.size;
		gameState.yOffset = 0;
		player.isJump	  = false;
	}

	RenderWeirdGradient(videoBuf, gameState.xOffset, gameState.yOffset, gameState.fader);
	DrawFloor(videoBuf, gameState.floorY);
	RenderPlayer(videoBuf, player);

	gameState.playerLast = player;

	for (szt curMsBut {}; curMsBut < GameInput::nMouseButtons; ++curMsBut) {
		if (input.mouseButtons[curMsBut].endedDown) {
			i32 mouseSz	   = 10;
			i32 x		   = input.mouseX + (curMsBut * 20);
			auto& curTrail = gameState.mouseTrails[curMsBut];
			DrawSquare(videoBuf, x, input.mouseY, mouseSz, curTrail.color);
			curTrail.trails[curTrail.curInd].x = x;
			curTrail.trails[curTrail.curInd].y = input.mouseY;
			++curTrail.curInd;

			if (curTrail.curInd == MouseTrails::nTrails) curTrail.curInd = 0;
			for (szt i {}; i < MouseTrails::nTrails; ++i) {
				DrawSquare(videoBuf, curTrail.trails[i].x, curTrail.trails[i].y, mouseSz,
						   curTrail.color);
			}
		}
	}
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
