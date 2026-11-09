#ifndef NORTH_ISLAND_CLIENT_RULES_VEHICLE_PLACING_H
#define NORTH_ISLAND_CLIENT_RULES_VEHICLE_PLACING_H

#include "formats/ItemRecord.h"
#include "protocol/ItemInstance.h"

#include <optional>
#include <string>

namespace client {

    /// @brief The state of the player and the map that the vehicle checks look at besides the vehicle.
    struct PlacingSituation {
        int mapId = 0;
        int sceneType = 0;
        bool houseMap = false;
        bool quizZone = false;
        bool trojanWarMap = false;
        bool nearWater = false;
        bool onBlockedCell = false;
        bool fighting = false;
        int placedCount = 0;
        bool samePlaced = false;
        int ridingItemId = 0;
        int placedState = 0;
    };

    /// @brief Why the vehicle may not be put down: the message and how long it shows in ms (no text: the game says nothing).
    struct VehicleRefusal {
        std::string text;
        int ms = 0;
    };

    /// @brief Checks whether a vehicle of the bag may be put on the map.
    /// @details Original: TSe_EquipForm2.UseGridItem's vehicle branch, then TryPlaceVehicleFromBag; the checks that both make twice are made once.
    /// @param item The vehicle's item record.
    /// @param stack The bag stack holding the vehicle (its wear is checked).
    /// @param where The state of the player and the map.
    /// @return The refusal, or none when it may be put down.
    std::optional<VehicleRefusal> placingRefusal(const formats::ItemRecord& item, const protocol::ItemInstance& stack, const PlacingSituation& where);

    /// @brief Gives the wear that destroys a vehicle: Ceil(Power(rank, 1.3)) * 1800.
    /// @param item The vehicle's item record.
    /// @return The wear limit.
    int vehicleWearLimit(const formats::ItemRecord& item);

}

#endif
