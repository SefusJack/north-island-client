#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_FORTUNE_EGG_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_FORTUNE_EGG_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/FortuneEgg.h"
#include "client/subsystems/minigames/MachineTables.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Fortune Egg's screen: machine picture with the ten prizes, their names and hints, the turn button, the dropping egg and the Exit button, over a FortuneEgg.
    /// @details Original: FortuneEggWindow.xml, DrawTrunEggGame.
    class FortuneEggWindow {
        public:
            /// @brief Loads the form layout and wires its buttons.
            /// @param services Game services (layouts, pictures, items).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to the FortuneEgg.
            FortuneEggWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the game state.
            /// @return The FortuneEgg behind the window.
            FortuneEgg& egg() { return m_egg; }

            /// @brief Starts the game, sets up the prize places and shows the form.
            /// @details Original: RecvSportStart, TTrunEggGame.Create and TTrunEggForm.SetupPrizeButtons.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Begins the game.
            void begin();

            /// @brief Hides the form.
            void close();

            /// @brief Steps the game and shows the buttons once the machine is active.
            /// @details Original: UpdateTrunEggGame.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

        private:
            /// @brief Draws the machine, prize boxes and icons, the button and egg animations and the prize names.
            /// @details Original: DrawTrunEggGame, then the names of DrawTrunEggPrizeNames.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the drawing area (unused).
            /// @param nowMs Current time in ms (unused).
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Places the ten prize areas over their icons and sets their hints.
            /// @details Original: TTrunEggForm.SetupPrizeButtons.
            void setUpPrizes();

            /// @brief Finds the picture of an item's icon.
            /// @param itemId Item id.
            /// @return The icon picture, or an empty id when the item or picture is unknown.
            gfx::PictureId iconOf(int itemId) const;

            GameServices& m_services;
            FortuneEgg m_egg;
            ui::Element& m_form;
            std::array<std::reference_wrapper<ui::DrawingArea>, MachineTables::kPrizes> m_prizes;
            ui::ImageButton& m_play;
            ui::ImageButton& m_exit;
            const ui::Spot& m_backgroundSpot;
            const ui::Spot& m_buttonSpot;
            const ui::Spot& m_eggSpot;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_nameSpot;
    };

}

#endif
