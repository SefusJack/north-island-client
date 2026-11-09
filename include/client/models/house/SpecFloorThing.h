#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_SPEC_FLOOR_THING_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_SPEC_FLOOR_THING_H

#include "client/models/world/Camera.h"
#include "core/Point.h"
#include "formats/ItemTable.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <optional>
#include <string>

namespace client {

    /// @brief The thing on the floor of a special house room, shown to the owner (to pick up) or to a visitor.
    /// @details Original: TGdThingInSpecFloor, packets 81/x and 23/160/13; places from SpecFloorPosX / SpecFloorPosY.
    class SpecFloorThing {
        public:
            /// @brief Creates the model on the picture library and the item table.
            /// @param pictures Picture library the icons come from.
            /// @param items Item table giving names and icon ids.
            SpecFloorThing(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items
            )
                : m_pictures(pictures),
                  m_items(items) {}

            /// @brief Handles the thing packet: sets the owner's or a visitor's thing.
            /// @details Original: HandleSpecFloorThingPacket (81/1).
            /// @param kind Room kind 1..3 (the player must be in room kind + 1).
            /// @param itemId Item shown.
            /// @param place Place in the room, 1..3.
            /// @param ownerId Id of the house owner.
            /// @param ownId Id of the player the packet is about.
            /// @param playerRoom Room the local player is in.
            void receive(
                int kind,
                int itemId,
                int place,
                std::int32_t ownerId,
                std::int32_t ownId,
                int playerRoom
            );

            /// @brief Sets or clears a visitor's view of the thing of a room.
            /// @details Original: RecvHouseRoomThing (23/160/13).
            /// @param room Room of the thing.
            /// @param place Place in the room.
            /// @param itemId Item shown.
            /// @param entering True when the visitor enters (shown), false when leaving.
            /// @param playerRoom Room the local player is in.
            void showVisitor(
                int room,
                int place,
                int itemId,
                bool entering,
                int playerRoom
            );

            /// @brief Hides the visitor's thing (81/2 for another player).
            void clearVisitor() { m_visitorShown = false; }

            /// @brief Hides the owner's thing (81/4).
            void clearOwner() { m_ownerShown = false; }

            /// @brief Hides both things.
            /// @details Original: HideSpecFloorThing.
            void hide();

            /// @brief Tells whether the thing shows for the player.
            /// @details Original: UpdateSpecFloorThing and DrawSpecFloorThing.
            /// @param owner True for the owner's thing, false for the visitor's.
            /// @param playerRoomMap Map id of the room the player is in.
            /// @return True when shown.
            bool isShown(bool owner, int playerRoomMap) const;

            /// @brief Item the thing shows.
            /// @param owner True for the owner's thing, false for the visitor's.
            /// @return Item id.
            int itemId(bool owner) const { return owner ? m_ownerItem : m_visitorItem; }

            /// @brief World point the icon's top-left corner stands on.
            /// @param owner True for the owner's thing, false for the visitor's.
            /// @return The point.
            core::Point place(bool owner) const { return owner ? m_ownerPlace : m_visitorPlace; }

            /// @brief Tells whether the icon is under a screen point.
            /// @param owner True for the owner's thing, false for the visitor's.
            /// @param screenPoint Point to test.
            /// @param camera Camera mapping world to screen.
            /// @return True when the point is on the icon.
            bool contains(bool owner, core::Point screenPoint, const Camera& camera) const;

            /// @brief Draws the icon, lit when hovered.
            /// @details Original: DrawSpecFloorThingPic.
            /// @param canvas Target canvas.
            /// @param owner True for the owner's thing, false for the visitor's.
            /// @param camera Camera mapping world to screen.
            /// @param hovered True when the mouse is on it.
            void draw(gfx::Canvas& canvas, bool owner, const Camera& camera, bool hovered) const;

            /// @brief Draws the item's name beside the mouse.
            /// @details Original: DrawSpecFloorThingHint.
            /// @param canvas Target canvas.
            /// @param owner True for the owner's thing, false for the visitor's.
            /// @param mouse Mouse position.
            void drawHint(gfx::Canvas& canvas, bool owner, core::Point mouse) const;

            /// @brief Picture of an item's icon, for the flight of a thing picked up.
            /// @param itemId Item id.
            /// @return The icon picture, or a fallback one.
            gfx::PictureId flightPicture(int itemId) const;

        private:
            /// @brief Looks up the icon picture of an item.
            /// @param itemId Item id.
            /// @return The picture id, or nothing when the item or picture is unknown.
            std::optional<gfx::PictureId> iconOf(int itemId) const;

            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            bool m_ownerShown = false;
            int m_ownerItem = 0;
            core::Point m_ownerPlace;
            int m_ownerRoomMap = 0;
            bool m_visitorShown = false;
            int m_visitorItem = 0;
            core::Point m_visitorPlace;
            int m_visitorRoomMap = 0;
    };

}

#endif
