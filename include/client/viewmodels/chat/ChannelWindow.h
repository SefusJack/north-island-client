#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_CHANNEL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_CHANNEL_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <array>

namespace client {

    /// @brief The chat channels window (ChannelWindow.xml, TTH_channelForm): 1 local, 2 whisper, 3 team, 4 party, 5 world, each on or off.
    /// @details Original: the server says how they stand (33/2, LoadChannelSettings) and hears the new ones on OK (33/3, ApplyChannelSettings).
    class ChannelWindow {
        public:
            static constexpr int kChannels = 5;

            /// @brief Loads ChannelWindow.xml and wires the window.
            /// @param services Game services (layout, connection, pictures).
            /// @param screen Screen the window is loaded into.
            ChannelWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows or hides the window; opening resets the checks to the enabled channels.
            /// @details Original: ToggleChannelForm.
            void toggle();

            /// @brief Takes the channel settings the server sent (33/2).
            /// @param bits Bit n - 1 is set for channel n when on.
            /// @details Original: LoadChannelSettings.
            void load(int bits);

            /// @brief Whether a channel is on.
            /// @param channel Channel number 1 .. 5.
            /// @return True when on.
            /// @details Original: FChannelEnabled.
            bool isOn(int channel) const { return channel >= 1 && channel <= kChannels && m_enabled[static_cast<std::size_t>(channel)]; }

        private:
            /// @brief Sets each channel button picture to On or Off by its check.
            void showButtons();

            GameServices& m_services;
            ui::Screen& m_screen;
            ui::Window& m_window;
            std::array<bool, kChannels + 1> m_enabled{false, true, true, true, true, true};
            std::array<bool, kChannels + 1> m_checked{false, true, true, true, true, true};
    };

}

#endif
