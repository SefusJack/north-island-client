#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_VEHICLE_BOX_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_VEHICLE_BOX_PAINTER_H

#include "client/services/GameServices.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "protocol/ItemInstance.h"

namespace client {

    /// @brief Draws a vehicle in the large place of a vehicle form: its picture in the middle of the 167 pixel square, with the rails of its wear and fuel along the top.
    /// @details Original: TCY_EquipImage.DrawControl for the slot kinds 16, 19, 21 and 34 (the garage's repair space, the Space Capsule's and the parking lot's places).
    class VehicleBoxPainter {
        public:
            static constexpr int kSide = 167; // side of the square vehicle place, px

            /// @brief Draws the vehicle's picture and its wear and fuel rails.
            /// @param services Game services (items, pictures).
            /// @param canvas Target canvas.
            /// @param origin Top left corner of the place.
            /// @param item The vehicle item.
            static void paint(GameServices& services, gfx::Canvas& canvas, core::Point origin, const protocol::ItemInstance& item);
    };

}

#endif
