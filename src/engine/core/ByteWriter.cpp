#include "core/ByteWriter.h"

#include <algorithm>
#include <cstring>

namespace core {

    ByteWriter& ByteWriter::u8(std::uint8_t value) {
        m_data.push_back(value);
        return *this;
    }

    ByteWriter& ByteWriter::u16(std::uint16_t value) {
        m_data.push_back(static_cast<std::uint8_t>(value));
        m_data.push_back(static_cast<std::uint8_t>(value >> 8)); // high byte
        return *this;
    }

    ByteWriter& ByteWriter::u32(std::uint32_t value) {
        for (int i = 0; i < 4; ++i) { // 4 bytes, lowest first
            m_data.push_back(static_cast<std::uint8_t>(value >> (8 * i))); // 8 bits per byte
        }
        return *this;
    }

    ByteWriter& ByteWriter::f64(double value) {
        std::uint64_t bits = 0;
        std::memcpy(&bits, &value, sizeof bits);
        for (int i = 0; i < 8; ++i) { // 8 bytes, lowest first
            m_data.push_back(static_cast<std::uint8_t>(bits >> (8 * i))); // 8 bits per byte
        }
        return *this;
    }

    ByteWriter& ByteWriter::text(const std::string& value) {
        m_data.insert(m_data.end(), value.begin(), value.end());
        return *this;
    }

    ByteWriter& ByteWriter::longText(const std::string& value) {
        i32(static_cast<std::int32_t>(value.size()));
        return text(value);
    }

    ByteWriter& ByteWriter::shortText(const std::string& value) {
        const std::size_t length = std::min<std::size_t>(value.size(), 255); // a length byte holds at most 255
        u8(static_cast<std::uint8_t>(length));
        m_data.insert(m_data.end(), value.begin(), value.begin() + static_cast<std::ptrdiff_t>(length));
        return *this;
    }

    ByteWriter& ByteWriter::bytes(const Bytes& value) {
        m_data.insert(m_data.end(), value.begin(), value.end());
        return *this;
    }

}
