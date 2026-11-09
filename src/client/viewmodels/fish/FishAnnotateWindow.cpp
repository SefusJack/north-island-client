#include "client/viewmodels/fish/FishAnnotateWindow.h"

namespace client {

    namespace {

        /// @brief Loads FishAnnotateWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "FishAnnotate" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("FishAnnotateWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("FishAnnotate");
        }

    }

    FishAnnotateWindow::FishAnnotateWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)) {
        const core::Rect area = m_window.bounds();
        m_window.setPosition({(800 - area.width) / 2, (600 - area.height) / 2}); // 800 by 600: screen size
        screen.on("onFishAnnotateClose", [this](const ui::EventArgs&) { hide(); });
    }

    void FishAnnotateWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

}
