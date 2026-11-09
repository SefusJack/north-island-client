#include "client/models/chat/ChatSilence.h"

namespace client {

    std::string ChatSilence::silence(int kind, int mask, std::uint32_t minutes) {
        // channel names by input bar channel number 1..7, index 0 unused
        static const char* const kNames[] = {"", "World", "Local", "Whisper", "Party", "Guild", "Ally", "GM"};
        m_channels.fill(false);
        std::string channels;
        for (int channel = 1; channel <= 7 && mask != 0; ++channel, mask /= 2) { // channels 1..7; one mask bit per channel
            if (mask % 2 == 1) {
                m_channels[static_cast<std::size_t>(channel)] = true;
                if (!channels.empty()) {
                    channels += ",";
                }
                channels += kNames[channel];
            }
        }
        const std::string tail = std::to_string(minutes) + " minute (Silence:" + channels + ")";
        if (kind == 1) { // kind 1: spam
            m_mail = true;
            return "Please do not SPAM." + tail;
        }
        if (kind == 2) { // kind 2: a GM's order
            m_mail = true;
            return "GM forbids spam." + tail;
        }
        return {};
    }

    void ChatSilence::release(bool mailToo) {
        m_channels.fill(false);
        if (mailToo) {
            m_mail = false;
        }
    }

}
