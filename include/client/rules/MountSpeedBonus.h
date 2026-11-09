#ifndef NORTH_ISLAND_CLIENT_RULES_MOUNT_SPEED_BONUS_H
#define NORTH_ISLAND_CLIENT_RULES_MOUNT_SPEED_BONUS_H

namespace client {

    /// @brief The walking speed change from the pet a player rides the old way (FOnHorse).
    /// @details Original: THuman.CalcMountSpeedBonus.
    class MountSpeedBonus {
        public:
            /// @brief The speed bonus of a ridden pet.
            /// @param spriteId The pet's sprite id (used when it has no body type).
            /// @param bodyType The pet NPC's BodyType (THorse.FKind), 0 for none.
            /// @param level The pet's NPC level (sprite 1010 only).
            /// @return Pixels per millisecond added to the walking speed.
            static double of(int spriteId, int bodyType, int level);
    };

}

#endif
