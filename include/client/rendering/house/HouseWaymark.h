#ifndef NORTH_ISLAND_CLIENT_RENDERING_HOUSE_HOUSE_WAYMARK_H
#define NORTH_ISLAND_CLIENT_RENDERING_HOUSE_HOUSE_WAYMARK_H

#include "client/models/world/Camera.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"
#include "gfx/PictureLibrary.h"

#include <optional>

namespace client {

    /// @brief The arrow at the door of a house's floor room; clicking it takes the player to the door, which sends him out.
    /// @details Original: MK_Menu.pas TWaymarkForm. Its picture ("WayMark") stacks three states; the second shows while the mouse is on it, with an "Exit" tip.
    class HouseWaymark {
        public:
            /// @brief Looks up the "WayMark" picture.
            /// @param pictures Picture library used for the arrow.
            explicit HouseWaymark(
                gfx::PictureLibrary& pictures
            );

            /// @brief Places the arrow relative to the door.
            /// @details Original: SetWaymarkPos.
            /// @param doorCorner Corner of the door's area in world coordinates.
            void setDoor(core::Point doorCorner);

            /// @brief Shows or hides the arrow (shown in the floor rooms of a house).
            /// @details Original: SetCtrlVisible.
            /// @param visible True to show the arrow.
            void setVisible(bool visible) { m_visible = visible; }

            /// @brief Draws the arrow in the state given by the mouse position, with the "Exit" tip when hovered.
            /// @details Original: DrawControl (with HandleMouseMove's state).
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param mouse Screen point of the mouse, none when it is off the map.
            void paint(gfx::Canvas& canvas, const Camera& camera, const std::optional<core::Point>& mouse);

            /// @brief Handles a click on the arrow.
            /// @details Original: HandleWaymarkClick; the returned spot is where JumpToWaymark puts the player.
            /// @param camera Camera used for world to screen conversion.
            /// @param screenPoint Clicked screen point.
            /// @return The door spot to walk to, none when the arrow was not hit.
            std::optional<core::Point> click(const Camera& camera, core::Point screenPoint) const;

        private:
            /// @brief Where the arrow is on the screen and its size (one state of the picture).
            /// @param camera Camera used for world to screen conversion.
            /// @return Screen rectangle, empty when the picture is missing.
            core::Rect area(const Camera& camera) const;

            gfx::PictureLibrary& m_pictures;
            gfx::PictureId m_picture;
            bool m_visible = false;
            core::Point m_position;
            int m_state = 0;
    };

}

#endif
