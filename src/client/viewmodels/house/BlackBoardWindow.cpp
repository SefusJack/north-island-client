#include "client/viewmodels/house/BlackBoardWindow.h"

#include "client/rules/TextFilter.h"
#include "core/ByteReader.h"
#include "core/DateTime.h"
#include "gfx/TextStyle.h"
#include "protocol/MessageId.h"
#include "protocol/BlackBoardRequests.h"
#include "ui/DrawingArea.h"

#include <cmath>
#include <utility>

namespace client {

    namespace {

        constexpr int kShortMs = 1000; // short notice display time, ms
        constexpr int kIllegalMs = 2000; // "illegal words" notice display time, ms
        constexpr int kMaxName = 14; // longest accepted writer name in a received message, bytes
        constexpr int kMaxText = 20; // longest accepted message text in a received message, bytes
        constexpr int kCharWidth = 8; // width of one text character, px
        constexpr double kRecentDays = 5.0; // messages younger than this many days are drawn yellow
        const char* const kColon = "\xA1G";

        /// @brief Loads BlackBoardWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded BlackBoard window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BlackBoardWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BlackBoard");
        }

    }

    BlackBoardWindow::BlackBoardWindow(
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
          m_edit(screen.get<ui::TextBox>("BlackBoardEdit")),
          m_ok(screen.get<ui::ImageButton>("BlackBoardOk")),
          m_delete(screen.get<ui::ImageButton>("BlackBoardDelete")),
          m_cancel(screen.get<ui::ImageButton>("BlackBoardCancel")),
          m_rowSpot(screen.get<ui::Spot>("BlackBoardRow")),
          m_recentSpot(screen.get<ui::Spot>("BlackBoardRecent")),
          m_oldSpot(screen.get<ui::Spot>("BlackBoardOld")) {
        screen.get<ui::DrawingArea>("BlackBoardPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onBlackBoardClose", [this](const ui::EventArgs&) { hide(); });
        screen.on("onBlackBoardOk", [this](const ui::EventArgs&) { submit(); });
        screen.on("onBlackBoardDelete", [this](const ui::EventArgs&) { erase(); });
        screen.on("onBlackBoardCancel", [this](const ui::EventArgs&) { cancel(); });
        screen.on("onBlackBoardMouseUp", [this](const ui::EventArgs&) { clicked(); });
    }

    void BlackBoardWindow::open() {
        showButtons(false, false);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_services.connection.send(protocol::blackBoardRequest(1, m_hooks.houseOwnerId())); // 72/1: list the messages
    }

    void BlackBoardWindow::hide() {
        m_window.setVisible(false);
        reset();
    }

    void BlackBoardWindow::reset() {
        m_editRow = 0;
        m_selectedRow = 0;
        m_edit.setVisible(false);
        m_edit.clear();
        showButtons(false, false);
    }

    void BlackBoardWindow::showButtons(bool write, bool erase) {
        m_ok.setVisible(write);
        m_cancel.setVisible(write);
        m_delete.setVisible(erase);
    }

    void BlackBoardWindow::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 1); // skip the first byte (message id)
        const int sub = reader.u8();
        if (sub == 1) { // sub 1: the list of messages
            for (Message& row : m_rows) {
                row = Message{};
            }
            while (reader.remaining() > 0 && !reader.failed()) {
                const int row = reader.u8();
                if (row == 0) {
                    continue;
                }
                const std::int32_t nameLength = reader.i32();
                const std::string name = reader.text(static_cast<std::size_t>(nameLength < 0 ? 0 : nameLength));
                const double time = reader.f64();
                const std::int32_t textLength = reader.i32();
                const std::string text = reader.text(static_cast<std::size_t>(textLength < 0 ? 0 : textLength));
                if (reader.failed() || row > kRows) {
                    break;
                }
                Message& message = m_rows[static_cast<std::size_t>(row)];
                if (nameLength >= 1 && nameLength <= kMaxName) {
                    message.name = name;
                }
                message.time = time;
                if (textLength >= 1 && textLength <= kMaxText) {
                    message.text = text;
                }
            }
        } else if (sub == 2 && reader.u8() == 1) { // sub 2 with result 1: someone left a word here
            m_messages.show("There is leave word here", kShortMs);
        }
    }

    int BlackBoardWindow::rowAt(core::Point mouse) const {
        const core::Point origin = m_window.bounds().position();
        for (int row = 1; row <= kRows; ++row) {
            const core::Rect area{
                origin.x + m_rowSpot.rect().x,
                origin.y + m_rowSpot.rect().y + (row - 1) * m_rowSpot.rect().height,
                m_rowSpot.rect().width,
                m_rowSpot.rect().height
            };
            if (mouse.x >= area.x && mouse.x < area.x + area.width && mouse.y >= area.y && mouse.y < area.y + area.height) {
                return row;
            }
        }
        return 0;
    }

    void BlackBoardWindow::clicked() {
        if (const int row = rowAt(m_screen.mouse())) {
            selectRow(row);
        }
    }

    void BlackBoardWindow::selectRow(int row) {
        if (m_editRow != 0) {
            m_messages.show("Someone is writing", kShortMs);
            return;
        }
        const Message& picked = m_rows[static_cast<std::size_t>(row)];
        const bool owner = m_hooks.houseOwnerId() == m_hooks.ownId();
        const bool others = !picked.name.empty() && picked.name != m_hooks.ownName();
        if (others && !owner) {
            m_messages.show("You are not the one that wrote the messages", kShortMs);
            return;
        }
        showButtons(row == m_editRow, !picked.name.empty());
        m_selectedRow = row;
        if (others && owner) {
            return;
        }
        showButtons(true, !picked.name.empty());
        m_editRow = row;
        // editor position: x 87, y 81 for the first row, rows 46 px apart
        m_edit.setPosition({87, (m_editRow - 1) * 46 + 81});
        m_edit.setVisible(true);
        m_screen.focus("BlackBoardEdit");
    }

    void BlackBoardWindow::submit() {
        if (m_editRow == 0) {
            return;
        }
        if (m_edit.text().empty()) {
            m_messages.show("Please write down your suggestions", kShortMs);
            return;
        }
        if (!isNameClean(m_edit.text(), false)) {
            m_messages.show("Don't use illegal words", kIllegalMs);
            return;
        }
        // 72/2: write the message
        m_services.connection.send(protocol::blackBoardRequest(2, m_hooks.houseOwnerId(), std::string(1, static_cast<char>(m_editRow)) + m_edit.text()));
        reset();
    }

    void BlackBoardWindow::cancel() { reset(); }

    void BlackBoardWindow::erase() {
        if (m_editRow != 0 && m_editRow != m_selectedRow) {
            m_messages.show("Someone is writing", kShortMs);
            return;
        }
        if (m_selectedRow == 0) {
            m_messages.show("Please write a message", kShortMs);
            return;
        }
        if (m_rows[static_cast<std::size_t>(m_selectedRow)].name != m_hooks.ownName() && m_hooks.houseOwnerId() != m_hooks.ownId()) {
            m_messages.show("You are not the one that wrote the messages", kShortMs);
            return;
        }
        // 72/3: delete the message
        m_services.connection.send(protocol::blackBoardRequest(3, m_hooks.houseOwnerId(), std::string(1, static_cast<char>(m_selectedRow))));
        reset();
    }

    void BlackBoardWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (const int hovered = rowAt(m_screen.mouse())) {
            canvas.fillRectAlpha(
                {at.x + m_rowSpot.rect().x,
                 at.y + m_rowSpot.rect().y + (hovered - 1) * m_rowSpot.rect().height,
                 m_rowSpot.rect().width,
                 m_rowSpot.rect().height},
                m_rowSpot.color()
            );
        }
        const double now = core::nowAsDateTime();
        for (int row = 1; row <= kRows; ++row) {
            const Message& message = m_rows[static_cast<std::size_t>(row)];
            if (message.name.empty()) {
                continue;
            }
            const gfx::TextStyle style{
                std::abs(now - message.time) < kRecentDays ? m_recentSpot.color() : m_oldSpot.color(),
                gfx::TextEffect::Plain,
                std::nullopt
            };
            const int y = (row - 1) * m_rowSpot.pitch().y;
            const auto draw = [&](int x, int dy, const std::string& text) {
                canvas.drawText({at.x + x, at.y + dy + y}, text, style, static_cast<int>(text.size()) * kCharWidth);
            };
            draw(18, 61, core::formatDateTimeAmPm(message.time)); // pixel position of the time
            draw(210, 65, std::string("The one who leaves message") + kColon + message.name); // pixel position of the writer's name
            draw(154, 82, std::string("Content") + kColon + (m_editRow != row ? message.text : std::string())); // pixel position of the text
        }
        if (m_editRow != 0 && m_rows[static_cast<std::size_t>(m_editRow)].name.empty()) {
            const gfx::TextStyle style{m_recentSpot.color(), gfx::TextEffect::Plain, std::nullopt};
            const int y = at.y + (m_editRow - 1) * m_rowSpot.pitch().y;
            const auto draw = [&](int x, int dy, const std::string& text) {
                canvas.drawText({at.x + x, y + dy}, text, style, static_cast<int>(text.size()) * kCharWidth);
            };
            draw(21, 65, core::formatDateTimeAmPm(now)); // pixel position of the time
            draw(210, 65, "From: " + m_hooks.ownName()); // pixel position of the writer's name
            draw(154, 82, "Message: "); // pixel position of the text label
        }
    }

}
