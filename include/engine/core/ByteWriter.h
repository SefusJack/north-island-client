#ifndef NORTH_ISLAND_CORE_BYTE_WRITER_H
#define NORTH_ISLAND_CORE_BYTE_WRITER_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace core {

    /// @brief Builds a byte block from little-endian values.
    class ByteWriter {
        public:
            /// @brief Appends one byte.
            /// @param value The byte.
            /// @return This writer.
            ByteWriter& u8(std::uint8_t value);

            /// @brief Appends a little-endian 16-bit value.
            /// @param value The value.
            /// @return This writer.
            ByteWriter& u16(std::uint16_t value);

            /// @brief Appends a little-endian 32-bit value.
            /// @param value The value.
            /// @return This writer.
            ByteWriter& u32(std::uint32_t value);

            /// @brief Appends a little-endian signed 32-bit value.
            /// @param value The value.
            /// @return This writer.
            ByteWriter& i32(std::int32_t value) { return u32(static_cast<std::uint32_t>(value)); }

            /// @brief Appends an IEEE double (a Delphi TDateTime).
            /// @param value The value.
            /// @return This writer.
            ByteWriter& f64(double value);

            /// @brief Appends raw bytes of a string, no length.
            /// @param value The text.
            /// @return This writer.
            ByteWriter& text(const std::string& value);

            /// @brief Appends a length byte followed by the bytes (cut at 255).
            /// @param value The text.
            /// @return This writer.
            ByteWriter& shortText(const std::string& value);

            /// @brief Appends a 32-bit length followed by the bytes.
            /// @param value The text.
            /// @return This writer.
            ByteWriter& longText(const std::string& value);

            /// @brief Appends raw bytes.
            /// @param value The bytes.
            /// @return This writer.
            ByteWriter& bytes(const Bytes& value);

            /// @brief Gives the bytes written so far.
            /// @return The block.
            const Bytes& data() const { return m_data; }

            /// @brief Moves the block out of the writer.
            /// @return The block.
            Bytes take() { return std::move(m_data); }

        private:
            Bytes m_data;
    };

}

#endif
