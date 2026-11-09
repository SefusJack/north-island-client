#include "client/models/chat/ChatLog.h"

#include "client/data/EmoteCodes.h"
#include "client/rules/TextFilter.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace client {

    namespace {

        constexpr std::size_t kKeptMessages = 100; // messages kept in the log
        constexpr std::size_t kMinLineChars = 8; // smallest allowed line width, bytes
        const std::string kContinuation = "    "; // four-space indent on continuation lines

        /// @brief Whether a byte is the lead byte of a Big5 pair.
        /// @param c The byte.
        /// @return True for a lead byte.
        bool isLeadByte(unsigned char c) { return c >= 0x81 && c <= 0xFE; } // Big5 lead byte range

        /// @brief The length of the next line's text: up to width bytes, never ending inside a Big5 pair.
        /// @param text The whole message.
        /// @param from Where the line starts.
        /// @param width Maximum bytes of the line.
        /// @return Length in bytes.
        std::size_t cutAt(const std::string& text, std::size_t from, std::size_t width) {
            std::size_t at = from;
            while (at < text.size()) {
                const std::size_t step = isLeadByte(static_cast<unsigned char>(text[at])) && at + 1 < text.size() ? 2 : 1; // a Big5 pair takes 2 bytes
                if (at + step - from > width) {
                    break;
                }
                at += step;
            }
            return at - from;
        }

    }

    void ChatLog::setChannelColor(int channel, std::uint16_t color) {
        if (channel >= 0 && channel < static_cast<int>(m_colors.size())) {
            m_colors[static_cast<std::size_t>(channel)] = color;
        }
    }

    std::size_t ChatLog::add(int channel, const std::string& message, std::int32_t senderId) {
        const std::size_t bodyAt = message.find(':');
        if (!isTextClean(bodyAt == std::string::npos ? message : message.substr(bodyAt + 1))) {
            return 0;
        }
        Message entry;
        entry.channel = channel;
        entry.text = message;
        entry.senderId = senderId;
        cut(entry);
        const std::size_t lines = entry.lines.size();
        m_messages.push_back(std::move(entry));
        if (m_messages.size() > kKeptMessages) {
            m_messages.pop_front();
        }
        return lines;
    }

    void ChatLog::setLineChars(std::size_t chars) {
        chars = std::max(chars, kMinLineChars);
        if (chars == m_lineChars) {
            return;
        }
        m_lineChars = chars;
        for (Message& message : m_messages) {
            cut(message);
        }
    }

    void ChatLog::cut(Message& entry) const {
        const std::string& message = entry.text;
        const int channel = entry.channel;
        const std::int32_t senderId = entry.senderId;
        entry.lines.clear();
        const std::size_t index = channel >= 0 && channel < static_cast<int>(m_colors.size()) ? static_cast<std::size_t>(channel) : 0;
        const core::Color color = core::Color::fromRgb565(m_colors[index]);
        std::string text = message;
        std::string emotes(message.size(), ' ');
        bool anyEmote = false;
        std::size_t from = 0;
        if (!message.empty() && message[0] == '(') {
            const std::size_t colon = message.find(':');
            from = colon == std::string::npos ? 0 : colon + 1;
        }
        for (std::size_t pos = from; pos < message.size();) {
            if (isLeadByte(static_cast<unsigned char>(message[pos]))) {
                pos += 2; // skip a Big5 pair
                continue;
            }
            const int emote = emoteAt(message, pos);
            if (emote == 0) {
                ++pos;
                continue;
            }
            for (std::size_t i = 0; i < emoteCode(emote).size(); ++i) {
                text[pos + i] = ' ';
                emotes[pos + i] = static_cast<char>(32 + emote); // emote marker: 32 + the emote's number
            }
            pos += emoteCode(emote).size();
            anyEmote = true;
        }
        std::size_t at = 0;
        do {
            const bool first = at == 0;
            const std::size_t width = first ? m_lineChars : m_lineChars - kContinuation.size();
            const std::size_t length = cutAt(message, at, width);
            const std::string indent = first ? std::string() : kContinuation;
            entry.lines.push_back({indent + text.substr(at, length), anyEmote ? indent + emotes.substr(at, length) : std::string(), color, channel, senderId});
            at += std::max<std::size_t>(length, 1);
        } while (at < message.size());
    }

    std::size_t ChatLog::lineCount() const {
        std::size_t count = 0;
        for (const Message& message : m_messages) {
            count += message.lines.size();
        }
        return count;
    }

    const ChatLine& ChatLog::line(std::size_t index) const {
        for (const Message& message : m_messages) {
            if (index < message.lines.size()) {
                return message.lines[index];
            }
            index -= message.lines.size();
        }
        return m_messages.back().lines.back();
    }

}
