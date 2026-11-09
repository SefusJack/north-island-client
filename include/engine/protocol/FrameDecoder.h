#ifndef NORTH_ISLAND_PROTOCOL_FRAME_DECODER_H
#define NORTH_ISLAND_PROTOCOL_FRAME_DECODER_H

#include "core/Bytes.h"

#include <deque>
#include <optional>

namespace protocol {

    /// @brief Collects received (still XORed) bytes and cuts them into message payloads.
    /// @details Like the original client it drops two bytes at a time while the buffer does not start with the frame magic.
    class FrameDecoder {
        public:
            /// @brief Appends received bytes and extracts any complete frames.
            /// @param received Bytes as read from the socket.
            void feed(const core::Bytes& received);

            /// @brief Takes the next complete payload.
            /// @return The payload, or nothing when none is ready.
            std::optional<core::Bytes> next();

        private:
            /// @brief Cuts complete frames out of the buffer into the payload queue.
            void extract();

            core::Bytes m_buffer;
            std::deque<core::Bytes> m_payloads;
    };

}

#endif
