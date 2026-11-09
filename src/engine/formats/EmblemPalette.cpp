#include "formats/EmblemPalette.h"

#include <array>

namespace formats {

    namespace {

        // the 16 emblem colours as RGB565 values (index = 4-bit emblem pixel value)
        constexpr std::array<std::uint16_t, 16> kColors = {
            32768, // 0: 0x8000, dark red
            63488, // 1: 0xF800, red
            64512, // 2: 0xFC00, orange
            65504, // 3: 0xFFE0, yellow
            2016, // 4: 0x07E0, green
            1024, // 5: 0x0400, dark green
            1055, // 6: 0x041F, azure blue
            33823, // 7: 0x841F, light violet-blue
            20, // 8: 0x0014, dark blue
            32784, // 9: 0x8010, purple
            63504, // 10: 0xF810, pink
            64536, // 11: 0xFC18, light pink
            32, // 12: 0x0020, almost black (faint green)
            33808, // 13: 0x8410, gray
            65535, // 14: 0xFFFF, white
            0 // 15: 0x0000, black (kEmpty in EmblemBlock)
        };

    }

    std::uint16_t emblemPaletteColor(int index) {
        return index >= 0 && index < static_cast<int>(kColors.size()) ? kColors[static_cast<std::size_t>(index)] : 0;
    }

}
