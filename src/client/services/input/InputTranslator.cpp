#include "client/services/input/InputTranslator.h"

namespace client {

    namespace {

        /// @brief Converts a window point to screen coordinates by subtracting the screen origin.
        /// @param x Window x.
        /// @param y Window y.
        /// @param origin Where the game's screen lies in the window.
        /// @return The point in screen coordinates.
        core::Point at(float x, float y, core::Point origin) { return {static_cast<int>(x) - origin.x, static_cast<int>(y) - origin.y}; }

        /// @brief Maps an SDL key code to the game's key enum.
        /// @param key SDL key code.
        /// @return The matching ui::Key, ui::Key::Other when the game does not use it.
        ui::Key translateKey(SDL_Keycode key) {
            switch (key) {
                case SDLK_RETURN: // Return key maps to Enter
                case SDLK_KP_ENTER: return ui::Key::Enter; // keypad Enter maps to Enter
                case SDLK_TAB: return ui::Key::Tab; // Tab key
                case SDLK_ESCAPE: return ui::Key::Escape; // Escape key
                case SDLK_BACKSPACE: return ui::Key::Backspace; // Backspace key
                case SDLK_DELETE: return ui::Key::Delete; // Delete key
                case SDLK_LEFT: return ui::Key::Left; // Left arrow
                case SDLK_RIGHT: return ui::Key::Right; // Right arrow
                case SDLK_UP: return ui::Key::Up; // Up arrow
                case SDLK_DOWN: return ui::Key::Down; // Down arrow
                case SDLK_HOME: return ui::Key::Home; // Home key
                case SDLK_END: return ui::Key::End; // End key
                case SDLK_INSERT: return ui::Key::Insert; // Insert key
                case SDLK_PAGEUP: return ui::Key::PageUp; // Page Up key
                case SDLK_PAGEDOWN: return ui::Key::PageDown; // Page Down key
                case SDLK_1: return ui::Key::Digit1; // digit 1
                case SDLK_2: return ui::Key::Digit2; // digit 2
                case SDLK_3: return ui::Key::Digit3; // digit 3
                case SDLK_4: return ui::Key::Digit4; // digit 4
                case SDLK_5: return ui::Key::Digit5; // digit 5
                case SDLK_6: return ui::Key::Digit6; // digit 6
                case SDLK_7: return ui::Key::Digit7; // digit 7
                case SDLK_8: return ui::Key::Digit8; // digit 8
                case SDLK_A: return ui::Key::A; // letter A
                case SDLK_D: return ui::Key::D; // letter D
                case SDLK_F: return ui::Key::F; // letter F
                case SDLK_H: return ui::Key::H; // letter H
                case SDLK_J: return ui::Key::J; // letter J
                case SDLK_E: return ui::Key::E; // letter E
                case SDLK_G: return ui::Key::G; // letter G
                case SDLK_I: return ui::Key::I; // letter I
                case SDLK_M: return ui::Key::M; // letter M
                case SDLK_P: return ui::Key::P; // letter P
                case SDLK_Q: return ui::Key::Q; // letter Q
                case SDLK_SPACE: return ui::Key::Space; // space bar
                case SDLK_S: return ui::Key::S; // letter S
                case SDLK_T: return ui::Key::T; // letter T
                case SDLK_V: return ui::Key::V; // letter V
                case SDLK_F1: return ui::Key::F1; // function key F1
                case SDLK_F2: return ui::Key::F2; // function key F2
                case SDLK_F3: return ui::Key::F3; // function key F3
                case SDLK_F4: return ui::Key::F4; // function key F4
                case SDLK_F5: return ui::Key::F5; // function key F5
                case SDLK_F6: return ui::Key::F6; // function key F6
                case SDLK_F7: return ui::Key::F7; // function key F7
                case SDLK_F8: return ui::Key::F8; // function key F8
                default: return ui::Key::Other; // any key the game does not use
            }
        }

        /// @brief Builds a screen key event from an SDL keyboard event (key plus Ctrl, Shift and Alt state).
        /// @param key SDL keyboard event.
        /// @return The key event.
        ui::KeyEvent keyEvent(const SDL_KeyboardEvent& key) {
            return {translateKey(key.key), (key.mod & SDL_KMOD_CTRL) != 0, (key.mod & SDL_KMOD_SHIFT) != 0, (key.mod & SDL_KMOD_ALT) != 0};
        }

        /// @brief Keeps only the ASCII characters of a UTF-8 string (the text fields take the game's single-byte characters).
        /// @param utf8 Zero-terminated UTF-8 text.
        /// @return The ASCII characters of the text.
        std::string asciiOnly(const char* utf8) {
            std::string text;
            for (const char* c = utf8; *c != '\0'; ++c) {
                if (static_cast<unsigned char>(*c) < 0x80) { // bytes from 0x80 up are not ASCII
                    text += *c;
                }
            }
            return text;
        }

    }

    bool forwardEvent(const SDL_Event& event, ui::Screen& screen, core::Point origin) {
        switch (event.type) {
            case SDL_EVENT_QUIT: // window close request: stop the program
                return false;
            case SDL_EVENT_MOUSE_MOTION: // mouse moved (flag: left button held)
                screen.mouseMove(at(event.motion.x, event.motion.y, origin), (event.motion.state & SDL_BUTTON_LMASK) != 0);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN: // left or right button pressed
            case SDL_EVENT_MOUSE_BUTTON_UP: { // left or right button released
                if (event.button.button != SDL_BUTTON_LEFT && event.button.button != SDL_BUTTON_RIGHT) {
                    break;
                }
                const ui::MouseButton button = event.button.button == SDL_BUTTON_LEFT ? ui::MouseButton::Left : ui::MouseButton::Right;
                const bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    screen.mouseDown(at(event.button.x, event.button.y, origin), button, event.button.clicks, ctrl);
                } else {
                    screen.mouseUp(at(event.button.x, event.button.y, origin), button, ctrl);
                }
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: // vertical wheel notches
                if (event.wheel.integer_y != 0) {
                    screen.mouseWheel(event.wheel.integer_y);
                }
                break;
            case SDL_EVENT_KEY_DOWN: // key pressed
                screen.keyDown(keyEvent(event.key));
                break;
            case SDL_EVENT_KEY_UP: // key released
                screen.keyUp(keyEvent(event.key));
                break;
            case SDL_EVENT_TEXT_INPUT: { // typed text (ASCII only is passed on)
                const std::string text = asciiOnly(event.text.text);
                if (!text.empty()) {
                    screen.textInput(text);
                }
                break;
            }
            default: // any other event is ignored
                break;
        }
        return true;
    }

}
