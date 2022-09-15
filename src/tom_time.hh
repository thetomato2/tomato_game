#ifndef TOM_TIME_HH
#define TOM_TIME_HH

#include "tom_core.hh"

namespace tom
{

#ifdef TOM_INTERNAL

enum
{

    debug_CycleCounter_ProcessPixel,
    debug_CycleCounter_DrawRect,
    debug_CycleCounter_Render,
    debug_CycleCounter_Count
};

struct debug_CycleCounter
{
    u32 hit_cnt;
    u64 cycle_cnt;
};

    #define BEGIN_TIMED_BLOCK(ID) u64 start_cycle_cnt_##ID = __rdtsc()
    #define END_TIMED_BLOCK(ID)                                          \
        debug_global_mem->counters[debug_CycleCounter_##ID].cycle_cnt += \
            __rdtsc() - start_cycle_cnt_##ID;                            \
        ++debug_global_mem->counters[debug_CycleCounter_##ID].hit_cnt
    #define END_TIMED_BLOCK_COUNTED(ID, CNT)                                    \
        {                                                                       \
            debug_global_mem->counters[debug_CycleCounter_##ID].cycle_cnt +=    \
                __rdtsc() - start_cycle_cnt_##ID;                               \
            debug_global_mem->counters[debug_CycleCounter_##ID].hit_cnt += CNT; \
        }

#else
    #define BEGIN_TIMED_BLOCK(ID)
    #define END_TIMED_BLOCK(ID)
    #define END_TIMED_BLOCK_COUNTED(ID, CNT)
#endif

inline i64 get_time()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart;
}

inline f32 get_seconds_elapsed(i64 start, i64 end, i64 performance_counter_frequency)
{
    return (f32)(end - start) / (f32)performance_counter_frequency;
}

}  // namespace tom

#endif
