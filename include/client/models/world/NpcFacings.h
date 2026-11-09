#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_NPC_FACINGS_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_NPC_FACINGS_H

#include <array>
#include <vector>

namespace client {

    /// @brief The way the NPCs an event turned to the player (and held) stood before, ten at most.
    /// @details Original: FSavedNpcFacing of the event manager. An NPC with a place here is not saved again until the place is cleared.
    class NpcFacings {
        public:
            static constexpr int kSlots = 10; // saved facing places

            /// @brief One saved facing.
            struct Entry {
                int npcIndex = 0;
                int direction = 0;
            };

            /// @brief Saves an NPC's facing.
            /// @details Original: CanSaveNpcFacing and the save.
            /// @param npcIndex The NPC's index in the map list.
            /// @param direction The direction it faces.
            /// @return False (nothing saved) when the NPC has a place already or none is free.
            bool save(int npcIndex, int direction);

            /// @brief Clears an NPC's saved place.
            /// @details Original: RestoreNpcFacing.
            /// @param npcIndex The NPC's index in the map list.
            /// @return False when it had none.
            bool restore(int npcIndex);

            /// @brief Gives the NPCs to turn back when an event ends.
            /// @details Original: RestoreAllNpcFacings, run by TInterForm.HideCtrl when the server ends an event (20/8), and ClearEventBytes
            /// after it; the places are cleared only when the first one is in use.
            /// @return The saved facings in use.
            std::vector<Entry> restoreAll();

        private:
            std::array<Entry, kSlots> m_slots{};
    };

}

#endif
