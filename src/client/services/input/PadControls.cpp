#include "client/services/input/PadControls.h"

#include "core/Log.h"
#include "ui/ImageButton.h"
#include "ui/LayoutError.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr SDL_MouseID kPadMouse = 0x50414431; // marks the mouse events made here ("PAD1"), so they do not end controller mode
        constexpr SDL_KeyboardID kPadKeyboard = 0x50414432; // the same for the key events ("PAD2")
        constexpr float kPointerSpeed = 0.9f; // pointer speed at full tilt, in window heights a second
        constexpr float kPointerBoost = 1.8f; // times as fast once the stick has been at full tilt for a while
        constexpr double kBoostAfterMs = 400.0; // how long at full tilt before the boost has grown in
        constexpr float kFullTilt = 0.95f; // a stick this far out counts as pushed all the way
        constexpr float kWalkTilt = 0.5f; // how far the left stick is pushed before the character walks
        constexpr float kOwnMoveSlack = 2.0f; // a mouse event this near the place the pointer was put is that move's, px
        constexpr double kOwnMoveMs = 500.0; // and no later than this after it
        constexpr float kMouseWake = 4.0f; // the mouse must move this far in one event to end controller mode, px
        constexpr double kPointerHideMs = 3000.0; // the pointer hides this long after its last use in controller mode
        constexpr double kLongestFrameMs = 100.0; // a longer gap between frames moves the pointer no further than this

        /// @brief Puts a mouse button press or release at the pointer onto SDL's event queue.
        /// @param window The game's window.
        /// @param button SDL_BUTTON_LEFT or SDL_BUTTON_RIGHT.
        /// @param down True for a press.
        /// @param clicks Which click in a row it is (2: a double click).
        /// @param at Where in the window, for a place the pointer was only just put at; null for where the pointer is.
        void pushMouseButton(SDL_Window* window, Uint8 button, bool down, int clicks = 1, const SDL_FPoint* at = nullptr) {
            SDL_Event event{};
            event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
            event.button.windowID = SDL_GetWindowID(window);
            event.button.which = kPadMouse;
            event.button.button = button;
            event.button.down = down;
            event.button.clicks = static_cast<Uint8>(clicks);
            SDL_GetMouseState(&event.button.x, &event.button.y);
            if (at != nullptr) {
                event.button.x = at->x;
                event.button.y = at->y;
            }
            SDL_PushEvent(&event);
        }

        constexpr double kDoubleMs = 500.0; // two presses of the click button within this time make a double click
        constexpr float kDoubleReach = 4.0f; // and with the pointer no further apart than this, px

        /// @brief Puts a key press and release onto SDL's event queue.
        /// @param window The game's window.
        /// @param key The key.
        /// @param ctrl True with Ctrl held.
        void pushKey(SDL_Window* window, SDL_Keycode key, bool ctrl) {
            for (const bool down : {true, false}) {
                SDL_Event event{};
                event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
                event.key.windowID = SDL_GetWindowID(window);
                event.key.which = kPadKeyboard;
                event.key.key = key;
                event.key.mod = ctrl ? SDL_KMOD_LCTRL : SDL_KMOD_NONE;
                event.key.down = down;
                SDL_PushEvent(&event);
            }
        }

        /// @brief Puts a mouse wheel turn onto SDL's event queue.
        /// @param window The game's window.
        /// @param steps Wheel notches, up positive.
        void pushWheel(SDL_Window* window, int steps) {
            SDL_Event event{};
            event.type = SDL_EVENT_MOUSE_WHEEL;
            event.wheel.windowID = SDL_GetWindowID(window);
            event.wheel.which = kPadMouse;
            event.wheel.y = static_cast<float>(steps);
            event.wheel.integer_y = steps;
            SDL_GetMouseState(&event.wheel.mouse_x, &event.wheel.mouse_y);
            SDL_PushEvent(&event);
        }

        /// @brief Clicks a button of the screen by its name, when the screen has it and it is shown.
        /// @param screen The screen.
        /// @param name The button's name in its layout.
        void clickButton(ui::Screen& screen, const char* name) {
            if (!screen.isShown(name)) {
                return;
            }
            try {
                screen.get<ui::ImageButton>(name).click(screen.context());
            } catch (const ui::LayoutError&) {
                // the element is no button on this screen: nothing to click
            }
        }

    }

    PadControls::~PadControls() { showPointer(true); }

    void PadControls::showPointer(bool shown) {
        if (shown == m_pointerShown) {
            return;
        }
        m_pointerShown = shown;
        if (shown) {
            SDL_ShowCursor();
        } else {
            SDL_HideCursor();
        }
    }

    void PadControls::setControllerMode(bool on, ui::Screen& screen) {
        if (on == m_controllerMode) {
            return;
        }
        m_controllerMode = on;
        core::logInfo(on ? "controller: in use" : "controller: mouse and keyboard in use");
        if (on) {
            SDL_GetMouseState(&m_pointerX, &m_pointerY);
            m_hudUp = true; // the HUD stays for its usual time, then hides
            m_menuMs = m_lastMs;
            return;
        }
        showPointer(true);
        for (Warp& warp : m_warps) {
            warp = {}; // no move of the pointer is waited for any more
        }
        m_menu.close(screen);
        constexpr ui::Key kArrows[4] = {ui::Key::Left, ui::Key::Right, ui::Key::Up, ui::Key::Down};
        for (int i = 0; i < 4; ++i) { // 4: the arrow keys
            if (m_walking[i]) {
                screen.holdKey(kArrows[i], false);
                m_walking[i] = false;
            }
        }
    }

    void PadControls::seeEvent(SDL_Event& event, ui::Screen& screen) {
        if (!m_controllerMode) {
            return;
        }
        switch (event.type) {
            case SDL_EVENT_MOUSE_MOTION: { // the pointer moved: by the stick here, or by the player's mouse
                if (isOwnMove(event.motion.x, event.motion.y)) {
                    if (m_leftHeld) {
                        event.motion.state |= SDL_BUTTON_LMASK; // the click button is held: the move drags
                    }
                } else if (std::abs(event.motion.xrel) + std::abs(event.motion.yrel) >= kMouseWake) {
                    setControllerMode(false, screen);
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN: // a mouse button: the player's unless made here
                if (event.button.which != kPadMouse) {
                    setControllerMode(false, screen);
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL: // the wheel: the player's unless made here
                if (event.wheel.which != kPadMouse) {
                    setControllerMode(false, screen);
                }
                break;
            case SDL_EVENT_KEY_DOWN: // a key: the player's unless made here
                if (event.key.which != kPadKeyboard) {
                    setControllerMode(false, screen);
                }
                break;
            default: // nothing else tells which device is in use
                break;
        }
    }

    void PadControls::warp(SDL_Window* window, float x, float y) {
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window, &width, &height);
        m_pointerX = std::clamp(x, 0.0f, static_cast<float>(std::max(0, width - 1)));
        m_pointerY = std::clamp(y, 0.0f, static_cast<float>(std::max(0, height - 1)));
        const float wholeX = std::floor(m_pointerX);
        const float wholeY = std::floor(m_pointerY);
        if (wholeX != m_warps[m_lastWarp].x || wholeY != m_warps[m_lastWarp].y) {
            m_lastWarp = (m_lastWarp + 1) % kWarpsKept;
            m_warps[m_lastWarp] = {wholeX, wholeY, m_lastMs};
            SDL_WarpMouseInWindow(window, wholeX, wholeY);
        }
    }

    core::Point PadControls::pointerOnScreen(SDL_Window* window) const {
        float x = m_pointerX;
        float y = m_pointerY;
        if (SDL_Renderer* renderer = SDL_GetRenderer(window)) {
            SDL_RenderCoordinatesFromWindow(renderer, x, y, &x, &y);
        }
        return {static_cast<int>(x) - m_origin.x, static_cast<int>(y) - m_origin.y};
    }

    void PadControls::warpToScreen(SDL_Window* window, core::Point point, core::Point origin) {
        float x = static_cast<float>(point.x + origin.x);
        float y = static_cast<float>(point.y + origin.y);
        if (SDL_Renderer* renderer = SDL_GetRenderer(window)) {
            SDL_RenderCoordinatesToWindow(renderer, x, y, &x, &y);
        }
        warp(window, x, y);
    }

    void PadControls::focusStep(int way, SDL_Window* window, ui::Screen& screen, core::Point origin) {
        const std::vector<ui::PadSpot> spots = m_focusWindow != 0 ? screen.padSpots() : m_sceneSpots;
        if (spots.empty()) {
            return;
        }
        float hereX = m_pointerX;
        float hereY = m_pointerY;
        if (SDL_Renderer* renderer = SDL_GetRenderer(window)) {
            SDL_RenderCoordinatesFromWindow(renderer, hereX, hereY, &hereX, &hereY);
        }
        const core::Point here{static_cast<int>(hereX) - origin.x, static_cast<int>(hereY) - origin.y};
        constexpr int kOnSpot = 6; // the pointer stands on a place when it is this near it, px
        const ui::PadSpot* current = nullptr;
        for (const ui::PadSpot& spot : spots) {
            // the pointer is on a slider anywhere along it
            if (std::abs(spot.point.x - here.x) <= (spot.slider ? spot.reach : kOnSpot) && std::abs(spot.point.y - here.y) <= kOnSpot) {
                current = &spot;
                break;
            }
        }
        if (current != nullptr && current->slider && (way == 1 || way == 2)) {
            // left and right move along the slider and click there, as the mouse sets it
            constexpr int kParts = 10; // steps from one end to the other
            constexpr double kArriveMs = 40.0; // the click follows the pointer
            const int step = std::max(1, current->reach * 2 / kParts);
            const int x = std::clamp(here.x + (way == 1 ? -step : step), current->point.x - current->reach + 1, current->point.x + current->reach - 1);
            warpToScreen(window, {x, current->point.y}, origin);
            m_sceneClick = true;
            m_sceneClickRight = false;
            m_sceneClickMs = m_lastMs + kArriveMs;
            return;
        }
        if (current == nullptr) {
            // what scrolled under the pointer need not stop with a control exactly under it: the nearest one close by counts
            constexpr int kNearby = 16; // px
            int nearest = kNearby + 1;
            for (const ui::PadSpot& spot : spots) {
                const int away = std::max(std::abs(spot.point.x - here.x), std::abs(spot.point.y - here.y));
                if (away < nearest) {
                    nearest = away;
                    current = &spot;
                }
            }
        }
        if (current != nullptr && ((way == 3 && current->scrollsUp) || (way == 4 && current->scrollsDown))) {
            // the list goes on: it scrolls and the pointer stays
            if (current->scrollBarId != 0) {
                screen.padScroll(current->scrollBarId, way == 3);
            } else {
                pushWheel(window, way == 3 ? 1 : -1);
            }
            return;
        }
        const int towardX = way == 1 ? -1 : way == 2 ? 1 : 0;
        const int towardY = way == 3 ? -1 : way == 4 ? 1 : 0;
        constexpr int kLeast = 4; // a place counts as lying that way from this many px on
        constexpr int kSideCost = 2; // a px to the side counts as this many px ahead
        const ui::PadSpot* best = nullptr;
        int bestCost = 0;
        for (const ui::PadSpot& spot : spots) {
            // up and down, a wide place (a heading) lies straight on from all that is under or over any part of it
            const int width = towardY != 0 ? spot.reach + (current != nullptr ? current->reach : 0) : 0;
            const int across = spot.point.x - here.x;
            const int x = across > 0 ? std::max(0, across - width) : std::min(0, across + width);
            const int y = spot.point.y - here.y;
            const int ahead = x * towardX + y * towardY;
            const int aside = std::abs(x * towardY) + std::abs(y * towardX);
            // twice as far aside as ahead is not that way; among places that stand as they like, further aside than ahead is not (a
            // step to the side stays in the row and does not go to the row above or below)
            if (&spot == current || ahead < kLeast || aside > ahead * (spot.loose ? 1 : 2)) {
                continue;
            }
            // among places that stand as they like (a game's holes) the nearest one that way is the next; else the one most straight on
            const int cost = spot.loose ? (ahead * ahead + aside * aside) / std::max(1, ahead + aside) : ahead + aside * kSideCost;
            if (best == nullptr || cost < bestCost) {
                best = &spot;
                bestCost = cost;
            }
        }
        if (best != nullptr) {
            warpToScreen(window, best->point, origin);
        } else if (m_focusWindow != 0 && towardY != 0) {
            pushWheel(window, -towardY); // nothing further that way: what is under the pointer scrolls on, if it scrolls
        }
    }

    bool PadControls::isOwnMove(float x, float y) const {
        for (const Warp& warp : m_warps) {
            if (m_lastMs - warp.atMs <= kOwnMoveMs && std::abs(x - warp.x) <= kOwnMoveSlack && std::abs(y - warp.y) <= kOwnMoveSlack) {
                return true;
            }
        }
        return false;
    }

    void PadControls::movePointer(const Gamepad& pad, SDL_Window* window, float seconds, double nowMs) {
        PadStick stick = pad.rightStick();
        if ((m_carry || m_dragging) && stick.x == 0.0f && stick.y == 0.0f) {
            stick = pad.leftStick(); // a thing is carried: the left stick moves the pointer too
        }
        const float tilt = std::sqrt(stick.x * stick.x + stick.y * stick.y);
        if (tilt <= 0.0f) {
            m_fullTiltMs = 0.0;
            return;
        }
        if (tilt < kFullTilt) {
            m_fullTiltMs = 0.0;
        } else if (m_fullTiltMs == 0.0) {
            m_fullTiltMs = nowMs;
        }
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window, &width, &height);
        const float boost =
            m_fullTiltMs == 0.0 ? 1.0f : 1.0f + (kPointerBoost - 1.0f) * static_cast<float>(std::min(1.0, (nowMs - m_fullTiltMs) / kBoostAfterMs));
        // the speed grows with the square of the tilt: small pushes place the pointer exactly
        const float speed = kPointerSpeed * static_cast<float>(height) * tilt * boost * seconds;
        m_pointerUsedMs = nowMs;
        warp(window, m_pointerX + stick.x * speed, m_pointerY + stick.y * speed);
    }

    void PadControls::walk(const Gamepad& pad, ui::Screen& screen) {
        const PadStick stick = pad.leftStick();
        bool wanted[4] = {pad.isHeld(PadButton::Left), pad.isHeld(PadButton::Right), pad.isHeld(PadButton::Up), pad.isHeld(PadButton::Down)};
        if (m_shortcuts) {
            for (bool& way : wanted) {
                way = false; // the D-pad uses the shortcut bar; the stick still walks
            }
        }
        if (m_menu.isOpen() || m_steps || m_focus) {
            for (bool& way : wanted) {
                way = false; // the D-pad and the stick move the menu's highlight, or step through a list of choices or a window
            }
        } else if (std::sqrt(stick.x * stick.x + stick.y * stick.y) >= kWalkTilt) {
            // eight directions: an axis joins in when the stick leans its way by more than tan(22.5 degrees) of the other
            constexpr float kLean = 0.4142f;
            const float acrossFromUpright = std::abs(stick.x) / std::max(std::abs(stick.y), 0.0001f);
            const bool sideways = acrossFromUpright > kLean;
            const bool upright = acrossFromUpright < 1.0f / kLean;
            wanted[0] = wanted[0] || (sideways && stick.x < 0.0f);
            wanted[1] = wanted[1] || (sideways && stick.x > 0.0f);
            wanted[2] = wanted[2] || (upright && stick.y < 0.0f);
            wanted[3] = wanted[3] || (upright && stick.y > 0.0f);
        }
        constexpr ui::Key kArrows[4] = {ui::Key::Left, ui::Key::Right, ui::Key::Up, ui::Key::Down};
        for (int i = 0; i < 4; ++i) { // 4: the arrow keys
            if (wanted[i] != m_walking[i]) {
                m_walking[i] = wanted[i];
                screen.holdKey(kArrows[i], wanted[i]);
            }
        }
    }

    void PadControls::act(const PadPress& press, SDL_Window* window, Scene& scene, double nowMs) {
        ui::Screen& screen = scene.screen();
        if (m_menu.isOpen() && press.down) {
            switch (press.button) {
                case PadButton::South: // press the highlighted button
                    m_menu.choose(screen);
                    m_menuMs = nowMs;
                    return;
                case PadButton::East: // back out: the HUD stays for its usual time
                    m_menu.close(screen);
                    m_menuMs = nowMs;
                    return;
                case PadButton::Start: // back out and send the HUD away
                    m_menu.close(screen);
                    m_hudUp = false;
                    return;
                case PadButton::North: // the bag and the quest notebook close the menu and open as always
                case PadButton::Back:
                    m_menu.close(screen);
                    break;
                default: // the D-pad moves the highlight (PadMenu::update); the other buttons do nothing in the menu
                    return;
            }
        }
        if (m_shortcuts && press.down) {
            int place = 0; // the shortcut bar's place, 1 to 8 (F1 to F8)
            switch (press.button) {
                case PadButton::Up: place = 1; break; // F1
                case PadButton::Right: place = 2; break; // F2
                case PadButton::Down: place = 3; break; // F3
                case PadButton::Left: place = 4; break; // F4
                case PadButton::South: place = 5; break; // F5
                case PadButton::East: place = 6; break; // F6
                case PadButton::West: place = 7; break; // F7
                case PadButton::North: place = 8; break; // F8
                default: break; // the other buttons do what they always do
            }
            if (place != 0) {
                pushKey(window, SDLK_F1 + static_cast<SDL_Keycode>(place - 1), false);
                return;
            }
        }
        if (press.down) {
            // what the scene takes itself: a talk box's answers, and without the pointer the NPC in front
            switch (press.button) {
                case PadButton::South:
                    if (scene.padAsk(PadAsk::Accept) || (!m_pointerShown && scene.padAsk(PadAsk::Interact))) {
                        return;
                    }
                    break;
                case PadButton::East:
                    if (scene.padAsk(PadAsk::Cancel)) {
                        return;
                    }
                    break;
                case PadButton::West:
                    if (scene.padAsk(PadAsk::Erase)) {
                        return;
                    }
                    break;
                case PadButton::North:
                    if (scene.padAsk(PadAsk::Space)) {
                        return;
                    }
                    break;
                case PadButton::Up:
                case PadButton::Down:
                case PadButton::Left:
                case PadButton::Right: {
                    const PadAsk ask = press.button == PadButton::Up     ? PadAsk::Up
                                       : press.button == PadButton::Down ? PadAsk::Down
                                       : press.button == PadButton::Left ? PadAsk::Left
                                                                         : PadAsk::Right;
                    if (scene.padAsk(ask)) {
                        return;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        switch (press.button) {
            case PadButton::South: { // left click at the pointer; held, the pointer drags; twice quickly in one place, a double click
                float x = 0.0f;
                float y = 0.0f;
                SDL_GetMouseState(&x, &y);
                const bool again = nowMs - m_clickMs <= kDoubleMs && std::abs(x - m_clickX) <= kDoubleReach && std::abs(y - m_clickY) <= kDoubleReach;
                if (m_carry) {
                    if (!press.down) {
                        break; // letting the button go does not put the thing down
                    }
                    // the second press puts the thing down where the pointer is
                    pushMouseButton(window, SDL_BUTTON_LEFT, false, m_clicks);
                    m_carry = false;
                    m_leftHeld = false;
                    m_pointerUsedMs = nowMs;
                    if (again) {
                        // twice quickly on one thing: it was not carried anywhere, and this is a double click, which uses it
                        m_clicks = 2;
                        m_leftHeld = true;
                        pushMouseButton(window, SDL_BUTTON_LEFT, true, m_clicks);
                    }
                    break;
                }
                if (m_dragging && !m_carry && !m_leftHeld) {
                    // a thing follows the pointer without a button held (half a stack taken up): a click puts it down
                    if (press.down) {
                        pushMouseButton(window, SDL_BUTTON_LEFT, true);
                        pushMouseButton(window, SDL_BUTTON_LEFT, false);
                        m_pointerUsedMs = nowMs;
                    }
                    break;
                }
                if (press.down != m_leftHeld) {
                    m_leftHeld = press.down;
                    m_pointerUsedMs = nowMs;
                    if (press.down) {
                        m_clicks = again ? m_clicks + 1 : 1;
                        m_clickMs = nowMs;
                        m_clickX = x;
                        m_clickY = y;
                    }
                    pushMouseButton(window, SDL_BUTTON_LEFT, press.down, m_clicks);
                    if (press.down && m_focus && m_clicks == 1) {
                        // on a thing that can be dragged the press picks it up: it follows the pointer until the next press
                        const auto cell = screen.padGridAt(pointerOnScreen(window));
                        m_carry = cell && cell->filled && cell->draggable;
                    }
                }
                break;
            }
            case PadButton::West: // on a thing in a grid: use it at once (a double click); else a right click at the pointer
                if (press.down && m_dragging) {
                    // a thing is carried: the right button puts one of it down (the modern way of moving things)
                    pushMouseButton(window, SDL_BUTTON_RIGHT, true);
                    pushMouseButton(window, SDL_BUTTON_RIGHT, false);
                    m_pointerUsedMs = nowMs;
                    break;
                }
                if (press.down && m_focus && !m_carry && !m_leftHeld) {
                    if (const auto cell = screen.padGridAt(pointerOnScreen(window)); cell && cell->filled && cell->doubleClick) {
                        for (const int click : {1, 2}) { // 2: the clicks of a double click
                            pushMouseButton(window, SDL_BUTTON_LEFT, true, click);
                            pushMouseButton(window, SDL_BUTTON_LEFT, false, click);
                        }
                        m_clickMs = nowMs - kDoubleMs * 2; // the click button's next press starts a click of its own
                        m_pointerUsedMs = nowMs;
                        break;
                    }
                }
                if (press.down != m_rightHeld) {
                    m_rightHeld = press.down;
                    m_pointerUsedMs = nowMs;
                    pushMouseButton(window, SDL_BUTTON_RIGHT, press.down);
                }
                break;
            case PadButton::East: // cancel: a thing picked up goes back where it was; else Escape closes the top window
                if (press.down && m_carry) {
                    const SDL_FPoint from{m_clickX, m_clickY};
                    warp(window, from.x, from.y);
                    pushMouseButton(window, SDL_BUTTON_LEFT, false, m_clicks, &from);
                    m_carry = false;
                    m_leftHeld = false;
                    break;
                }
                if (press.down && m_dragging) {
                    screen.cancelDrag(); // what follows the pointer goes back
                    break;
                }
                if (press.down && !screen.pressPadCancel()) { // a window Escape does not close is left by its own button
                    pushKey(window, SDLK_ESCAPE, false);
                }
                break;
            case PadButton::North: // on a thing in a grid: a right click (what the game offers for it); else the bag (Ctrl+I)
                if (press.down && m_dragging) {
                    pushMouseButton(window, SDL_BUTTON_RIGHT, true); // as X: one of what is carried is put down
                    pushMouseButton(window, SDL_BUTTON_RIGHT, false);
                    m_pointerUsedMs = nowMs;
                    break;
                }
                if (press.down && m_focus && !m_carry) {
                    if (const auto cell = screen.padGridAt(pointerOnScreen(window)); cell && cell->filled) {
                        pushMouseButton(window, SDL_BUTTON_RIGHT, true);
                        pushMouseButton(window, SDL_BUTTON_RIGHT, false);
                        m_pointerUsedMs = nowMs;
                        break;
                    }
                }
                if (press.down) {
                    pushKey(window, SDLK_I, true);
                }
                break;
            case PadButton::Back: // the quest notebook (Ctrl+M)
                if (press.down) {
                    pushKey(window, SDLK_M, true);
                }
                break;
            case PadButton::LeftShoulder: // with a window open the wheel up, else the shortcut bar's page before
            case PadButton::RightShoulder: { // the wheel down, else the page after
                if (!press.down) {
                    break;
                }
                const bool before = press.button == PadButton::LeftShoulder;
                if (screen.pressPadShoulder(!before)) {
                    m_focusWindow = -1; // the window's tab before or after, or its own two buttons: the pointer starts anew on what shows now
                    break;
                }
                if (!screen.shownWindows().empty()) {
                    pushWheel(window, before ? 1 : -1);
                } else {
                    clickButton(screen, before ? "HotKeyPagePrevious" : "HotKeyPageNext");
                }
                break;
            }
            case PadButton::RightStick: // the pointer back to the middle of the window, where the character stands
                if (press.down) {
                    int width = 0;
                    int height = 0;
                    SDL_GetWindowSize(window, &width, &height);
                    m_pointerUsedMs = nowMs;
                    warp(window, static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
                }
                break;
            case PadButton::Start: // the HUD up with the menu over its bars; without bars on the screen, the HUD up or away
                if (press.down) {
                    if (m_menu.open(screen)) {
                        m_hudUp = true;
                    } else if (!scene.padAsk(PadAsk::Menu)) {
                        m_hudUp = !m_hudUp;
                    }
                    m_menuMs = nowMs;
                }
                break;
            default: // the D-pad walks (walk); Guide, the left stick's click and the triggers have no use yet
                break;
        }
    }

    void PadControls::update(double nowMs, Gamepad& pad, SDL_Window* window, Scene& scene, core::Point origin, int hudHideSeconds) {
        ui::Screen& screen = scene.screen();
        const float seconds = static_cast<float>(std::clamp(nowMs - m_lastMs, 0.0, kLongestFrameMs) / 1000.0);
        m_lastMs = nowMs;
        const std::vector<PadPress> presses = pad.takePresses();
        const PadStick left = pad.leftStick();
        const PadStick right = pad.rightStick();
        const bool padUsed = !presses.empty() || left.x != 0.0f || left.y != 0.0f || right.x != 0.0f || right.y != 0.0f;
        if (!m_controllerMode) {
            constexpr ui::Key kArrows[4] = {ui::Key::Left, ui::Key::Right, ui::Key::Up, ui::Key::Down};
            for (int i = 0; i < 4; ++i) { // 4: the arrow keys; the mouse or keyboard took over while they were held
                if (m_walking[i]) {
                    screen.holdKey(kArrows[i], false);
                    m_walking[i] = false;
                }
            }
            if (!padUsed || !pad.isConnected()) {
                scene.setPadAim(false, {});
                scene.setPadInUse(false);
                return;
            }
            m_focusWindow = screen.padWindowId(); // a window open already keeps the pointer where the mouse left it
            setControllerMode(true, screen);
        }
        if (!pad.isConnected()) {
            setControllerMode(false, screen);
            scene.setPadAim(false, {});
            scene.setPadInUse(false);
            return;
        }
        m_steps = scene.wantsPadSteps();
        // the left stick steps through a list of choices as the D-pad does, again and again while held
        constexpr float kStepTilt = 0.6f; // how far the stick must be pushed to step
        constexpr double kFirstStepMs = 350.0; // held, the next step comes after this
        constexpr double kNextStepMs = 140.0; // and then every this
        const bool sideways = std::abs(left.x) > std::abs(left.y);
        const int stickStep = !m_steps                             ? 0
                              : !sideways && left.y <= -kStepTilt ? 1
                              : !sideways && left.y >= kStepTilt  ? 2
                              : sideways && left.x <= -kStepTilt  ? 3
                              : sideways && left.x >= kStepTilt   ? 4
                                                                  : 0;
        constexpr PadAsk kStepAsks[5] = {PadAsk::Up, PadAsk::Up, PadAsk::Down, PadAsk::Left, PadAsk::Right}; // by m_stickStep; the first is not used
        if (stickStep != m_stickStep) {
            m_stickStep = stickStep;
            m_stickStepMs = nowMs + kFirstStepMs;
            if (stickStep != 0) {
                scene.padAsk(kStepAsks[stickStep]);
            }
        } else if (stickStep != 0 && nowMs >= m_stickStepMs) {
            m_stickStepMs = nowMs + kNextStepMs;
            scene.padAsk(kStepAsks[stickStep]);
        }
        m_shortcuts = pad.isHeld(PadButton::LeftTrigger) && !m_menu.isOpen();
        // a window: the pointer starts on its first place and steps from place to place; without one it hides at once
        const int topWindow = m_menu.isOpen() ? 0 : screen.padWindowId();
        if (topWindow != m_focusWindow) {
            m_focusWindow = topWindow;
            if (topWindow == 0) {
                m_pointerUsedMs = nowMs - kPointerHideMs;
            } else if (const auto start = screen.padStartPoint()) {
                warpToScreen(window, *start, origin);
            }
        }
        // without a window the scene may have places of its own (a fight's targets): the pointer starts on the first and steps through them
        m_sceneSpots.clear();
        if (topWindow == 0 && !m_menu.isOpen()) {
            for (const core::Point& point : scene.padSpots()) {
                ui::PadSpot spot;
                spot.point = point;
                m_sceneSpots.push_back(spot);
            }
        }
        const bool sceneFocus = !m_sceneSpots.empty();
        if (sceneFocus != m_sceneFocus) {
            m_sceneFocus = sceneFocus;
            if (sceneFocus) {
                warpToScreen(window, m_sceneSpots.front().point, origin);
            } else if (topWindow == 0) {
                m_pointerUsedMs = nowMs - kPointerHideMs; // aimed or let go: the pointer hides at once
            }
        }
        m_focus = (topWindow != 0 || sceneFocus) && !m_steps;
        m_origin = origin;
        if (m_carry && !m_focus) {
            // the window went while a thing was carried: it is let go
            pushMouseButton(window, SDL_BUTTON_LEFT, false, m_clicks);
            m_carry = false;
            m_leftHeld = false;
        }
        int focusWay = 0;
        if (m_focus) {
            m_pointerUsedMs = nowMs; // the pointer stays while the window is open
            const PadStick steer = m_carry || m_dragging ? PadStick{} : left; // a thing carried: the stick moves the pointer freely, the D-pad still steps
            constexpr float kFocusTilt = 0.6f; // how far the stick must be pushed to step
            if (m_shortcuts) {
                focusWay = 0; // the D-pad uses the shortcut bar
            } else if (pad.isHeld(PadButton::Left) || (steer.x <= -kFocusTilt && std::abs(steer.x) >= std::abs(steer.y))) {
                focusWay = 1;
            } else if (pad.isHeld(PadButton::Right) || (steer.x >= kFocusTilt && std::abs(steer.x) >= std::abs(steer.y))) {
                focusWay = 2;
            } else if (pad.isHeld(PadButton::Up) || steer.y <= -kFocusTilt) {
                focusWay = 3;
            } else if (pad.isHeld(PadButton::Down) || steer.y >= kFocusTilt) {
                focusWay = 4;
            }
        }
        constexpr double kFocusFirstMs = 350.0; // held, the next step comes after this
        constexpr double kFocusNextMs = 140.0; // and then every this
        if (focusWay != m_focusWay) {
            m_focusWay = focusWay;
            m_focusStepMs = nowMs + kFocusFirstMs;
            if (focusWay != 0) {
                focusStep(focusWay, window, screen, origin);
            }
        } else if (focusWay != 0 && nowMs >= m_focusStepMs) {
            m_focusStepMs = nowMs + kFocusNextMs;
            focusStep(focusWay, window, screen, origin);
        }
        m_dragging = screen.isDragging();
        movePointer(pad, window, seconds, nowMs);
        if (m_sceneClick && nowMs >= m_sceneClickMs) {
            // the pointer has been where the scene wanted it for some frames: what lies under it knows by now
            m_sceneClick = false;
            const Uint8 button = m_sceneClickRight ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT;
            pushMouseButton(window, button, true);
            pushMouseButton(window, button, false);
        }
        for (const PadPress& press : presses) {
            act(press, window, scene, nowMs);
        }
        if (const auto click = scene.takePadClick()) {
            constexpr double kArriveMs = 90.0; // the scene finds what is under the pointer as it draws: some frames
            warpToScreen(window, click->point, origin);
            m_sceneClick = true;
            m_sceneClickRight = click->right;
            m_sceneClickMs = nowMs + kArriveMs;
        }
        walk(pad, screen);
        m_menu.update(nowMs, pad, screen);
        if (m_menu.isOpen() || !screen.shownWindows().empty()) {
            m_hudUp = true; // the menu or a window is open: the menus are in use
            m_menuMs = nowMs;
        } else if (m_hudUp && hudHideSeconds > 0 && nowMs - m_menuMs > hudHideSeconds * 1000.0) { // seconds to ms
            m_hudUp = false;
        }
        showPointer(nowMs - m_pointerUsedMs < kPointerHideMs);
        scene.setPadAim(m_controllerMode && !m_pointerShown && !m_menu.isOpen(), m_aimHand);
        scene.setPadInUse(m_controllerMode);
    }

}
