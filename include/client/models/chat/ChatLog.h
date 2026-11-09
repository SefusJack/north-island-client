#ifndef NORTH_ISLAND_CLIENT_MODELS_CHAT_CHAT_LOG_H
#define NORTH_ISLAND_CLIENT_MODELS_CHAT_CHAT_LOG_H

#include "client/models/chat/ChatLine.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace client {

    /// @brief The chat's messages: the last 100, cut into lines of the box's width in characters, in their channel's colour.
    /// @details Original: TSe_CharMsg. 50 characters at first; Big5 pairs are not split; the lines after the first start with four spaces.
    class ChatLog {
        public:
            /// @brief Adds a message.
            /// @details Channels: 0 system, 1 world, 2 local, 3 whisper, 4 GM, 5 team, 6 guild, 7 ally.
            /// @param channel Channel number.
            /// @param message The message text.
            /// @param senderId The player who said it (0: none).
            /// @return How many lines it takes.
            std::size_t add(int channel, const std::string& message, std::int32_t senderId = 0);

            /// @brief Whether one of the last count messages has this channel and text.
            /// @param channel Channel number.
            /// @param message The message text.
            /// @param count How many of the newest messages to look at.
            /// @return True when found.
            bool hasRecent(int channel, const std::string& message, std::size_t count) const {
                std::size_t seen = 0;
                for (auto it = m_messages.rbegin(); it != m_messages.rend() && seen < count; ++it, ++seen) {
                    if (it->channel == channel && it->text == message) {
                        return true;
                    }
                }
                return false;
            }

            /// @brief Sets the width of the box in characters; all messages are cut again.
            /// @details Original: SetListBounds.
            /// @param chars Line width in characters.
            void setLineChars(std::size_t chars);

            /// @brief Sets the colour new messages of a channel get (the system window's channel colours).
            /// @param channel Channel number.
            /// @param color Colour as RGB565.
            void setChannelColor(int channel, std::uint16_t color);

            /// @brief Removes every message and its lines.
            /// @details Original: ClearChatList.
            void clear() { m_messages.clear(); }

            /// @brief Number of lines over all messages.
            /// @return Line count.
            std::size_t lineCount() const;

            /// @brief Line i, counted over all messages from the oldest.
            /// @param index Line index.
            /// @return The line.
            const ChatLine& line(std::size_t index) const;

        private:
            /// @brief One chat message with its cut lines.
            struct Message {
                int channel = 0;
                std::string text;
                std::int32_t senderId = 0;
                std::deque<ChatLine> lines;
            };

            /// @brief Cuts a message into lines of the box's width.
            /// @param message The message to cut.
            void cut(Message& message) const;

            std::deque<Message> m_messages;
            std::size_t m_lineChars = 50; // default box width in characters
            // 12 channels; RGB565 colour per channel
            std::array<std::uint16_t, 12> m_colors{0xF800, 0xFFB3, 0xFF80, 0xFC00, 0xFC00, 0xFFFF, 0x6E7E, 0x8E27, 0xC6F3, 0xFE31, 0xF800, 0xF800};
    };

}

#endif
