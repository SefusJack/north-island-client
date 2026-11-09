#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_CAMP_STYLE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_CAMP_STYLE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief Tent style chooser window.
    /// @details Original: CampStyleWindow.xml, TChangeCampForm. The Tent Palette (set 1, eleven styles) or the Shape
    /// Clay (set 2, eighteen) was used on a pitched tent; the arrows go through the styles, OK asks for the one shown (65/12).
    class CampStyleWindow {
        public:
            /// @brief Callbacks the window uses to reach the server and the item window.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<core::Rect()> itemWindowBounds;
                std::function<void(int left)> moveItemWindow;
                std::function<void()> marksChanged;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Game services (layout, sprites).
            /// @param screen Screen the window is loaded into.
            /// @param marks Bag cell marks.
            /// @param hooks Callbacks to the server and the item window.
            CampStyleWindow(
                GameServices& services,
                ui::Screen& screen,
                BagMarks& marks,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Asks the server to use the item of a bag place on the tent.
            /// @details Original: RequestChangeCamp; the server answers 23/159 or 23/158.
            /// @param bagSlot Bag place of the item.
            /// @param set Style set (1 = Tent Palette, 2 = Shape Clay).
            void request(int bagSlot, int set);

            /// @brief Opens the chooser for the item.
            /// @details Original: 23/159, ShowChangeCampForm.
            /// @param bagSlot Bag place of the item.
            /// @param set Style set (1 = Tent Palette, 2 = Shape Clay).
            void open(int bagSlot, int set);

            /// @brief Closes the chooser and frees the cell mark.
            /// @details Original: 23/158 (item cannot be used on the tent) closes the form; HideCtrl/ResetCampKind.
            void close();

        private:
            /// @brief Draws the tent in the style shown.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Moves to the previous or next style, wrapping around.
            /// @details Original: PrevCampKind / NextCampKind.
            /// @param direction +1 for next, -1 for previous.
            void step(int direction);

            /// @brief Sends the shown style to the server and closes the window.
            /// @details Original: ConfirmChangeCamp (65/12).
            void confirm();

            /// @brief Resets the style index and frees the cell mark.
            /// @details Original: ResetCampKind.
            void reset();

            /// @brief Number of styles in the current set.
            /// @return 18 for the Shape Clay, else 11.
            int styleCount() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            BagMarks& m_marks;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_tentSpot;
            int m_slot = 0;
            int m_set = 0;
            int m_index = 1;
    };

}

#endif
