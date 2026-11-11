#ifndef NORTH_ISLAND_CORE_CLOCK_H
#define NORTH_ISLAND_CORE_CLOCK_H

#include <chrono>
#include <cstdint>

namespace core {

    /// @brief Reads a monotonic clock in whole milliseconds.
    /// @return Milliseconds (the game's GetTickCount / timeGetTime).
    inline std::uint64_t nowMs() {
        using namespace std::chrono;
        return static_cast<std::uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }

    /// @brief Reads the same clock with a fraction, to time the parts of a frame.
    /// @return Milliseconds with fraction.
    inline double nowPreciseMs() {
        using namespace std::chrono;
        return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
    }

}

#endif
