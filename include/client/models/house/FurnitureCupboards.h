#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_CUPBOARDS_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_CUPBOARDS_H

#include "client/models/house/HouseUtensils.h"
#include "formats/ItemTable.h"
#include "protocol/FurnitureCupboardItem.h"
#include "protocol/ItemInstance.h"

#include <array>

namespace client {

    /// @brief What the cupboards of a house hold: up to 50 items, each kept for a place (1..10) of a piece that the piece's content slots point to.
    /// @details Original: DZ_Thing.pas TUtensilManage.FCupboardItems with what HandleUtensilPacket's 62/41 and 62/42 do to them.
    class FurnitureCupboards {
        public:
            static constexpr int kItems = 50; // items a cupboard holds in all
            static constexpr int kPlaces = 10; // places shown at once in the cupboard window

            /// @brief Creates the cupboard list over the item table and the house utensils.
            /// @param items Item table (to tell cupboards).
            /// @param utensils The house's furniture pieces.
            FurnitureCupboards(
                const formats::ItemTable& items,
                HouseUtensils& utensils
            )
                : m_items(items),
                  m_utensils(utensils) {}

            /// @brief Forgets all kept items.
            void clear();

            /// @brief An item put in a place (a piece that is not standing in a room keeps it unlinked).
            /// @details Original: server packet 62/41.
            /// @param stored The item, its piece and place.
            void put(const protocol::FurnitureCupboardItem& stored);

            /// @brief The item of that number left its place.
            /// @details Original: server packet 62/42.
            /// @param furnIndex Furniture piece index.
            /// @param place Place in the piece.
            /// @param id Number the item is kept under.
            void remove(int furnIndex, int place, int id);

            /// @brief Points the places of the piece to the items kept for it again.
            /// @details Original: the tail of AddHomeUtensil in the list handler.
            /// @param furnIndex Furniture piece index.
            void relink(int furnIndex);

            /// @brief The item in a place of a cupboard piece.
            /// @param furnIndex Furniture piece index.
            /// @param place Place in the piece.
            /// @return The item, empty for none.
            const protocol::ItemInstance& itemAt(int furnIndex, int place) const;

            /// @brief The first free place of the piece.
            /// @details Original: FindEmptyCupboardSlot.
            /// @param furnIndex Furniture piece index.
            /// @return The place, 0 for none.
            int emptyPlace(int furnIndex) const;

        private:
            /// @brief An item kept for a piece's place.
            struct Kept {
                int furnIndex = 0;
                int place = 0;
                protocol::ItemInstance item;
            };

            const formats::ItemTable& m_items;
            HouseUtensils& m_utensils;
            std::array<Kept, kItems + 1> m_kept{};
    };

}

#endif
