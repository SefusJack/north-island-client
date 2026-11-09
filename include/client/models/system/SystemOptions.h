#ifndef NORTH_ISLAND_CLIENT_MODELS_SYSTEM_SYSTEM_OPTIONS_H
#define NORTH_ISLAND_CLIENT_MODELS_SYSTEM_SYSTEM_OPTIONS_H

#include "client/services/settings/SystemSettings.h"

#include <array>
#include <optional>
#include <utility>

namespace client {

    /// @brief What the system window changes (original TSe_SystemForm's rules, without the form).
    /// @details The switches the server keeps (PK, fighting, trade, party; 33/x), and the character's settings: music and sound on or off with
    /// volumes 0..10, the colours of the chat channels 1..10 and the camera switch.
    class SystemOptions {
        public:
            /// @brief The four switches the server keeps.
            enum class Switch { Pk, Fighting, Trade, Party };
            static constexpr int kMaxValue = 10; // highest step of a slider option (e.g. volume 0..10)

            SystemSettings settings;

            /// @brief Gets the state of a switch.
            /// @param which The switch.
            /// @return 1 on, 2 off.
            int state(Switch which) const { return m_switches[index(which)]; }

            /// @brief Tells whether a switch is on.
            /// @param which The switch.
            /// @return True when on.
            bool isOn(Switch which) const { return state(which) == 1; }

            /// @brief Flips a switch (original SystemToggleClick).
            /// @param which The switch.
            /// @return What the server hears: the 33/x sub id and the new state.
            std::pair<int, int> flip(Switch which);

            /// @brief Takes in the switch states the server sent; only 1 (on) and 2 (off) change a switch.
            /// @details Original: SetOptionButtonState, packet 33/2.
            /// @param switches States of PK, fighting, trade, party.
            void setServerStates(const std::array<int, 4>& switches); // four switches: Pk, Fighting, Trade, Party

            /// @brief Tells whether the player may ask for a PK (option 1; off keeps him from asking).
            /// @return True unless the PK switch is off.
            bool pkAllowed() const { return state(Switch::Pk) != 2; } // state 2: off

            /// @brief Tells whether teams may be formed (option 10, "Party Option").
            /// @return True when the party switch is on.
            bool partyAllowed() const { return state(Switch::Party) == 1; }

            /// @brief Steps the music volume down (-1) or up (+1), kept to 0..10 (original SystemArrowClick).
            /// @param step -1 or +1.
            /// @return True when the volume changed.
            bool stepMusic(int step) { return stepValue(settings.musicVolume, step, 0); }

            /// @brief Steps the sound volume down (-1) or up (+1), kept to 0..10 (original SystemArrowClick).
            /// @param step -1 or +1.
            /// @return True when the volume changed.
            bool stepSound(int step) { return stepValue(settings.soundVolume, step, 0); }

            /// @brief Steps the colour of a chat channel, kept to 1..10.
            /// @param channel Channel 0..4: local, whisper, team, guild, world.
            /// @param step -1 or +1.
            /// @return True when the colour changed.
            bool stepColor(int channel, int step);

            /// @brief Tells whether the camera zoom of a skill is on (BattleSwitch 0).
            /// @return True when on.
            bool cameraOn() const { return settings.battleSwitch == 0; }

            /// @brief Flips the camera switch.
            void flipCamera() { settings.battleSwitch = cameraOn() ? 1 : 0; }

            /// @brief Gives the loudness of a volume as a percentage of full loudness.
            /// @details Original: ApplySoundVolumes, -(10 - volume) * 300 hundredths of a decibel.
            /// @param volume Volume 0..10.
            /// @return The loudness in percent.
            static int loudnessPercent(int volume);

        private:
            /// @brief Gets the array index of a switch.
            /// @param which The switch.
            /// @return The index.
            static std::size_t index(Switch which) { return static_cast<std::size_t>(which); }

            /// @brief Moves a value by a step within lowest..kMaxValue.
            /// @param value The value.
            /// @param step The step.
            /// @param lowest The lowest allowed value.
            /// @return True when the value changed.
            static bool stepValue(int& value, int step, int lowest);

            // all four switches start on (1)
            std::array<int, 4> m_switches{1, 1, 1, 1};
    };

}

#endif
