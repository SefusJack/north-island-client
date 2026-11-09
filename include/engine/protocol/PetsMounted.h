#ifndef NORTH_ISLAND_PROTOCOL_PETS_MOUNTED_H
#define NORTH_ISLAND_PROTOCOL_PETS_MOUNTED_H

#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

    /// @brief The pets of another player, one of which may be ridden the old way (FOnHorse).
    /// @details Original: server 15/7 (RecvPetMounted).
    struct PetsMounted {
        /// @brief One pet of the player.
        struct Entry {
            int place = 0;
            int npcId = 0;
            bool mounted = false;
            int tradable = 0;
            std::string name;
        };

        std::int32_t playerId = 0;
        std::vector<Entry> pets;
    };

}

#endif
