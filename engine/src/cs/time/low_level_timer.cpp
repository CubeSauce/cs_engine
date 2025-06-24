#include "cs/time/low_level_timer.hpp"

#include <iostream>

#if defined(__APPLE__) && defined(__MACH__)
    #include <mach/mach_time.h>
    uint64 get_ticks() {
        return mach_absolute_time();
    }

    uint64 convert_ticks_to_ns(uint64 ticks) 
    {
        mach_timebase_info_data_t timebase_info;
        mach_timebase_info(&timebase_info);
        return ticks * timebase_info.numer / timebase_info.denom;
    }

#elif defined(__GNUC__) || defined(WIN32)
    #include <intrin.h>

    uint64 get_ticks()
    {
        return __rdtsc();  // On x86 systems, this works for Intel/AMD
    }

    uint64 convert_ticks_to_ns(uint64 ticks)
    {
        // You would need a TSC calibration step here for cross-platform accuracy.
        return ticks;  // Placeholder
    }

#else
    #error "Unsupported platform"
#endif

uint64 get_ns()
{
    return convert_ticks_to_ns(get_ticks());
}