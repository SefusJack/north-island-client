#include "client/viewmodels/events/LeaveWordWindow.h"

#include "client/rules/TextFilter.h"
#include "core/ByteReader.h"
#include "core/DateTime.h"
#include "gfx/TextStyle.h"
#include "protocol/LeaveWordRequests.h"

#include <algorithm>
#include <utility>

namespace client {

    namespace {

        constexpr int kRowsShown = 5; // messages per page
        constexpr int kMostRows = 500; // most messages the list keeps
        constexpr int kMostText = 60; // longest message text, characters
        constexpr int kMostOwn = 3; // most messages one player may have written
        constexpr int kShortMs = 2000; // short notice display time, ms
        constexpr int kLongMs = 3000; // long notice display time, ms
        constexpr int kFaceSlot = 0; // portrait slot used for drawing the face

        /// @brief Loads LeaveWordWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "LeaveWord" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LeaveWordWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("LeaveWord");
        }

        /// @brief Cuts a text to at most 60 characters.
        /// @details Original: ShortString[60].
        /// @param text Text to cut.
        /// @return The cut text.
        std::string shortened(std::string text) {
            if (text.size() > static_cast<std::size_t>(kMostText)) {
                text.resize(static_cast<std::size_t>(kMostText));
            }
            return text;
        }

    }

    LeaveWordWindow::LeaveWordWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::DrawingArea>("LeaveWordList")),
          m_message(screen.get<ui::TextBox>("LeaveWordMessage")),
          m_name(screen.get<ui::TextBox>("LeaveWordName")),
          m_rowSpot(screen.get<ui::Spot>("LeaveWordRow")),
          m_columnSpot(screen.get<ui::Spot>("LeaveWordColumn")),
          m_faceSpot(screen.get<ui::Spot>("LeaveWordFace")),
          m_nameColorSpot(screen.get<ui::Spot>("LeaveWordNameColor")),
          m_timeColorSpot(screen.get<ui::Spot>("LeaveWordTimeColor")),
          m_rowTextColorSpot(screen.get<ui::Spot>("LeaveWordRowTextColor")),
          m_hoverColorSpot(screen.get<ui::Spot>("LeaveWordHoverColor")),
          m_selectColorSpot(screen.get<ui::Spot>("LeaveWordSelectColor")),
          m_pageSlashSpot(screen.get<ui::Spot>("LeaveWordPageSlash")),
          m_textColorSpot(screen.get<ui::Spot>("LeaveWordTextColor")),
          m_pageIndexSpot(screen.get<ui::Spot>("LeaveWordPageIndex")),
          m_pageCountSpot(screen.get<ui::Spot>("LeaveWordPageCount")),
          m_digitSpot(screen.get<ui::Spot>("LeaveWordDigit")) {
        m_list.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paintList(canvas, at, nowMs); });
        screen.get<ui::DrawingArea>("LeaveWordPages").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintPages(canvas, at); });
        m_message.setText("I have come here.");
        screen.on("onLeaveWordClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onLeaveWordSend", [this](const ui::EventArgs&) { send(); });
        screen.on("onLeaveWordClear", [this](const ui::EventArgs&) { clear(); });
        screen.on("onLeaveWordFind", [this](const ui::EventArgs&) { find(); });
        screen.on("onLeaveWordNext", [this](const ui::EventArgs&) { nextPage(); });
        screen.on("onLeaveWordPrevious", [this](const ui::EventArgs&) { previousPage(); });
        screen.on("onLeaveWordSelect", [this](const ui::EventArgs& args) { selectOwn(rowAt(args.position)); });
        screen.on("onLeaveWordWhisper", [this](const ui::EventArgs& args) { whisperTo(rowAt(args.position)); });
    }

    void LeaveWordWindow::received(const core::Bytes& payload) {
        if (payload.size() < 2) { // action byte plus sub id
            return;
        }
        switch (payload[1]) { // sub id of server action 36
            case 3: // 36/3: the form opens
                open();
                break;
            case 4: // 36/4: every message (list)
                listAll(payload);
                break;
            case 5: // 36/5: one more message
                added(payload);
                break;
            case 6: // 36/6: a message is gone
                removed(payload);
                break;
            case 7:
                // 36/7: answer to a request
                m_requestState = 0;
                if (payload.size() >= 4 && payload[2] == 1 && payload[3] == 7) { // needs bytes 2 and 3; kind 1, code 7 = cannot leave a message
                    m_messages.show("You are unable to leave a message.", kShortMs);
                }
                break;
            default: // other sub ids are ignored
                break;
        }
    }

    void LeaveWordWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        setTop(static_cast<int>(m_rows.size()) - kRowsShown);
        m_requestState = 0;
        updatePages();
    }

    void LeaveWordWindow::listAll(const core::Bytes& payload) {
        m_rows.clear();
        m_top = 0;
        m_selected = -1;
        core::ByteReader reader(payload, 2); // skip action byte and sub id
        while (reader.remaining() > 1) {
            reader.i32();
            Row row;
            row.name = reader.shortText();
            const int bodyType = reader.u8();
            const int head = reader.u8();
            const std::int32_t code1 = reader.i32();
            const std::int32_t code5 = reader.i32();
            row.time = core::formatDateTimeLeading(reader.f64());
            row.text = shortened(reader.shortText());
            if (reader.failed()) {
                break;
            }
            row.look = CharacterLook::fromColorCodes(bodyType, head, code1, code5, m_services.items);
            row.hasFace = bodyType >= 1 && bodyType <= 4; // body types 1..4 have a face
            m_rows.push_back(std::move(row));
        }
    }

    void LeaveWordWindow::added(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action byte and sub id
        const std::int32_t writerId = reader.i32();
        const double time = reader.f64();
        if (reader.failed()) {
            m_requestState = 0;
            return;
        }
        Row row;
        row.time = core::formatDateTimeLeading(time);
        row.text = shortened(reader.rest());
        if (const auto writer = m_hooks.writerOf(writerId)) {
            row.name = writer->name;
            row.look = writer->look;
            row.hasFace = true;
        } else {
            row.name = "Unknown name";
        }
        if (static_cast<int>(m_rows.size()) >= kMostRows) {
            erase(0);
        }
        m_rows.push_back(std::move(row));
        if (writerId == m_hooks.ownId()) {
            setTop(static_cast<int>(m_rows.size()) - kRowsShown);
        }
        m_requestState = 0;
        updatePages();
    }

    void LeaveWordWindow::removed(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action byte and sub id
        const int row = reader.u16();
        if (reader.failed()) {
            m_requestState = 0;
            return;
        }
        const bool atEnd = static_cast<int>(m_rows.size()) - m_top <= kRowsShown;
        erase(row);
        if (atEnd) {
            setTop(static_cast<int>(m_rows.size()) - kRowsShown);
        }
        m_requestState = 0;
        updatePages();
    }

    void LeaveWordWindow::erase(int index) {
        if (index < 0 || index >= static_cast<int>(m_rows.size())) {
            return;
        }
        m_rows.erase(m_rows.begin() + index);
        if (m_selected == index) {
            m_selected = -1;
        } else if (m_selected > index) {
            --m_selected;
        }
    }

    void LeaveWordWindow::setTop(int top) {
        top = std::max(top, 0);
        if (top <= static_cast<int>(m_rows.size()) - 1) {
            m_top = top;
        } else if (m_rows.empty()) {
            m_top = 0;
        }
    }

    void LeaveWordWindow::updatePages() {
        const int count = static_cast<int>(m_rows.size());
        const auto pagesOf = [](int rows) { return (rows + kRowsShown - 1) / kRowsShown; };
        m_pageCount = pagesOf(count);
        m_pageIndex = count - kRowsShown <= m_top ? m_pageCount : m_pageCount - pagesOf(count - (m_top + kRowsShown));
    }

    void LeaveWordWindow::close() {
        if (m_requestState != 0) {
            return;
        }
        m_hooks.closed();
        m_window.setVisible(false);
    }

    void LeaveWordWindow::send() {
        if (m_requestState != 0 || m_message.text().empty()) {
            return;
        }
        if (!isNameClean(m_message.text(), false)) {
            m_messages.show("There are irregular letters in the message.", kShortMs);
            return;
        }
        const std::string own = m_hooks.ownName();
        const auto written = std::count_if(m_rows.begin(), m_rows.end(), [&](const Row& row) { return row.name == own; });
        if (written >= kMostOwn) {
            m_messages.show("There can't be more than 3 messages.", kLongMs);
            return;
        }
        m_requestState = 2; // request state 2: a message was sent
        m_services.connection.send(protocol::leaveWordSendRequest(m_message.text()));
    }

    void LeaveWordWindow::clear() {
        if (m_requestState != 0 || m_selected == -1 || m_selected >= static_cast<int>(m_rows.size())) {
            return;
        }
        if (m_rows[static_cast<std::size_t>(m_selected)].name == m_hooks.ownName()) {
            m_requestState = 1; // request state 1: a clear was asked
            m_services.connection.send(protocol::leaveWordClearRequest(static_cast<std::uint16_t>(m_selected + 1)));
        }
    }

    void LeaveWordWindow::find() {
        if (m_requestState != 0 || m_name.text().empty()) {
            return;
        }
        const int count = static_cast<int>(m_rows.size());
        if (m_selected == -1) {
            setTop(0);
        }
        bool found = false;
        int lastVisible = m_top + kRowsShown - 1;
        for (int index = m_selected + 1; index < count; ++index) {
            if (m_rows[static_cast<std::size_t>(index)].name.find(m_name.text()) == std::string::npos) {
                continue;
            }
            while (index > lastVisible) {
                setTop(count - lastVisible > kRowsShown ? lastVisible + 1 : m_top + 1);
                lastVisible = m_top + kRowsShown - 1;
            }
            m_selected = index;
            found = true;
            break;
        }
        if (!found) {
            m_messages.show("Scroll down and find no available messages.", kLongMs);
            m_selected = -1;
        }
        updatePages();
    }

    void LeaveWordWindow::nextPage() {
        if (m_pageIndex >= m_pageCount) {
            return;
        }
        ++m_pageIndex;
        const int count = static_cast<int>(m_rows.size());
        if (count - kRowsShown > m_top) {
            setTop(count - (m_top + kRowsShown) >= kRowsShown ? m_top + kRowsShown : count - kRowsShown);
        }
    }

    void LeaveWordWindow::previousPage() {
        if (m_pageIndex <= 1) {
            return;
        }
        --m_pageIndex;
        if (m_top > 0) {
            setTop(m_top >= kRowsShown ? m_top - kRowsShown : 0);
        }
    }

    void LeaveWordWindow::selectOwn(int row) {
        if (row >= 0 && m_rows[static_cast<std::size_t>(row)].name == m_hooks.ownName()) {
            m_selected = row;
        }
    }

    void LeaveWordWindow::whisperTo(int row) {
        if (row < 0) {
            return;
        }
        const std::string& name = m_rows[static_cast<std::size_t>(row)].name;
        if (!name.empty() && name == m_hooks.ownName()) {
            m_messages.show("Unable to whisper to yourself", kShortMs);
            return;
        }
        if (const auto target = m_hooks.playerNamed(name)) {
            m_hooks.whisper(*target);
        } else {
            m_messages.show("The player is offline. Unable to talk with the target.", kShortMs);
        }
    }

    void LeaveWordWindow::colorsChanged(int part, std::int32_t code) {
        const std::string own = m_hooks.ownName();
        for (Row& row : m_rows) {
            if (row.name == own) {
                row.look.basePattern.apply(code, part);
            }
        }
    }

    int LeaveWordWindow::rowAt(core::Point local) const {
        const int row = local.y / m_rowSpot.size().height;
        const int index = m_top + row;
        return local.y >= 0 && index < static_cast<int>(m_rows.size()) ? index : -1;
    }

    void LeaveWordWindow::paintList(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const int shown = std::min(kRowsShown, static_cast<int>(m_rows.size()) - m_top);
        const int rowPitch = m_rowSpot.size().height;
        const int listWidth = m_rowSpot.size().width;
        const int textIndent = m_columnSpot.at().x;
        const int timeIndent = m_columnSpot.pitch().x;
        const int lineHeight = m_columnSpot.size().height;
        const int pageWidth = listWidth - textIndent;
        const gfx::TextStyle nameStyle{m_nameColorSpot.color(), gfx::TextEffect::Shadow, std::nullopt};
        const gfx::TextStyle timeStyle{m_timeColorSpot.color(), gfx::TextEffect::Shadow, std::nullopt};
        const gfx::TextStyle textStyle{m_rowTextColorSpot.color(), gfx::TextEffect::Shadow, std::nullopt};
        for (int row = 0; row < shown; ++row) {
            const Row& item = m_rows[static_cast<std::size_t>(m_top + row)];
            canvas.drawText({at.x + 2 + textIndent, at.y + rowPitch * row}, item.name, nameStyle, pageWidth, lineHeight); // 2: pixel margin left of the text
        }
        const int frame = m_blink.frame(nowMs);
        for (int row = 0; row < shown; ++row) {
            const Row& item = m_rows[static_cast<std::size_t>(m_top + row)];
            if (item.hasFace) {
                m_services.characters.drawPortrait(canvas, item.look, {at.x + m_faceSpot.at().x, at.y + rowPitch * row + m_faceSpot.at().y}, frame, kFaceSlot);
            }
        }
        // highlight bar: 2 px above the row, 3 px narrower than the list
        const auto bar = [&](int row, core::Color color) { canvas.fillRectAlpha({at.x, at.y - 2 + rowPitch * row, listWidth - 3, rowPitch}, color); };
        const int selectedRow = m_selected - m_top;
        if (m_selected >= 0 && selectedRow >= 0 && selectedRow < kRowsShown && m_top + selectedRow < static_cast<int>(m_rows.size())) {
            bar(selectedRow, m_selectColorSpot.color());
        }
        if (m_screen.hoveredId() == m_list.id()) {
            const int hovered = rowAt({m_screen.mouse().x - at.x, m_screen.mouse().y - at.y});
            if (hovered >= 0 && hovered - m_top < kRowsShown) {
                bar(hovered - m_top, m_hoverColorSpot.color());
            }
        }
        for (int row = 0; row < shown; ++row) {
            const Row& item = m_rows[static_cast<std::size_t>(m_top + row)];
            canvas.drawText(
                {at.x + 2 + textIndent, at.y + rowPitch / 2 - lineHeight / 2 + rowPitch * row},
                item.text,
                textStyle,
                pageWidth,
                lineHeight
            ); // 2: pixel margin left of the text
        }
        for (int row = 0; row < shown; ++row) {
            const Row& item = m_rows[static_cast<std::size_t>(m_top + row)];
            canvas.drawText(
                {at.x + 2 + textIndent + timeIndent, at.y + rowPitch * row},
                item.time,
                timeStyle,
                pageWidth - timeIndent,
                lineHeight
            ); // 2: pixel margin left of the text
        }
    }

    void LeaveWordWindow::paintPages(gfx::Canvas& canvas, core::Point at) {
        // text box 100 wide and 16 high
        canvas.drawText(
            {at.x + m_pageSlashSpot.at().x, at.y + m_pageSlashSpot.at().y},
            "/",
            {m_textColorSpot.color(), gfx::TextEffect::Shadow, std::nullopt},
            100,
            16
        );
        const int indexLength = static_cast<int>(std::to_string(m_pageIndex).size());
        // 12: pixel width of a digit; the page number grows to the left
        drawNumber(canvas, {at.x + m_pageIndexSpot.at().x - (indexLength - 1) * 12, at.y + m_pageIndexSpot.at().y}, m_pageIndex);
        drawNumber(canvas, {at.x + m_pageCountSpot.at().x, at.y + m_pageCountSpot.at().y}, m_pageCount);
    }

    void LeaveWordWindow::drawNumber(gfx::Canvas& canvas, core::Point at, int value) {
        const auto digits = m_services.pictures.find("Num_White3_2");
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = m_services.pictures.picture(*digits);
        const std::string text = std::to_string(value);
        for (std::size_t i = 0; i < text.size(); ++i) {
            const int digit = text[i] - '0';
            // the picture holds the ten digits one above the other
            canvas.drawPictureScaled(
                *digits,
                {at.x + static_cast<int>(i) * m_digitSpot.pitch().x, at.y, m_digitSpot.size().width, m_digitSpot.size().height},
                core::Rect{0, digit * (picture.height() / 10), picture.width(), picture.height() / 10}
            );
        }
    }

}
