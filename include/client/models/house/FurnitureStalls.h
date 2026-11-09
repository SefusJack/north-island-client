#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_STALLS_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_STALLS_H

#include "client/models/house/HomeFurniture.h"
#include "client/models/house/HouseUtensils.h"
#include "client/models/house/StallCaseChange.h"
#include "client/models/house/StallCaseRecord.h"
#include "client/models/house/StallInFurItem.h"
#include "formats/ItemTable.h"
#include "gfx/PictureLibrary.h"
#include "protocol/FurnitureCaseSale.h"
#include "protocol/FurnitureCaseUpdate.h"
#include "protocol/FurnitureStallItem.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The items on sale on shelves and in vending machines, in display cases, and the contents of Fortune Egg machines in the house.
    /// @details Original: DZ_Thing.pas TUtensilManage.FStallItems / FStallInFurItems (HandleUtensilPacket 62/21, 62/22, 62/24), FStall4Items with
    /// HL_Menu.pas RecvStallCaseUpdate (items of display case 39086, shelf 39059, vending machine 39058), Player.FStallRecs, FFurnItemSlots (62/31..34).
    class FurnitureStalls {
        public:
            static constexpr int kShelfItems = 50; // items a Shelf (39059) holds
            static constexpr int kVendingItems = 20; // items a vending machine (Coin Machine, 39058) holds
            static constexpr int kContents = 60; // content places of one stall record
            static constexpr int kCases = 4; // stall cases kept (display case, shelf, vending machine records)

            /// @brief A stack in a place of a Fortune Egg machine (original TFurnItemSlot).
            struct Content {
                int furnIndex = 0;
                int place = 0;
                int itemId = 0;
                int count = 0;
            };

            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<void(int, bool)> markBag;
                std::function<void(int, int)> reclaimed;
                std::function<void(const std::string&)> notice;
                std::function<std::int32_t()> ownId;
                std::function<void(int, std::int32_t)> priceChanged;
                std::function<void(const StallCaseChange&)> caseChanged;
            };

            /// @brief Creates the stall store.
            /// @param pictures Picture library for item icons.
            /// @param items Item table.
            /// @param furniture The house's furniture.
            /// @param utensils The drawn pieces whose content slots are kept in step.
            /// @param hooks Callbacks into the rest of the client.
            FurnitureStalls(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items,
                const HomeFurniture& furniture,
                HouseUtensils& utensils,
                Hooks hooks
            );

            /// @brief Empties every shelf, machine, case and content record.
            /// @details Original: ClearFurnitureSlots.
            void clear();

            /// @brief The shelf item of a number.
            /// @param number Shelf item number 1..kShelfItems.
            /// @return The record, or an empty one when out of range.
            const StallInFurItem& shelf(int number) const;

            /// @brief The vending machine item of a number.
            /// @param number Vending item number 1..kVendingItems.
            /// @return The record, or an empty one when out of range.
            const StallInFurItem& vending(int number) const;

            /// @brief The item of a display case as the piece shows it.
            /// @param number Case number 1..kCases.
            /// @return The record, or an empty one when out of range.
            const StallInFurItem& displayed(int number) const;

            /// @brief The owner's own record of a display case.
            /// @param number Case number 1..kCases.
            /// @return The record, or an empty one when out of range.
            const StallCaseRecord& ownCase(int number) const;

            /// @brief Puts an item on a shelf or into a vending machine.
            /// @details Original: server action 62/21 (HandleUtensilPacket).
            /// @param stalled The stalled item message.
            void put(const protocol::FurnitureStallItem& stalled);

            /// @brief Takes the item of a number away.
            /// @details Original: server action 62/22.
            /// @param ownerId Id of the item's owner.
            /// @param number Shelf or vending item number.
            /// @param furnIndex The piece the item was in.
            void remove(std::int32_t ownerId, int number, int furnIndex);

            /// @brief Sells so many of an item; all gone frees the place.
            /// @details Original: server action 62/24.
            /// @param ownerId Id of the item's owner.
            /// @param number Shelf or vending item number.
            /// @param count How many were sold.
            /// @param furnIndex The piece the item is in.
            void take(std::int32_t ownerId, int number, int count, int furnIndex);

            /// @brief Applies a display case change.
            /// @details Original: server action 62/58 option 1 (SetStall4SlotItem and the rest of RecvStallCaseUpdate).
            /// @param update The case update message.
            /// @param inHouse True when the player's place is a house.
            void updateCase(const protocol::FurnitureCaseUpdate& update, bool inHouse);

            /// @brief Sells so many of a case's item.
            /// @details Original: server action 62/58 option 2.
            /// @param sale The case sale message.
            /// @param inHouse True when the player's place is a house.
            void sellCase(const protocol::FurnitureCaseSale& sale, bool inHouse);

            /// @brief Re-places the items lying on a piece that now stands somewhere else.
            /// @details Original: RebuildHomeUtensil.
            /// @param furnIndex The moved piece.
            void relink(int furnIndex);

            /// @brief Shows why a stall refused.
            /// @details Original: server action 62/25, codes 1..7.
            /// @param code Refusal code 1..7.
            void refuse(int code);

            /// @brief The stack of a content number.
            /// @param number Content number 1..kContents.
            /// @return The stack, or an empty one when out of range.
            const Content& content(int number) const;

            /// @brief Puts a stack into a place of a Fortune Egg machine; the seller must be the house owner.
            /// @details Original: server action 62/31.
            /// @param sellerId Id of the seller.
            /// @param furnIndex The machine piece.
            /// @param place Place in the machine (1..10).
            /// @param itemId Item of the stack.
            /// @param count Stack size.
            /// @param number Content number the stack is kept under.
            void putContent(
                std::int32_t sellerId,
                int furnIndex,
                int place,
                int itemId,
                int count,
                int number
            );

            /// @brief Sets the price of a play of a machine (0: no longer for sale).
            /// @details Original: server action 62/32.
            /// @param furnIndex The machine piece.
            /// @param price New price.
            void setSalePrice(int furnIndex, std::int32_t price);

            /// @brief A play sold one egg from a place.
            /// @details Original: server action 62/33.
            /// @param furnIndex The machine piece.
            /// @param place Place in the machine.
            void eggSold(int furnIndex, int place);

            /// @brief Sets how many eggs a machine holds.
            /// @details Original: server action 62/34.
            /// @param furnIndex The machine piece.
            /// @param count Egg count.
            void setEggCount(int furnIndex, int count);

        private:
            /// @brief Offsets of an item against its piece's floor point, per facing of the piece's picture.
            struct Offsets {
                core::Point left;
                core::Point right;
            };

            /// @brief Works out where an item of a shelf or display case lies on the map, and its picture.
            /// @details Original: SetStallSlotItem and SetStall4SlotItem.
            /// @param item The item to place.
            /// @param utensilSlot Utensil slot of the piece.
            /// @param piece Furniture number of the piece.
            /// @param offsets Offsets per facing.
            void place(StallInFurItem& item, int utensilSlot, int piece, const Offsets& offsets);

            /// @brief Sets the item of a display case, linked to its place in the piece.
            /// @details Original: SetStall4SlotItem.
            /// @param number Case number.
            /// @param record The case record.
            void setCase(int number, const StallCaseRecord& record);

            /// @brief Clears an item: frees the owner's bag place and the place in the piece.
            /// @param item The item to clear.
            /// @param utensilSlot Utensil slot of the piece it lay in.
            /// @param ownerId Id of the item's owner.
            void empty(StallInFurItem& item, int utensilSlot, std::int32_t ownerId);

            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            const HomeFurniture& m_furniture;
            HouseUtensils& m_utensils;
            Hooks m_hooks;
            std::array<StallInFurItem, kShelfItems + 1> m_shelf{};
            std::array<StallInFurItem, kVendingItems + 1> m_vending{};
            std::array<StallInFurItem, kCases + 1> m_cases{};
            std::array<StallCaseRecord, kCases + 1> m_ownCases{};
            std::array<Content, kContents + 1> m_contents{};
    };

}

#endif
