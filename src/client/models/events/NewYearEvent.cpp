#include "client/models/events/NewYearEvent.h"

namespace client {

    namespace {

        constexpr int kFreePlayCount = 7;

    }

    void NewYearEvent::setOn(bool on) {
        m_on = on;
        if (!on) {
            m_lucky7Count = 0;
        }
    }

    void NewYearEvent::setLucky7Count(int count) { m_lucky7Count = count; }

    bool NewYearEvent::freePlay() const { return m_on && m_lucky7Count >= kFreePlayCount; }

}
