#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_HUD_REVEAL_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_HUD_REVEAL_H

#include "ui/Screen.h"

namespace client {

    /// @brief Slides the world's HUD off the screen's edges and back, for play with a game pad.
    /// @details The pieces are found by their layout names, so a screen without them (the login) is left alone. The button
    /// bars, the shortcut bar and the small map always go; the status panel and the chat box go when their setting says so.
    /// What the game hides itself stays hidden: only the place a piece is drawn at changes here.
    class HudReveal {
        public:
            /// @brief The parts of the HUD that hide separately.
            enum class Group { Bars, Shortcuts, Status, Chat, Count };

            /// @brief Runs one frame of the slide.
            /// @param nowMs The time in milliseconds.
            /// @param screen The screen shown.
            /// @param hidden Whether the HUD should be out of the way now.
            /// @param hideStatus Whether the status panel goes with it.
            /// @param hideChat Whether the chat box goes with it.
            /// @param showShortcuts Whether the shortcut bar stays, or comes back alone, while the rest is hidden.
            void update(double nowMs, ui::Screen& screen, bool hidden, bool hideStatus, bool hideChat, bool showShortcuts);

        private:
            double m_lastMs = 0.0;
            float m_away[static_cast<int>(Group::Count)] = {}; // how far each group has gone: 0 in its place, 1 off the screen
    };

}

#endif
