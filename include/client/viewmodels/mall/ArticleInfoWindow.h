#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MALL_ARTICLE_INFO_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MALL_ARTICLE_INFO_WINDOW_H

#include "client/services/GameServices.h"
#include "client/data/GiftBoxContents.h"
#include "client/models/mall/LuckyBags.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief Window listing what a random pack or Lucky Bag of the mall holds.
    /// @details Original: ArticleInfoWindow.xml, se_mainUI.pas TArticleInfoForm. Ten items per column with icons and
    /// names (the 7 Colors Egg lists its pets); the item under the mouse is described.
    class ArticleInfoWindow {
        public:
            /// @brief Loads the window layout and wires its close button.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param bags Lucky Bag contents.
            /// @param info Item info popup used for the hovered item.
            ArticleInfoWindow(
                GameServices& services,
                ui::Screen& screen,
                const LuckyBags& bags,
                ItemInfoPopup& info
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the contents of a box or Lucky Bag.
            /// @details Original: ShowGiftBoxContents. Nothing is shown for a bag not known or with more than 41 rows.
            /// @param boxId Item id of the box or bag.
            void show(int boxId);

            /// @brief Hides the window and its item info.
            /// @details Original: HideCtrl.
            void hide();

            /// @brief Shows the info of the item under the mouse, hides it otherwise.
            void update();

        private:
            /// @brief Fills and sizes the window for a list of entries and shows it centred.
            /// @details Original: AddArticleItem, AddArticlePet and ResizeArticleForm, ShowArticleInfoCentered.
            /// @param title Title text.
            /// @param entries Items held.
            /// @param pets True when the entries are listed as pets.
            void fill(const std::string& title, const std::vector<GiftBoxContents::Entry>& entries, bool pets);

            /// @brief Paints the frame, title, icons and names (or pets).
            /// @details Original: DrawControl, DrawArticleItemNames, DrawArticlePets.
            /// @param canvas Target canvas.
            /// @param at Top-left of the window.
            /// @param nowMs Current time in milliseconds (unused).
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Place of an entry's icon from the form's corner.
            /// @param number 1-based entry number.
            /// @return Top-left of the icon.
            core::Point iconAt(int number) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const LuckyBags& m_bags;
            ItemInfoPopup& m_info;
            ui::Window& m_window;
            ui::DrawingArea& m_content;
            ui::ImageButton& m_close;
            ui::ImageButton& m_cancel;
            const ui::Spot& m_formSpot;
            const ui::Spot& m_cellsSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_petSpot;
            const ui::Spot& m_petNameSpot;
            const ui::Spot& m_titleSpot;
            const ui::Spot& m_textSpot;
            std::string m_title;
            std::vector<GiftBoxContents::Entry> m_entries;
            bool m_pets = false;
            int m_width = 0;
            bool m_infoShown = false;
    };

}

#endif
