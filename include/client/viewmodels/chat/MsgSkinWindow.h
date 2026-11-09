#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_MSG_SKIN_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_MSG_SKIN_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <cstdint>
#include <filesystem>

namespace client {

    /// @brief The talk box's look window (MsgSkinWindow.xml, TTH_MsgSkinsForm): skin 1 or 2 for the main talk box.
    /// @details Original: kept per character in user/<player>MsgSkin.txt (LoadMsgSkinSetting writes "1" when missing).
    class MsgSkinWindow {
        public:
            /// @brief Loads MsgSkinWindow.xml and wires the window.
            /// @param services Game services (layout, pictures, user root).
            /// @param screen Screen the window is loaded into.
            /// @param talk Talk dialog whose skin is set.
            MsgSkinWindow(
                GameServices& services,
                ui::Screen& screen,
                TalkDialog& talk
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows or hides the window.
            /// @details Original: ToggleMsgSkinsForm.
            void toggle();

            /// @brief Reads the player's saved skin and applies it; writes "1" when the file is missing.
            /// @param playerId Player id (names the file).
            /// @details Original: LoadMsgSkinSetting, on entering the world.
            void load(std::int32_t playerId);

        private:
            /// @brief Shows which of the two skins is checked.
            /// @details Original: UpdateMsgSkinChecks.
            void showChecks();

            /// @brief Path of the player's skin file.
            /// @return user/<player>MsgSkin.txt.
            std::filesystem::path file() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            TalkDialog& m_talk;
            ui::Window& m_window;
            std::int32_t m_playerId = 0;
            bool m_first = true;
    };

}

#endif
