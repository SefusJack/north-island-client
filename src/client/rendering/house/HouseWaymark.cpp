#include "client/rendering/house/HouseWaymark.h"

#include "ui/WrappedTooltip.h"

namespace client {

    namespace {

        constexpr core::Point kDoorOffset{50, 12}; // px: exit marker offset from the door corner
        constexpr int kStates = 3; // WayMark picture holds 3 stacked frames (0 normal, 1 hovered)
        constexpr int kTipChars = 4; // characters per tooltip line
        constexpr int kTipLineHeight = 8; // px per tooltip line
        constexpr int kTipAbove = 20; // px: tooltip sits this far above the marker
        const core::Color kTipFill{61, 139, 249, 200}; // tooltip fill: light blue, RGBA

    }

    HouseWaymark::HouseWaymark(
        gfx::PictureLibrary& pictures
    )
        : m_pictures(pictures),
          m_picture(pictures.find("WayMark").value_or(gfx::PictureId{})) {}

    void HouseWaymark::setDoor(core::Point doorCorner) { m_position = doorCorner + kDoorOffset; }

    core::Rect HouseWaymark::area(const Camera& camera) const {
        if (!m_picture.isValid()) {
            return {};
        }
        const gfx::Picture& data = m_pictures.picture(m_picture);
        const core::Point at = camera.toScreen(m_position);
        return {at.x, at.y, data.width(), data.height() / kStates};
    }

    void HouseWaymark::paint(gfx::Canvas& canvas, const Camera& camera, const std::optional<core::Point>& mouse) {
        if (!m_visible || !m_picture.isValid()) {
            return;
        }
        const core::Rect box = area(camera);
        m_state = mouse && mouse->x > box.x && mouse->y > box.y && box.contains(*mouse) ? 1 : 0;
        canvas.drawPicture(m_picture, {box.x, box.y}, core::Rect{0, m_state * box.height, box.width, box.height});
        if (m_state == 1) {
            ui::drawWrappedTooltip(
                canvas,
                {box.x, box.y - kTipAbove},
                "Exit",
                kTipChars,
                kTipLineHeight,
                {255, 255, 255, 255},
                kTipFill,
                {0, 0, 255, 255}
            ); // tip text colour white, border colour blue (RGBA)
        }
    }

    std::optional<core::Point> HouseWaymark::click(const Camera& camera, core::Point screenPoint) const {
        if (!m_visible) {
            return std::nullopt;
        }
        const core::Rect box = area(camera);
        if (screenPoint.x > box.x && screenPoint.y > box.y && box.contains(screenPoint)) {
            return m_position - kDoorOffset;
        }
        return std::nullopt;
    }

}
