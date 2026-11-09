#include "core/ThickLine.h"

#include <algorithm>
#include <cmath>

namespace core {

    std::vector<Rect> thickLineRows(Point from, Point to, int width) {
        std::vector<Rect> rows;
        if (width < 1) {
            return rows;
        }
        const double radius = width / 2.0;
        const int reach = static_cast<int>(std::ceil(radius));
        const int left = std::min(from.x, to.x) - reach;
        const int right = std::max(from.x, to.x) + reach;
        const int top = std::min(from.y, to.y) - reach;
        const int bottom = std::max(from.y, to.y) + reach;
        const double dx = to.x - from.x;
        const double dy = to.y - from.y;
        const double lengthSquared = dx * dx + dy * dy;
        for (int y = top; y <= bottom; ++y) {
            int first = 0;
            int count = 0;
            for (int x = left; x <= right; ++x) {
                const double px = x + 0.5 - (from.x + 0.5);
                const double py = y + 0.5 - (from.y + 0.5);
                const double along = lengthSquared == 0 ? 0 : std::clamp((px * dx + py * dy) / lengthSquared, 0.0, 1.0);
                const double ex = px - along * dx;
                const double ey = py - along * dy;
                if (ex * ex + ey * ey <= radius * radius) {
                    if (count == 0) {
                        first = x;
                    }
                    ++count;
                }
            }
            if (count > 0) {
                rows.push_back({first, y, count, 1});
            }
        }
        return rows;
    }

}
