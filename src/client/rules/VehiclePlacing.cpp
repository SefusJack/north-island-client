#include "client/rules/VehiclePlacing.h"

#include "client/data/VehicleKinds.h"

#include <cmath>

namespace client {

    namespace {

        constexpr int kWeddingHall = 10085; // map 10085: Wedding Hall
        constexpr int kMessageMs = 2000; // how long a refusal message shows, ms
        constexpr int kWearPerRank = 1800; // wear points per rank step of a vehicle (see vehicleWearLimit)
        const char* const kProhibited = "Vehicles are prohibited in this scene";
        const char* const kNotHere = "Cannot use transportation vehicle here";
        const char* const kUsed = "The vehicle is used";

        // scene types 8, 9, 11 and 14 take no vehicle

        /// @brief Tells whether a scene type takes no vehicle.
        /// @param sceneType The map's scene type.
        /// @return True when vehicles are prohibited there.
        bool prohibitedScene(int sceneType) { return sceneType == 8 || sceneType == 9 || sceneType == 11 || sceneType == 14; }

    }

    int vehicleWearLimit(const formats::ItemRecord& item) {
        // 1.3: exponent of the rank in the wear formula
        return static_cast<int>(std::ceil(std::pow(static_cast<double>(item.rank), 1.3))) * kWearPerRank;
    }

    std::optional<VehicleRefusal> placingRefusal(const formats::ItemRecord& item, const protocol::ItemInstance& stack, const PlacingSituation& where) {
        if (where.mapId == kWeddingHall || prohibitedScene(where.sceneType)) {
            return VehicleRefusal{kProhibited, kMessageMs};
        }
        if (where.houseMap) {
            return VehicleRefusal{"Unable to use a vehicle in house scene", kMessageMs};
        }
        if (where.quizZone) {
            return VehicleRefusal{kNotHere, kMessageMs};
        }
        if (VehicleKinds::classOf(VehicleKinds::kindOfItem(item.id)) == VehicleKinds::kWaterClass && !where.nearWater) {
            return VehicleRefusal{"Unable to throw, too far away from water surface", kMessageMs};
        }
        if (vehicleWearLimit(item) <= stack.usedAmount) {
            return VehicleRefusal{"The vehicle has been destroyed", kMessageMs};
        }
        if (item.id == where.ridingItemId || where.placedState != 0) {
            return VehicleRefusal{kUsed, kMessageMs};
        }
        if (item.aliasItemId != 0 && (where.placedState != 0 || item.aliasItemId == where.ridingItemId)) {
            return VehicleRefusal{kUsed, kMessageMs};
        }
        if (where.placedCount >= 50) { // most vehicles a map holds (50)
            return VehicleRefusal{"The scene is full", kMessageMs};
        }
        if (where.samePlaced) {
            return VehicleRefusal{"The same vehicle has been placed", kMessageMs};
        }
        if (where.fighting) {
            return VehicleRefusal{};
        }
        if (where.onBlockedCell) {
            return VehicleRefusal{kNotHere, kMessageMs};
        }
        // map ids of the arenas that take no vehicle: 12065 4F the Capitol, 12064 2F the Capitol, 12766 Altar
        if (where.trojanWarMap || where.mapId == 12065 || where.mapId == 12064 || where.mapId == 12766) {
            return VehicleRefusal{kNotHere, kMessageMs};
        }
        return std::nullopt;
    }

}
