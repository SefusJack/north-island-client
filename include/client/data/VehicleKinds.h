#ifndef NORTH_ISLAND_CLIENT_DATA_VEHICLE_KINDS_H
#define NORTH_ISLAND_CLIENT_DATA_VEHICLE_KINDS_H

#include "core/Point.h"

namespace client {

    /// @brief Tables and tests for the vehicle kinds 1..17.
    /// @details Original: CH_Traffic.pas. The item gives the kind (GetVehicleKind); the kind gives the seats, the speed bonus and the class that decides where it can go (GetVehicleClass: 1 ground, 2 water, 3 the rest).
    class VehicleKinds {
        public:
            static constexpr int kCount = 17; // vehicle kinds 1..17 (GetVehicleKind)
            static constexpr int kWaterClass = 2; // vehicle class 2: water (GetVehicleClass)

            /// @brief The vehicle kind an item gives (the vehicle and its capsule give the same kind).
            /// @details Original: GetVehicleKind.
            /// @param itemId Item id.
            /// @return Kind 1..17, 0 for an item that is no vehicle.
            static int kindOfItem(int itemId);

            /// @brief The class of a kind that decides where it can go (1 ground, 2 water, 3 the rest).
            /// @details Original: GetVehicleClass.
            /// @param kind Vehicle kind.
            /// @return The class.
            static int classOf(int kind);

            /// @brief The seats of a kind: the driver and the passengers.
            /// @details Original: VehicleSeatCounts.
            /// @param kind Vehicle kind.
            /// @return Seat count.
            static int seatCount(int kind);

            /// @brief The part of its speed a vehicle adds.
            /// @details Original: VehicleKindValues, GetVehicleSpeedBonus.
            /// @param kind Vehicle kind.
            /// @return Speed bonus fraction.
            static double speedBonus(int kind);

            /// @brief How much lower the sprite of a vehicle is drawn.
            /// @details Original: VehicleRiderOfsY.
            /// @param kind Vehicle kind.
            /// @return Pixels of drop.
            static int spriteDrop(int kind);

            /// @brief The shadow kind of a vehicle: 1 is the large monster shadow, else the player's.
            /// @details Original: VehicleShadowKind.
            /// @param kind Vehicle kind.
            /// @return Shadow kind.
            static int shadowKind(int kind);

            /// @brief The points a repair costs.
            /// @details Original: VehicleRepairCosts.
            /// @param kind Vehicle kind.
            /// @return Cost, 0 when the kind cannot be repaired.
            static int repairCost(int kind);

            /// @brief Whether a front layer (the sprite after the look's) is drawn over the rider.
            /// @details Original: InitFromVehicleItem.
            /// @param kind Vehicle kind.
            /// @return True when a front layer exists.
            static bool hasFrontLayer(int kind);

            /// @brief Whether the rider of this kind shows no shadow of the vehicle.
            /// @details Original: DrawVehicleBack.
            /// @param kind Vehicle kind.
            /// @return True when the shadow is hidden.
            static bool hidesShadow(int kind);

            /// @brief The trips a vehicle's item makes (1 the village robot's, 2 the submarine's, 3 the landing craft's).
            /// @details Original: GetTeleportVehicleKind.
            /// @param itemId Item id.
            /// @return Trip kind, 0 for none.
            static int teleportKind(int itemId);

            /// @brief Whether the item is a vehicle capsule (48019..48034).
            /// @details Original: IsVehicleCapsuleItem.
            /// @param itemId Item id.
            /// @return True for a capsule.
            static bool isCapsule(int itemId);

            /// @brief Whether the item is a vehicle voucher.
            /// @details Original: IsVehicleVoucher.
            /// @param itemId Item id.
            /// @return True for a voucher.
            static bool isVoucher(int itemId);

            /// @brief The vehicle a voucher is exchanged for.
            /// @details Original: GetVoucherVehicleId.
            /// @param itemId Item id.
            /// @return Vehicle id, 0 when the item is no voucher or is not honoured.
            static int voucherVehicle(int itemId);

            /// @brief Whether the item is a robot.
            /// @details Original: IsRobotItem.
            /// @param itemId Item id.
            /// @return True for a robot item.
            static bool isRobot(int itemId);

            /// @brief Whether the item is a magic carpet.
            /// @details Original: IsMagicCarpetItem.
            /// @param itemId Item id.
            /// @return True for a magic carpet item.
            static bool isMagicCarpet(int itemId);

            /// @brief The first slot of the riding pose: 18, or 46 for the kinds 2..12 and 15..17.
            /// @details Original: ApplyVehicleRidePose.
            /// @param kind Vehicle kind.
            /// @return First pose slot.
            static int ridePoseSlot(int kind);

            /// @brief Whether the vehicle hides its rider's body (the item of the kind 10, 12, 16, 17).
            /// @details Original: IsRiderShownOnVehicle.
            /// @param itemId Item id of the vehicle.
            /// @return Per the original's test of that name.
            static bool showsRider(int itemId);
    };

}

#endif
