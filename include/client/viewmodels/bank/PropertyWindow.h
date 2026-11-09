#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_PROPERTY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_PROPERTY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "gfx/Canvas.h"
#include "ui/BlockList.h"
#include "ui/ScrollBar.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Lost property office window: lists lost items, the player marks rows and pays points to reclaim them.
    /// @details Original: PropertyWindow.xml, MK_Menu.pas TPropertyForm; server 86/1 list, 86/2 reclaim done, 86/3 reclaim error; client 86/2 reclaim request.
    class PropertyWindow {
        public:
            /// @brief Callbacks into the rest of the game.
            struct Hooks {
                std::function<std::int32_t()> points;
                std::function<void(const std::string& text, std::function<void()> onOk)> ask;
                std::function<void(const std::string&)> chatLine;
                std::function<void(const core::Bytes&)> send;
            };

            /// @brief Builds the window and wires its list, scroll bar and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param messages Player message sink.
            /// @param hooks Callbacks into the player state.
            PropertyWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Handles a server 86 message (list, reclaim done or reclaim error).
            /// @details Original: RecvLostPropertyList (86/1), RecvReclaimDone (86/2), RecvReclaimError (86/3).
            /// @param payload The whole message, starting with the action id.
            void received(const core::Bytes& payload);

            /// @brief Hides the window and drops the list and the marks.
            /// @details Original: HideCtrl (ClearPropertyList).
            void close();

        private:
            /// @brief One lost item as the server sent it.
            struct Record {
                int itemId = 0;
                int fusion = 0;
                int pose = 0;
            };

            /// @brief Reads the lost property list and fills the rows.
            /// @details Original: RecvLostPropertyList (86/1).
            /// @param payload The whole 86/1 message.
            void list(const core::Bytes& payload);

            /// @brief Paints the cost and the marker bars over marked rows.
            /// @details Original: TPropertyForm.DrawControl.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paint(gfx::Canvas& canvas, core::Point at) const;

            /// @brief Counts the marked rows.
            /// @return Number of marked rows.
            int markedCount() const;

            /// @brief Marks a row, once, in the first free place.
            /// @details Original: PickPropertyRow.
            /// @param row Row index.
            void mark(int row);

            /// @brief Removes the mark of a row.
            /// @param row Row index.
            void unmark(int row);

            /// @brief Asks the player to confirm the reclaim.
            /// @details Original: ConfirmReclaimProperty.
            void confirm();

            /// @brief Sends the reclaim request for the marked rows and closes the form.
            /// @details Original: SendReclaimProperty (86/2).
            void send();

            /// @brief Scrolls the list so a row is at the top, clamped to the range.
            /// @param top Row index for the top.
            void scrollTo(int top);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_list;
            ui::ScrollBar& m_scroll;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_costSpot;
            const ui::Spot& m_markSpot;
            std::vector<Record> m_records;
            std::vector<std::int32_t> m_values;
            std::array<int, 50> m_marked{}; // per bag slot (50 slots): the record row marked there, -1 for none
    };

}

#endif
