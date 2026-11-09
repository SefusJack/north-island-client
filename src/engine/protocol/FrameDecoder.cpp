#include "protocol/FrameDecoder.h"

#include "protocol/Framing.h"

#include <algorithm>

namespace protocol {

    void FrameDecoder::feed(const core::Bytes& received) {
        core::Bytes plain = received;
        applyStreamKey(plain);
        m_buffer.insert(m_buffer.end(), plain.begin(), plain.end());
        extract();
    }

    void FrameDecoder::extract() {
        std::size_t start = 0;
        // a frame head is 4 bytes: 2 magic bytes, then the u16 length (low byte at +2, high byte at +3, so << 8); out-of-sync data is dropped 2 bytes at a time
        while (m_buffer.size() - start >= 4) {
            if (m_buffer[start] != kFrameMagic0 || m_buffer[start + 1] != kFrameMagic1) {
                start += 2;
                continue;
            }
            const std::size_t length = static_cast<std::size_t>(m_buffer[start + 2] | (m_buffer[start + 3] << 8));
            if (m_buffer.size() - start - 4 < length) {
                break;
            }
            const auto begin = m_buffer.begin() + static_cast<std::ptrdiff_t>(start + 4);
            m_payloads.emplace_back(begin, begin + static_cast<std::ptrdiff_t>(length));
            start += 4 + length;
        }
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(std::min(start, m_buffer.size())));
    }

    std::optional<core::Bytes> FrameDecoder::next() {
        if (m_payloads.empty()) {
            return std::nullopt;
        }
        core::Bytes payload = std::move(m_payloads.front());
        m_payloads.pop_front();
        return payload;
    }

}
