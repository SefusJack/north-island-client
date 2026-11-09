#ifndef NORTH_ISLAND_CLIENT_DATA_RIDE_OFFSETS_H
#define NORTH_ISLAND_CLIENT_DATA_RIDE_OFFSETS_H

#include "core/Point.h"

namespace client {

    /// @brief Where a rider sits on its pet; the ride skill of a pet is its sprite id.
    /// @details Original: CH_TRidePet.
    class RideOffsets {
        public:
            /// @brief The rider's shift for the direction slot of its riding pose (18..25, or 46..53 for skills that sit).
            /// @details Original: GetRiderOffset.
            /// @param bodyType Body type of the rider (0..4).
            /// @param slot Direction slot of the riding pose.
            /// @param skillId Ride skill of the pet.
            /// @return Pixel shift of the rider.
            static core::Point rider(int bodyType, int slot, int skillId);

            /// @brief The pet's own shift (skills 1185 and 1456, facing one of the eight walking ways).
            /// @details Original: GetRideOffset.
            /// @param bodyType Body type of the rider (0..4).
            /// @param slot Direction slot of the pet.
            /// @param skillId Ride skill of the pet.
            /// @return Pixel shift of the pet.
            static core::Point pet(int bodyType, int slot, int skillId);

            /// @brief Whether the pet is drawn as its first frame.
            /// @details Original: IsListedRideSkill.
            /// @param skillId Ride skill of the pet.
            /// @return True when the skill is listed.
            static bool isListed(int skillId);

            /// @brief Whether the skill seats the rider in the sitting pose (28 slots further).
            /// @details Original: SetRidingAction.
            /// @param skillId Ride skill of the pet.
            /// @return True when the rider sits.
            static bool sits(int skillId);
    };

}

#endif
