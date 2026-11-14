#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_MENU_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_MENU_H

#include "client/services/input/Gamepad.h"
#include "ui/Screen.h"

#include <string>
#include <vector>

namespace client {

    /// @brief The game pad's menu over the world's two button bars: one button is highlighted, the D-pad or left stick moves the highlight, and the button is pressed from the pad.
    /// @details It starts on the bar at the bottom right; up goes to the bar at the top right and down comes back. The buttons
    /// are taken in the order they stand on the screen, and those not shown are passed over.
    class PadMenu {
        public:
            /// @brief Whether the menu is open.
            /// @return True while a button is highlighted.
            bool isOpen() const { return m_open; }

            /// @brief Sets the hands the highlight is shown with.
            /// @param above The hand for the bar at the bottom: it stands above a button.
            /// @param below The hand for the bar at the top: it stands below a button.
            void setPointers(const ui::PadPointer& above, const ui::PadPointer& below) {
                m_above = above;
                m_below = below;
            }

            /// @brief Opens the menu on the button it was last on, or the first of the bottom bar.
            /// @param screen The screen shown.
            /// @return False when the screen shows none of the bars' buttons (the menu stays closed).
            bool open(ui::Screen& screen);

            /// @brief Closes the menu and takes the highlight away.
            /// @param screen The screen shown.
            void close(ui::Screen& screen);

            /// @brief Presses the highlighted button and closes the menu.
            /// @param screen The screen shown.
            void choose(ui::Screen& screen);

            /// @brief Runs one frame: moves the highlight by the D-pad or the left stick, again and again while held.
            /// @param nowMs The time in milliseconds.
            /// @param pad The pad.
            /// @param screen The screen shown.
            void update(double nowMs, const Gamepad& pad, ui::Screen& screen);

        private:
            /// @brief A way the highlight moves.
            enum class Way { None, Left, Right, Up, Down };

            /// @brief A button of a bar and where it stands.
            struct Shown {
                std::string name;
                int x = 0;
            };

            /// @brief Lists the buttons of a bar that are shown, from left to right.
            /// @param screen The screen shown.
            /// @param top True for the bar at the top right.
            /// @return The buttons.
            static std::vector<Shown> shownOf(ui::Screen& screen, bool top);

            /// @brief Moves the highlight one step.
            /// @param way Which way.
            /// @param screen The screen shown.
            void step(Way way, ui::Screen& screen);

            bool m_open = false;
            ui::PadPointer m_above; // the hand over a button of the bottom bar
            ui::PadPointer m_below; // the hand under a button of the top bar
            bool m_top = false; // the highlight is on the bar at the top right
            std::string m_current; // the highlighted button's name
            Way m_held = Way::None;
            double m_repeatMs = 0.0; // when the held way steps again
    };

}

#endif
