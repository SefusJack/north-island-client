#include "client/viewmodels/chat/MsgSkinWindow.h"

#include "core/FileSystem.h"
#include "ui/ImageButton.h"

#include <string>

namespace client {

    namespace {

        /// @brief Loads MsgSkinWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "MsgSkin" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MsgSkinWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MsgSkin");
        }

        /// @brief Writes the skin number and a line break to the skin file.
        /// @param file Skin file path.
        /// @param skin Skin number.
        void writeSkin(const std::filesystem::path& file, int skin) {
            const std::string line = std::to_string(skin) + "\r\n";
            core::writeFile(file, core::Bytes(line.begin(), line.end()));
        }

    }

    MsgSkinWindow::MsgSkinWindow(
        GameServices& services,
        ui::Screen& screen,
        TalkDialog& talk
    )
        : m_services(services),
          m_screen(screen),
          m_talk(talk),
          m_window(loadWindow(services, screen)) {
        screen.on("onMsgSkinChoose", [this](const ui::EventArgs& args) {
            m_first = args.source == "MsgSkin1";
            showChecks();
        });
        screen.on("onMsgSkinConfirm", [this](const ui::EventArgs&) {
            const int skin = m_first ? 1 : 2;
            if (m_talk.skin() != skin) {
                writeSkin(file(), skin);
            }
            m_talk.setSkin(skin);
            m_window.setVisible(false);
        });
        screen.on("onMsgSkinClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
    }

    std::filesystem::path MsgSkinWindow::file() const { return m_services.game.userRoot() / (std::to_string(m_playerId) + "MsgSkin.txt"); }

    void MsgSkinWindow::load(std::int32_t playerId) {
        m_playerId = playerId;
        const auto bytes = core::readFile(file());
        if (!bytes) {
            writeSkin(file(), 1);
            m_talk.setSkin(1);
            return;
        }
        const std::string text(bytes->begin(), bytes->end());
        m_talk.setSkin(!text.empty() && text[0] == '2' ? 2 : 1); // skin numbers: 2 when the file starts with '2', else 1
    }

    void MsgSkinWindow::toggle() {
        if (isOpen()) {
            m_window.setVisible(false);
            return;
        }
        m_first = m_talk.skin() == 1;
        showChecks();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void MsgSkinWindow::showChecks() {
        const auto picture = [this](bool checked) { return m_services.pictures.find(checked ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}); };
        m_screen.get<ui::ImageButton>("MsgSkin1").setPicture(picture(m_first));
        m_screen.get<ui::ImageButton>("MsgSkin2").setPicture(picture(!m_first));
    }

}
