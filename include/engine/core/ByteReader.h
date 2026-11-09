#ifndef NORTH_ISLAND_CORE_BYTE_READER_H
#define NORTH_ISLAND_CORE_BYTE_READER_H

#include "core/Bytes.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace core {

    /// @brief Reads little-endian values from a byte block; reading past the end yields zeros and sets the failed flag instead of throwing.
    class ByteReader {
        public:
            /// @brief Starts reading a block.
            /// @param data The bytes to read (must outlive the reader).
            /// @param start Offset to begin at.
            explicit ByteReader(
                const Bytes& data,
                std::size_t start = 0
            )
                : m_data(data),
                  m_position(start) {}

            /// @brief Reads one byte.
            /// @return The value, 0 past the end.
            std::uint8_t u8();

            /// @brief Reads a little-endian 16-bit value.
            /// @return The value, 0 past the end.
            std::uint16_t u16();

            /// @brief Reads a little-endian 32-bit value.
            /// @return The value, 0 past the end.
            std::uint32_t u32();

            /// @brief Reads a little-endian signed 32-bit value.
            /// @return The value, 0 past the end.
            std::int32_t i32() { return static_cast<std::int32_t>(u32()); }

            /// @brief Reads an IEEE double (a Delphi TDateTime on the wire).
            /// @return The value, 0 past the end.
            double f64();

            /// @brief Reads raw bytes as a string (no terminator, no length prefix).
            /// @param count Number of bytes.
            /// @return The text, empty past the end.
            std::string text(std::size_t count);

            /// @brief Reads a length byte followed by that many bytes.
            /// @return The text.
            std::string shortText() { return text(u8()); }

            /// @brief Reads a 32-bit length followed by that many bytes (the notice boards' strings).
            /// @return The text, empty for a non-positive length.
            std::string longText();

            /// @brief Reads everything up to the end.
            /// @return The remaining bytes as text.
            std::string rest() { return text(remaining()); }

            /// @brief Reads raw bytes.
            /// @param count Number of bytes.
            /// @return The bytes, empty past the end.
            Bytes bytes(std::size_t count);

            /// @brief Skips bytes.
            /// @param count Number of bytes.
            void skip(std::size_t count);

            /// @brief Moves to an absolute offset; past the end sets the failed flag.
            /// @param position New offset.
            void seek(std::size_t position);

            /// @brief Gives the read offset.
            /// @return Current position.
            std::size_t position() const { return m_position; }

            /// @brief Gives the block size.
            /// @return Total bytes.
            std::size_t size() const { return m_data.size(); }

            /// @brief Gives the bytes left.
            /// @return Bytes between the position and the end.
            std::size_t remaining() const { return m_position < m_data.size() ? m_data.size() - m_position : 0; }

            /// @brief Tells whether all bytes were read.
            /// @return True when nothing remains.
            bool atEnd() const { return remaining() == 0; }

            /// @brief Tells whether a read ran past the end.
            /// @return True after any failed read.
            bool failed() const { return m_failed; }

        private:
            /// @brief Checks that count bytes remain; otherwise sets the failed flag and moves to the end.
            /// @param count Bytes about to be read.
            /// @return True when they remain.
            bool take(std::size_t count);

            const Bytes& m_data;
            std::size_t m_position = 0;
            bool m_failed = false;
    };

}

#endif
