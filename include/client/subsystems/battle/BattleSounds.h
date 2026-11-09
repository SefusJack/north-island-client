#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_SOUNDS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_SOUNDS_H

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief The sounds a motion makes: its own, the weapon's and the body's, each played once when due.
    /// @details Original: CH_TBattleSound, ScheduleBattleSounds, PlayBattleSounds. A delay below 101 is a step of the
    /// motion, otherwise milliseconds. The game's character voices are not built.
    class BattleSounds {
        public:
            /// @brief Forgets all sounds.
            /// @details Original: ClearBattleSounds.
            void clear();

            /// @brief Makes all sounds due again from now.
            /// @details Original: RestartBattleSounds.
            /// @param nowMs Current time in milliseconds.
            void restart(std::uint64_t nowMs);

            /// @brief Adds a sound of the motion itself.
            /// @param file Sound file name.
            /// @param delay Motion step (1..100) or milliseconds.
            void addSound(const std::string& file, int delay);

            /// @brief Sets one of the weapon's sounds (A).
            /// @param index Slot of the sound.
            /// @param file Sound file name.
            /// @param delay Motion step (1..100) or milliseconds.
            void setWeaponSound(int index, const std::string& file, int delay);

            /// @brief Sets one of the body's sounds (B).
            /// @param index Slot of the sound.
            /// @param file Sound file name.
            /// @param delay Motion step (1..100) or milliseconds.
            void setBodySound(int index, const std::string& file, int delay);

            /// @brief Marks the sounds that have come due.
            /// @details Original: ScheduleBattleSounds.
            /// @param motionPoint Current step of the motion.
            /// @param nowMs Current time in milliseconds.
            void schedule(int motionPoint, std::uint64_t nowMs);

            /// @brief Plays the due sounds once.
            /// @details Original: PlayBattleSounds.
            /// @param playSound Callback that plays a sound file.
            void play(const std::function<void(const std::string&)>& playSound);

        private:
            /// @brief One scheduled sound and its state.
            struct Sound {
                std::string file;
                int delay = 0;
                std::uint64_t timeMs = 0;
                int state = 0;
            };

            /// @brief Marks one sound due when its motion step or time has come.
            /// @param sound The sound.
            /// @param motionPoint Current step of the motion.
            /// @param nowMs Current time in milliseconds.
            static void scheduleOne(Sound& sound, int motionPoint, std::uint64_t nowMs);

            std::vector<Sound> m_sounds;
            std::array<Sound, 6> m_weapon{};
            std::array<Sound, 6> m_body{};
    };

}

#endif
