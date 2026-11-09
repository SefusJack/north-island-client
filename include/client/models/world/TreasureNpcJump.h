#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_TREASURE_NPC_JUMP_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_TREASURE_NPC_JUMP_H

#include <cstdint>
#include <optional>

namespace client {

    /// @brief The jump of a treasure NPC: one step for each frame of the game, five up by ten pixels, then down by ten more each step until back on the ground.
    /// @details Original: StartTreasureNpcJump, UpdateTreasureNpcJump, server 20/46. Only one runs at a time.
    class TreasureNpcJump {
        public:
            /// @brief One step of the jump.
            struct Step {
                int npcIndex = 0;
                int offsetY = 0;
                bool landed = false;
            };

            /// @brief Starts a jump, ignored while a jump runs.
            /// @details Original: StartTreasureNpcJump.
            /// @param npcIndex The NPC's index in the map list.
            /// @param nowMs Current time in ms.
            void start(int npcIndex, std::uint64_t nowMs);

            /// @brief Tells whether a jump runs.
            /// @return True when active.
            bool isActive() const { return m_active; }

            /// @brief Advances the jump.
            /// @details Original: UpdateTreasureNpcJump.
            /// @param nowMs Current time in ms.
            /// @return The next step when one is due.
            std::optional<Step> update(std::uint64_t nowMs);

            /// @brief Ends the jump because the NPC is gone (the server is told like after a landing).
            void cancel() { m_active = false; }

        private:
            bool m_active = false;
            int m_npcIndex = 0;
            int m_step = 0;
            int m_offsetY = 0;
            std::uint64_t m_stepMs = 0;
    };

}

#endif
