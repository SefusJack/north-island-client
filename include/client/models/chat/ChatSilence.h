#ifndef NORTH_ISLAND_CLIENT_MODELS_CHAT_CHAT_SILENCE_H
#define NORTH_ISLAND_CLIENT_MODELS_CHAT_CHAT_SILENCE_H

#include <array>
#include <cstdint>
#include <string>

namespace client {

    /// @brief The channels a GM has silenced for the player.
    /// @details Original: TPlayer.FChatSilenced and Se_SendMailForm.FChatSilenced, RecvChatSilence. Indexed by the input bar's channel numbers (1 world, 2 local, 3 whisper, 4 team, 5 guild, 6 ally, 7 GM).
    class ChatSilence {
        public:
            /// @brief Silences the channels of the bits of mask (bit 0 is channel 1) for some minutes.
            /// @details Original: RecvChatSilence. Kind 1 is spam, 2 a GM's order (others change nothing).
            /// @param kind Silence kind.
            /// @param mask Channel bit mask.
            /// @param minutes Minutes of silence.
            /// @return The line the game puts in the chat, "" for none.
            std::string silence(int kind, int mask, std::uint32_t minutes);

            /// @brief Frees the channels again.
            /// @details Server 16/8 (with the mail and quick phrases) and 16/13.
            /// @param mailToo Whether the mail and quick phrases are freed too.
            void release(bool mailToo);

            /// @brief Whether a channel is silenced.
            /// @param channel Input bar channel number 1..7.
            /// @return True when silenced.
            bool isSilenced(int channel) const { return channel >= 1 && channel <= 7 && m_channels[static_cast<std::size_t>(channel)]; }

            /// @brief Whether the quick phrases and the mail are refused.
            /// @details Original: Se_SendMailForm.FChatSilenced.
            /// @return True when refused.
            bool isMailSilenced() const { return m_mail; }

            /// @brief The text shown when a silenced player tries to talk.
            /// @return The refusal text.
            static const char* refusal() { return "Unable to enter words because you are forbidden."; }

        private:
            std::array<bool, 8> m_channels{}; // channels 1..7, index 0 unused
            bool m_mail = false;
    };

}

#endif
