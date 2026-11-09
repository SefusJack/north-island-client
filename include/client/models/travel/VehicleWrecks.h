#ifndef NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLE_WRECKS_H
#define NORTH_ISLAND_CLIENT_MODELS_TRAVEL_VEHICLE_WRECKS_H

#include "client/models/travel/VehicleWreck.h"
#include "core/Point.h"
#include "formats/ItemTable.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief The wrecks of the vehicles that broke: a wreck falls where its owner stood, lies there for a minute, and the server hears of each fall once it ended.
    /// @details Original: CH_TTrafficManage's five places, HandleVehicleDestroyedPacket and DrawVehicleWrecks.
    class VehicleWrecks {
        public:
            static constexpr int kPlaces = 5;

            /// @brief Creates an empty set of wrecks.
            /// @param pictures Picture library for the wreck pictures.
            /// @param items Item table (capsule aliases).
            VehicleWrecks(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items
            )
                : m_pictures(pictures),
                  m_items(items) {}

            /// @brief Adds the wreck of an item (a capsule item stands for its alias item) falling at a map point; its picture "<id>_B" is three frames of the fall.
            /// @param itemId Item id of the vehicle.
            /// @param ownerId Owner the server is told about at the end (0: nobody).
            /// @param position Map point where the wreck lies.
            /// @param screen Screen point where the fall starts.
            /// @param nowMs Current time in ms.
            void add(
                int itemId,
                std::int32_t ownerId,
                core::Point position,
                core::Point screen,
                std::uint64_t nowMs
            );

            /// @brief Draws the falls and the wrecks lying (original DrawVehicleWrecks).
            /// @param canvas Canvas to draw on.
            /// @param origin Map point at the screen's top left.
            /// @param nowMs Current time in ms.
            /// @param onMap Tells whether an owner is still on the map.
            /// @return The owners whose falls ended since the last call, once each, who are still on the map.
            std::vector<std::int32_t> draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs, const std::function<bool(std::int32_t)>& onMap);

            /// @brief Removes all wrecks.
            void clear();

        private:
            /// @brief Finds a place for a new wreck: an empty one, else one whose fall ended (original FindFreeWreckSlot).
            /// @return The place index, nothing when all are falling.
            std::optional<std::size_t> freePlace() const;

            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            std::array<std::optional<VehicleWreck>, kPlaces> m_wrecks;
    };

}

#endif
