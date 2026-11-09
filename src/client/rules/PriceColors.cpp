#include "client/rules/PriceColors.h"

namespace client {

    core::Color priceColor(std::size_t digits) {
        switch (digits) {
            case 4: return core::Color::fromRgb565(0x540); // 4 digits: green (RGB565 0x540)
            case 5: return core::Color::fromRgb565(0x1F); // 5 digits: blue (RGB565 0x1F)
            case 6: return core::Color::fromRgb565(0xF81F); // 6 digits: magenta (RGB565 0xF81F)
            case 7: return core::Color::fromRgb565(0xF800); // 7 digits: red (RGB565 0xF800)
            default: return core::Color::fromRgb565(0x841); // other digit counts: dark grey (RGB565 0x841)
        }
    }

}
