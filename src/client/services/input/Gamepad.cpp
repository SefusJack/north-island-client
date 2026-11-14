#include "client/services/input/Gamepad.h"

#include "core/Clock.h"
#include "core/Log.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <system_error>

namespace client {

    namespace {

        constexpr const char* kLayoutFileName = "gamecontrollerdb.txt"; // extra button layouts in the user folder, in SDL's mapping format
        constexpr float kAxisRange = 32767.0f; // an axis's largest value
        constexpr float kStickDeadZone = 0.2f; // how far a stick moves before it counts, of its range
        constexpr int kTriggerDown = 16000; // a trigger counts as pressed past this
        constexpr int kTriggerUp = 9000; // and as let go below this

        // Button layouts for pads SDL's own list lacks, in SDL's mapping format.
        const char* const kOwnLayouts[] = {
            // 8BitDo Ultimate 3-mode controller for Xbox over Bluetooth (reports itself as "8BitDo 81HB")
            "03000000c82d00001b90000000000000,8BitDo Ultimate 3-mode (Bluetooth),a:b0,b:b1,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,"
            "dpup:h0.1,guide:b12,leftshoulder:b6,leftstick:b13,lefttrigger:b8,leftx:a0,lefty:a2,paddle1:b5,paddle2:b2,rightshoulder:b7,"
            "rightstick:b14,righttrigger:b9,rightx:a3,righty:a5,start:b11,x:b3,y:b4,platform:Windows,",
        };

        /// @brief Gives a text for a pad's connection.
        /// @param pad The pad.
        /// @return "wired", "wireless" or "unknown connection".
        const char* connectionOf(SDL_Gamepad* pad) {
            switch (SDL_GetGamepadConnectionState(pad)) {
                case SDL_JOYSTICK_CONNECTION_WIRED: return "wired"; // by cable
                case SDL_JOYSTICK_CONNECTION_WIRELESS: return "wireless"; // by radio
                default: return "unknown connection"; // the driver does not tell
            }
        }

        /// @brief Maps SDL's button to the game's.
        /// @param button SDL's button.
        /// @return The button, nothing for those the game does not use (paddles, touchpad, share).
        std::optional<PadButton> buttonOf(int button) {
            switch (button) {
                case SDL_GAMEPAD_BUTTON_SOUTH: return PadButton::South; // bottom face button
                case SDL_GAMEPAD_BUTTON_EAST: return PadButton::East; // right face button
                case SDL_GAMEPAD_BUTTON_WEST: return PadButton::West; // left face button
                case SDL_GAMEPAD_BUTTON_NORTH: return PadButton::North; // top face button
                case SDL_GAMEPAD_BUTTON_BACK: return PadButton::Back; // Back, View, Share, Minus
                case SDL_GAMEPAD_BUTTON_GUIDE: return PadButton::Guide; // Home
                case SDL_GAMEPAD_BUTTON_START: return PadButton::Start; // Start, Menu, Options, Plus
                case SDL_GAMEPAD_BUTTON_LEFT_STICK: return PadButton::LeftStick; // left stick pressed in
                case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return PadButton::RightStick; // right stick pressed in
                case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return PadButton::LeftShoulder; // left bumper
                case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return PadButton::RightShoulder; // right bumper
                case SDL_GAMEPAD_BUTTON_DPAD_UP: return PadButton::Up; // D-pad up
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return PadButton::Down; // D-pad down
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return PadButton::Left; // D-pad left
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return PadButton::Right; // D-pad right
                default: return std::nullopt; // a button the game has no use for
            }
        }

    }

