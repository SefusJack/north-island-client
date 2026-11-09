#include "client/data/VehicleKinds.h"

#include <array>

namespace client {

    namespace {

        // Per-kind tables below are indexed by vehicle kind (index 0 unused). Kinds: 1 Motorbike, 2 Magic Carpet, 3 Turtle Car,
        // 4 Raft, 5 Boat, 6 Hot-air Balloon, 7 Jalor, 8 Airship, 9 Mighty Plane, 10 Steam Ship, 11 Cabriolet, 12 Yacht,
        // 13 UFO, 14 Robot, 15 Lifeboat, 16 Submarine, 17 Space Craft.
        // seats per vehicle kind (rider plus partners)
        constexpr std::array<int, VehicleKinds::kCount + 1> kSeatCounts = {0, 2, 1, 2, 2, 1, 1, 2, 2, 2, 4, 4, 4, 1, 1, 1, 4, 4};
        // movement speed bonus per kind (fraction, 0.03 = +3%)
        constexpr std::array<double, VehicleKinds::kCount + 1> kSpeedBonuses =
            {0.0, 0.03, 0.02, 0.04, 0.03, 0.02, 0.02, 0.03, 0.03, 0.04, 0.04, 0.04, 0.04, 0.06, 0.03, 0.0, 0.02, 0.04};
        // px the vehicle sprite is drawn lower than the feet position (0 = none)
        constexpr std::array<int, VehicleKinds::kCount + 1> kSpriteDrops = {0, 68, 68, 68, 68, 68, 0, 68, 0, 0, 68, 68, 68, 0, 0, 68, 0, 0};
        // shadow kind per vehicle kind (1 = large monster shadow, 0 = the player's shadow)
        constexpr std::array<int, VehicleKinds::kCount + 1> kShadowKinds = {0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1};

        // repair cost in points per vehicle kind (0 = cannot be repaired)
        constexpr std::array<int, VehicleKinds::kCount + 1> kRepairCosts = {0, 4, 0, 5, 0, 0, 4, 2, 5, 8, 4, 6, 5, 25, 5, 0, 7, 6};

        /// @brief Whether a vehicle kind is in 1..kCount.
        /// @param kind Vehicle kind.
        /// @return True when valid.
        bool validKind(int kind) { return kind >= 1 && kind <= VehicleKinds::kCount; }

    }

    int VehicleKinds::kindOfItem(int itemId) {
        switch (itemId) {
            case 48007: // Motorbike
            case 48025: return 1; // Motorbike (capsule item) -> kind 1
            case 48008: // Magic Carpet
            case 48026: return 2; // (Not opened) capsule item -> kind 2
            case 48009: // Turtle Car
            case 48027: return 3; // Turtle Car Capsule -> kind 3
            case 48010: // Raft
            case 48016: // Robinson's Raft
            case 48028: return 4; // (Not opened) capsule item -> kind 4
            case 48003: // Boat
            case 48021: return 5; // (Not opened) capsule item -> kind 5
            case 48001: // Hot-air Balloon
            case 48019: return 6; // Air Balloon Capsule -> kind 6
            case 48002: // Airship
            case 48020: return 8; // Airship Capsule -> kind 8
            case 48004: // Mighty Plane
            case 48022: return 9; // Mighty Plane Capsule -> kind 9
            case 48005: // Jalor
            case 48023: return 7; // Jalor Capsule -> kind 7
            case 48006: // Steam Ship
            case 48024: return 10; // Steam Ship (capsule item) -> kind 10
            case 48011: // Cabriolet
            case 48029: return 11; // Cabriolet Capsule -> kind 11
            case 48012: // Yacht
            case 48030: return 12; // Yacht Capsule -> kind 12
            case 48013: // UFO
            case 48031: return 13; // UFO Capsule -> kind 13
            case 48014: // Robot
            case 48032: return 14; // Robot Capsule -> kind 14
            case 48015: return 15; // Lifeboat (unopened) -> kind 15
            case 48017: // Submarine
            case 48033: return 16; // Submarine Capsule -> kind 16
            case 48018: // Space Craft
            case 48034: return 17; // Space Craft Capsule -> kind 17
            default: return 0; // not a vehicle
        }
    }

    int VehicleKinds::classOf(int kind) {
        switch (kind) {
            case 1:
            case 2:
            case 3:
            case 11:
            case 14: return 1; // ground kinds
            case 4:
            case 5:
            case 7:
            case 10:
            case 12:
            case 15:
            case 16: return 2; // water kinds
            case 6:
            case 8:
            case 9:
            case 13:
            case 17: return 3; // the rest (air / space kinds)
            default: return 0; // no class
        }
    }

    int VehicleKinds::seatCount(int kind) { return validKind(kind) ? kSeatCounts[kind] : 0; }

