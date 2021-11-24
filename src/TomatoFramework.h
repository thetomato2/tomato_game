#pragma once

// C
#include <stdint.h>
#include <tchar.h>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cwchar>

// NOTE: use C++ stuff?
#define CXX 0
#if CXX
	#include <iostream>
#endif

#ifdef _WIN32
	#ifdef _EMACS
using wchar_t = uint16_t;
	#endif
	// WinHelp is deprecate
	#define NOHELP
	// DirectX apps don't need GDI
	// NOTE: I am using GDI to slowly blit to the screen
	//#define NODRAWTEXT
	//#define NOGDI
	//#define NOBITMAP
	//#define WIN32_LEAN_AND_MEAN

	// Use the C++ standard templated min/max
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <Windows.h>

	#include <xinput.h>

	#include <mmdeviceapi.h>
	#include <audioclient.h>

//#include <wrl/client.h>

//#include <d3d11_1.h>
//#include <dxgi1_6.h>

//#include <DirectXMath.h>
//#include <DirectXColors.h>
#endif

// STL

//#ifdef _DEBUG
//	#include <dxgidebug.h>
//#endif

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

#ifdef WIN32
using wchar = wchar_t;
#endif

// NOTE: for visual studio because its retarded and doesn't read the defines passed from the
// compiler for some reason
#ifndef TOM_INTERNAL
	#define TOM_INTERNAL
#endif

using bool32 = i32;

#define internal		static
#define local_persist	static
#define global_variable static

#include "Macros.h"
#include "Utils.h"
#include "Console.h"
