#include "client/rules/UtensilSeat.h"

namespace client {

    namespace {

        constexpr int kSitting = 46; // action group 46: the sitting pose
        constexpr int kRiding = 18; // action group 18: the riding pose
        constexpr int kFacingEven = 5; // direction slot faced on a piece with an even picture id
        constexpr int kFacingOdd = 3; // direction slot faced on a piece with an odd picture id

        /// @brief Builds a seat place offset from the piece's floor point.
        /// @param world Floor point of the piece.
        /// @param dx Horizontal offset in pixels.
        /// @param dy Vertical offset in pixels.
        /// @param direction Direction slot the player faces.
        /// @param group Action group (sitting by default).
        /// @return The place.
        UtensilSeat::Place at(
            core::Point world,
            int dx,
            int dy,
            int direction,
            int group = kSitting
        ) {
            UtensilSeat::Place place;
            place.position = {world.x + dx, world.y + dy};
            place.direction = direction;
            place.group = group;
            return place;
        }

    }

    int UtensilSeat::seatCount(const formats::ItemTable& items, int itemId) {
        if (!items.find(itemId)) {
            return 0;
        }
        switch (itemId) {
            case 39046: return 1; // Sofa: 1 seat
            case 39156: return 2; // Great Sofa: 2 seats
            case 39219: return 3; // Cannon Toy: kind 3 (a ride)
            default: return 0; // any other piece
        }
    }

    int UtensilSeat::sitKind(int itemId) {
        switch (itemId) {
            case 39116: return 1; // Cushion
            case 39155: return 2; // White Sofa
            case 39079: // Red and Blue Carpet
            case 39080: return 3; // Red and Blue Carpet
            default: return 0; // any other piece
        }
    }

    // 39060: Toilet Bowl item id
    bool UtensilSeat::isToilet(const formats::ItemTable& items, int itemId) { return itemId == 39060 && items.find(itemId).has_value(); }

    // 39218: Lovely Octopus item id
    bool UtensilSeat::isOctopus(const formats::ItemTable& items, int itemId) { return itemId == 39218 && items.find(itemId).has_value(); }

    bool UtensilSeat::isSeat(const formats::ItemTable& items, int itemId) {
        return seatCount(items, itemId) != 0 || sitKind(itemId) != 0 || isToilet(items, itemId) || isOctopus(items, itemId);
    }

    std::optional<UtensilSeat::Place> UtensilSeat::place(
        const formats::ItemTable& items,
        int itemId,
        int seat,
        bool evenPicture,
        core::Point world
    ) {
        const int facing = evenPicture ? kFacingEven : kFacingOdd;
        switch (seatCount(items, itemId)) {
            case 1: return at(world, 0, 0, facing); // one-seat piece (Sofa, item 39046): at the floor point
            case 2: // two-seat piece (Great Sofa)
                // seat 1 offset (+-10, 5) pixels
                if (seat == 1) {
                    return at(world, evenPicture ? -10 : 10, 5, facing);
                }
                // seat 2 offset (+-38, 19) pixels
                if (seat == 2) {
                    return at(world, evenPicture ? -38 : 38, 19, facing);
                }
                return at(world, 0, 0, facing);
            case 3: { // Cannon Toy (a ride)
                // offset (+-8, 8) pixels; direction slot 3 (even) or 5 (odd) of group 18
                Place place = at(world, evenPicture ? -8 : 8, 8, evenPicture ? 3 : 5, kRiding);
                place.holdFirstFrame = true;
                return place;
            }
            default: break; // not a seat-count piece
        }
        switch (sitKind(itemId)) {
            case 1: return at(world, 0, 10, facing); // Cushion: 10 pixels down
            case 2:
                return at(world, 0, 0, facing); // White Sofa: at the floor point
            // Red and Blue Carpets: 38 (even) or 45 (odd) pixels down
            case 3: return at(world, 0, evenPicture ? 38 : 45, facing);
            default: break; // not a sit-kind piece
        }
        if (isToilet(items, itemId) || isOctopus(items, itemId)) {
            return at(world, 0, 0, evenPicture ? kFacingOdd : kFacingEven);
        }
        return std::nullopt;
    }

}
