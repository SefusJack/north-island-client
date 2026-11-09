#include "client/viewmodels/login/CafeIdWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/CafeRequests.h"
#include "ui/DrawingArea.h"

#include <utility>

namespace client {

    namespace {

        /// @brief Loads CafeIdWindow.xml into the screen.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CafeIdWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CafeId");
        }

    }

    CafeIdWindow::CafeIdWindow(
        GameServices& services,
        ui::Screen& screen,
        std::function<void()> closeConnection
    )
        : m_services(services),
          m_screen(screen),
          m_closeConnection(std::move(closeConnection)),
          m_window(loadWindow(services, screen)),
          m_number(screen.get<ui::TextBox>("CafeIdNumber")) {
        screen.get<ui::DrawingArea>("CafeIdPaint").setPainter([](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            // text position (50, 48) in the form; 2113 is the RGB565 dark text colour; 150 max text width, 16 line height
            canvas.drawText({at.x + 50, at.y + 48}, "Please enter cafe number", {core::Color::fromRgb565(2113), gfx::TextEffect::Plain, std::nullopt}, 150, 16);
        });
        screen.on("onCafeIdOk", [this](const ui::EventArgs&) { send(); });
        screen.on("onCafeIdCancel", [this](const ui::EventArgs&) { cancel(); });
    }

    void CafeIdWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus("CafeIdNumber");
    }

    void CafeIdWindow::hide() {
        m_window.setVisible(false);
        m_number.clear();
    }

    void CafeIdWindow::send() {
        m_services.connection.send(protocol::cafeNumberRequest(m_number.text()));
        hide();
    }

    void CafeIdWindow::cancel() {
        hide();
        m_closeConnection();
    }

}
