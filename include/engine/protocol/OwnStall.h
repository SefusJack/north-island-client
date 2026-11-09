#ifndef NORTH_ISLAND_PROTOCOL_OWN_STALL_H
#define NORTH_ISLAND_PROTOCOL_OWN_STALL_H

#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

    /// @brief The own stall as the server opened it: name, sign and the bag (or pet) places on sale with their prices.
    /// @details Original: server 23/30 (RecvOwnItemStall), 23/110 (RecvOwnPetStall).
    struct OwnStall {
        /// @brief One place on sale and its price.
        struct Price {
            int bagSlot = 0;
            std::int32_t price = 0;
        };

        std::string name;
        int sign = 0;
        std::vector<Price> prices;
    };

}

#endif
