#include "client/viewmodels/events/GalaxyRankWindow.h"

#include "core/ByteReader.h"
#include "gfx/TextStyle.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kRowsShown = 5; // ranking rows visible at once
        constexpr int kEntryBytes = 72; // bytes per ranking entry in packet 36/9
        constexpr int kMostEntries = 100; // most entries kept per group
        constexpr int kRankOffset = 20; // entry byte offset of the rank (u8)
        constexpr int kNameOffset = 5; // entry byte offset of the name length; the name text follows it
        constexpr int kServerOffset = 4; // entry byte offset of the server number (u8)

        /// @brief Loads GalaxyRankWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GalaxyRankWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("GalaxyRank");
        }

    }

    GalaxyRankWindow::GalaxyRankWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_services(services),
          m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::DrawingArea>("GalaxyList")),
          m_scroll(screen.get<ui::ScrollBar>("GalaxyScroll")),
          m_titleSpot(screen.get<ui::Spot>("GalaxyTitle")),
          m_pageSpot(screen.get<ui::Spot>("GalaxyPage")),
          m_smallDigitSpot(screen.get<ui::Spot>("GalaxySmallDigit")),
          m_rankAtSpot(screen.get<ui::Spot>("GalaxyRankAt")),
          m_rankShiftSpot(screen.get<ui::Spot>("GalaxyRankShift")),
          m_bigDigitSpot(screen.get<ui::Spot>("GalaxyBigDigit")),
          m_whiteSpot(screen.get<ui::Spot>("GalaxyWhite")),
          m_rowSpot(screen.get<ui::Spot>("GalaxyRow")),
          m_nameSpot(screen.get<ui::Spot>("GalaxyName")),
          m_serverSpot(screen.get<ui::Spot>("GalaxyServer")),
          m_hoverColorSpot(screen.get<ui::Spot>("GalaxyHoverColor")) {
        m_list.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintList(canvas, at); });
        screen.get<ui::DrawingArea>("GalaxyPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintForm(canvas, at); });
        screen.on("onGalaxyClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
        screen.on("onGalaxyNext", [this](const ui::EventArgs&) { nextGroup(); });
        screen.on("onGalaxyPrevious", [this](const ui::EventArgs&) { previousGroup(); });
        screen.on("onGalaxyScrolled", [this](const ui::EventArgs& args) { scrollTo(args.index); });
        screen.on("onGalaxyWheel", [this](const ui::EventArgs& args) { scrollTo(m_top - args.index); });
        m_scroll.setRange(0, kRowsShown);
    }

    void GalaxyRankWindow::received(const core::Bytes& payload) {
        if (payload.size() < 2) { // action id and sub id take 2 bytes
            return;
        }
        if (payload[1] == 9) { // 36/9: ranking of one group
            store(payload);
        } else if (payload[1] == 10) { // 36/10: open the form on the first group
            open();
        }
    }

    int GalaxyRankWindow::rowCount() const { return m_current > 0 ? static_cast<int>(m_groups[static_cast<std::size_t>(m_current - 1)].entries.size()) : 0; }

    std::string GalaxyRankWindow::titleOf(int arena, int kind) {
        std::string title;
        switch (arena) {
            case 99: title = "[The first arena]"; break; // arena 99: the first arena
            case 98: title = "[The second arena]"; break; // arena 98: the second arena
            case 97: title = "[The third arena]"; break; // arena 97: the third arena
            case 96: title = "[The fourth arena]"; break; // arena 96: the fourth arena
            default: title = "The " + std::to_string(arena) + "Server"; break; // any other arena number is a server
        }
        if (kind == 1) { // kind 1: Group P
            title += "  Group P";
        } else if (kind == 2) { // kind 2: Chaos fight
            title += "  Chaos fight";
        }
        return title + "Competition";
    }

    void GalaxyRankWindow::store(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // payload offset past action id and sub id
        const int arena = reader.u8();
        const int kind = reader.u8();
        const int count = std::min<int>(reader.u16(), kMostEntries);
        if (reader.failed()) {
            return;
        }
        std::vector<Entry> entries;
        for (int index = 0; index < count && reader.remaining() >= static_cast<std::size_t>(kEntryBytes); ++index) {
            const core::Bytes record = reader.bytes(kEntryBytes);
            Entry entry;
            entry.server = record[kServerOffset];
            const std::size_t nameLength = std::min<std::size_t>(record[kNameOffset], kEntryBytes - kNameOffset - 1);
            const auto nameStart = record.begin() + kNameOffset + 1;
            entry.name.assign(nameStart, nameStart + static_cast<std::ptrdiff_t>(nameLength));
            entry.rank = record[kRankOffset];
            entries.push_back(std::move(entry));
        }
        std::stable_sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) { return a.rank < b.rank; });
        std::size_t index = 0;
        while (index < m_groups.size() && !(m_groups[index].arena == arena && m_groups[index].kind == kind)) {
            ++index;
        }
        if (index == m_groups.size()) {
            m_groups.push_back(Group{arena, kind, titleOf(arena, kind), {}});
            if (m_current == 0) {
                m_current = 1;
            }
        }
        m_groups[index].entries = std::move(entries);
        if (m_current == static_cast<int>(index) + 1) {
            showGroup(m_current);
        }
    }

    void GalaxyRankWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        if (!m_groups.empty()) {
            showGroup(1);
        } else {
            m_current = 0;
        }
    }

    void GalaxyRankWindow::showGroup(int number) {
        m_current = number;
        m_top = 0;
        m_scroll.setRange(rowCount(), kRowsShown);
        m_scroll.setPosition(0);
    }

    void GalaxyRankWindow::nextGroup() {
        if (m_groups.size() > 1 && m_current < static_cast<int>(m_groups.size())) {
            showGroup(m_current + 1);
        }
    }

    void GalaxyRankWindow::previousGroup() {
        if (m_current > 1) {
            showGroup(m_current - 1);
        }
    }

    void GalaxyRankWindow::scrollTo(int top) {
        m_top = std::clamp(top, 0, std::max(0, rowCount() - kRowsShown));
        m_scroll.setPosition(m_top);
    }

    int GalaxyRankWindow::rowAt(core::Point local) const {
        const int index = m_top + local.y / m_rowSpot.size().height;
        return local.y >= 0 && local.x >= 0 && local.x < m_rowSpot.size().width && index < rowCount() ? index : -1;
    }

    void GalaxyRankWindow::drawNumber(
        gfx::Canvas& canvas,
        core::Point at,
        int value,
        const char* picture,
        core::Size digit
    ) {
        const auto digits = m_services.pictures.find(picture);
        if (!digits) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*digits);
        const std::string text = std::to_string(value);
        for (std::size_t i = 0; i < text.size(); ++i) {
            const int figure = text[i] - '0';
            canvas.drawPictureScaled(
                *digits,
                {at.x + static_cast<int>(i) * digit.width, at.y, digit.width, digit.height},
                core::Rect{0, figure * (data.height() / 10), data.width(), data.height() / 10}
            ); // the digit sheet holds the 10 digits stacked
        }
    }

    void GalaxyRankWindow::paintForm(gfx::Canvas& canvas, core::Point at) {
        const int rowPitch = m_rowSpot.size().height;
        const std::string title = m_groups.empty() ? "No competition ranks at present" : m_groups[static_cast<std::size_t>(std::max(m_current, 1) - 1)].title;
        canvas.drawText(
            {at.x + m_titleSpot.at().x - static_cast<int>(title.size()) * m_titleSpot.pitch().x / 2, at.y + m_titleSpot.at().y},
            title,
            {m_whiteSpot.color(), gfx::TextEffect::Shadow, std::nullopt},
            m_rowSpot.size().width,
            static_cast<int>(title.size()) * m_titleSpot.pitch().x
        );
        // slash: 3 pixels left of the centre, text area 100 x 16
        canvas.drawText({at.x + m_pageSpot.at().x - 3, at.y + m_pageSpot.at().y}, "/", {m_whiteSpot.color(), gfx::TextEffect::Plain, std::nullopt}, 100, 16);
        const int currentLength = static_cast<int>(std::to_string(m_current).size());
        // current group number: 12 pixels per digit plus 6 left of the centre; group count: 11 right of the centre
        drawNumber(canvas, {at.x + m_pageSpot.at().x - (currentLength * 12 + 6), at.y + m_pageSpot.at().y}, m_current, "Num_White3_2", m_smallDigitSpot.size());
        drawNumber(
            canvas,
            {at.x + m_pageSpot.at().x + 11, at.y + m_pageSpot.at().y},
            static_cast<int>(m_groups.size()),
            "Num_White3_2",
            m_smallDigitSpot.size()
        );
        if (m_current == 0) {
            return;
        }
        const std::vector<Entry>& entries = m_groups[static_cast<std::size_t>(m_current - 1)].entries;
        const int last = std::min(static_cast<int>(entries.size()) - 1, m_top + kRowsShown - 1);
        for (int index = m_top; index <= last; ++index) {
            const int rank = entries[static_cast<std::size_t>(index)].rank;
            core::Point where{at.x + m_rankAtSpot.at().x, at.y + m_rankAtSpot.at().y + (index - m_top) * rowPitch + m_rankShiftSpot.at().y};
            const int length = static_cast<int>(std::to_string(rank).size());
            // shift by the rank number's length: 15 for ranks up to 3 or one digit, 6 for two digits
            if (rank <= 3 || length == 1) { // one-digit ranks (and ranks up to 3): shift 15 px right
                where.x += 15;
            } else if (length == 2) {
                where.x += 6;
            }
            drawNumber(canvas, where, rank, rank == 1 ? "Num_Red1_1" : "Num_Blue1_1", m_bigDigitSpot.size()); // rank 1 is drawn in red digits, others in blue
        }
    }

    void GalaxyRankWindow::paintList(gfx::Canvas& canvas, core::Point at) {
        const int rowPitch = m_rowSpot.size().height;
        const int listWidth = m_rowSpot.size().width;
        const int textIndent = m_nameSpot.at().x;
        const int serverIndent = m_serverSpot.pitch().x;
        const int serverTrim = m_serverSpot.pitch().y;
        const int textShiftY = m_nameSpot.at().y;
        const int lineHeight = m_nameSpot.size().height;
        if (m_current == 0) {
            return;
        }
        const std::vector<Entry>& entries = m_groups[static_cast<std::size_t>(m_current - 1)].entries;
        const int shown = std::min(kRowsShown, static_cast<int>(entries.size()) - m_top);
        if (m_screen.hoveredId() == m_list.id()) {
            const int hovered = rowAt({m_screen.mouse().x - at.x, m_screen.mouse().y - at.y});
            if (hovered >= 0 && hovered - m_top < kRowsShown) {
                canvas.fillRectAlpha(
                    {at.x, at.y - 2 + rowPitch * (hovered - m_top), listWidth - 3, rowPitch},
                    m_hoverColorSpot.color()
                ); // highlight: 2 pixels up, 3 pixels narrower than the list
            }
        }
        const gfx::TextStyle style{m_whiteSpot.color(), gfx::TextEffect::Shadow, std::nullopt};
        for (int row = 0; row < shown; ++row) {
            // names start 2 pixels right of the text indent
            canvas.drawText(
                {at.x + 2 + textIndent, at.y + textShiftY + rowPitch * row},
                entries[static_cast<std::size_t>(m_top + row)].name,
                style,
                listWidth - textIndent,
                lineHeight
            );
        }
        for (int row = 0; row < shown; ++row) {
            const std::string server = "The" + std::to_string(entries[static_cast<std::size_t>(m_top + row)].server) + "Server";
            // servers start 2 pixels right of the server indent
            canvas.drawText(
                {at.x + 2 + textIndent + serverIndent, at.y + textShiftY + rowPitch * row},
                server,
                style,
                listWidth - textIndent - serverIndent - serverTrim,
                lineHeight
            );
        }
    }

}
