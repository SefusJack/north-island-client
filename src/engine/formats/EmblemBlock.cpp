#include "formats/EmblemBlock.h"

#include <algorithm>

namespace formats {

    EmblemBlock EmblemBlock::parse(const core::Bytes& data) {
        EmblemBlock block;
        std::copy_n(data.begin(), std::min(data.size(), kBytes), block.m_bytes.begin());
        return block;
    }

    int EmblemBlock::pixel(int index) const {
        const std::uint8_t byte = m_bytes[static_cast<std::size_t>(index / 2)];
        return index % 2 == 0 ? byte >> 4 : byte & 15; // 4 = shift past the low nibble; even pixel in the high 4 bits, odd pixel in the low 4 bits (mask 15)
    }

    void EmblemBlock::setPixel(int index, int color) {
        std::uint8_t& byte = m_bytes[static_cast<std::size_t>(index / 2)];
        byte = static_cast<std::uint8_t>(
            index % 2 == 0 ? (byte & 15) + (color << 4) : (byte & 240) + color
        ); // keep the other 4-bit half (masks 15 / 240); even pixel goes in the high half
    }

}
