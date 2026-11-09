#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_LOTTERY_PRIZES_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_LOTTERY_PRIZES_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "protocol/LotteryLists.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <vector>

namespace client {

    /// @brief Window listing the prizes of the online draw, ten to a column.
    /// @details Original: LotteryPrizesWindow.xml, TLa_LotteryOLViewForm.
    class LotteryPrizesWindow {
        public:
            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param prizes Prize list owned by the draw window.
            /// @param info Item info popup shown over a hovered prize.
            LotteryPrizesWindow(
                GameServices& services,
                ui::Screen& screen,
                const std::vector<protocol::LotteryPrize>& prizes,
                ItemInfoPopup& info
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Refreshes and shows the window, 60 pixels from the top and centred.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Resizes the window and its buttons to the number of prizes.
            /// @details Original: RefreshItems and LayoutForCount.
            void refresh();

        private:
            /// @brief Paints the frame, the title and each prize's icon and name.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const std::vector<protocol::LotteryPrize>& m_prizes;
            ItemInfoPopup& m_info;
            ui::Window& m_window;
            ui::DrawingArea& m_page;
            ui::ImageButton& m_cancel;
            ui::ImageButton& m_close;
            const ui::Spot& m_formSpot;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_textColorSpot;
    };

}

#endif
