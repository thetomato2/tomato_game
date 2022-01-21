#pragma once
#include <stdint.h>

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using szt     = size_t;
using mem_ind = size_t;
using byt     = u8;

#ifdef _WIN32
using wchar = wchar_t;
using ul    = unsigned long;
using ull   = unsigned long long;
#endif

using bool32 = s32;
