#include "client/models/system/SystemOptions.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr std::array<int, 4> kSwitchSubs = {1, 2, 4, 5};

    }

    std::pair<int, int> SystemOptions::flip(Switch which) {
        int& value = m_switches[index(which)];
        value = value == 1 ? 2 : 1; // state 1 on, 2 off
        return {kSwitchSubs[index(which)], value};
    }

    void SystemOptions::setServerStates(const std::array<int, 4>& switches) {
        for (std::size_t i = 0; i < switches.size(); ++i) {
            if (switches[i] == 1 || switches[i] == 2) { // state 1 on, 2 off
                m_switches[i] = switches[i];
            }
        }
    }

    bool SystemOptions::stepColor(int channel, int step) {
        if (channel < 0 || channel >= static_cast<int>(settings.channelColors.size())) {
            return false;
        }
        return stepValue(settings.channelColors[static_cast<std::size_t>(channel)], step, 1); // colours start at 1
    }

    bool SystemOptions::stepValue(int& value, int step, int lowest) {
        const int next = std::clamp(value + step, lowest, kMaxValue);
        if (next == value) {
            return false;
        }
        value = next;
        return true;
    }

    int SystemOptions::loudnessPercent(int volume) {
        // 100 percent; base 10; 0.15 = 3 dB per volume step (300 hundredths of a decibel) over 20
        return static_cast<int>(std::lround(100.0 * std::pow(10.0, -(kMaxValue - volume) * 0.15)));
    }

}