    Gamepad::Gamepad(const std::filesystem::path& userRoot, bool enabled) {
        if (!enabled) {
            core::logInfo("controller: off in the settings");
            return;
        }
        const double startMs = core::nowPreciseMs();
        m_started = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
        if (!m_started) {
            core::logWarning("controller: SDL's gamepad subsystem did not start: ", SDL_GetError());
            return;
        }
        for (const char* layout : kOwnLayouts) {
            if (SDL_AddGamepadMapping(layout) < 0) {
                core::logWarning("controller: a built-in button layout was not taken: ", SDL_GetError());
            }
        }
        const std::filesystem::path layoutFile = userRoot / kLayoutFileName;
        std::error_code ignored;
        if (std::filesystem::exists(layoutFile, ignored)) {
            const int added = SDL_AddGamepadMappingsFromFile(layoutFile.string().c_str());
            if (added < 0) {
                core::logWarning("controller: ", kLayoutFileName, " was not read: ", SDL_GetError());
            } else {
                core::logInfo("controller: ", added, " button layouts from ", kLayoutFileName);
            }
        }
        int count = 0;
        SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);
        for (int i = 0; joysticks != nullptr && i < count; ++i) {
            if (SDL_IsGamepad(joysticks[i])) {
                open(joysticks[i]);
                continue;
            }
            const char* name = SDL_GetJoystickNameForID(joysticks[i]);
            char guid[33] = {}; // 32 hex digits and the end
            SDL_GUIDToString(SDL_GetJoystickGUIDForID(joysticks[i]), guid, sizeof(guid));
            SDL_Joystick* joystick = SDL_OpenJoystick(joysticks[i]);
            m_unknown.push_back(joystick);
            core::logInfo(
                "controller: ",
                name ? name : "a joystick",
                " has no button layout known to SDL, not used (guid ",
                guid,
                ", axes ",
                joystick ? SDL_GetNumJoystickAxes(joystick) : -1,
                " buttons ",
                joystick ? SDL_GetNumJoystickButtons(joystick) : -1,
                " hats ",
                joystick ? SDL_GetNumJoystickHats(joystick) : -1,
                "); its raw input is logged, for a layout in ",
                kLayoutFileName
            );
        }
        SDL_free(joysticks);
        core::logInfo("controller: ", m_pads.size(), " found, started in ", static_cast<int>(core::nowPreciseMs() - startMs), " ms");
    }

    Gamepad::~Gamepad() {
        for (const Opened& opened : m_pads) {
            SDL_CloseGamepad(opened.pad);
        }
        for (SDL_Joystick* joystick : m_unknown) {
            if (joystick != nullptr) {
                SDL_CloseJoystick(joystick);
            }
        }
        if (m_started) {
            SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        }
    }

    void Gamepad::open(SDL_JoystickID id) {
        for (const Opened& opened : m_pads) {
            if (opened.id == id) {
                return;
            }
        }
        SDL_Gamepad* pad = SDL_OpenGamepad(id);
        if (pad == nullptr) {
            core::logWarning("controller: cannot open the pad: ", SDL_GetError());
            return;
        }
        m_pads.push_back({id, pad});
        if (m_active == nullptr) {
            m_active = pad;
            m_activeId = id;
        }
        const char* name = SDL_GetGamepadName(pad);
        const char* kind = SDL_GetGamepadStringForType(SDL_GetGamepadType(pad));
        core::logInfo("controller: ", name ? name : "unnamed pad", " (", kind ? kind : "unknown kind", ", ", connectionOf(pad), ")");
    }

    void Gamepad::close(SDL_JoystickID id) {
        const auto found = std::find_if(m_pads.begin(), m_pads.end(), [id](const Opened& opened) { return opened.id == id; });
        if (found == m_pads.end()) {
            return;
        }
        core::logInfo("controller: a pad was disconnected");
        SDL_CloseGamepad(found->pad);
        m_pads.erase(found);
        if (m_activeId == id) {
            m_active = nullptr;
            m_activeId = 0;
            for (int button = 0; button < static_cast<int>(PadButton::Count); ++button) {
                press(static_cast<PadButton>(button), false);
            }
            if (!m_pads.empty()) {
                m_active = m_pads.front().pad;
                m_activeId = m_pads.front().id;
            }
        }
    }

    void Gamepad::use(SDL_JoystickID id) {
        if (id == m_activeId) {
            return;
        }
        for (const Opened& opened : m_pads) {
            if (opened.id == id) {
                for (int button = 0; button < static_cast<int>(PadButton::Count); ++button) {
                    press(static_cast<PadButton>(button), false);
                }
                m_active = opened.pad;
                m_activeId = id;
                return;
            }
        }
    }

    void Gamepad::press(PadButton button, bool down) {
        bool& held = m_held[static_cast<int>(button)];
        if (held != down) {
            held = down;
            m_presses.push_back({button, down});
        }
    }

    std::vector<PadPress> Gamepad::takePresses() {
        std::vector<PadPress> presses;
        presses.swap(m_presses);
        return presses;
    }

    PadStick Gamepad::stick(SDL_GamepadAxis xAxis, SDL_GamepadAxis yAxis) const {
        if (m_active == nullptr) {
            return {};
        }
        const float x = static_cast<float>(SDL_GetGamepadAxis(m_active, xAxis)) / kAxisRange;
        const float y = static_cast<float>(SDL_GetGamepadAxis(m_active, yAxis)) / kAxisRange;
        const float length = std::sqrt(x * x + y * y);
        if (length <= kStickDeadZone) {
            return {};
        }
        // past the dead zone the stick's travel starts at 0 again, so slow moves are possible
        const float scale = std::min(1.0f, (length - kStickDeadZone) / (1.0f - kStickDeadZone)) / length;
        return {x * scale, y * scale};
    }

    bool Gamepad::handleEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_GAMEPAD_ADDED: // a pad was plugged in (also sent for each pad there at the start)
                open(event.gdevice.which);
                return true;
            case SDL_EVENT_GAMEPAD_REMOVED: // a pad went away
                close(event.gdevice.which);
                return true;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN: // a button went down
            case SDL_EVENT_GAMEPAD_BUTTON_UP: { // a button came up
                const bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                if (down) {
                    use(event.gbutton.which);
                }
                if (const auto button = buttonOf(event.gbutton.button); button && event.gbutton.which == m_activeId) {
                    press(*button, down);
                }
                return true;
            }
            case SDL_EVENT_GAMEPAD_AXIS_MOTION: { // a stick or trigger moved
                const int value = event.gaxis.value;
                if (std::abs(value) > kTriggerDown) {
                    use(event.gaxis.which);
                }
                if (event.gaxis.which == m_activeId &&
                    (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)) {
                    const PadButton trigger = event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? PadButton::LeftTrigger : PadButton::RightTrigger;
                    if (value > kTriggerDown) {
                        press(trigger, true);
                    } else if (value < kTriggerUp) {
                        press(trigger, false);
                    }
                }
                return true;
            }
            case SDL_EVENT_JOYSTICK_BUTTON_DOWN: // a raw joystick button; logged for joysticks without a layout
                if (!SDL_IsGamepad(event.jbutton.which)) {
                    core::logInfo("controller: raw button ", static_cast<int>(event.jbutton.button));
                }
                return true;
            case SDL_EVENT_JOYSTICK_HAT_MOTION: // a raw joystick hat
                if (!SDL_IsGamepad(event.jhat.which) && event.jhat.value != 0) {
                    core::logInfo("controller: raw hat ", static_cast<int>(event.jhat.hat), " value ", static_cast<int>(event.jhat.value));
                }
                return true;
            case SDL_EVENT_JOYSTICK_AXIS_MOTION: { // a raw joystick axis
                constexpr int kHalfWay = 16384; // half of an axis's range
                const int axis = event.jaxis.axis;
                if (!SDL_IsGamepad(event.jaxis.which) && axis >= 0 && axis < 16) { // 16: the axes m_rawZone has room for
                    const int zone = event.jaxis.value > kHalfWay ? 1 : event.jaxis.value < -kHalfWay ? -1 : 0;
                    if (zone != m_rawZone[axis]) {
                        m_rawZone[axis] = zone;
                        core::logInfo("controller: raw axis ", axis, zone > 0 ? " high" : zone < 0 ? " low" : " middle");
                    }
                }
                return true;
            }
            default: // the other joystick and gamepad events (touchpad, sensors, battery) and everything that is not a pad's
                return event.type >= SDL_EVENT_JOYSTICK_AXIS_MOTION && event.type <= SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED;
        }
    }

}
