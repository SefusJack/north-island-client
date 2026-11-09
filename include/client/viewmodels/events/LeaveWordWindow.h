#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_LEAVE_WORD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_LEAVE_WORD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "gfx/Canvas.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Guestbook window listing the leave-word messages, five rows per page.
    /// @details Original: LeaveWordWindow.xml, BK_Menu.pas TBK_LeaveWordForm; server action 36 (36/3 open, 36/4 all, 36/5 one more, 36/6 one removed).
    class LeaveWordWindow {
        public:
            /// @brief Name and look of the player who wrote a message.
            struct Writer {
                std::string name;
                CharacterLook look;
            };

            /// @brief Callbacks into the rest of the game that the window needs.
            struct Hooks {
                std::function<std::int32_t()> ownId;
                std::function<std::string()> ownName;
                std::function<std::optional<Writer>(std::int32_t)> writerOf;
                std::function<std::optional<std::int32_t>(const std::string&)> playerNamed;
                std::function<void(std::int32_t)> whisper;
                std::function<void()> closed;
            };

            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Sink for short player messages.
            /// @param hooks Callbacks into the game.
            LeaveWordWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Handles a server action 36 packet.
            /// @details Original: 36/3 opens the form, 36/4 all messages, 36/5 a message, 36/6 a message is gone, 36/7 an answer to a request.
            /// @param payload The whole message; byte 1 is the sub id.
            void received(const core::Bytes& payload);

            /// @brief Recolours every row written under the own name.
            /// @details Original: SetLeaveWordRolePatt1 and SetLeaveWordRolePatt5 (23/53, 23/103).
            /// @param part Colour part, 1 or 5.
            /// @param code New colour code.
            void colorsChanged(int part, std::int32_t code);

        private:
            /// @brief One message in the list.
            struct Row {
                std::string name;
                std::string time;
                std::string text;
                CharacterLook look;
                bool hasFace = false;
            };

            /// @brief Shows the window, scrolled to the end of the list.
            /// @details Original: TBK_LeaveWordForm.ShowCtrl, ScrollLeaveWordToEnd.
            void open();

            /// @brief Replaces the list with every message from the server.
            /// @details Original: RevAllLeaveWord (36/4).
            /// @param payload The whole message.
            void listAll(const core::Bytes& payload);

            /// @brief Appends one message from the server.
            /// @details Original: RevLeaveWord (36/5).
            /// @param payload The whole message.
            void added(const core::Bytes& payload);

            /// @brief Removes one message on the server's word.
            /// @details Original: RevClearLeaveWord (36/6).
            /// @param payload The whole message.
            void removed(const core::Bytes& payload);

            /// @brief Erases a row and keeps the selection on its row.
            /// @details Original: DeleteListItem.
            /// @param index Row index from 0.
            void erase(int index);

            /// @brief Sets the first row shown.
            /// @details Original: SetListTopIndex.
            /// @param top Wanted first row.
            void setTop(int top);

            /// @brief Recomputes the page count and current page.
            /// @details Original: UpdateLeaveWordPages.
            void updatePages();

            /// @brief Closes the window unless a request is pending.
            /// @details Original: CloseLeaveWordForm.
            void close();

            /// @brief Sends the typed message to the server (C 36/1).
            /// @details Original: SendLeaveWord.
            void send();

            /// @brief Asks the server to remove the selected own message (C 36/2).
            /// @details Original: ClearLeaveWords.
            void clear();

            /// @brief Selects the next message whose writer's name holds the typed text.
            /// @details Original: DoForwardFind.
            void find();

            /// @brief Turns to the next page.
            /// @details Original: NextLeaveWordPage.
            void nextPage();

            /// @brief Turns to the previous page.
            /// @details Original: PrevLeaveWordPage.
            void previousPage();

            /// @brief Selects a row when it is an own message.
            /// @details Original: SelectOwnRoleItem.
            /// @param row Row index, or -1 for none.
            void selectOwn(int row);

            /// @brief Starts a whisper to the writer of a row.
            /// @details Original: WhisperToWriter.
            /// @param row Row index, or -1 for none.
            void whisperTo(int row);

            /// @brief Finds the row under a point in the list.
            /// @details Original: GetListHoverIndex.
            /// @param local Point relative to the list.
            /// @return Row index, or -1 for none.
            int rowAt(core::Point local) const;

            /// @brief Paints names, faces, highlights, texts and times.
            /// @param canvas Target canvas.
            /// @param at Top left of the list on the canvas.
            /// @param nowMs Current time in milliseconds.
            void paintList(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the page number, a slash and the page count.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            void paintPages(gfx::Canvas& canvas, core::Point at);

            /// @brief Draws a number with the digit picture.
            /// @details Original: DrawNumber with the number font 6 at the size 9.
            /// @param canvas Target canvas.
            /// @param at Top left of the first digit.
            /// @param value Number to draw.
            void drawNumber(gfx::Canvas& canvas, core::Point at, int value);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::DrawingArea& m_list;
            ui::TextBox& m_message;
            ui::TextBox& m_name;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_columnSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_nameColorSpot;
            const ui::Spot& m_timeColorSpot;
            const ui::Spot& m_rowTextColorSpot;
            const ui::Spot& m_hoverColorSpot;
            const ui::Spot& m_selectColorSpot;
            const ui::Spot& m_pageSlashSpot;
            const ui::Spot& m_textColorSpot;
            const ui::Spot& m_pageIndexSpot;
            const ui::Spot& m_pageCountSpot;
            const ui::Spot& m_digitSpot;
            std::vector<Row> m_rows;
            int m_top = 0;
            int m_selected = -1;
            int m_requestState = 0;
            int m_pageCount = 0;
            int m_pageIndex = 0;
            PortraitBlink m_blink;
    };

}

#endif
