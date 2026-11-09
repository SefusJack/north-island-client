#include "client/viewmodels/guild/GuildNoticePage.h"

#include "client/models/character/CharacterLook.h"
#include "core/DateTime.h"
#include "ui/Element.h"

#include <string>

namespace client {

    namespace {

        constexpr int kRowsPerPage = 5; // notices per page
        constexpr int kRowPitch = 65; // y distance between notice rows, px
        constexpr int kFaceSlot = 1; // portrait slot used for drawing the face
        constexpr int kShortPage = 5; // page text shorter than this many characters is not shifted left
        constexpr int kRowLeft = 93; // left edge of the clickable row, px
        constexpr int kRowTop = 91; // top edge of the first clickable row, px
        constexpr int kRowWidth = 241; // clickable row width, px
        constexpr int kRowHeight = 62; // clickable row height, px

        /// @brief Gives the area of a notice row.
        /// @param at Top left of the window.
        /// @param row Row number from 1.
        /// @return Rectangle of the row.
        core::Rect rowArea(core::Point at, int row) { return {at.x + kRowLeft, at.y + kRowTop + (row - 1) * kRowPitch, kRowWidth, kRowHeight}; }

    }

    GuildNoticePage::GuildNoticePage(
        GameServices& services,
        ui::Screen& screen,
        const Guild& guild,
        NoticeBoard& board,
        NoticeReadForm& read
    )
        : m_services(services),
          m_screen(screen),
          m_guild(guild),
          m_board(board),
          m_read(read),
          m_rowSpot(screen.get<ui::Spot>("GuildNoticeRow")),
          m_faceSpot(screen.get<ui::Spot>("GuildNoticeFace")),
          m_senderSpot(screen.get<ui::Spot>("GuildNoticeSender")),
          m_subjectSpot(screen.get<ui::Spot>("GuildNoticeSubject")),
          m_dateSpot(screen.get<ui::Spot>("GuildNoticeDate")),
          m_pageSpot(screen.get<ui::Spot>("GuildNoticePageNumbers")),
          m_textSpot(screen.get<ui::Spot>("GuildNoticeText")),
          m_barSpot(screen.get<ui::Spot>("GuildNoticeBar")) {
        screen.on("onGuildNoticeAdd", [this](const ui::EventArgs&) { m_board.askToPost(0); });
        screen.on("onGuildNoticePrevious", [this](const ui::EventArgs&) { m_board.previous(); });
        screen.on("onGuildNoticeNext", [this](const ui::EventArgs&) { m_board.next(); });
        screen.on("onGuildNoticeUpdate", [this](const ui::EventArgs&) { m_board.refresh(); });
    }

    void GuildNoticePage::layout(bool shown) {
        for (const char* name : {"GuildNoticeAdd", "GuildNoticePrevious", "GuildNoticeNext", "GuildNoticeUpdate"}) {
            m_screen.get<ui::Element>(name).setVisible(shown);
        }
    }

    void GuildNoticePage::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto text = [&](core::Point where, int y, const std::string& value, const gfx::TextStyle& withStyle) {
            canvas.drawText(
                {at.x + where.x, at.y + where.y + y},
                value,
                withStyle,
                static_cast<int>(value.size()) * 8
            ); // 8: text box width per character in pixels
        };
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const protocol::NoticeRow& notice = m_board.row(row);
            const int member = notice.authorId != 0 ? m_guild.indexOf(notice.authorId) : -1;
            if (member < 0) {
                continue;
            }
            const protocol::GuildMemberRecord& record = m_guild.at(member).record;
            if (record.bodyType < 1 || record.bodyType > 4) { // body types 1..4 have a face
                continue;
            }
            const CharacterLook look = CharacterLook::fromColorCodes(record.bodyType, record.head, record.colorCode1, record.colorCode5, m_services.items);
            const core::Point corner{at.x + m_rowSpot.at().x, at.y + m_rowSpot.at().y + (row - 1) * kRowPitch};
            m_services.characters.drawPortrait(canvas, look, corner + m_faceSpot.at(), 0, kFaceSlot); // 0: blink frame (eyes open)
        }
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const protocol::NoticeRow& notice = m_board.row(row);
            const int y = (row - 1) * kRowPitch;
            text(m_subjectSpot.at(), y, notice.subject, style);
            text(m_senderSpot.at(), y, notice.author, style);
            if (notice.date != 0) {
                text(m_dateSpot.at(), y, core::formatShortDate(notice.date), style);
            }
        }
        if (m_board.pageCount() != 0) {
            const std::string pages = std::to_string(m_board.currentPage()) + " / " + std::to_string(m_board.pageCount());
            const int length = static_cast<int>(pages.size());
            // a longer page text moves 2 px left per extra character (starting from a 2 px nudge)
            const int x = length < kShortPage ? m_pageSpot.at().x : m_pageSpot.at().x + 2 - (length - kShortPage) * 2;
            text({x, m_pageSpot.at().y}, 0, pages, {m_pageSpot.color(), gfx::TextEffect::Plain, std::nullopt});
        }
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const core::Rect area = rowArea(at, row);
            if (area.contains(m_screen.mouse())) {
                canvas.fillRectAlpha(area, m_barSpot.color());
            }
        }
    }

    void GuildNoticePage::doubleClicked(core::Point windowAt) {
        for (int row = 1; row <= kRowsPerPage; ++row) {
            if (rowArea(windowAt, row).contains(m_screen.mouse())) {
                m_read.close();
                if (const auto picked = m_board.open(row)) {
                    m_read.expect(true, *picked);
                }
                return;
            }
        }
    }

}
