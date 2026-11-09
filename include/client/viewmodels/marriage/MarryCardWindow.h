#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MARRIAGE_MARRY_CARD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MARRIAGE_MARRY_CARD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/social/Marriage.h"
#include "core/Point.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Marry tab of the social window: the spouse's card in a window that takes the social window's place.
    /// @details Original: MarryCardWindow.xml, TMarryCard. Its Friend, Emotion and Setup tabs bring the social window
    /// back on that tab; the mail icon writes to the spouse and a double click on the face whispers to him.
    class MarryCardWindow {
        public:
            /// @brief Callbacks into the social window, mail and whisper.
            struct Hooks {
                std::function<void(core::Point)> showFriends;
                std::function<void(core::Point)> showEmotions;
                std::function<void(core::Point)> showSetup;
                std::function<void()> mailSpouse;
                std::function<void(std::int32_t)> whisper;
            };

            /// @brief Loads the window layout and wires its tabs, mail icon and painter.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param marriage Marriage state holding the spouse's card.
            /// @param hooks Callbacks into the surrounding windows.
            MarryCardWindow(
                GameServices& services,
                ui::Screen& screen,
                Marriage& marriage,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the card at the place the social window was.
            /// @details Original: ShowMarryCard.
            /// @param at Top-left position for the window.
            void open(core::Point at);

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Paints the figure, the face and the card's texts and icons.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Whispers to the spouse when the double click is on the face.
            /// @details Original: HandleDblClick.
            void doubleClicked();

            /// @brief Closes this window and lets the social window take its place on a tab.
            /// @param show Hook that shows the social window on the wanted tab.
            void backTo(const std::function<void(core::Point)>& show);

            GameServices& m_services;
            ui::Screen& m_screen;
            Marriage& m_marriage;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_figureSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_elementSpot;
            const ui::Spot& m_onlineSpot;
            const ui::Spot& m_onlinePartSpot;
            const ui::Spot& m_levelSpot;
            const ui::Spot& m_jobSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_guildSpot;
            const ui::Spot& m_nicknameSpot;
            const ui::Spot& m_locationSpot;
            const ui::Spot& m_statSpot;
            FigureAnimation m_animation;
    };

}

#endif
