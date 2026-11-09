#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_LOVE_FLOWER_MESSAGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_LOVE_FLOWER_MESSAGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Passion Bouquet's message form (original LoveFlowerMessageWindow.xml, MK_Menu.pas TEnterMsgForm).
    /// @details The words (20 characters) for the player chosen in the name form (LoveFlowerWindow). OK sends 23/118 (the bouquet's place, the player, the length of the words, the words) and the bouquet is used up when 23/209 answers.
    class LoveFlowerMessageWindow {
        public:
            /// @brief Callbacks the form uses to reach the rest of the client.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void()> releaseFlower;
            };

            /// @brief Builds the form.
            /// @param services Shared game services.
            /// @param screen Screen the window is added to.
            /// @param messages Sink for player messages.
            /// @param hooks Callbacks to the client.
            LoveFlowerMessageWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form with the keyboard in it.
            /// @details Original ShowCtrl.
            /// @param bagSlot Bag place of the bouquet.
            /// @param targetId Id of the player chosen in the name form.
            void open(int bagSlot, std::int32_t targetId);

            /// @brief Marks that a bouquet may be sent again.
            /// @details Original Player.FFlowerMsgPending := 0.
            void setDone() { m_pending = false; }

        private:
            /// @brief Hides the form with its words and frees the bouquet's cell.
            /// @details Original CancelFlowerMessage.
            void cancel();

            /// @brief Sends the message (23/118) when no bouquet is out and the words are clean.
            /// @details Original SendFlowerMessage.
            void send();

            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Memo& m_text;
            int m_bagSlot = 0;
            std::int32_t m_targetId = 0;
            bool m_pending = false;
    };

}

#endif
