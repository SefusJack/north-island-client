#include "client/rendering/painters/VehicleBoxPainter.h"

#include "client/rendering/painters/ItemCells.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kCellSize = 32; // px: side of one inventory cell / item icon
        constexpr int kCellGap = 2; // px: gap between neighbouring cells of a multi-cell item

    }

    void VehicleBoxPainter::paint(GameServices& services, gfx::Canvas& canvas, core::Point origin, const protocol::ItemInstance& item) {
        const ui::IconCell cell = itemIconCell(services, item, false, false, true);
        if (!cell.picture.isValid()) {
            return;
        }
        const auto record = services.items.find(item.itemId);
        const int columns = record ? std::max(1, record->cellWidth) : 1;
        const int rows = record ? std::max(1, record->cellHeight) : 1;
        const int cellWidth = columns * kCellSize + (columns - 1) * kCellGap;
        const int cellHeight = rows * kCellSize + (rows - 1) * kCellGap;
        const gfx::Picture& picture = services.pictures.picture(cell.picture);
        // 2: pixels of the picture's border that are not part of the middle
        int sourceX = std::max(0, (picture.width() - 2) / 2 - cellWidth / 2);
        int sourceY = std::max(0, (picture.height() - 2) / 2 - cellHeight / 2);
        sourceX -= (kSide - cellWidth) / 2;
        sourceY -= (kSide - cellHeight) / 2;
        const int left = std::max(sourceX, 0);
        const int top = std::max(sourceY, 0);
        const int right = std::min(sourceX + kSide, picture.width());
        const int bottom = std::min(sourceY + kSide, picture.height());
        if (right > left && bottom > top) {
            canvas.drawPicture(cell.picture, {origin.x + left - sourceX, origin.y + top - sourceY}, core::Rect{left, top, right - left, bottom - top});
        }
        for (const ui::IconOverlay& overlay : cell.overlays) {
            canvas.drawPicture(overlay.picture, origin + overlay.offset, overlay.source);
        }
    }

}
