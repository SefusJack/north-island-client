#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_ATM_TRANSFER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_ATM_TRANSFER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The ATM's transfer form: the gold left in the ATM, an online player's name and the amount to move into his ATM, asked once more before it goes.
    /// @details Original: AtmTransferWindow.xml, TATMTransferForm; request 29/5.
    class AtmTransferWindow {
        public:
            /// @brief Callbacks into the game: player lookup, the confirm question and the player list.
            struct Hooks {
                std::function<std::optional<std::int32_t>(const std::string&)> findPlayer;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
                std::function<void()> openList;
                std::function<void()> closeList;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Game services.
            /// @param screen Screen the form is loaded into.
            /// @param atmGold Gold in the ATM.
            /// @param messages Short messages to the player.
            /// @param hooks Callbacks into the game.
            AtmTransferWindow(
                GameServices& services,
                ui::Screen& screen,
                const std::uint32_t& atmGold,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is showing.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with no name and the amount 0.
            /// @details Original: ShowCtrl.
            /// @param onClosed Called when the form closes (HideCtrl asks for the next event result).
            void show(std::function<void()> onClosed);

            /// @brief Hides the form and the player list, and calls the close handler.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Puts a picked player's name into the name field.
            /// @details Original: PickListPlayer.
            /// @param name Player name.
            void setName(const std::string& name) { m_name.setText(name); }

        private:
            /// @brief Checks the amount and the player, then asks once more before sending.
            /// @details Original: ConfirmATMTransfer.
            void confirm();

            /// @brief Sends the transfer request and closes the form.
            /// @details Original: SendATMTransfer; request 29/5.
            /// @param playerId Id of the receiving player.
            /// @param amount Gold to move.
            void send(std::int32_t playerId, std::int32_t amount);

            /// @brief Paints the gold in the ATM.
            /// @param canvas Target canvas.
            /// @param at Top-left of the painting area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const std::uint32_t& m_atmGold;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_name;
            ui::TextBox& m_amount;
            const ui::Spot& m_textSpot;
            std::function<void()> m_onClosed;
    };

}

#endif
