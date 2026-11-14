#include "client/services/input/PadPointers.h"

#include "core/Log.h"
#include "formats/AnimatedCursor.h"
#include "formats/Image.h"

namespace client {

    namespace {

        /// @brief Turns a picture a quarter.
        /// @param image The picture.
        /// @param left True to turn it to the left (what pointed left points down), false to the right (it points up).
        /// @return The turned picture.
        formats::Image quarterTurn(const formats::Image& image, bool left) {
            formats::Image turned(image.height(), image.width());
            for (int y = 0; y < turned.height(); ++y) {
                for (int x = 0; x < turned.width(); ++x) {
                    turned.at(x, y) = left ? image.at(image.width() - 1 - y, x) : image.at(y, image.height() - 1 - x);
                }
            }
            return turned;
        }

        /// @brief Finds the fingertip: the middle of what is drawn in the first row from one end that has anything drawn.
        /// @param image The picture.
        /// @param fromBottom True to look from the bottom row up, false from the top row down.
        /// @return The fingertip's pixel; the picture's corner when nothing is drawn.
        core::Point tipOf(const formats::Image& image, bool fromBottom) {
            for (int i = 0; i < image.height(); ++i) {
                const int y = fromBottom ? image.height() - 1 - i : i;
                int first = -1;
                int last = -1;
                for (int x = 0; x < image.width(); ++x) {
                    if (image.at(x, y).a != 0) {
                        first = first < 0 ? x : first;
                        last = x;
                    }
                }
                if (first >= 0) {
                    return {(first + last) / 2, y};
                }
            }
            return {};
        }

    }

    PadPointers::PadPointers(const GameDirectory& game, const gfx::RenderDevice& device) {
        const auto file = game.data("cursor/cursor16.ani");
        const auto hand = formats::AnimatedCursor::load(file);
        if (!hand || hand->frames().empty() || hand->frames().front().image.isEmpty()) {
            core::logWarning("cannot read the cursor ", file.string());
            return;
        }
        const formats::Image down = quarterTurn(hand->frames().front().image, true);
        const formats::Image up = quarterTurn(hand->frames().front().image, false);
        m_down = gfx::Texture::create(device, down);
        m_up = gfx::Texture::create(device, up);
        m_downTip = tipOf(down, true);
        m_upTip = tipOf(up, false);
    }

}
