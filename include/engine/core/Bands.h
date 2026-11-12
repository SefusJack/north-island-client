#ifndef NORTH_ISLAND_CORE_BANDS_H
#define NORTH_ISLAND_CORE_BANDS_H

#include <algorithm>
#include <cstddef>
#include <thread>
#include <vector>

namespace core {

    /// @brief Runs work over the range [0, count) in bands, one band per hardware thread, when the range is large enough.
    /// @details The calling thread takes the first band and waits for the others. Each band must touch only its own part of
    /// the data, so the result is the same as one pass over the whole range.
    /// @param count Size of the range.
    /// @param fromCount The smallest range worth splitting; a smaller one runs as one band on this thread.
    /// @param work Called as work(begin, end) for each band.
    template <class Work>
    void inBands(std::size_t count, std::size_t fromCount, const Work& work) {
        const std::size_t threadCount = std::max(1u, std::thread::hardware_concurrency());
        if (count < fromCount || threadCount < 2) {
            work(std::size_t{0}, count);
            return;
        }
        const std::size_t bandSize = (count + threadCount - 1) / threadCount;
        std::vector<std::thread> threads;
        for (std::size_t begin = bandSize; begin < count; begin += bandSize) {
            threads.emplace_back([&work, begin, end = std::min(count, begin + bandSize)] { work(begin, end); });
        }
        work(std::size_t{0}, std::min(count, bandSize));
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

}

#endif
