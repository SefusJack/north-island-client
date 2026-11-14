#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_CONTROLS_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_CONTROLS_H

#include "client/services/input/Gamepad.h"
#include "client/scenes/Scene.h"
#include "client/services/input/PadMenu.h"
#include "ui/Screen.h"

#include <SDL3/SDL.h>

namespace client {

    /// @brief Plays the game with a pad: the left stick and D-pad walk, the right stick moves the mouse pointer, the buttons click and send the game's shortcuts.
    /// @details Controller mode starts with any pad input and ends when the mouse or the keyboard is used; outside it nothing here does anything.
    /// The pointer is the system's own, and clicks and keys go onto SDL's event queue, so everything that reads the mouse or the keyboard works as it is.
    class PadControls {
        public:
            ~PadControls();

            /// @brief Sets the hands the menu's highlight is shown with (PadMenu::setPointers).
            /// @param above The hand for the bar at the bottom.
            /// @param below The hand for the bar at the top.
            void setPointers(const ui::PadPointer& above, const ui::PadPointer& below) {
                m_menu.setPointers(above, below);
                m_aimHand = above;
            }

            /// @brief Whether the pad was the last thing used.
            /// @return True in controller mode.
            bool isControllerMode() const { return m_controllerMode; }

            /// @brief Whether the HUD should be out of the way: in controller mode, some time after the last use of the menus.
            /// @details Start brings the HUD up and sends it away again; an open window keeps it up.
            /// @return True when the HUD may hide.
            bool wantsHudHidden() const { return m_controllerMode && !m_hudUp; }

            /// @brief Whether the shortcut bar should show although the HUD is hidden: while the left trigger is held.
            /// @details With the trigger held the D-pad's up, right, down and left use the bar's places F1 to F4, and the four face
            /// buttons (bottom, right, left, top) F5 to F8.
            /// @return True while the shortcut bar is in use from the pad.
            bool wantsShortcutBar() const { return m_controllerMode && m_shortcuts; }

            /// @brief Looks at every event before it is handed on: mouse or keyboard use ends controller mode; a pointer move made here while the click button is held is marked as a drag.
            /// @param event The event, in window coordinates (before SDL_ConvertEventToRenderCoordinates).
            /// @param screen The screen shown.
            void seeEvent(SDL_Event& event, ui::Screen& screen);

            /// @brief Runs one frame: reads the pad and moves the pointer, walks, clicks and sends keys.
            /// @param nowMs The time in milliseconds.
            /// @param pad The pad.
            /// @param window The game's window.
            /// @param screen The screen shown.
            /// @param hudHideSeconds How long after the last use of the menus the HUD hides (0: only when Start sends it away).
            void update(double nowMs, Gamepad& pad, SDL_Window* window, Scene& scene, core::Point origin, int hudHideSeconds);

        private:
            /// @brief Enters or leaves controller mode.
            /// @param on True for controller mode.
            /// @param screen The screen shown (the walking keys come up and the menu closes when leaving).
            void setControllerMode(bool on, ui::Screen& screen);

            /// @brief Holds the arrow keys the walking direction asks for.
            /// @param pad The pad.
            /// @param screen The screen shown.
            void walk(const Gamepad& pad, ui::Screen& screen);

            /// @brief Moves the pointer with the right stick.
            /// @param pad The pad.
            /// @param window The game's window.
            /// @param seconds The time since the last frame.
            /// @param nowMs The time in milliseconds.
            void movePointer(const Gamepad& pad, SDL_Window* window, float seconds, double nowMs);

            /// @brief Acts on a button going down or up.
            /// @param press The button and which way it went.
            /// @param window The game's window.
            /// @param screen The screen shown.
            /// @param nowMs The time in milliseconds.
            void act(const PadPress& press, SDL_Window* window, Scene& scene, double nowMs);

            /// @brief Puts the pointer somewhere in the window.
            /// @param window The game's window.
            /// @param x Window x.
            /// @param y Window y.
            void warp(SDL_Window* window, float x, float y);

