#include "client/viewmodels/events/NoticeBoardWindow.h"

#include "client/rules/GmIds.h"
#include "core/DateTime.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kRowLeft = 15; // left edge of the clickable rows, px
        constexpr int kRowTop = 77; // top edge of the first row, px
        constexpr int kRowWidth = 620; // row width, px
        constexpr int kRowHeight = 20; // row height (and pitch), px

        /// @brief Loads NoticeBoardWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "NoticeBoardWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NoticeBoardWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("NoticeBoardWindow");
        }

        /// @brief Gives the screen area of a notice row.
        /// @param at Top left of the window (or rows area).
        /// @param row Row number from 1.
        /// @return Rectangle of the row.
        core::Rect rowArea(core::Point at, int row) { return {at.x + kRowLeft, at.y + kRowTop + (row - 1) * kRowHeight, kRowWidth, kRowHeight}; }

    }

    NoticeBoardWindow::NoticeBoardWindow(
        GameServices& services,
        ui::Screen& screen,
        NoticeBoard& board,
        NoticeReadForm& read,
        Hooks hooks
    )
        : m_screen(screen),
          m_board(board),
          m_read(read),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_subjectSpot(screen.get<ui::Spot>("NoticeBoardSubject")),
          m_senderSpot(screen.get<ui::Spot>("NoticeBoardSender")),
          m_dateSpot(screen.get<ui::Spot>("NoticeBoardDate")),
          m_pageNumbersSpot(screen.get<ui::Spot>("NoticeBoardPageNumbers")),
          m_textColorSpot(screen.get<ui::Spot>("NoticeBoardTextColor")),
          m_pageColorSpot(screen.get<ui::Spot>("NoticeBoardPageColor")),
          m_barColorSpot(screen.get<ui::Spot>("NoticeBoardBarColor")) {
        screen.get<ui::DrawingArea>("NoticeBoardRows").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onNoticeBoardClick", [this](const ui::EventArgs&) { clicked(); });
        screen.on("onNoticeBoardPrevious", [this](const ui::EventArgs&) { m_board.previous(); });
        screen.on("onNoticeBoardNext", [this](const ui::EventArgs&) { m_board.next(); });
        screen.on("onNoticeBoardUpdate", [this](const ui::EventArgs&) { m_board.refresh(); });
        screen.on("onNoticeBoardAdd", [this](const ui::EventArgs&) { m_board.askToPost(m_hooks.ownLevel()); });
        screen.on("onNoticeBoardClose", [this](const ui::EventArgs&) { close(); });
    }

    void NoticeBoardWindow::open() {
        m_screen.get<ui::Element>("NoticeBoardAdd").setVisible(!isGmId(m_hooks.ownId()));
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_board.reopen();
    }

    void NoticeBoardWindow::close() {
        m_window.setVisible(false);
        m_hooks.closeForms();
    }

    void NoticeBoardWindow::clicked() {
        const core::Point origin{m_window.bounds().x, m_window.bounds().y};
        for (int row = 1; row <= NoticeBoard::kMostRows; ++row) {
            if (rowArea(origin, row).contains(m_screen.mouse())) {
                m_read.close();
                if (const auto picked = m_board.open(row)) {
                    m_read.expect(false, *picked);
                }
                return;
            }
        }
    }

    void NoticeBoardWindow::paint(gfx::Canvas& canvas, core::Point at) {
        for (int row = 1; row <= NoticeBoard::kMostRows; ++row) {
            const core::Rect area = rowArea(at, row);
            if (area.contains(m_screen.mouse())) {
                canvas.fillRectAlpha(area, m_barColorSpot.color());
            }
        }
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto text = [&](int x, int y, const std::string& value, const gfx::TextStyle& withStyle) {
            canvas.drawText({at.x + x, at.y + y}, value, withStyle, static_cast<int>(value.size()) * 8); // 8: text box width per character in pixels
        };
        for (int row = 1; row <= NoticeBoard::kMostRows; ++row) {
            const protocol::NoticeRow& notice = m_board.row(row);
            const int y = m_subjectSpot.at().y + (row - 1) * kRowHeight;
            text(m_subjectSpot.at().x, y, notice.subject, style);
            text(m_senderSpot.at().x, y, notice.author, style);
            if (notice.date != 0) {
                text(m_dateSpot.at().x, y, core::formatDateTime(notice.date), style);
            }
        }
        if (m_board.pageCount() != 0) {
            text(
                m_pageNumbersSpot.at().x,
                m_pageNumbersSpot.at().y,
                std::to_string(m_board.currentPage()) + " / " + std::to_string(m_board.pageCount()),
                {m_pageColorSpot.color(), gfx::TextEffect::Plain, std::nullopt}
            );
        }
    }

}
