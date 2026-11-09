#include "client/rendering/painters/ScaledItemIcon.h"

#include "client/rules/FurnitureKinds.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kCellSize = 32; // px: side of one inventory cell / item icon
        constexpr int kCellGap = 2; // px: gap between neighbouring cells of a multi-cell item

        /// @brief Scales a size and rounds it to the nearest integer.
        /// @param size Size in pixels.
        /// @param scale Scale factor.
        /// @return The scaled size.
        int shrunk(int size, double scale) { return static_cast<int>(std::nearbyint(size * scale)); }

    }

    ScaledItemIcon scaledItemIcon(GameServices& services, int itemId, core::Size area) {
        ScaledItemIcon icon;
        const auto record = services.items.find(itemId);
        if (!record) {
            return icon;
        }
        icon.picture = services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{});
        if (!icon.picture.isValid()) {
            return icon;
        }
        const gfx::Picture& picture = services.pictures.picture(icon.picture);
        icon.size = {picture.width(), picture.height()};
        if (!FurnitureKinds::isLargeItem(services.items, itemId)) {
            return icon;
        }
        const int width = record->cellWidth * kCellSize + (record->cellWidth - 1) * kCellGap;
        const int height = record->cellHeight * kCellSize + (record->cellHeight - 1) * kCellGap;
        // 2: pixels of the picture's border that are not part of the middle
        const int left = std::max(0, (picture.width() - 2) / 2 - width / 2);
        const int top = std::max(0, (picture.height() - 2) / 2 - height / 2);
        icon.source = core::Rect{left, top, width, height};
        icon.size = {width, height};
        if (width > area.width || height > area.height) {
            const double scale = std::abs(width - area.width) >= std::abs(height - area.height) ? static_cast<double>(area.width) / width
                                                                                                : static_cast<double>(area.height) / height;
            const double capped = std::min(scale, 1.0);
            icon.size = {shrunk(width, capped), shrunk(height, capped)};
        }
        return icon;
    }

}
