#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_GAUGE_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_GAUGE_H

#include "core/Point.h"
#include "formats/Image.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"
#include "gfx/Texture.h"

#include <optional>
#include <string>

namespace client {

    /// @brief A gauge of the status panel (HP or SP): its picture wiped to the value's percent, rebuilt when the percent changes.
    class Gauge {
        public:
            /// @brief Decodes the gauge picture.
            /// @param pictures Picture library to decode from.
            /// @param picture Name of the gauge picture.
            Gauge(
                gfx::PictureLibrary& pictures,
                const std::string& picture
            );

            /// @brief Draws the gauge at the value's percent, rebuilding its texture when the percent changed.
            /// @param canvas Target canvas.
            /// @param at Position of the gauge.
            /// @param value Current value.
            /// @param maximum Maximum value.
            void paint(gfx::Canvas& canvas, core::Point at, long long value, long long maximum);

        private:
            std::optional<gfx::PictureLibrary::Pixels> m_picture;
            std::optional<gfx::Texture> m_texture;
            int m_percent = -1;
    };

}

#endif
