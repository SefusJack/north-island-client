#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_JIGSAW_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_JIGSAW_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/JigsawGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief A Jigsaw machine's screen: machine picture with the ten prizes and their hints, three strips, the Bingo picture over the middle, the Start and Quit buttons, over a JigsawGame of the kind started (11, 12 or 14).
    /// @details Original: JigsawWindow.xml, DrawMarioGame.
    class JigsawWindow {
        public:
            /// @brief Loads the form layout and wires its buttons.
            /// @param services Game services (layouts, pictures, items).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to each JigsawGame.
            /// @param instructionsOpen Tells whether the instructions are showing.
            JigsawWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the game state (only valid while a game exists).
            /// @return The JigsawGame behind the window.
            JigsawGame& game() { return *m_game; }

            /// @brief Creates the machine of a kind, sets up the prizes and shows its picture under the instructions.
            /// @details Original: RecvSportStart, TSportMario.Create and TMarioForm.Create.
            /// @param kind Machine kind (11, 12 or 14).
            /// @param nowMs Current time in ms.
            void open(int kind, std::uint64_t nowMs);

            /// @brief Hides the form and drops the game.
            void close();

        private:
            /// @brief Updates the game and draws the machine, prizes, strips, Bingo picture and prize names.
            /// @details Original: UpdateMarioGame and DrawMarioGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Places the ten prize areas over their icons and sets their hints.
            /// @details Original: TMarioForm.Create. A hint is the prize's description; the second machine's prizes name what they add.
            void setUpPrizes();

            /// @brief Finds the picture of an item's icon.
            /// @param itemId Item id.
            /// @return The icon picture, or an empty id when the item or picture is unknown.
            gfx::PictureId iconOf(int itemId) const;

            GameServices& m_services;
            GameHooks m_hooks;
            std::function<bool()> m_instructionsOpen;
            std::unique_ptr<JigsawGame> m_game;
            ui::Element& m_form;
            std::array<std::reference_wrapper<ui::DrawingArea>, MachineTables::kPrizes> m_prizes;
            ui::ImageButton& m_start;
            ui::ImageButton& m_quit;
            const ui::Spot& m_groundSpot;
            const ui::Spot& m_stripSpot;
            const ui::Spot& m_bingoSpot;
            const ui::Spot& m_nameSpot;
    };

}

#endif
