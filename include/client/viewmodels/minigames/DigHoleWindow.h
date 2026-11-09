#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_DIG_HOLE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_DIG_HOLE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/DigHoleGame.h"
#include "client/subsystems/minigames/SportScene.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief The dig hole game's screen: sliding board picture, dug holes, rising prize, pointing finger, Quit and Dig buttons, over a DigHoleGame.
    /// @details Original: DigHoleWindow.xml, DrawDigHoleScene. A sport scene of kind 20.
    class DigHoleWindow : public SportScene {
        public:
            /// @brief Loads the form layout and wires its events.
            /// @param services Game services (layouts, pictures, items).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to each DigHoleGame.
            /// @param instructionsOpen Tells whether the instructions are showing.
            /// @param requestBoxOpen Tells whether a request box is open.
            DigHoleWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen,
                std::function<bool()> requestBoxOpen
            );

            /// @brief Gives the sport scene kind.
            /// @return 20 (dig hole).
            int kind() const override { return 20; } // sport scene kind 20: dig hole

            /// @brief Creates a fresh game and shows the board under the instructions.
            /// @details Original: RecvSportStart, TDigHole.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs) override;

            /// @brief Hides the form; the game is kept until the next one starts.
            void close() override;

            /// @brief Leaves the game.
            void leave() override;

            /// @brief Begins the round.
            /// @param nowMs Current time in ms (unused).
            void begin(std::uint64_t nowMs) override;

            /// @brief Passes a server payload to the game.
            /// @param payload Packet payload.
            void receive(const core::Bytes& payload) override;

            /// @brief Passes the points won to the game.
            /// @param points Points that arrived.
            void pointsArrived(std::int32_t points) override;

        private:
            /// @brief Updates the game and draws the board and the finger.
            /// @details Original: UpdateDigHoleGame and DrawDigHoleGame, with the mouse move hook of the sport loop.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the board picture, the Dig icon, the holes dug and the prize rising out of its hole.
            /// @details Original: DrawDigHoleScene.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between the last frame's slide and rise and the current ones to draw (1 = the current ones).
            void drawBoard(gfx::Canvas& canvas, double amount);

            /// @brief Draws the finger at the mouse, pointing over the board and pressed on a hole picked.
            /// @param canvas Canvas to draw on.
            void drawFinger(gfx::Canvas& canvas);

            GameServices& m_services;
            ui::Screen& m_screen;
            GameHooks m_hooks;
            std::function<bool()> m_instructionsOpen;
            std::function<bool()> m_requestBoxOpen;
            std::unique_ptr<DigHoleGame> m_game;
            ui::Element& m_form;
            ui::DrawingArea& m_page;
            ui::ImageButton& m_quit;
            ui::ImageButton& m_dig;
            const ui::Spot& m_boardSpot;
            const ui::Spot& m_holeSpot;
            const ui::Spot& m_prizeSpot;
            const ui::Spot& m_fingerSpot;
            const ui::Spot& m_fingerUpSpot;
            const ui::Spot& m_fingerDownSpot;
            core::Point m_lastMouse;
    };

}

#endif
