#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_BLUEPRINT_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_BLUEPRINT_WINDOW_H

#include "client/services/GameServices.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <string>
#include <vector>

namespace client {

    /// @brief The blueprint recipe window (BlueprintWindow.xml, TTH_blueprintForm): product and one to five materials on a form of one of three sizes.
    /// @details Original: TH_Menu.pas TTH_blueprintForm; opened by a double click on a blueprint of the bag (item sub type 103).
    class BlueprintWindow {
        public:
            /// @brief Loads the window layout and wires the close buttons and painter.
            /// @param services Game services (layouts, items, compounds, pictures).
            /// @param screen Screen the layout is loaded into.
            BlueprintWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Opens the recipe of a blueprint, or closes the window if it is open (ToggleBlueprintForm after UseGridItem stored the item).
            /// @param blueprintItemId Item id of the blueprint.
            void toggle(int blueprintItemId);

        private:
            /// @brief An item icon with its place on the form and the text line beside it (FItemIcon and FPanels).
            struct Entry {
                int itemId = 0;
                core::Point at;
                std::string text;
                core::Point textAt;
            };

            /// @brief Lays out the recipe of the blueprint: form picture and size, buttons and entries (TTH_blueprintForm.DrawControl).
            void layout();

            /// @brief Paints the lines, then the icons over them, then the title picture.
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Draws an entry's item icon, as large as the item's cells.
            /// @param canvas Target canvas.
            /// @param entry Entry holding the item and its place.
            /// @param at Top-left of the paint area.
            void icon(gfx::Canvas& canvas, const Entry& entry, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::ImageButton& m_close;
            ui::ImageButton& m_closeSmall;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_titleSpot;
            int m_blueprintItem = 0;
            std::string m_titlePicture;
            core::Point m_completeAt;
            Entry m_product;
            std::vector<Entry> m_materials;
    };

}

#endif
