#include "client/viewmodels/chat/InnAnswerWindow.h"

#include <utility>

namespace client {

    namespace {

        /// @brief Loads InnAnswerWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "InnAnswer" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("InnAnswerWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("InnAnswer");
        }

    }

    InnAnswerWindow::InnAnswerWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("InnAnswerText")) {
        screen.on("onInnAnswerClose", [this](const ui::EventArgs&) { close(); });
    }

    void InnAnswerWindow::show(const std::string& line, std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        m_text.clear();
        m_text.appendLine(line);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void InnAnswerWindow::close() {
        m_window.setVisible(false);
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

}
