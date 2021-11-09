#pragma once

#ifdef _DEBUG
	#define TOM_INTERNAL
	#define TOM_DEBUG
	#define TOM_ASSERT(x, ...)                                                                                    \
		{                                                                                                         \
			if (!(x)) {                                                                                           \
				tomato::util::Print("Assertion Failed in {0} at line {1}: {2}", __FILE__, __LINE__, __VA_ARGS__); \
				__debugbreak();                                                                                   \
			}                                                                                                     \
		}
#else
	#define TOM_ASSERT(x, ...)
#endif
