#include "client/services/frame/FrameStats.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace client {

    void FrameStats::add(const FrameSample& sample) {
        m_samples[m_next] = sample;
        m_next = (m_next + 1) % kCapacity;
        m_size = std::min(m_size + 1, kCapacity);
    }

    FrameSample FrameStats::average() const {
        FrameSample sum;
        if (m_size == 0) {
            return sum;
        }
        double gpuSum = 0;
        std::size_t gpuCount = 0;
        for (std::size_t i = 0; i < m_size; ++i) {
            const FrameSample& s = at(i);
            sum.eventsMs += s.eventsMs;
            sum.updateMs += s.updateMs;
            sum.drawMs += s.drawMs;
            sum.presentMs += s.presentMs;
            sum.waitMs += s.waitMs;
            sum.totalMs += s.totalMs;
            if (s.gpuMs) {
                gpuSum += *s.gpuMs;
                ++gpuCount;
            }
        }
        const double n = static_cast<double>(m_size);
        sum.eventsMs /= n;
        sum.updateMs /= n;
        sum.drawMs /= n;
        sum.presentMs /= n;
        sum.waitMs /= n;
        sum.totalMs /= n;
        if (gpuCount > 0) {
            sum.gpuMs = gpuSum / static_cast<double>(gpuCount);
        }
        return sum;
    }

    double FrameStats::fps() const {
        const double mean = average().totalMs;
        return mean > 0 ? 1000.0 / mean : 0; // 1000 ms per second
    }

    double FrameStats::lowFps() const {
        if (m_size == 0) {
            return 0;
        }
        std::vector<double> totals;
        totals.reserve(m_size);
        for (std::size_t i = 0; i < m_size; ++i) {
            totals.push_back(at(i).totalMs);
        }
        std::sort(totals.begin(), totals.end(), std::greater<>());
        const std::size_t slowest = std::max<std::size_t>(1, m_size / 100); // the slowest 1% of the frames
        double sum = 0;
        for (std::size_t i = 0; i < slowest; ++i) {
            sum += totals[i];
        }
        const double mean = sum / static_cast<double>(slowest);
        return mean > 0 ? 1000.0 / mean : 0; // 1000 ms per second
    }

}
