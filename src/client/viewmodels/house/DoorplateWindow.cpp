#include "client/viewmodels/house/DoorplateWindow.h"

#include "protocol/DoorplateRequests.h"

#include <utility>

namespace client {

    namespace {

        /// @brief Loads DoorplateWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded Doorplate window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("DoorplateWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Doorplate");
        }

    }

    DoorplateWindow::DoorplateWindow(
        GameServices& services,
        ui::Screen& screen,
        const Doorplates& doorplates,
        Hooks hooks
    )
        : m_screen(screen),
          m_doorplates(doorplates),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::TextBox>("DoorplateText")) {
        screen.on("onDoorplateOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onDoorplateClose", [this](const ui::EventArgs&) { hide(); });
    }

    void DoorplateWindow::open() {
        m_text.setText(m_doorplates.text(m_hooks.ownId()));
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void DoorplateWindow::confirm() {
        m_hooks.send(protocol::doorplateRequest(m_text.text()));
        hide();
    }

    void DoorplateWindow::update() {
        if (m_window.isVisible() && !m_hooks.inFloorRoom()) {
            hide();
        }
    }

}
