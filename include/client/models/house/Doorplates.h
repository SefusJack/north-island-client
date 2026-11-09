#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_DOORPLATES_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_DOORPLATES_H

#include "core/Bytes.h"

#include <cstdint>
#include <map>
#include <string>

namespace client {

    /// @brief The doorplates of the players: the house's name that the Doorplate (item 39056) sets, shown over a tent.
    /// @details Original: FDoorplate of the player and of the people of the world, string[14].
    class Doorplates {
        public:
            /// @brief Removes a player's doorplate (the player has none now).
            /// @details Original: server packet 62/45.
            /// @param playerId Player id.
            void clear(std::int32_t playerId);

            /// @brief Stores a list of doorplates.
            /// @details Original: server packet 62/46 (n x i32 player, u8 length, text), HandleUtensilPacket.
            /// @param payload Whole message.
            void receiveList(const core::Bytes& payload);

            /// @brief The text of a player's doorplate.
            /// @param playerId Player id.
            /// @return The text, empty for none.
            const std::string& text(std::int32_t playerId) const;

        private:
            std::map<std::int32_t, std::string> m_texts;
    };

}

#endif
