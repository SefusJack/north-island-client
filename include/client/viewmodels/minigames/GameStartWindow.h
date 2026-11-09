#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_GAME_START_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_GAME_START_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <functional>

namespace client {

    /// @brief The instructions a mini game opens with: the game's picture with Begin and Leave under it and a small close button.
    /// @details Original: GameStartWindow.xml, CH_GameStartForm. Begin starts the game; Leave and the close button end it.
    class GameStartWindow {
        public:
            /// @brief Callbacks for the form's buttons.
            struct Hooks {
                std::function<void(int kind)> begin;
                std::function<void(int kind)> leave;
            };

            /// @brief Loads the form layout and wires its buttons.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks for Begin and Leave.
            GameStartWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the instructions of a game kind with its picture and button places.
            /// @details Original: SetupGameStartForm(kind) and ShowCtrl.
            /// @param kind Mini game kind.
            /// @param variant Look of a game with several looks (Hit Mouse: mouse, rabbit, turtle = 0, 1, 2; sheep game: pig 0, sheep 1).
            void open(int kind, int variant = 0);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Draws the figures (points, prices) a game puts on its instructions.
            /// @details Original: DrawControl of CH_GameStartForm.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            Hooks m_hooks;
            ui::Element& m_window;
            ui::ImageView& m_picture;
            ui::ImageButton& m_begin;
            ui::ImageButton& m_leave;
            ui::ImageButton& m_close;
            const ui::Spot& m_standardSpot;
            const ui::Spot& m_pointsSpot;
            int m_kind = 0;
            int m_variant = 0;
    };

}

#endif
