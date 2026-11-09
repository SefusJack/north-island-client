#include "client/viewmodels/social/LoveFlowerMessageWindow.h"

#include "client/rules/TextFilter.h"
#include "core/StringUtil.h"
#include "protocol/WishRequests.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kMessageMs = 2000; // message display time, ms

        /// @brief Loads LoveFlowerMessageWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The LoveFlowerMessage window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LoveFlowerMessageWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("LoveFlowerMessage");
        }

    }

    LoveFlowerMessageWindow::LoveFlowerMessageWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("LoveFlowerMessageText")) {
        screen.on("onLoveFlowerMessageClose", [this](const ui::EventArgs&) { cancel(); });
        screen.on("onLoveFlowerMessageSend", [this](const ui::EventArgs&) { send(); });
    }

    void LoveFlowerMessageWindow::open(int bagSlot, std::int32_t targetId) {
        m_bagSlot = bagSlot;
        m_targetId = targetId;
        m_window.setVisible(true);
        m_window.fitToScreen();
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
        m_screen.focus("LoveFlowerMessageText");
    }

    void LoveFlowerMessageWindow::cancel() {
        m_window.setVisible(false);
        m_text.clear();
        m_hooks.releaseFlower();
    }

    void LoveFlowerMessageWindow::send() {
        if (m_targetId == 0 || m_pending) {
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
        m_hooks.send(protocol::flowerMessageRequest(m_bagSlot, m_targetId, static_cast<int>(core::trim(m_text.text()).size()), text));
        m_pending = true;
        m_text.clear();
        m_window.setVisible(false);
    }

}
