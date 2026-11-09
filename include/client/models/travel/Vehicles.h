#ifndef NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLES_H
#define NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLES_H

#include "client/models/travel/Vehicle.h"
#include "core/Point.h"
#include "formats/ItemTable.h"
#include "gfx/Canvas.h"
#include "protocol/SceneVehicle.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace client {

    /// @brief The vehicles placed on the map (up to 50 places).
    /// @details Original: CH_TTrafficManage; the server puts a vehicle down (15/12, 15/18, map list 23/102) and takes it away (15/22, 15/13).
    class Vehicles {
        public:
            static constexpr int kMaxPlaced = 50; // most vehicles put down on the map at once

            /// @brief Callbacks into the owning scene.
            struct Hooks {
                std::function<void(Vehicle&, bool fly)> placed;
            };

            /// @brief Creates the vehicle list.
            /// @param items Item table used to look up vehicle items.
            /// @param hooks Scene callbacks.
            Vehicles(
                const formats::ItemTable& items,
                Hooks hooks
            );

            /// @brief Finds the first free place.
            /// @details Original: FindFreeVehicleSlot.
            /// @return The slot number, 0 when none is free.
            int freeSlot() const;

            /// @brief Finds the place of an owner's vehicle.
            /// @details Original: FindVehicleSlot.
            /// @param ownerId Owner player id.
            /// @param itemId Vehicle item id.
            /// @return The slot number, 0 when none.
            int slotOf(std::int32_t ownerId, int itemId) const;

            /// @brief Tells whether the owner has that vehicle (or the one its capsule stands for) on the map.
            /// @details Original: IsVehiclePlaced.
            /// @param ownerId Owner player id.
            /// @param itemId Vehicle or capsule item id.
            /// @return True when placed.
            bool isPlaced(std::int32_t ownerId, int itemId) const;

            /// @brief Number of vehicles placed.
            /// @details Original: FToolCount.
            /// @return The count.
            int count() const { return m_count; }

            /// @brief Puts a vehicle down on the map.
            /// @details Original: PlaceSceneVehicle.
            /// @param slot Place number (1..50).
            /// @param ownerId Owner player id.
            /// @param itemId Vehicle item id.
            /// @param position Map position.
            /// @param bagSlot Bag slot the vehicle came from.
            /// @param fly Whether the icon flies there from the owner.
            /// @return False when the map holds 50 already.
            bool place(
                int slot,
                std::int32_t ownerId,
                int itemId,
                core::Point position,
                int bagSlot,
                bool fly
            );

            /// @brief Replaces the placed vehicles with the server's map list.
            /// @details Original: server 23/102, HandleSceneVehicleListPacket.
            /// @param vehicles The vehicles of the map.
            void receiveList(const std::vector<protocol::SceneVehicle>& vehicles);

            /// @brief Takes an owner's vehicle away.
            /// @details Original: RemoveSceneVehicle.
            /// @param ownerId Owner player id.
            /// @param itemId Vehicle item id.
            void remove(std::int32_t ownerId, int itemId);

            /// @brief Removes all vehicles (a map change).
            /// @details Original: ResetSceneVehicles.
            void clear();

            /// @brief Updates and draws the icons of the vehicles still flying to their places.
            /// @details Original: UpdateSceneVehicleLights.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in ms.
            void drawFlights(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief The placed vehicles by slot.
            /// @return Mutable map of slot to vehicle.
            std::map<int, Vehicle>& placedVehicles() { return m_placed; }

            /// @brief The placed vehicles by slot.
            /// @return Read-only map of slot to vehicle.
            const std::map<int, Vehicle>& placedVehicles() const { return m_placed; }

            /// @brief Looks up the vehicle in a slot.
            /// @param slot Place number.
            /// @return The vehicle, or nothing when the slot is empty.
            std::optional<std::reference_wrapper<Vehicle>> find(int slot);

        private:
            const formats::ItemTable& m_items;
            Hooks m_hooks;
            std::map<int, Vehicle> m_placed;
            int m_count = 0;
    };

}

#endif
