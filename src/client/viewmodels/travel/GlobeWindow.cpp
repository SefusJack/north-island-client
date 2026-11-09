#include "client/viewmodels/travel/GlobeWindow.h"

namespace client {

    namespace {

        /// @brief Loads GlobeWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "GlobeWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GlobeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("GlobeWindow");
        }

    }

    GlobeWindow::GlobeWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)) {
        screen.on("onGlobeClose", [this](const ui::EventArgs&) { close(); });
    }

    void GlobeWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

}
