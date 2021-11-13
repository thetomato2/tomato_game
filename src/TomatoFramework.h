#pragma once

// C
#include <stdint.h>
#include <tchar.h>

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// WinHelp is deprecated
//#define NOHELP

//  TODO: this is temp, fix build stuff later, prob with cmake
#ifndef _DEBUG
	#define _DEBUG
#endif

#ifdef _WIN32
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

using bool32 = i32;

#define internal		static
#define local_persist	static
#define global_variable static

#include "Macros.h"
#include "Utils.h"
#include "Console.h"
#include "../resource.h"
