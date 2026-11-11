#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_GROUND_FLIGHTS_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_GROUND_FLIGHTS_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

#include <cstdint>
#include <vector>

namespace client {

    /// @brief Things flying between the player and the ground: an item dropped flies from the player to its place, one picked up from its place to the player.
    /// @details Original: TGdThingManage.FDropAnims, FPickupAnims. They move and show while the player is not fighting (UpdateGdThingAnims).
    class GroundFlights {
        public:
            /// @brief Starts a drop flight from the player to the item's place.
            /// @details Original: AddDropAnim / AddMoneyDropAnim.
            /// @param thing Ground index that appears where it lands.
            /// @param picture Picture of the flying item.
            /// @param from Start position.
            /// @param to Landing position.
            /// @param nowMs Current time in milliseconds.
            void drop(
                int thing,
                gfx::PictureId picture,
                core::Point from,
                core::Point to,
                std::uint64_t nowMs
            );

            /// @brief Starts a pickup flight from the item's place to the player.
            /// @details Original: AddPickupAnim / AddMoneyPickupAnim.
            /// @param picture Picture of the flying item.
            /// @param from Start position.
            /// @param to End position.
            /// @param nowMs Current time in milliseconds.
            void pickUp(gfx::PictureId picture, core::Point from, core::Point to, std::uint64_t nowMs);

            /// @brief Moves and draws the flights.
            /// @details Original: UpdateGdThingAnims.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in milliseconds.
            /// @return The ground indexes of the drops that landed.
            std::vector<int> update(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Drops all flights.
            void clear();

        private:
            /// @brief One flying picture and the ground index it stands for.
            struct Flight {
                LightEffect light;
                int thing = 0;
            };

            /// @brief Adds an arc flight (StartLightEffect with move mode 4, StartArcMove).
            /// @param flights List to add the flight to.
            /// @param thing Ground index of the thing.
            /// @param picture Picture of the flying item.
            /// @param from Start position.
            /// @param to End position.
            /// @param nowMs Current time in milliseconds.
            static void start(
                std::vector<Flight>& flights,
                int thing,
                gfx::PictureId picture,
                core::Point from,
                core::Point to,
                std::uint64_t nowMs
            );

            std::vector<Flight> m_pickups;
            std::vector<Flight> m_drops;
    };

}

#endif