    double VehicleKinds::speedBonus(int kind) { return validKind(kind) ? kSpeedBonuses[kind] : 0.0; }

    int VehicleKinds::spriteDrop(int kind) { return validKind(kind) ? kSpriteDrops[kind] : 0; }

    int VehicleKinds::repairCost(int kind) { return validKind(kind) ? kRepairCosts[kind] : 0; }

    int VehicleKinds::shadowKind(int kind) { return validKind(kind) ? kShadowKinds[kind] : 0; }

    bool VehicleKinds::hasFrontLayer(int kind) {
        switch (kind) {
            case 1:
            case 3:
            case 6:
            case 7:
            case 8:
            case 9:
            case 11:
            case 13:
            case 14:
            case 15: return true; // kinds with a front layer
            default: return false; // no front layer
        }
    }

    bool VehicleKinds::hidesShadow(int kind) {
        switch (kind) {
            case 4:
            case 5:
            case 7:
            case 10:
            case 12:
            case 15:
            case 16: return true; // kinds whose rider shows no vehicle shadow
            default: return false; // shadow shown
        }
    }

    int VehicleKinds::teleportKind(int itemId) {
        switch (itemId) {
            case 48013: // UFO
            case 48031: return 1; // UFO Capsule: village robot trips
            case 48017: // Submarine
            case 48033: return 2; // Submarine Capsule: submarine trips
            case 48018: // Space Craft
            case 48034: return 3; // Space Craft Capsule: landing craft trips
            default: return 0; // no trips
        }
    }

    // 0xBB93..0xBBA2 = item ids 48019..48034, the capsules
    bool VehicleKinds::isCapsule(int itemId) { return itemId >= 0xBB93 && itemId <= 0xBBA2; }

    bool VehicleKinds::isVoucher(int itemId) {
        switch (itemId) {
            case 30045: // Boat Voucher
            case 30046: // Hot-air Balloon Card
            case 30047: // Airship Voucher
            case 30048: // Labor Plane Voucher
            case 30049: // Steam Ship Voucher
            case 30050: // Motorbike Voucher
            case 30051: // Turtle Car Voucher
            case 30052: // Raft Voucher
            case 34075: // Raft Voucher (second item type)
            case 30065: // Turtle Car Voucher (second id)
            case 30066: // Motorbike Ticket
            case 30067: // Yacht Voucher
            case 30064: // UFO Voucher
                return true;
            default: // not a voucher
                return false;
        }
    }

    int VehicleKinds::voucherVehicle(int itemId) {
        switch (itemId) {
            case 30045: return 48003; // Boat Voucher -> Boat
            case 30046: return 48001; // Hot-air Balloon Card -> Hot-air Balloon
            case 30047: return 48002; // Airship Voucher -> Airship
            case 30048: return 48004; // Labor Plane Voucher -> Mighty Plane
            case 30049: return 48006; // Steam Ship Voucher -> Steam Ship
            case 30050: // Motorbike Voucher
            case 30066: return 48007; // Motorbike Ticket -> Motorbike
            case 30051: // Turtle Car Voucher
            case 30065: return 48009; // Turtle Car Voucher (second id) -> Turtle Car
            case 30052: // Raft Voucher
            case 34075: return 48010; // Raft Voucher (second item type) -> Raft
            case 30067: return 48012; // Yacht Voucher -> Yacht
            case 30064: return 48013; // UFO Voucher -> UFO
            default: return 0; // not honoured
        }
    }

    // item ids of the two robot items: 48014 Robot, 48032 Robot Capsule
    bool VehicleKinds::isRobot(int itemId) { return itemId == 48014 || itemId == 48032; }

    // 0xBB88 = item id 48008, the magic carpet
    bool VehicleKinds::isMagicCarpet(int itemId) { return itemId == 0xBB88; }

    int VehicleKinds::ridePoseSlot(int kind) {
        constexpr int kRiding = 18; // figure pose slot 18: riding standing
        constexpr int kSitting = 46; // figure pose slot 46: sitting in the vehicle
        switch (kind) {
            case 1:
            case 13:
            case 14: return kRiding; // kinds 1, 13, 14 ride standing
            default: return kSitting; // all other kinds sit
        }
    }

    bool VehicleKinds::showsRider(int itemId) {
        switch (itemId) {
            case 48006: // Steam Ship
            case 48012: // Yacht
            case 48017: // Submarine
            case 48018: // Space Craft
            case 48024: // Steam Ship (capsule item)
            case 48030: // Yacht Capsule
            case 48033: // Submarine Capsule
            case 48034: return false; // Space Craft Capsule // vehicle items of kinds 10, 12, 16, 17 hide the rider
            default: return true; // rider shown
        }
    }

}
