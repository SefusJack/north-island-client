#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_TOUCH_POINTER_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_TOUCH_POINTER_H

#include <SDL3/SDL.h>

#include <vector>

namespace client {

    /// @brief Turns fingers on a touch screen into the mouse the game is played with.
    /// @details A tap is a left click, a finger that moves drags with the left button, a finger held still is a right click,
    /// and two fingers moved apart or together zoom the view. SDL's own mouse buttons for a touch are dropped (its pointer
    /// moves are kept), because a left press must wait until it is known which of these the touch is.
    class TouchPointer {
        public:
            /// @brief Starts or stops reading touches; while stopped every event is left alone.
            /// @param enabled True to read them.
            void setEnabled(bool enabled) { m_enabled = enabled; }

            /// @brief Reads one event from SDL's queue, before anything else gets it.
            /// @param event The event, in window coordinates.
            /// @param window The game's window.
            /// @param nowMs The time now, ms.
            /// @return True when the event is used up here and must go no further.
            bool seeEvent(const SDL_Event& event, SDL_Window* window, double nowMs);

            /// @brief Notices a finger that has been held still long enough for a right click; once a frame.
            /// @param window The game's window.
            /// @param nowMs The time now, ms.
            void update(SDL_Window* window, double nowMs);

            /// @brief Takes the zoom steps two fingers have made since the last call.
            /// @return Steps as wheel notches with Ctrl: negative zooms in, positive out; 0 for none.
            int takeZoomSteps();

        private:
            /// @brief What the fingers on the screen are doing.
            enum class Phase {
                Idle, ///< no finger down
                Pending, ///< one finger down, not yet known whether a tap, a drag or a hold
                Dragging, ///< one finger moving with the left button down
                Spent, ///< the touch has done its work (a right click, a zoom); waits for every finger to lift
                Pinching, ///< two fingers down
            };

            /// @brief One finger on the screen.
            struct Finger {
                SDL_FingerID id;
                SDL_FPoint at; ///< px in the window
            };

            Finger* find(SDL_FingerID id);
            float spread() const;

            bool m_enabled = false;
            Phase m_phase = Phase::Idle;
            std::vector<Finger> m_fingers;
            SDL_FPoint m_start{0.0f, 0.0f}; // where the first finger came down, px in the window
            double m_startMs = 0.0;
            float m_pinchSpread = 0.0f; // the distance between the two fingers at the last zoom step, px
            int m_zoomSteps = 0;
            SDL_FPoint m_lastTap{0.0f, 0.0f};
            double m_lastTapMs = -1.0e9;
    };

}

#endif
