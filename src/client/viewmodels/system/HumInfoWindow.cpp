#include "client/viewmodels/system/HumInfoWindow.h"

#include "ui/ImageButton.h"

#include <string>

namespace client {

    namespace {

        /// @brief Loads HumInfoWindow.xml into the screen.
        /// @param services Shared game services (layout path and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The HumInfo window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("HumInfoWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("HumInfo");
        }

        /// @brief Extracts the tick number from a control name "HumInfoCheckN".
        /// @param name Control name.
        /// @return N, or 0 when the name does not start with the prefix.
        int checkOf(const std::string& name) {
            const std::string prefix = "HumInfoCheck";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

    }

    HumInfoWindow::HumInfoWindow(
        GameServices& services,
        ui::Screen& screen,
        std::array<int, 9>& applied
    )
        : m_services(services),
          m_screen(screen),
          m_applied(applied),
          m_window(loadWindow(services, screen)) {
        screen.on("onHumInfoCheck", [this](const ui::EventArgs& args) {
            const int check = checkOf(args.source);
            if (check >= 1 && check <= static_cast<int>(m_checks.size())) {
                m_checks[static_cast<std::size_t>(check - 1)] = !m_checks[static_cast<std::size_t>(check - 1)];
                showChecks();
            }
        });
        screen.on("onHumInfoOk", [this](const ui::EventArgs&) {
            for (std::size_t i = 0; i < m_checks.size(); ++i) {
                m_applied[i] = m_checks[i] ? 1 : 0;
            }
            m_window.setVisible(false);
        });
        screen.on("onHumInfoClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
    }

    void HumInfoWindow::toggle() {
        if (isOpen()) {
            m_window.setVisible(false);
            return;
        }
        for (std::size_t i = 0; i < m_checks.size(); ++i) {
            m_checks[i] = m_applied[i] != 0;
        }
        showChecks();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void HumInfoWindow::showChecks() {
        for (std::size_t i = 0; i < m_checks.size(); ++i) {
            m_screen.get<ui::ImageButton>("HumInfoCheck" + std::to_string(i + 1))
                .setPicture(m_services.pictures.find(m_checks[i] ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}));
        }
    }

}
