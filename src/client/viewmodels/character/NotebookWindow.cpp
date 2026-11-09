#include "client/viewmodels/character/NotebookWindow.h"

#include "core/Log.h"

namespace client {

    namespace {

        /// @brief Loads NotebookWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "Notebook" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NotebookWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Notebook");
        }

    }

    NotebookWindow::NotebookWindow(
        GameServices& services,
        ui::Screen& screen,
        std::function<void()> toggleQuests,
        std::function<void()> toggleMonsters,
        std::function<void()> openStars
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_toggleQuests(std::move(toggleQuests)),
          m_toggleMonsters(std::move(toggleMonsters)),
          m_openStars(std::move(openStars)) {
        screen.on("onNotebookQuests", [this](const ui::EventArgs&) { m_toggleQuests(); });
        screen.on("onNotebookMonsters", [this](const ui::EventArgs&) { m_toggleMonsters(); });
        screen.on("onNotebookStars", [this](const ui::EventArgs&) { m_openStars(); });
        screen.on("onNotebookClose", [this](const ui::EventArgs&) { close(); });
    }

    void NotebookWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

}
