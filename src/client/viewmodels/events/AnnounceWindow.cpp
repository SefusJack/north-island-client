#include "client/viewmodels/events/AnnounceWindow.h"

#include <utility>

namespace client {

    namespace {

        /// @brief Loads AnnounceWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("AnnounceWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Announce");
        }

    }

    AnnounceWindow::AnnounceWindow(
        GameServices& services,
        ui::Screen& screen,
        std::function<void()> onClosed
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("AnnounceText")),
          m_onClosed(std::move(onClosed)) {
        screen.get<ui::ScrollBar>("AnnounceScroll").setRange(1, 1);
        screen.on("onAnnounceClose", [this](const ui::EventArgs&) { close(); });
    }

    void AnnounceWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void AnnounceWindow::setText(const std::string& text) {
        m_screen.get<ui::ScrollBar>("AnnounceScroll").setPosition(0);
        m_text.clear();
        m_text.appendLine(text);
    }

    void AnnounceWindow::close() {
        m_window.setVisible(false);
        m_onClosed();
    }

}
