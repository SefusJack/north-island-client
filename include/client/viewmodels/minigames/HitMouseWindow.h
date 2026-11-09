#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_HIT_MOUSE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_HIT_MOUSE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/rendering/effects/LightEffect.h"
#include "client/subsystems/minigames/HitMouseGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>

namespace client {

    /// @brief The hit-the-mouse game's screen: field picture, seven holes with what comes up, the light of a hit, time, score and countdown, with the mallet cursor and the Exit button, over a HitMouseGame.
    /// @details Original: HitMouseWindow.xml, DrawHitMouseGame.
    class HitMouseWindow {
        public:
            /// @brief Loads the form layout and wires its events.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to the HitMouseGame.
            HitMouseWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the game state.
            /// @return The HitMouseGame behind the window.
            HitMouseGame& game() { return m_game; }

            /// @brief Starts the game and shows the field under the instructions.
            /// @details Original: CH_THitMouseGame.Create.
            /// @param npcId Npc id of the game's host.
            /// @param nowMs Current time in ms.
            void open(int npcId, std::uint64_t nowMs);

            /// @brief Begins the game and shows the Exit button.
            /// @details Original: the form's Begin (GameExplainForm.ShowCtrl).
            void begin();

            /// @brief Hides the form and stops the hit light.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Steps the game and draws the field, holes, light, time, score and countdown.
            /// @details Original: UpdateHitMouseGame and DrawHitMouseGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws what comes up out of a hole: a frame of the target or the bomb, or of the target's hit.
            /// @param canvas Canvas to draw on.
            /// @param hole The hole to draw.
            void drawHole(gfx::Canvas& canvas, const HitMouseGame::Hole& hole) const;

            /// @brief Handles the mouse button going up: the mallet comes down and a hit or bomb lights up.
            /// @details Original: HitMouseMouseUp.
            /// @param args Event arguments with the mouse position.
            void released(const ui::EventArgs& args);

            GameServices& m_services;
            HitMouseGame m_game;
            LightEffect m_light;
            ui::Element& m_form;
            ui::DrawingArea& m_page;
            ui::ImageButton& m_exit;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_timeNumberSpot;
            const ui::Spot& m_scoreSpot;
            const ui::Spot& m_scoreNumberSpot;
            const ui::Spot& m_messageSpot;
            std::uint64_t m_frameMs = 0;
    };

}

#endif
