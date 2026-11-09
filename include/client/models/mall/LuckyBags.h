#ifndef NORTH_ISLAND_CLIENT_MODELS_MALL_LUCKY_BAGS_H
#define NORTH_ISLAND_CLIENT_MODELS_MALL_LUCKY_BAGS_H

#include "client/data/GiftBoxContents.h"
#include "core/Bytes.h"
#include "formats/ItemTable.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief What the mall's Lucky Bags (item type 41) can hold, asked from the server and kept in a file.
    /// @details Original: CJ_Menu.pas TReadLuckyBag. The server's lists (91/2) are asked for when a bag is under the mouse and kept in Data\Lbd.dat for the next time.
    /// A bag read from the file is asked for once more (its row count goes with the request); the server answers with the whole list or only the bag's number when the copy is current.
    class LuckyBags {
        public:
            /// @brief The known contents of one Lucky Bag.
            struct Bag {
                int bagId = 0;
                int itemCount = 0;
                std::vector<GiftBoxContents::Entry> items;
                bool current = false;
            };

            /// @brief Creates the store.
            /// @param file File the lists are kept in.
            /// @param send Callback that sends a message to the server.
            LuckyBags(
                std::filesystem::path file,
                std::function<void(const core::Bytes&)> send
            );

            /// @brief Reads the file; nothing is asked for yet.
            /// @details Original: ResetLuckyBag, LoadLuckyBagData.
            void load();

            /// @brief Finds the known contents of a bag.
            /// @param bagId Item id of the bag.
            /// @return The bag, or nothing when unknown.
            std::optional<std::reference_wrapper<const Bag>> find(int bagId) const;

            /// @brief Tells whether a bag's list is known and current; if not, asks for it (not oftener than once a second).
            /// @details Original: RequestLuckyBag; the request is sent as 91/1 with the bag id and the row count of the copy.
            /// @param items Item table (the item must be of type 41).
            /// @param bagId Item id of the bag.
            /// @param nowMs Current time in ms.
            /// @return True when the list is known and current.
            bool request(const formats::ItemTable& items, int bagId, std::uint64_t nowMs);

            /// @brief Takes in the server's list of a bag.
            /// @details Original: RecvLuckyBagPacket, 91/2: u16 bag, u8 rows, then rows of u16 item and u8 count; the bag's number alone says the copy is current.
            /// @param payload The message.
            void received(const core::Bytes& payload);

        private:
            /// @brief Writes the known bags to the file.
            /// @details Original: SaveLuckyBagData.
            void save() const;

            std::filesystem::path m_file;
            std::function<void(const core::Bytes&)> m_send;
            std::vector<Bag> m_bags;
            std::optional<std::uint64_t> m_lastRequestMs;
    };

}

#endif
