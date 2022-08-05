namespace tom
{
#if 0
struct cycle_counter
{
    enum type
    {
        temp1,
        update,
        total,
        count
    };

    type t_;
    u64 cycle_cnt;
};

inline constexpr const char* get_cycle_counter_name(cycle_counter::type type)
{
    switch (type) {
        case cycle_counter::type::temp1: return "temp1";
        case cycle_counter::type::update: return "update";
        case cycle_counter::type::total: return "total";
        case cycle_counter::type::count: return "count";

        default: {
            INVALID_CODE_PATH;
        } break;
    }

    return nullptr;
}


inline cycle_counter* get_cycle_counter(vector<cycle_counter>* counters, cycle_counter::type type)
{
    for (auto& counter : *counters) {
        if (counter.t_ == type) {
            return &counter;
        }
    }

    return nullptr;
}

    #define BEGIN_TIMED_BLOCK(ID) u64 start_cycle_cnt_##ID = __rdtsc()
    #define END_TIMED_BLOCK(ID)                                                                \
        {                                                                                      \
            auto* counter      = get_cycle_counter(&state->counters, cycle_counter::type::ID); \
            counter->cycle_cnt = __rdtsc() - start_cycle_cnt_##ID;                             \
        }

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