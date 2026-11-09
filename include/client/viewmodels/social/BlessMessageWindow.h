#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_BLESS_MESSAGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_BLESS_MESSAGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Bliss Lamp's message form (original BlessMessageWindow.xml, CJ_Menu.pas TEnterMsgForm2).
    /// @details The words (20 characters) for the player chosen in the name form (BlessBalloonWindow). OK sends 23/129 (the lamp's place, the player, the length of the words, the words) and the lamp is used up when 23/215 answers. Only one lamp is out at a time (FBlessDone).
    class BlessMessageWindow {
        public:
            /// @brief Callbacks the form uses to reach the rest of the client.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void()> closeBalloon;
            };

            /// @brief Builds the form.
            /// @param services Shared game services.
            /// @param screen Screen the window is added to.
            /// @param messages Sink for player messages.
            /// @param hooks Callbacks to the client.
            BlessMessageWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form with the words cleared and the keyboard in it.
            /// @details Original ShowCtrl.
            /// @param bagSlot Bag place of the lamp.
            /// @param targetId Id of the player chosen in the name form.
            void open(int bagSlot, std::int32_t targetId);

            /// @brief Hides the form and the name form with it.
            /// @details Original HideCtrl.
            void close();

            /// @brief Marks that a lamp may be sent again.
            /// @details Original Player.FBlessDone.
            void setDone() { m_done = true; }

        private:
            /// @brief Sends the message (23/129) when no lamp is out and the words are clean.
            /// @details Original SendBlessMessage.
            void send();

            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Memo& m_text;
            int m_bagSlot = 0;
            std::int32_t m_targetId = 0;
            bool m_done = true;
    };

}

#endif
