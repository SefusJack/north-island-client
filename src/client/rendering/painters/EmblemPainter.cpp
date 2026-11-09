#include "client/rendering/painters/EmblemPainter.h"

#include "formats/EmblemPalette.h"

namespace client {

    void EmblemPainter::paint(gfx::Canvas& canvas, const formats::EmblemBlock& badge, core::Point at) {
        constexpr int kSide = formats::EmblemBlock::kSide;
        for (int x = 0; x < kSide; ++x) {
            for (int y = 0; y < kSide; ++y) {
                const std::uint16_t colour = formats::emblemPaletteColor(badge.pixel(x * kSide + y));
                if (colour != 0) {
                    canvas.fillRect({at.x + x, at.y + y, 1, 1}, core::Color::fromRgb565(colour));
                }
            }
        }
    }

}
