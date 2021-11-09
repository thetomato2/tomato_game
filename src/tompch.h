#pragma once

#if 0
	#include <winsdkver.h>
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0603
	#endif
	#include <sdkddkver.h>
#endif

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
//#define NODRAWTEXT
//#define NOGDI
//#define NOBITMAP

// Include <mcx.h> if you need this
//#define NOMCX

// Include <winsvc.h> if you need this
//#define NOSERVICE

// WinHelp is deprecated
//#define NOHELP

//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <xinput.h>

#include <mmdeviceapi.h>
#include <audioclient.h>

//#include <wrl/client.h>

//#include <d3d11_1.h>
//#include <dxgi1_6.h>

//#include <DirectXMath.h>
//#include <DirectXColors.h>

// C
#include <tchar.h>

// STL
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <fstream>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

//#ifdef _DEBUG
//	#include <dxgidebug.h>
//#endif

// TomatoDX
#include "BaseTypes.h"
#include "Macros.h"
#include "Utils.h"
#include "Console.h"
#include "../resource.h"
//#include "../resource.h"
