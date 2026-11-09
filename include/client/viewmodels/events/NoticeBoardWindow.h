#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_NOTICE_BOARD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_NOTICE_BOARD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/events/NoticeBoard.h"
#include "client/viewmodels/events/NoticeReadForm.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The public notice board window: ten notices a page, a click opens one.
    /// @details Original: NoticeBoardWindow.xml, TNoticeBoardForm; the server opens it with 82/9.
    class NoticeBoardWindow {
        public:
            /// @brief Callbacks into the rest of the game that the window needs.
            struct Hooks {
                std::function<std::int32_t()> ownId;
                std::function<int()> ownLevel;
                std::function<void()> closeForms;
            };

            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param board The notice board model.
            /// @param read Form that shows a picked notice.
            /// @param hooks Callbacks into the game.
            NoticeBoardWindow(
                GameServices& services,
                ui::Screen& screen,
                NoticeBoard& board,
                NoticeReadForm& read,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window and asks for the list again.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window and the forms of the board.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Opens the notice of the clicked row in the read form.
            /// @details Original: HandleMouseUp.
            void clicked();

            /// @brief Paints the hover bar over the rows and their texts.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top left of the rows area on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            ui::Screen& m_screen;
            NoticeBoard& m_board;
            NoticeReadForm& m_read;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_subjectSpot;
            const ui::Spot& m_senderSpot;
            const ui::Spot& m_dateSpot;
            const ui::Spot& m_pageNumbersSpot;
            const ui::Spot& m_textColorSpot;
            const ui::Spot& m_pageColorSpot;
            const ui::Spot& m_barColorSpot;
    };

}

#endif
