#include "core/Color.h"

namespace core {

    Color Color::fromRgb(std::uint32_t rgb) {
        // red is bits 16..23, green bits 8..15; 255 = opaque alpha
        return {static_cast<std::uint8_t>(rgb >> 16), static_cast<std::uint8_t>(rgb >> 8), static_cast<std::uint8_t>(rgb), 255};
    }

    Color Color::fromRgb565(std::uint16_t value) {
        // RGB565: red bits 11..15, green bits 5..10 (6 bits), blue bits 0..4; masks 0x1F/0x3F keep 5/6 bits
        const int r5 = (value >> 11) & 0x1F;
        const int g6 = (value >> 5) & 0x3F;
        const int b5 = value & 0x1F;
        // bit replication widens 5/6 bits to 8; 255 = opaque alpha
        return {
            static_cast<std::uint8_t>((r5 << 3) | (r5 >> 2)),
            static_cast<std::uint8_t>((g6 << 2) | (g6 >> 4)),
            static_cast<std::uint8_t>((b5 << 3) | (b5 >> 2)),
            255
        };
    }

    std::uint16_t Color::toRgb565() const {
        return static_cast<std::uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)); // RGB565: red at bit 11, green at bit 5, blue at bit 0
    }

    std::optional<Color> Color::parse(const std::string& text) {
        if (text.empty() || text[0] != '#') {
            return std::nullopt;
        }
        const std::string digits = text.substr(1);
        if (digits.size() != 6 && digits.size() != 8) { // RRGGBB or AARRGGBB hex digit counts
            return std::nullopt;
        }
        std::uint32_t value = 0;
        for (char c : digits) {
            int nibble = 0;
            if (c >= '0' && c <= '9') {
                nibble = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                nibble = c - 'a' + 10; // 'a' is hex digit 10
            } else if (c >= 'A' && c <= 'F') {
                nibble = c - 'A' + 10; // 'A' is hex digit 10
            } else {
                return std::nullopt;
            }
            value = (value << 4) | static_cast<std::uint32_t>(nibble); // 4 bits per hex digit
        }
        Color color = fromRgb(value & 0xFFFFFF); // mask keeps the RRGGBB part
        if (digits.size() == 8) { // AARRGGBB form
            color.a = static_cast<std::uint8_t>(value >> 24); // alpha is the top byte
        }
        return color;
    }

}
