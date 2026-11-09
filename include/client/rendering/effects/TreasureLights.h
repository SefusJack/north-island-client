#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_TREASURE_LIGHTS_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_TREASURE_LIGHTS_H

#include "client/rendering/effects/LightEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief The lights of the treasure event.
    /// @details Original: TEventManage.FTreasureLights, StartTreasureLight mode 0. Up to six, each the eleven frames of S10061 played once
    /// below the place a treasure NPC landed on.
    class TreasureLights {
        public:
            /// @brief Creates the lights holder.
            /// @param pictures Picture library the light is looked up in.
            explicit TreasureLights(
                gfx::PictureLibrary& pictures
            )
                : m_pictures(pictures) {}

            /// @brief Starts a light at a place of the map; with six running the first one is cut off.
            /// @details Original: StartTreasureLight.
            /// @param place Map place of the treasure NPC.
            /// @param nowMs Current tick in milliseconds.
            void start(core::Point place, std::uint64_t nowMs);

            /// @brief Draws the running lights.
            /// @param canvas Canvas to draw on.
            /// @param origin Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs);

        private:
            /// @brief One of the six lights.
            struct Slot {
                LightEffect light;
                core::Point place;
            };

            gfx::PictureLibrary& m_pictures;
            std::array<Slot, 6> m_slots;
    };

}

#endif
