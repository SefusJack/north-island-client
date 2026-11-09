#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_BLACK_BOARD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_BLACK_BOARD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The house's blackboard window (BlackBoardWindow.xml, TH_BlackBoardForm): five message rows the owner or the writer can write or delete.
    /// @details Original: TH_Menu.pas TH_BlackBoardForm, opened by the owner's click on a Blackboard 39073. A row shows when the message was left, who left it and its text (yellow for five days, then red). The server keeps the messages (72/1 lists them, C 72/2 writes, C 72/3 deletes).
    class BlackBoardWindow {
        public:
            /// @brief Callbacks into the rest of the client that the blackboard needs.
            struct Hooks {
                std::function<std::int32_t()> houseOwnerId;
                std::function<std::int32_t()> ownId;
                std::function<std::string()> ownName;
            };

            static constexpr int kRows = 5;

            /// @brief Loads the window layout and wires the buttons and the paint area.
            /// @param services Game services (layouts, connection).
            /// @param screen Screen the window is loaded into.
            /// @param messages Player message line for notices.
            /// @param hooks Callbacks into the rest of the client.
            BlackBoardWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window (OK, Delete and Cancel out of sight) and asks the server for the messages (72/1).
            /// @details Original: the owner's click on the blackboard, ShowCtrl.
            void open();

            /// @brief Takes a server answer: 72/1 the messages, 72/2 with 1 "There is leave word here".
            /// @details Original: RecvBlackBoard.
            /// @param payload The packet payload.
            void receive(const core::Bytes& payload);

        private:
            /// @brief One board message.
            struct Message {
                std::string name;
                double time = 0;
                std::string text;
            };

            /// @brief Puts the window back to nothing picked, no editor, no buttons.
            /// @details Original: HideCtrl, and the end of a write, a delete and Cancel.
            void reset();

            /// @brief Hides the window and resets it.
            void hide();

            /// @brief Sends the written message (C 72/2 with the row and the text) after checking it.
            /// @details Original: SubmitBoardMessage.
            void submit();

            /// @brief Drops the message being written.
            /// @details Original: CancelBoardEdit.
            void cancel();

            /// @brief Deletes the selected message (C 72/3 with the row).
            /// @details Original: DeleteBoardMessage.
            void erase();

            /// @brief Picks a row: opens the editor for an empty or own message, only Delete for another's one (owner of the house).
            /// @details Original: SelectBoardRow.
            /// @param row Row number (1..5).
            void selectRow(int row);

            /// @brief Handles a mouse release: picks the row under the mouse.
            /// @details Original: HandleMouseUp.
            void clicked();

            /// @brief Finds the row under a point.
            /// @param mouse Mouse position on the screen.
            /// @return Row number (1..5), 0 for none.
            int rowAt(core::Point mouse) const;

            /// @brief Shows or hides the OK/Cancel and Delete buttons.
            /// @param write Show OK and Cancel.
            /// @param erase Show Delete.
            void showButtons(bool write, bool erase);

            /// @brief Paints the hovered row and the time, name and text of each message.
            /// @details Original: DrawBoardHover and DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_edit;
            ui::ImageButton& m_ok;
            ui::ImageButton& m_delete;
            ui::ImageButton& m_cancel;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_recentSpot;
            const ui::Spot& m_oldSpot;
            std::array<Message, kRows + 1> m_rows{};
            int m_editRow = 0;
            int m_selectedRow = 0;
    };

}

#endif
