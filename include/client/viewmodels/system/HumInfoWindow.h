#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_HUM_INFO_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_HUM_INFO_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <array>

namespace client {

    /// @brief Window choosing what the map shows of characters, nine tick boxes.
    /// @details Original: HumInfoWindow.xml, TTH_HuminfoForm; ticks shown from the settings (LoadHumInfoChecks), written back on OK (ApplyHumInfoOptions).
    class HumInfoWindow {
        public:
            /// @brief Loads HumInfoWindow.xml and wires the tick, OK and close events.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param applied The settings' HumInfo1..9 values (1 = shown), written back on OK.
            HumInfoWindow(
                GameServices& services,
                ui::Screen& screen,
                std::array<int, 9>& applied
            );

            /// @brief Tells whether the window is visible.
            /// @return True while the window is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window (ticks reloaded from the settings) or closes it.
            /// @details Original: ToggleHumInfoForm.
            void toggle();

        private:
            /// @brief Sets each tick button's picture from the pending check states.
            void showChecks();

            GameServices& m_services;
            ui::Screen& m_screen;
            std::array<int, 9>& m_applied;
            std::array<bool, 9> m_checks{};
            ui::Window& m_window;
    };

}

#endif
