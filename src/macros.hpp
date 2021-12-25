#ifndef MACROS_H_
#define MACROS_H_

#ifdef TOM_INTERNAL
	#define TOM_ASSERT(x, ...)                                                            \
		{                                                                                 \
			if (!(x)) {                                                                   \
				tomato::util::Print("Assertion Failed in {0} at line {1}: {2}", __FILE__, \
									__LINE__, __VA_ARGS__);                               \
				__debugbreak();                                                           \
			}                                                                             \
		}
#else
	#define TOM_ASSERT(x, ...)
#endif

#define KILOBYTES(val) ((val)*1024)
#define MEGABYTES(val) (KILOBYTES(val) * 1024)
#define GIGABYTES(val) (MEGABYTES(val) * 1024)
#define TERABYTES(val) (GIGABYTES(val) * 1024)

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#if TOM_INTERNAL
	#define Assert(expression) \
		if (!(expression)) {   \
			*(int*)0 = 0;      \
		}
#elif
	#define Assert(expression)
#endif

#endif
