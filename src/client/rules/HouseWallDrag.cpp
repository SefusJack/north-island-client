#include "client/rules/HouseWallDrag.h"

#include "client/rules/FurnitureKinds.h"
#include "client/models/house/HousePillars.h"

namespace client {

    namespace {

        constexpr int kWallItem = 39041; // item 39041: Wallpaper, whose picture is hit-tested for wall cells
        constexpr int kWallBox = 300; // width and height of the wall picture hit box, px
        constexpr int kPictureDrop = 15; // downward shift of the wall/pillar picture relative to its cell centre, px

    }

    bool HouseWallDrag::onWallCell(int column, int row, core::Point mapMouse) const {
        const bool left = column == 0;
        const gfx::PictureId picture = m_interior.utensils().facingPicture(kWallItem, left ? 1 : 0);
        if (!picture.isValid()) {
            return false;
        }
        const core::Point world = left ? HouseGrid::cellToScreen(column + HouseGrid::kGridColumn, row + HouseGrid::kGridRow - 1, 1)
                                       : HouseGrid::cellToScreen(column + HouseGrid::kGridColumn - 1, row + HouseGrid::kGridRow, 1);
        const core::Point origin{world.x - kWallBox / 2, world.y - kWallBox / 2 + kPictureDrop};
        const core::Point local{mapMouse.x - origin.x, mapMouse.y - origin.y};
        return local.x >= 0 && local.y >= 0 && local.x < kWallBox && local.y < kWallBox && m_interior.utensils().isOpaque(picture, local);
    }

    bool HouseWallDrag::onPillar(int column, int row, core::Point mapMouse) const {
        const gfx::PictureId picture = m_interior.surfaces().pillarPicture();
        if (!picture.isValid()) {
            return false;
        }
        const core::Point size = m_interior.utensils().pictureSize(picture);
        const core::Point world = HousePillars::worldOf(column, row);
        const core::Point local{mapMouse.x - (world.x - size.x / 2), mapMouse.y - (world.y - size.y / 2 + kPictureDrop)};
        return local.x >= 0 && local.y >= 0 && local.x < size.x && local.y < size.y && m_interior.utensils().isOpaque(picture, local);
    }

    core::Point HouseWallDrag::hitCell(core::Point mouse, const Camera& camera) const {
        const HouseRoomRect& room = m_interior.furniture().room(m_interior.room());
        const int topRow = HousePillars::kRows - (room.bottom - room.top + 1);
        const core::Point mapMouse = camera.toMap(mouse);
        for (int row = HousePillars::kRows; row >= topRow; --row) {
            for (int column = HousePillars::kColumns; column >= 0; --column) {
                if (column == 0 && row == topRow) {
                    continue;
                }
                const bool wall = column == 0 || row == topRow;
                if (wall ? onWallCell(column, row, mapMouse) : (m_interior.pillars().has(column, row, m_interior.room()) && onPillar(column, row, mapMouse))) {
                    return {column, row};
                }
            }
        }
        return {0, 0};
    }

    std::optional<HouseWallDrag::Place> HouseWallDrag::place(int itemId, core::Point rowCol, const HouseGrid::Cell& under, core::Point mapMouse) const {
        const int level = FurnitureKinds::wallLevel(m_items, itemId);
        if (level == 0) {
            return std::nullopt;
        }
        const HouseRoomRect& room = m_interior.furniture().room(m_interior.room());
        const HousePillars& pillars = m_interior.pillars();
        const int floor = m_interior.room();
        const int leftColumn = room.left - HouseGrid::kGridColumn;
        const int topRow = room.top - HouseGrid::kGridRow;
        const int x = rowCol.x;
        const int y = rowCol.y;
        int side = 0;
        if (leftColumn == x && topRow < y) {
            if (!pillars.has(1, y, floor)) {
                side = 2;
            }
        } else if (leftColumn < x && topRow == y) {
            if (!pillars.has(x, 1, floor)) {
                side = 1;
            }
        } else if (leftColumn < x && topRow < y) {
            if (room.bottom > y + HouseGrid::kGridRow - 1 && !pillars.has(x, y + 1, floor)) {
                ++side;
            }
            if (room.right > x + HouseGrid::kGridColumn - 1 && !pillars.has(x + 1, y, floor)) {
                side += 2;
            }
            if (side == 3) {
                const core::Point middle = HouseGrid::cellToScreen(x + HouseGrid::kGridColumn - 1, y + HouseGrid::kGridRow - 1, level);
                side = mapMouse.x < middle.x ? 1 : 2;
            }
        }
        Place result;
        result.cell = under;
        if (side == 1) {
            result.cell = {x + HouseGrid::kGridColumn - 1, y + HouseGrid::kGridRow, level};
        } else if (side == 2) {
            result.direction = 1;
            result.cell = {x + HouseGrid::kGridColumn, y + HouseGrid::kGridRow - 1, level};
        }
        return result;
    }

}
