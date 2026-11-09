#include "client/viewmodels/chat/ChannelWindow.h"

#include "protocol/SystemRequests.h"
#include "ui/ImageButton.h"

#include <string>

namespace client {

    namespace {

        /// @brief Loads ChannelWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "Channels" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ChannelWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Channels");
        }

        /// @brief Channel number from a button name such as "Channel3".
        /// @param name Control name.
        /// @return The number, or 0 when the name has no "Channel" prefix.
        int channelOf(const std::string& name) {
            const std::string prefix = "Channel";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

    }

    ChannelWindow::ChannelWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_services(services),
          m_screen(screen),
          m_window(loadWindow(services, screen)) {
        screen.on("onChannelToggle", [this](const ui::EventArgs& args) {
            const int channel = channelOf(args.source);
            if (channel >= 1 && channel <= kChannels) {
                m_checked[static_cast<std::size_t>(channel)] = !m_checked[static_cast<std::size_t>(channel)];
                showButtons();
            }
        });
        screen.on("onChannelOk", [this](const ui::EventArgs&) {
            int bits = 0;
            for (int channel = 1; channel <= kChannels; ++channel) {
                m_enabled[static_cast<std::size_t>(channel)] = m_checked[static_cast<std::size_t>(channel)];
                if (m_enabled[static_cast<std::size_t>(channel)]) {
                    bits |= 1 << (channel - 1);
                }
            }
            m_services.connection.send(protocol::channelsRequest(bits));
            m_window.setVisible(false);
        });
        screen.on("onChannelClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
    }

    void ChannelWindow::toggle() {
        if (isOpen()) {
            m_window.setVisible(false);
            return;
        }
        m_checked = m_enabled;
        showButtons();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ChannelWindow::load(int bits) {
        for (int channel = 1; channel <= kChannels; ++channel) {
            m_enabled[static_cast<std::size_t>(channel)] = ((bits >> (channel - 1)) & 1) != 0;
        }
        m_checked = m_enabled;
        showButtons();
    }

    void ChannelWindow::showButtons() {
        for (int channel = 1; channel <= kChannels; ++channel) {
            const char* picture = m_checked[static_cast<std::size_t>(channel)] ? "Btn_On_1" : "Btn_Off_1";
            m_screen.get<ui::ImageButton>("Channel" + std::to_string(channel)).setPicture(m_services.pictures.find(picture).value_or(gfx::PictureId{}));
        }
    }

}
