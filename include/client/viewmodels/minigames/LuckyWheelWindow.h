#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_LUCKY_WHEEL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_LUCKY_WHEEL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/FrameEffect.h"
#include "client/subsystems/minigames/LuckyWheel.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "ui/Element.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Lucky Wheel's form: the form picture, the prizes, the box that runs round them, the frames of the top prizes, the lights of a win, the texts and the buttons, over a LuckyWheel.
    /// @details Original: LuckyWheelWindow.xml, TTurnGameForm's drawing, DrawTurnGame.
    class LuckyWheelWindow {
        public:
            /// @brief Loads the form layout, centres it on the screen and wires its buttons.
            /// @param services Game services (layouts, pictures, items).
            /// @param screen Screen the form is loaded into.
            /// @param info Item info popup used for the panel's prizes.
            /// @param hooks Callbacks handed to the LuckyWheel.
            /// @param imPoints Returns the player's IM points.
            LuckyWheelWindow(
                GameServices& services,
                ui::Screen& screen,
                ItemInfoPopup& info,
                LuckyWheel::Hooks hooks,
                std::function<int()> imPoints
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the game state.
            /// @return The LuckyWheel behind the window.
            LuckyWheel& wheel() { return m_wheel; }

            /// @brief Starts the game and shows the form with its buttons away.
            /// @details Original: StartTurnGame.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Hides the form and the item info.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Each frame: shows the buttons the game wants, the panel's items and the item under the mouse.
            /// @details Original: SetPrizeSlotItem for the panel; TurnGameMouseMove for the hover.
            void update();

        private:
            /// @brief Draws the form, prizes, buttons, spin box, frames, lights and texts.
            /// @details Original: DrawTurnGame, then the texts of DrawTurnGameInfo.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the drawing area (unused).
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Draws the panel's texts, the IM points, the prize counts and the hovered prize's name.
            /// @details Original: DrawTurnGameInfo.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the drawing area (unused).
            void paintInfo(gfx::Canvas& canvas, core::Point at);

            /// @brief Finds the picture of an item's icon.
            /// @param itemId Item id.
            /// @return The icon picture, or an empty id when the item or picture is unknown.
            gfx::PictureId iconOf(int itemId);

            /// @brief Shows the buttons the game wants shown.
            void showButtons();

            /// @brief Fills the panel with the top prizes.
            /// @details Original: SetPrizeSlotItem.
            void showPanel();

            /// @brief Starts the frame effect round a top prize.
            /// @param slot Panel slot of the prize.
            /// @param prizePosition Position of the prize on the form.
            /// @param tier Prize tier, 1 to 3 (picture A, B or C).
            void startFrameEffect(int slot, core::Point prizePosition, int tier);

            GameServices& m_services;
            ui::Screen& m_screen;
            ItemInfoPopup& m_info;
            std::function<int()> m_imPoints;
            LuckyWheel m_wheel;
            ui::Element& m_form;
            ui::IconGrid& m_panel;
            ui::ImageButton& m_refresh;
            ui::ImageButton& m_double;
            ui::ImageButton& m_start;
            ui::ImageButton& m_exit;
            const ui::Spot& m_prizeBoxSpot;
            const ui::Spot& m_rowTextSpot;
            const ui::Spot& m_pointsSpot;
            const ui::Spot& m_countSpot;
            const ui::Spot& m_doubleMarkSpot;
            const ui::Spot& m_frameSpot;
            const ui::Spot& m_getPrizeSpot;
            const ui::Spot& m_lightSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_tipTextSpot;
            const ui::Spot& m_tipFillSpot;
            const ui::Spot& m_tipBorderSpot;
            std::array<FrameEffect, LuckyWheel::kPanelSlots + 1> m_effects;
            int m_hoverPrize = 0;
            int m_hoverSlot = 0;
    };

}

#endif
