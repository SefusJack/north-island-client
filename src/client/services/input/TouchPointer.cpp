#include "client/services/input/TouchPointer.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr SDL_MouseID kTouchPointerMouse = 0x544F5543; // marks the mouse events made here ("TOUC")
        constexpr float kSlack = 0.025f; // a finger that moves further than this part of the window's height is dragging
        constexpr double kHoldMs = 450.0; // a finger held still this long is a right click
        constexpr double kDoubleMs = 500.0; // two taps within this time make a double click
        constexpr float kDoubleReach = 0.06f; // and no further apart than this part of the window's height
        constexpr float kZoomRatio = 1.2f; // the fingers' distance changes by this factor for each zoom step

        /// @brief Puts a mouse button press or release onto SDL's event queue.
        /// @param window The game's window.
        /// @param button SDL_BUTTON_LEFT or SDL_BUTTON_RIGHT.
        /// @param down True for a press.
        /// @param at Where in the window, px.
        /// @param clicks Which click in a row it is (2: a double click).
        void pushButton(SDL_Window* window, Uint8 button, bool down, SDL_FPoint at, int clicks = 1) {
            SDL_Event event{};
            event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
            event.button.windowID = SDL_GetWindowID(window);
            event.button.which = kTouchPointerMouse;
            event.button.button = button;
            event.button.down = down;
            event.button.clicks = static_cast<Uint8>(clicks);
            event.button.x = at.x;
            event.button.y = at.y;
            SDL_PushEvent(&event);
        }

        /// @brief Puts a pointer move onto SDL's event queue.
        /// @param window The game's window.
        /// @param at Where in the window, px.
        /// @param held True while the left button is down.
        void pushMove(SDL_Window* window, SDL_FPoint at, bool held) {
            SDL_Event event{};
            event.type = SDL_EVENT_MOUSE_MOTION;
            event.motion.windowID = SDL_GetWindowID(window);
            event.motion.which = kTouchPointerMouse;
            event.motion.state = held ? SDL_BUTTON_LMASK : 0;
            event.motion.x = at.x;
            event.motion.y = at.y;
            SDL_PushEvent(&event);
        }

        float distance(SDL_FPoint a, SDL_FPoint b) {
            return std::hypot(a.x - b.x, a.y - b.y);
        }

    }

    TouchPointer::Finger* TouchPointer::find(SDL_FingerID id) {
        const auto found = std::find_if(m_fingers.begin(), m_fingers.end(), [id](const Finger& finger) { return finger.id == id; });
        return found != m_fingers.end() ? &*found : nullptr;
    }

    float TouchPointer::spread() const {
        return m_fingers.size() >= 2 ? distance(m_fingers[0].at, m_fingers[1].at) : 0.0f;
    }

    bool TouchPointer::seeEvent(const SDL_Event& event, SDL_Window* window, double nowMs) {
        if (!m_enabled) {
            return false;
        }
        switch (event.type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                return event.button.which == SDL_TOUCH_MOUSEID; // SDL's own press for a touch: the ones made here stand for it
            case SDL_EVENT_MOUSE_MOTION:
                return event.motion.which == SDL_TOUCH_MOUSEID; // and its moves: they carry the first finger on during a zoom
            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_MOTION:
            case SDL_EVENT_FINGER_UP:
            case SDL_EVENT_FINGER_CANCELED:
                break;
            default:
                return false;
        }
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window, &width, &height);
        const SDL_FPoint at{event.tfinger.x * static_cast<float>(width), event.tfinger.y * static_cast<float>(height)};
        const float slack = kSlack * static_cast<float>(height);
        if (event.type == SDL_EVENT_FINGER_DOWN) {
            if (find(event.tfinger.fingerID) == nullptr) {
                m_fingers.push_back({event.tfinger.fingerID, at});
            }
            if (m_fingers.size() == 1) {
                m_phase = Phase::Pending;
                m_start = at;
                m_startMs = nowMs;
                pushMove(window, at, false); // the pointer is under the finger before anything is pressed
            } else if (m_fingers.size() == 2 && m_phase != Phase::Spent) {
                if (m_phase == Phase::Dragging) {
                    pushButton(window, SDL_BUTTON_LEFT, false, m_fingers[0].at);
                }
                m_phase = Phase::Pinching;
                m_pinchSpread = spread();
            }
            return true;
        }
        Finger* finger = find(event.tfinger.fingerID);
        if (finger == nullptr) {
            return true;
        }
        if (event.type == SDL_EVENT_FINGER_MOTION) {
            finger->at = at;
            if (m_phase == Phase::Pending && distance(at, m_start) > slack) {
                pushButton(window, SDL_BUTTON_LEFT, true, m_start);
                m_phase = Phase::Dragging;
            }
            if (m_phase == Phase::Dragging) {
                pushMove(window, at, true);
            } else if (m_phase == Phase::Pinching && m_fingers.size() >= 2 && m_pinchSpread > slack) {
                float now = spread();
                while (now >= m_pinchSpread * kZoomRatio) {
                    --m_zoomSteps; // apart: nearer
                    m_pinchSpread *= kZoomRatio;
                }
                while (now > slack && now * kZoomRatio <= m_pinchSpread) {
                    ++m_zoomSteps; // together: further
                    m_pinchSpread /= kZoomRatio;
                }
            }
            return true;
        }
        // the finger lifted (or the system took the touch away)
        const bool canceled = event.type == SDL_EVENT_FINGER_CANCELED;
        if (m_phase == Phase::Pending && !canceled) {
            const bool again = nowMs - m_lastTapMs <= kDoubleMs && distance(m_start, m_lastTap) <= kDoubleReach * static_cast<float>(height);
            pushButton(window, SDL_BUTTON_LEFT, true, m_start, again ? 2 : 1);
            pushButton(window, SDL_BUTTON_LEFT, false, m_start, again ? 2 : 1);
            m_lastTap = m_start;
            m_lastTapMs = again ? -1.0e9 : nowMs;
        } else if (m_phase == Phase::Dragging) {
            pushButton(window, SDL_BUTTON_LEFT, false, at);
        }
        m_fingers.erase(m_fingers.begin() + (finger - m_fingers.data()));
        m_phase = m_fingers.empty() ? Phase::Idle : Phase::Spent;
        return true;
    }

    void TouchPointer::update(SDL_Window* window, double nowMs) {
        if (m_enabled && m_phase == Phase::Pending && nowMs - m_startMs >= kHoldMs) {
            pushButton(window, SDL_BUTTON_RIGHT, true, m_start);
            pushButton(window, SDL_BUTTON_RIGHT, false, m_start);
            m_phase = Phase::Spent;
        }
    }

    int TouchPointer::takeZoomSteps() {
        const int steps = m_zoomSteps;
        m_zoomSteps = 0;
        return steps;
    }

}
