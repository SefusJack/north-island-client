#include "core/ByteReader.h"

#include <cstring>

namespace core {

    bool ByteReader::take(std::size_t count) {
        if (remaining() < count) {
            m_failed = true;
            m_position = m_data.size();
            return false;
        }
        return true;
    }

    std::string ByteReader::longText() {
        const std::int32_t count = i32();
        return count > 0 ? text(static_cast<std::size_t>(count)) : std::string();
    }

    std::uint8_t ByteReader::u8() {
        if (!take(1)) {
            return 0;
        }
        return m_data[m_position++];
    }

    std::uint16_t ByteReader::u16() {
        if (!take(2)) { // a 16-bit value is 2 bytes
            return 0;
        }
        const std::uint16_t value = static_cast<std::uint16_t>(m_data[m_position] | (m_data[m_position + 1] << 8)); // high byte shifted by 8 bits
        m_position += 2; // 2 bytes read
        return value;
    }

    std::uint32_t ByteReader::u32() {
        if (!take(4)) { // a 32-bit value is 4 bytes
            return 0;
        }
        std::uint32_t value = 0;
        for (int i = 3; i >= 0; --i) { // highest byte (index 3) first
            value = (value << 8) | m_data[m_position + static_cast<std::size_t>(i)]; // 8 bits per byte
        }
        m_position += 4; // 4 bytes read
        return value;
    }

    double ByteReader::f64() {
        if (!take(8)) { // a double is 8 bytes
            return 0.0;
        }
        std::uint64_t bits = 0;
        for (int i = 7; i >= 0; --i) { // highest byte (index 7) first
            bits = (bits << 8) | m_data[m_position + static_cast<std::size_t>(i)]; // 8 bits per byte
        }
        m_position += 8; // 8 bytes read
        double value = 0.0;
        std::memcpy(&value, &bits, sizeof value);
        return value;
    }

    std::string ByteReader::text(std::size_t count) {
        if (!take(count)) {
            return {};
        }
        std::string value(m_data.begin() + static_cast<std::ptrdiff_t>(m_position), m_data.begin() + static_cast<std::ptrdiff_t>(m_position + count));
        m_position += count;
        return value;
    }

    Bytes ByteReader::bytes(std::size_t count) {
        if (!take(count)) {
            return {};
        }
        Bytes value(m_data.begin() + static_cast<std::ptrdiff_t>(m_position), m_data.begin() + static_cast<std::ptrdiff_t>(m_position + count));
        m_position += count;
        return value;
    }

    void ByteReader::skip(std::size_t count) {
        if (take(count)) {
            m_position += count;
        }
    }

    void ByteReader::seek(std::size_t position) {
        if (position > m_data.size()) {
            m_failed = true;
            m_position = m_data.size();
            return;
        }
        m_position = position;
    }

}
