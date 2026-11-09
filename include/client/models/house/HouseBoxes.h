#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_BOXES_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_BOXES_H

#include "core/Bytes.h"

#include <array>

namespace client {

    /// @brief What the house's storage furniture holds: Fridge (box 1), Material Cabinet (2), Potion Cabinet (3), Medicine Case (4) with ten places each, Locker (5) with twenty-five.
    /// @details Original: Player.FBox1Slots .. FBox4Slots and FPledgeBoxSlots (TBoxSlotItem); 62/57 tells one place at a time (RecvIceBoxUpdate, RecvPledgeBoxUpdate).
    class HouseBoxes {
        public:
            static constexpr int kBoxes = 5; // storage pieces numbered 1..5 (Fridge, Material Cabinet, Potion Cabinet, Medicine Case, Locker)
            static constexpr int kLocker = 5; // box number of the Locker
            static constexpr int kBoxPlaces = 10; // places in boxes 1..4
            static constexpr int kLockerPlaces = 25; // places in the Locker

            /// @brief One place of a box: the item and its count.
            struct Place {
                int itemId = 0;
                int count = 0;
            };

            /// @brief The number of places of a box.
            /// @param box Box number 1..5.
            /// @return 25 for the Locker, else 10.
            static int placesOf(int box) { return box == kLocker ? kLockerPlaces : kBoxPlaces; }

            /// @brief Applies a 62/57 message: one place of a box now holds an item or is empty.
            /// @details Original: server action 62/57: u8 option (odd 1..9: holds; even 2..10: empty; box = (option + 1) / 2, 4 the Locker's), u8 place, [u16 item, u8 count].
            /// @param payload The whole message payload.
            /// @return False when the packet is cut short or names a place the box lacks.
            bool receive(const core::Bytes& payload);

            /// @brief The content of a place.
            /// @param box Box number 1..5.
            /// @param place Place number.
            /// @return The place; an empty one (item 0) for none.
            const Place& place(int box, int place) const;

            /// @brief Finds the first empty place of a box.
            /// @details Original: FindFreeIceBoxSlot, FindFreePledgeSlot.
            /// @param box Box number 1..5.
            /// @return The place number, 0 for none.
            int firstFree(int box) const;

        private:
            std::array<std::array<Place, kLockerPlaces + 1>, kBoxes + 1> m_places{};
    };

}

#endif
