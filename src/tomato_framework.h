#ifndef TOMATO_FRAMEWORK_H_
#define TOMATO_FRAMEWORK_H_
// C
#include <tchar.h>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cwchar>

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
	#include <windows.h>

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

// NOTE: for visual studio because its retarded and doesn't read the defines passed from the
// compiler for some reason
#ifndef TOM_INTERNAL
	#define TOM_INTERNAL
#endif

#define internal		static
#define local_persist	static
#define global_variable static

#include "base_types.h"
#include "macros.h"
#include "tomato_math.h"
#include "utils.h"
#include "console.h"

#endif
