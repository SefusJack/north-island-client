#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_THROW_MONEY_BOX_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_THROW_MONEY_BOX_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The place-money box: an amount (10 to begin with) that OK drops on the ground.
    /// @details Original: ThrowMoneyBox.xml, TCY_ThrowMoenyMenu; sends 23/16 when the player has that much.
    class ThrowMoneyBox {
        public:
            /// @brief Loads the box layout and wires its buttons.
            /// @param services Game services (layouts, connection).
            /// @param screen Screen the box is loaded into.
            /// @param status Player status (gold).
            /// @param messages Sink for short player messages.
            /// @param mapId Returns the map the player is on.
            ThrowMoneyBox(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                PlayerMessages& messages,
                std::function<int()> mapId
            );

            /// @brief Tells whether the box is showing.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the box with the amount reset to 10, or closes it when open.
            /// @details Original: ShowCtrl.
            void toggle();

            /// @brief Closes the box.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Reads the amount field (an empty field becomes 0).
            /// @return The amount typed in the field.
            int amount();

            /// @brief Validates the amount and sends the throw-money request.
            /// @details Original: ConfirmThrowMoney.
            void confirm();

            GameServices& m_services;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            std::function<int()> m_mapId;
            ui::Window& m_window;
            ui::TextBox& m_amount;
    };

}

#endif