            /// @brief Puts the pointer on a point of the game's screen.
            /// @param window The game's window.
            /// @param point The point in the screen's coordinates.
            /// @param origin Where the game's screen lies in what is drawn.
            void warpToScreen(SDL_Window* window, core::Point point, core::Point origin);

            /// @brief Steps the pointer to the next place of the topmost window in a direction; at the end of a list that goes on, the list scrolls.
            /// @param way 1 left, 2 right, 3 up, 4 down.
            /// @param window The game's window.
            /// @param screen The screen shown.
            /// @param origin Where the game's screen lies in what is drawn.
            void focusStep(int way, SDL_Window* window, ui::Screen& screen, core::Point origin);

            /// @brief Gives where the pointer stands on the game's screen.
            /// @param window The game's window.
            /// @return The point in the screen's coordinates.
            core::Point pointerOnScreen(SDL_Window* window) const;

            /// @brief Shows or hides the system's pointer.
            /// @param shown True to show it.
            void showPointer(bool shown);

            bool m_controllerMode = false;
            PadMenu m_menu; // the highlight over the two button bars
            bool m_shortcuts = false; // the left trigger is held: the D-pad and the face buttons use the shortcut bar
            bool m_hudUp = true; // the HUD is wanted on the screen
            double m_menuMs = 0.0; // when the menus were last used
            double m_lastMs = 0.0;
            float m_pointerX = 0.0f; // the pointer in window coordinates, with the fraction a frame's move leaves over
            float m_pointerY = 0.0f;
            /// @brief Whether a mouse event's place is one the pointer was put at here a moment ago.
            /// @param x Window x.
            /// @param y Window y.
            /// @return True for a move made here.
            bool isOwnMove(float x, float y) const;

            /// @brief A place the pointer was put at here.
            struct Warp {
                float x = -1.0f;
                float y = -1.0f;
                double atMs = -1000000.0; // long ago: not waited for
            };

            static constexpr int kWarpsKept = 16; // the system may report a move a few frames late
            Warp m_warps[kWarpsKept]; // the last places, newest at m_lastWarp (their mouse events are not the player's)
            int m_lastWarp = 0;
            double m_fullTiltMs = 0.0; // since when the right stick is pushed all the way
            double m_pointerUsedMs = 0.0;
            bool m_pointerShown = true;
            bool m_leftHeld = false; // the click button holds the left mouse button
            int m_clicks = 1; // which click in a row the left button's last press was: the second makes a double click
            double m_clickMs = -1000000.0; // when that press was
            float m_clickX = 0.0f; // where the pointer was then
            float m_clickY = 0.0f;
            ui::PadPointer m_aimHand; // the hand a scene marks with what the click button would use
            bool m_steps = false; // the scene shows a list of choices: up and down step through it
            core::Point m_origin; // where the game's screen lies in what is drawn
            bool m_sceneClick = false; // a click the scene asked for waits for the pointer to have arrived
            bool m_sceneClickRight = false; // it is made with the right button
            double m_sceneClickMs = 0.0; // when it is made
            bool m_dragging = false; // the screen drags a thing (the pad carries it, or it was taken up to follow the pointer)
            bool m_carry = false; // a thing was picked up in a grid: the left button stays held until the click button puts it down
            std::vector<ui::PadSpot> m_sceneSpots; // the places of the scene itself the pointer steps through (a fight's targets)
            bool m_sceneFocus = false; // there were such places last frame
            int m_focusWindow = 0; // the topmost window the pointer steps through (0: none)
            bool m_focus = false; // a window is open: the D-pad and the left stick step the pointer through it
            int m_focusWay = 0; // the way held for stepping: 1 left, 2 right, 3 up, 4 down
            double m_focusStepMs = 0.0; // when the held way steps again
            int m_stickStep = 0; // the way the left stick is held for stepping: 1 up, 2 down, 3 left, 4 right
            double m_stickStepMs = 0.0; // when the held stick steps again
            bool m_rightHeld = false;
            bool m_walking[4] = {}; // the arrow keys held for walking: left, right, up, down
    };

}

#endif
