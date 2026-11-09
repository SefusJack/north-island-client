#include "client/models/house/SpecFloorThing.h"

namespace client {

    namespace {

        constexpr int kHoverLight = 15; // brightness added to the piece under the mouse
        const core::Color kHintColor = core::Color::fromRgb565(65504); // RGB565 0xFFE0: yellow

        // world pixel position of places 1..3; rows are room kinds 1..2
        constexpr int kPlaceX[2][3] = {{622, 702, 542}, {422, 502, 372}};
        constexpr int kPlaceY[2][3] = {{575, 605, 615}, {595, 635, 635}};
        // map 10081: Cherry Grassplot, map 10083: Summer Beach, map 10084: Happy House
        constexpr int kRoomMaps[3] = {10081, 10083, 10084};

        /// @brief World point of a place in a room kind.
        /// @details Original: SpecFloorPosX / SpecFloorPosY tables; kind 3 reads past the tables in the original, so nothing shows for it.
        /// @param kind Room kind 1..2.
        /// @param place Place 1..3.
        /// @return The point, or nothing when kind or place is out of range.
        std::optional<core::Point> placeOf(int kind, int place) {
            // kinds 1..2 and places 1..3 are the table bounds
            if (kind < 1 || kind > 2 || place < 1 || place > 3) {
                return std::nullopt;
            }
            return core::Point{kPlaceX[kind - 1][place - 1], kPlaceY[kind - 1][place - 1]};
        }

    }

    void SpecFloorThing::receive(
        int kind,
        int itemId,
        int place,
        std::int32_t ownerId,
        std::int32_t ownId,
        int playerRoom
    ) {
        if (kind == 0 || place == 0) {
            return;
        }
        const auto at = placeOf(kind, place);
        const int roomMap = kind >= 1 && kind <= 3 ? kRoomMaps[kind - 1] : 0; // room kinds 1..3 index kRoomMaps
        if (ownId == ownerId) {
            m_ownerShown = true;
            if (at) {
                m_ownerItem = itemId;
                m_ownerPlace = *at;
                m_ownerRoomMap = roomMap;
            }
        } else if (playerRoom == kind + 1) {
            m_visitorShown = true;
            if (at) {
                m_visitorItem = itemId;
                m_visitorPlace = *at;
                m_visitorRoomMap = roomMap;
            }
        }
    }

    void SpecFloorThing::showVisitor(
        int room,
        int place,
        int itemId,
        bool entering,
        int playerRoom
    ) {
        if (playerRoom != room + 1) {
            return;
        }
        m_visitorShown = entering;
        // only rooms 1 and 2 have place tables
        if (entering && room != 0 && place != 0 && (room == 1 || room == 2)) {
            m_visitorItem = itemId;
            m_visitorPlace = *placeOf(room, place);
            m_visitorRoomMap = kRoomMaps[room - 1];
        }
    }

    void SpecFloorThing::hide() {
        m_ownerShown = false;
        m_visitorShown = false;
    }

    bool SpecFloorThing::isShown(bool owner, int playerRoomMap) const {
        if (owner) {
            return m_ownerShown && playerRoomMap == m_ownerRoomMap && iconOf(m_ownerItem);
        }
        return m_visitorShown && playerRoomMap == m_visitorRoomMap && iconOf(m_visitorItem);
    }

    std::optional<gfx::PictureId> SpecFloorThing::iconOf(int itemId) const {
        const auto record = m_items.find(itemId);
        if (!record) {
            return std::nullopt;
        }
        return m_pictures.find(std::to_string(record->iconId));
    }

    gfx::PictureId SpecFloorThing::flightPicture(int itemId) const {
        const auto record = m_items.find(itemId);
        if (!record) {
            return {};
        }
        return m_pictures.find(std::to_string(record->iconId)).value_or(m_pictures.find("1000").value_or(gfx::PictureId{}));
    }

    bool SpecFloorThing::contains(bool owner, core::Point screenPoint, const Camera& camera) const {
        const auto icon = iconOf(itemId(owner));
        if (!icon) {
            return false;
        }
        const core::Point at = camera.toScreen(place(owner));
        const gfx::Picture& picture = m_pictures.picture(*icon);
        return screenPoint.x >= at.x && screenPoint.x <= at.x + picture.width() && screenPoint.y >= at.y && screenPoint.y <= at.y + picture.height();
    }

    void SpecFloorThing::draw(gfx::Canvas& canvas, bool owner, const Camera& camera, bool hovered) const {
        const auto icon = iconOf(itemId(owner));
        if (!icon) {
            return;
        }
        const core::Point at = camera.toScreen(place(owner));
        canvas.drawPicture(*icon, at);
        if (hovered) {
            canvas.drawPictureLight(*icon, at, kHoverLight);
        }
    }

    void SpecFloorThing::drawHint(gfx::Canvas& canvas, bool owner, core::Point mouse) const {
        const auto record = m_items.find(itemId(owner));
        if (!record) {
            return;
        }
        // hint offset from the mouse (+15, -15); text width is 8 px per character; 12 is the font height
        canvas.drawText(
            {mouse.x + 15, mouse.y - 15},
            record->name,
            {kHintColor, gfx::TextEffect::Shadow, std::nullopt},
            static_cast<int>(record->name.size()) * 8,
            12
        );
    }

}
