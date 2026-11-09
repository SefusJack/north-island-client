#include "client/services/frame/FrameLog.h"

namespace client {

    FrameLog::FrameLog(
        const std::filesystem::path& file
    )
        : m_file(file, std::ios::trunc) {
        if (m_file) {
            m_file << "events,update,draw,present,wait,total,gpu\n";
        }
    }

    void FrameLog::add(const FrameSample& sample) {
        if (!m_file) {
            return;
        }
        m_file << sample.eventsMs << ',' << sample.updateMs << ',' << sample.drawMs << ',' << sample.presentMs << ',' << sample.waitMs << ',' << sample.totalMs
               << ',';
        if (sample.gpuMs) {
            m_file << *sample.gpuMs;
        }
        m_file << '\n';
    }

}
