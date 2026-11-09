#include "client/models/guild/EmblemGrid.h"

namespace client {

    EmblemGrid::EmblemGrid() {
        for (auto& column : cells) {
            column.fill(formats::EmblemBlock::kEmpty);
        }
    }

    EmblemGrid EmblemGrid::decode(const formats::EmblemBlock& block) {
        EmblemGrid grid;
        for (int x = 0; x < kSide; ++x) {
            for (int y = 0; y < kSide; ++y) {
                grid.cells[static_cast<std::size_t>(x)][static_cast<std::size_t>(y)] = static_cast<std::uint8_t>(block.pixel(x * kSide + y));
            }
        }
        return grid;
    }

    formats::EmblemBlock EmblemGrid::encode() const {
        formats::EmblemBlock block;
        for (int x = 0; x < kSide; ++x) {
            for (int y = 0; y < kSide; ++y) {
                block.setPixel(x * kSide + y, cells[static_cast<std::size_t>(x)][static_cast<std::size_t>(y)]);
            }
        }
        return block;
    }

}
