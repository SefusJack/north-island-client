#ifndef NORTH_ISLAND_UI_TILED_FRAME_H
#define NORTH_ISLAND_UI_TILED_FRAME_H

#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"
#include "gfx/Picture.h"
#include "gfx/PictureId.h"

namespace ui {

    /// @brief A skin that covers any size from one picture: corners once, edges and centre tiled.
    class TiledFrame {
        public:
            TiledFrame() = default;

            /// @brief Creates a frame from a picture region and its border sizes.
            /// @param picture The skin picture.
            /// @param source Region of the picture holding one state.
            /// @param left Width of the left border.
            /// @param top Height of the top border.
            /// @param right Width of the right border.
            /// @param bottom Height of the bottom border.
            TiledFrame(
                gfx::PictureId picture,
                const core::Rect& source,
                int left,
                int top,
                int right,
                int bottom
            )
                : m_picture(picture),
                  m_source(source),
                  m_left(left),
                  m_top(top),
                  m_right(right),
                  m_bottom(bottom) {}

            /// @brief Tells whether the frame has a picture.
            /// @return True when it has.
            bool isValid() const { return m_picture.isValid(); }

            /// @brief Draws the frame over an area.
            /// @param canvas Target canvas.
            /// @param area Rectangle to cover.
            /// @param state Which state of the picture (stacked below each other) to use.
            void draw(gfx::Canvas& canvas, const core::Rect& area, int state = 0) const;

            /// @brief Tells whether the picture pixel that covers a point (relative to an area of that size) is opaque.
            /// @param picture The skin's picture data.
            /// @param local Point relative to the area.
            /// @param width Area width.
            /// @param height Area height.
            /// @return True when the pixel is opaque.
            bool isOpaqueAt(const gfx::Picture& picture, core::Point local, int width, int height) const;

        private:
            /// @brief Draws one horizontal band: left piece, the middle repeated, right piece.
            /// @param canvas Target canvas.
            /// @param sourceTop Top of the band in the picture.
            /// @param sourceHeight Height of the band.
            /// @param destY Top of the band on the canvas.
            /// @param area Rectangle being covered.
            void tileRow(
                gfx::Canvas& canvas,
                int sourceTop,
                int sourceHeight,
                int destY,
                const core::Rect& area
            ) const;

            gfx::PictureId m_picture;
            core::Rect m_source;
            int m_left = 0;
            int m_top = 0;
            int m_right = 0;
            int m_bottom = 0;
    };

}

#endif
