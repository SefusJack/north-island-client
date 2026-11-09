#include "client/data/EquipDrawOrder.h"

#include "client/models/character/CharacterLook.h"

#include <algorithm>

namespace client {
    namespace {

        // 20 rows of 7 equipment slots (0 = head, 1..6 = equipment slots), the order of one layer set per row
        constexpr std::array<std::array<int, 7>, 20> kRows{{{3, 1, 0, 2, 4, 5, 6}, {3, 1, 0, 4, 2, 5, 6}, {3, 1, 0, 5, 2, 4, 6},
                                                            {3, 1, 0, 4, 5, 2, 6}, // rows 0..3: ordinary looks (row picked by weapon kind)
                                                            {3, 6, 1, 0, 2, 4, 5}, {3, 6, 1, 0, 4, 2, 5}, {3, 6, 1, 0, 5, 2, 4},
                                                            {3, 6, 1, 0, 4, 5, 2}, // rows 4..7: special base looks, first row of the group is 4
                                                            {3, 1, 0, 6, 2, 4, 5}, {3, 1, 0, 6, 4, 2, 5}, {3, 1, 0, 6, 5, 2, 4},
                                                            {3, 1, 0, 6, 4, 5, 2}, // rows 8..11: manteau / special costume facing away
                                                            {3, 1, 2, 6, 0, 4, 5}, {3, 1, 4, 2, 6, 0, 5}, {3, 1, 5, 2, 6, 0, 4},
                                                            {3, 1, 4, 5, 2, 6, 0}, // rows 12..15: manteau in front of the body
                                                            {3, 1, 0, 6, 2, 4, 5}, {3, 1, 0, 6, 4, 2, 5}, {3, 1, 0, 6, 5, 2, 4},
                                                            {3, 1, 0, 6, 4, 5, 2}}}; // rows 16..19: manteau beside the body

        /// @brief Tells whether a direction (animation slot) is one the special looks turn their order around for.
        /// @param direction The figure's animation slot.
        /// @return True for the back-facing slots.
        bool backFacing(int direction) {
            // animation slots that face away from the viewer
            return direction == 0 || direction == 1 || (direction >= 7 && direction <= 9) || direction == 15 || direction == 18 || direction == 19 ||
                   direction == 25 || direction == 46 || direction == 47 || (direction >= 53 && direction <= 55) || direction == 61;
        }

        /// @brief Tells whether a draw look is a special base look (tail 232 or 331 of a body type's 2000-5999 range).
        /// @param look The draw look id.
        /// @return True for a special base look.
        bool isSpecialBase(int look) {
            const int tail = look % 1000; // look id without the body type thousands
            // looks 2000..5999 are the four body types; tails 232 and 331 are the special bases (the cape 331 included)
            return look >= 2000 && look < 6000 && (tail == 232 || tail == 331);
        }

        /// @brief Tells whether a draw look is a manteau (rebirth cape) look: 2347, 3347, 4347 or 5347.
        /// @param look The draw look id.
        /// @return True for a manteau look.
        bool isManteau(int look) {
            // looks 2000..5999 are the four body types; tail 347 is the manteau
            return look >= 2000 && look < 6000 && look % 1000 == 347;
        }

        /// @brief Tells whether the manteau is drawn beside the body for a direction and frame (Original: GetEquipDrawSlotX347).
        /// @param direction The figure's animation slot.
        /// @param frame The animation frame (FAnimFrame).
        /// @param weaponless True when the weapon kind is 0.
        /// @return True when the manteau is drawn beside the body.
        bool manteauSideFacing(int direction, int frame, bool weaponless) {
            switch (direction) {
                // always beside the body in these side-pose slots
                case 2:
                case 6:
                case 10:
                case 14:
                case 26:
                case 27:
                case 38:
                case 39:
                case 48:
                case 52:
                case 64:
                case 65: return true;
                case 56: // beside the body only without a weapon
                case 60: // beside the body only without a weapon
                    return weaponless;
                // slots 32 and 33: beside the body on frame 3
                case 32:
                case 33: return frame == 3;
                // slots 34, 35, 40, 41: beside the body after frame 0
                case 34:
                case 35:
                case 40:
                case 41: return frame != 0;
                // slots 36 and 37: frames 2 and 3 without a weapon, frame 2 with one
                case 36:
                case 37: return weaponless ? (frame == 2 || frame == 3) : frame == 2;
                default: // any other slot: not beside the body
                    return false;
            }
        }

        /// @brief Tells whether a draw look is a special-costume look (tails 332..335, like 2332..2335).
        /// @param look The draw look id.
        /// @return True for a special costume look.
        bool isSpecialCostume(int look) {
            const int tail = look % 1000; // look id without the body type thousands
            // looks 2000..5999 are the four body types; tails 332..335 are the special costumes
            return look >= 2000 && look < 6000 && tail >= 332 && tail <= 335;
        }

        /// @brief Gives which of the four rows of a group the weapon kind picks.
        /// @param look The figure's look (weapon kind and body type).
        /// @return The row offset 1..3, or 0 when the kind picks none of them.
        int rowOffset(const CharacterLook& look) {
            const int kind = look.weaponKind;
            const bool bodyOdd = look.bodyType == 1 || look.bodyType == 3; // body types 1 and 3 are male
            // weapon kinds 5..7 (male), 9..11 (female) and 13..15 (both) pick rows 1..3; the subtractions map each range to 1..3
            if ((kind >= 5 && kind <= 7 && bodyOdd) || (kind >= 9 && kind <= 11 && !bodyOdd) || (kind >= 13 && kind <= 15)) {
                return kind - (kind >= 13 ? 12 : kind >= 9 ? 8 : 4);
            }
            return 0;
        }

    }

    std::array<int, 7> equipDrawOrder(const CharacterLook& look, int direction, int frame) {
        int row = 0;
        if (isSpecialBase(look.drawLook)) {
            row = 4; // first row of the special base group
        } else if (isManteau(look.drawLook)) {
            const bool weaponless = look.weaponKind == 0;
            // first row of the group: 8 behind the body, 16 beside it, 12 in front of it
            row = (backFacing(direction) ? 8 : manteauSideFacing(direction, frame, weaponless) ? 16 : 12) + rowOffset(look);
        } else if (isSpecialCostume(look.drawLook)) {
            const bool back = backFacing(direction);
            row = (back ? 8 : 4) + rowOffset(look); // first row of the group: 8 facing away, 4 otherwise
        } else {
            row = rowOffset(look);
        }
        std::array<int, 7> order = kRows[static_cast<std::size_t>(row)];
        std::reverse(order.begin(), order.end());
        return order;
    }

    bool canDrawEquipSlot(const CharacterLook& look, int slot) {
        // the cape look is 2331 / 3331 / 4331 / 5331 by body type (1..4)
        const bool cape = look.drawLook == look.bodyType * 1000 + 1331;
        // the cape hides slots 1 (headgear), 2 (clothes), 4 and 5 (shoes)
        return !(cape && (slot == 1 || slot == 2 || slot == 4 || slot == 5));
    }

}
