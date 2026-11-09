#include "client/viewmodels/social/BlessMessageWindow.h"

#include "client/rules/TextFilter.h"
#include "core/StringUtil.h"
#include "protocol/WishRequests.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kMessageMs = 2000; // message display time, ms

        /// @brief Loads BlessMessageWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The BlessMessage window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BlessMessageWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BlessMessage");
        }

    }

    BlessMessageWindow::BlessMessageWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("BlessMessageText")) {
        screen.on("onBlessMessageClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onBlessMessageSend", [this](const ui::EventArgs&) { send(); });
    }

    void BlessMessageWindow::open(int bagSlot, std::int32_t targetId) {
        m_bagSlot = bagSlot;
        m_targetId = targetId;
        m_text.clear();
        m_window.setVisible(true);
        m_window.fitToScreen();
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
        m_screen.focus("BlessMessageText");
    }

    void BlessMessageWindow::close() {
        m_hooks.closeBalloon();
        m_window.setVisible(false);
    }

    void BlessMessageWindow::send() {
        if (m_targetId == 0) {
            return;
        }
        if (!m_done) {
            m_messages.show("You cannot set more than one delivery at a time", kMessageMs);
            return;
        }
        std::string text;
        for (const char c : m_text.text()) {
            text += c == '\r' ? std::string("\r\n") : std::string(1, c);
        }
        text += "\r\n";
        if (!isTextClean(text)) {
            m_messages.show("Don't use illegal words", kMessageMs);
            return;
        }
        m_hooks.send(protocol::blessMessageRequest(m_bagSlot, m_targetId, static_cast<int>(core::trim(m_text.text()).size()), text));
        m_done = false;
        close();
    }

}
