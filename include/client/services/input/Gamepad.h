#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_GAMEPAD_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_GAMEPAD_H

#include <SDL3/SDL.h>

#include <filesystem>
#include <vector>

namespace client {

    /// @brief A button of the pad by where it sits, the same on every make of pad; the triggers count as buttons.
    enum class PadButton {
        South, // A on an Xbox pad, the cross on a PlayStation pad
        East, // B, the circle
        West, // X, the square
        North, // Y, the triangle
        Back,
        Guide,
        Start,
        LeftStick,
        RightStick,
        LeftShoulder,
        RightShoulder,
        Up,
        Down,
        Left,
        Right,
        LeftTrigger,
        RightTrigger,
        Count
    };

    /// @brief A button going down or coming up.
    struct PadPress {
        PadButton button = PadButton::South;
        bool down = false;
    };

    /// @brief Where a stick is, each way from -1 to 1 (right and down positive), with the dead zone taken out.
    struct PadStick {
        float x = 0.0f;
        float y = 0.0f;
    };

    /// @brief The game controllers: starts SDL's gamepad subsystem, opens the pads and follows plugging in and out.
    /// @details The pad used last is the one read. Without the subsystem or without a pad the game runs as before.
    class Gamepad {
        public:
            /// @brief Starts SDL's gamepad subsystem and opens the pads connected; logs what it found.
            /// @details Button layouts SDL lacks come from the client's own list and from gamecontrollerdb.txt in the user folder.
            /// @param userRoot The user folder.
            /// @param enabled False to leave the pads alone: nothing is started.
            Gamepad(
                const std::filesystem::path& userRoot,
                bool enabled
            );

            ~Gamepad();

            Gamepad(const Gamepad&) = delete;
            Gamepad& operator=(const Gamepad&) = delete;

            /// @brief Whether a pad is open.
            /// @return True with a pad in use.
            bool isConnected() const { return m_active != nullptr; }

            /// @brief Takes the gamepad events out of the event loop: pads added and removed, buttons and sticks.
            /// @param event The SDL event.
            /// @return True when the event was a gamepad's or a joystick's (nothing else needs it).
            bool handleEvent(const SDL_Event& event);

            /// @brief Gives the buttons that went down or came up since the last call, in order.
            /// @return The presses and releases.
            std::vector<PadPress> takePresses();

            /// @brief Whether a button is down now.
            /// @param button The button.
            /// @return True while held.
            bool isHeld(PadButton button) const { return m_held[static_cast<int>(button)]; }

            /// @brief Gives where the left stick is.
            /// @return The stick, 0,0 inside its dead zone or without a pad.
            PadStick leftStick() const { return stick(SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY); }

            /// @brief Gives where the right stick is.
            /// @return The stick, 0,0 inside its dead zone or without a pad.
            PadStick rightStick() const { return stick(SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY); }

        private:
            /// @brief An open pad.
            struct Opened {
                SDL_JoystickID id = 0;
                SDL_Gamepad* pad = nullptr;
            };

            /// @brief Opens a pad and logs its name, kind and connection.
            /// @param id The pad's joystick id.
            void open(SDL_JoystickID id);

            /// @brief Closes a pad.
            /// @param id The pad's joystick id.
            void close(SDL_JoystickID id);

            /// @brief Makes the pad an event came from the one that is read; the buttons held on the other one come up.
            /// @param id The pad's joystick id.
            void use(SDL_JoystickID id);

            /// @brief Notes a button going down or up.
            /// @param button The button.
            /// @param down True when it went down.
            void press(PadButton button, bool down);

            /// @brief Reads a stick of the pad in use.
            /// @param xAxis Its left-right axis.
            /// @param yAxis Its up-down axis.
            /// @return The stick with the dead zone taken out.
            PadStick stick(SDL_GamepadAxis xAxis, SDL_GamepadAxis yAxis) const;

            bool m_started = false;
            std::vector<Opened> m_pads;
            SDL_Gamepad* m_active = nullptr;
            SDL_JoystickID m_activeId = 0;
            bool m_held[static_cast<int>(PadButton::Count)] = {};
            std::vector<PadPress> m_presses;
            std::vector<SDL_Joystick*> m_unknown; // joysticks without a button layout, kept open so their raw input reaches the log
            int m_rawZone[16] = {}; // where each raw joystick axis is: -1 low, 0 middle, 1 high (for the log)
    };

}

#endif
