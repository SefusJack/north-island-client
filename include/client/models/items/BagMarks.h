#ifndef NORTH_ISLAND_CLIENT_MODELS_ITEMS_BAG_MARKS_H
#define NORTH_ISLAND_CLIENT_MODELS_ITEMS_BAG_MARKS_H

#include <array>
#include <optional>
#include <string>

namespace client {

    /// @brief Marks of the item window's bag cells besides the items: in use by another window, hidden, priced, busy.
    /// @details Original: TCY_EquipImage.FUseFlags and its visibility. A cell in use (UseMark: 1 the compound window, ...) is drawn on the busy frame; a hidden cell (an item still flying to it) is not drawn.
    class BagMarks {
        public:
            /// @brief Why a cell in use may not be touched, and how long the game says so.
            struct LockNotice {
                std::string text;
                int ms = 0;
            };

            static constexpr int kSlots = 50; // bag places 1..50
            // use marks: the number says which window has the cell in use (FUseFlags.UseMark)
            static constexpr int kCompound = 1; // mark 1: compound window
            static constexpr int kSale = 3; // mark 3: sale window
            static constexpr int kBank = 5; // mark 5: bank item window
            static constexpr int kStall = 4; // mark 4: stall window
            static constexpr int kExpress = 6; // mark 6: express window
            static constexpr int kRepair = 7; // mark 7: repair window
            static constexpr int kForge = 8; // mark 8: forge window
            static constexpr int kItemRepair = 11; // mark 11: item repair window
            static constexpr int kFishingRod = 9; // mark 9: fishing rod in use
            static constexpr int kRemoteControl = 10; // mark 10: remote control window
            static constexpr int kForageBone = 12; // mark 12: forage bone window
            static constexpr int kDismantle = 13; // mark 13: dismantle window
            static constexpr int kCampStyle = 14; // mark 14: camp style window
            static constexpr int kMagicMap = 16; // mark 16: magic map window
            static constexpr int kSewingKit = 15; // mark 15: sewing kit window
            static constexpr int kMagicBox = 21; // mark 21: magic box window
            static constexpr int kPacking = 18; // mark 18: packing window
            static constexpr int kLoveFlower = 23; // mark 23: love flower window
            static constexpr int kReinforce = 24; // mark 24: reinforce window
            static constexpr int kConfuseStone = 27; // mark 27: confuse stone window
            static constexpr int kBlessBalloon = 28; // mark 28: bless balloon window
            static constexpr int kExpBox = 29; // mark 29: exp box window
            static constexpr int kMemoryCard = 25; // mark 25: memory card window
            static constexpr int kPicnic = 30; // mark 30: picnic window

            /// @brief Window mark of a cell (0: free).
            /// @param slot Bag place 1..50.
            /// @return The mark.
            int useMark(int slot) const { return m_marks[static_cast<std::size_t>(slot)]; }

            /// @brief Sets the window mark of a cell.
            /// @param slot Bag place 1..50.
            /// @param mark Mark (one of the k constants).
            void setUseMark(int slot, int mark) { m_marks[static_cast<std::size_t>(slot)] = mark; }

            /// @brief Frees every cell of one mark.
            /// @param mark Mark to free.
            void clearUseMarks(int mark);

            /// @brief Tells why a cell in use may not be touched by a window other than the asking one.
            /// @details Original: GetBagItemLockState.
            /// @param slot Bag place 1..50.
            /// @param allowedMark Mark of the asking window.
            /// @return The notice, or nothing when the cell may be used.
            std::optional<LockNotice> lockNotice(int slot, int allowedMark) const;

            /// @brief Tells whether an item is on sale in the own stall.
            /// @details Original: TThing.FPriced, SetThingStallPrice.
            /// @param slot Bag place.
            /// @return True when priced.
            bool isPriced(int slot) const { return m_priced[static_cast<std::size_t>(slot)]; }

            /// @brief Sets the on-sale flag of an item.
            /// @param slot Bag place.
            /// @param priced True when on sale.
            void setPriced(int slot, bool priced) { m_priced[static_cast<std::size_t>(slot)] = priced; }

            /// @brief Clears the on-sale flag of every item.
            void clearPriced() { m_priced.fill(false); }

            /// @brief What an item is busy with (the forge's 4 while the server works on it).
            /// @details Original: FUseFlags.BusyKind.
            /// @param slot Bag place.
            /// @return The busy kind (0: not busy).
            int busyKind(int slot) const { return m_busy[static_cast<std::size_t>(slot)]; }

            /// @brief Sets what an item is busy with.
            /// @param slot Bag place.
            /// @param kind Busy kind.
            void setBusyKind(int slot, int kind) { m_busy[static_cast<std::size_t>(slot)] = kind; }

            /// @brief Tells whether an item is on a shelf or in a vending machine of the own house.
            /// @details Original: FInFurnitureStall.
            /// @param slot Bag place.
            /// @return True when in a furniture stall.
            bool isInFurnitureStall(int slot) const { return m_inFurnitureStall[static_cast<std::size_t>(slot)]; }

            /// @brief Sets the furniture stall flag of an item.
            /// @param slot Bag place.
            /// @param inStall True when in a furniture stall.
            void setInFurnitureStall(int slot, bool inStall) { m_inFurnitureStall[static_cast<std::size_t>(slot)] = inStall; }

            /// @brief Tells whether an item is not on sale, not on a shelf and not busy.
            /// @details Original: IsBagItemAvailable.
            /// @param slot Bag place.
            /// @return True when available.
            bool isAvailable(int slot) const { return !isPriced(slot) && !isInFurnitureStall(slot) && busyKind(slot) == 0; }

            /// @brief Tells whether the cell is drawn on the busy frame: in use, on sale, on a shelf or busy.
            /// @details Original: TCY_EquipImage.DrawControl.
            /// @param slot Bag place.
            /// @return True when busy.
            bool isBusy(int slot) const { return useMark(slot) != 0 || isPriced(slot) || isInFurnitureStall(slot) || busyKind(slot) != 0; }

            /// @brief State of a vehicle of the bag: 0 none, 1 put down on the map, 2 ridden; the item may not move then.
            /// @details Original: TThing.FPlacedVehicle with FUseFlags.VehicleUse.
            /// @param slot Bag place.
            /// @return The state.
            int vehicleState(int slot) const { return m_vehicle[static_cast<std::size_t>(slot)]; }

            /// @brief Sets the vehicle state of an item.
            /// @param slot Bag place.
            /// @param state Vehicle state.
            void setVehicleState(int slot, int state) { m_vehicle[static_cast<std::size_t>(slot)] = state; }

            /// @brief Tells whether a cell is hidden (an item still flying to it).
            /// @param slot Bag place.
            /// @return True when hidden.
            bool isHidden(int slot) const { return m_hidden[static_cast<std::size_t>(slot)]; }

            /// @brief Sets the hidden flag of a cell.
            /// @param slot Bag place.
            /// @param hidden True to hide.
            void setHidden(int slot, bool hidden) { m_hidden[static_cast<std::size_t>(slot)] = hidden; }

        private:
            std::array<int, kSlots + 1> m_marks{};
            std::array<bool, kSlots + 1> m_hidden{};
            std::array<bool, kSlots + 1> m_priced{};
            std::array<bool, kSlots + 1> m_inFurnitureStall{};
            std::array<int, kSlots + 1> m_busy{};
            std::array<int, kSlots + 1> m_vehicle{};
    };

}

#endif
