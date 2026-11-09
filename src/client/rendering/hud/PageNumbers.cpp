#include "client/rendering/hud/PageNumbers.h"

#include "gfx/PictureLibrary.h"

#include <string>

namespace client {

    namespace {

        constexpr int kDigitRows = 11; // Num_MainInfo stacks 11 rows: digits 0-9 and the slash
        constexpr int kSlashRow = 11; // row index of the slash glyph

        /// @brief Draws the page number, the slash and the number of pages from Num_MainInfo.
        /// @param canvas Target canvas.
        /// @param y Y position of all glyphs.
        /// @param pageX X of the page number's first digit.
        /// @param slashX X of the slash.
        /// @param pagesX X of the page count's first digit.
        /// @param page Current page number.
        /// @param pages Number of pages.
        void drawGlyphs(
            gfx::Canvas& canvas,
            int y,
            int pageX,
            int slashX,
            int pagesX,
            int page,
            int pages
        ) {
            const auto digits = canvas.pictures().find("Num_MainInfo");
            if (!digits) {
                return;
            }
            const gfx::Picture& picture = canvas.pictures().picture(*digits);
            const int width = picture.width();
            const int height = picture.height() / kDigitRows;
            const auto glyph = [&](int row, int x) { canvas.drawPicture(*digits, {x, y}, core::Rect{0, row * height, width, height}); };
            const std::string pageText = std::to_string(page);
            for (std::size_t i = 0; i < pageText.size(); ++i) {
                glyph(pageText[i] - '0', pageX + static_cast<int>(i) * width);
            }
            glyph(kSlashRow, slashX);
            const std::string pagesText = std::to_string(pages);
            for (std::size_t i = 0; i < pagesText.size(); ++i) {
                glyph(pagesText[i] - '0', pagesX + static_cast<int>(i) * width);
            }
        }

        /// @brief The width of one Num_MainInfo glyph.
        /// @param canvas Canvas providing the picture library.
        /// @return Glyph width in pixels, 0 when the picture is missing.
        int glyphWidth(gfx::Canvas& canvas) {
            const auto digits = canvas.pictures().find("Num_MainInfo");
            return digits ? canvas.pictures().picture(*digits).width() : 0;
        }

    }

    void drawPageNumbers(gfx::Canvas& canvas, core::Point slash, int page, int pages) {
        const int width = glyphWidth(canvas);
        const int pageX = slash.x - static_cast<int>(std::to_string(page).size()) * width - 1;
        drawGlyphs(canvas, slash.y, pageX, slash.x, slash.x + width, page, pages);
    }

    void drawPageNumbersFrom(gfx::Canvas& canvas, core::Point start, int page, int pages) {
        const int width = glyphWidth(canvas);
        const int length = static_cast<int>(std::to_string(page).size());
        const int pageX = length == 2 ? start.x - width : start.x; // two-digit page numbers start one glyph further left
        drawGlyphs(canvas, start.y, pageX, pageX + length * width, pageX + (length + 1) * width, page, pages);
    }

}
