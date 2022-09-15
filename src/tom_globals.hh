#include "tom_core.hh"

#ifdef TOM_INTERNAL
struct AppMemory;
extern AppMemory *debug_global_mem;
#endif

global constexpr u32 g_max_ent_cnt = 65536;
global f32 g_meters_to_pixels      = 30.0f;

