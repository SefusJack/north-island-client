#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_DOLL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_DOLL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/DollGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief The claw machine's screen: machine picture, dolls, claw, mask picture, tooltip and the form's buttons (Move left, Move right, Grasp, Quit), over a DollGame.
    /// @details Original: DollWindow.xml, DrawCatchDollGame.
    class DollWindow {
        public:
            /// @brief Loads the form layout and wires its buttons.
            /// @param services Game services (layouts, pictures, characters, npcs).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to each DollGame.
            /// @param instructionsOpen Tells whether the instructions are showing.
            DollWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the play state (only valid while a game exists).
            /// @return The DollGame behind the window.
            DollGame& game() { return *m_game; }

            /// @brief Creates the machine with its dolls and shows its picture under the instructions.
            /// @details Original: RecvSportStart, TSportCatchDoll.Create and TCatchDollForm.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Hides the form and drops the game.
            void close();

        private:
            /// @brief Updates the game and draws background, dolls, claw, foreground and tooltip.
            /// @details Original: UpdateCatchDollGame and DrawCatchDollGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws each doll, a fifth larger than a figure, lit where the mouse is on it.
            /// @details Original: DrawHumanSprite for each doll.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between where the last frame took the claw and the held doll from and to to draw them (1 = where they are).
            /// @return Index of the first doll under the mouse, or -1 for none.
            int drawDolls(gfx::Canvas& canvas, double amount);

            /// @brief Draws the claw from the pictures the frame's draw chose.
            /// @details Original: DrawCatcherClaw.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between where the last frame took the claw from and to to draw it (1 = where it is).
            void drawClaw(gfx::Canvas& canvas, double amount);

            /// @brief Draws the name of the doll under the mouse as a tooltip.
            /// @details Original: DrawCatchDollTooltip.
            /// @param canvas Canvas to draw on.
            /// @param hovered Index of the doll under the mouse, or -1 for none.
            void drawTooltip(gfx::Canvas& canvas, int hovered);

            /// @brief Finds the sprite slot a doll's direction names, or one the sprite has when it lacks that.
            /// @param npc The doll's npc record.
            /// @param direction The doll's direction.
            /// @return The slot to draw the doll in.
            int slotOf(const formats::NpcInfo& npc, int direction) const;

            /// @brief Builds the hooks the DollGame needs (npc lookup, sprite size, frame count).
            /// @return The hooks, including the shared game hooks.
            DollGame::Hooks makeHooks() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            GameHooks m_hooks;
            std::function<bool()> m_instructionsOpen;
            std::unique_ptr<DollGame> m_game;
            ui::Element& m_form;
            ui::ImageButton& m_left;
            ui::ImageButton& m_right;
            ui::ImageButton& m_grasp;
            ui::ImageButton& m_quit;
            const ui::Spot& m_groundSpot;
            const ui::Spot& m_maskSpot;
    };

}

#endif
