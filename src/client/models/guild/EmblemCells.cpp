#include "client/models/guild/EmblemCells.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        /// @brief Rounds like Delphi's Round (halves go to the even number).
        /// @param value Value to round.
        /// @return The rounded integer.
        int delphiRound(double value) { return static_cast<int>(std::nearbyint(value)); }

    }

    std::vector<core::Point> lineCells(core::Point from, core::Point to) {
        std::vector<core::Point> cells;
        if (from.x < 0) {
            return cells;
        }
        if (to.x == from.x) {
            const int count = std::abs(to.y - from.y);
            for (int i = 0; i <= count; ++i) {
                cells.push_back({from.x, i + std::min(from.y, to.y)});
            }
        } else if (to.y == from.y) {
            const int count = std::abs(to.x - from.x);
            for (int i = 0; i <= count; ++i) {
                cells.push_back({i + std::min(from.x, to.x), from.y});
            }
        } else if (std::abs(to.x - from.x) > std::abs(to.y - from.y)) {
            const int count = std::abs(to.x - from.x);
            for (int i = 0; i <= count; ++i) {
                const int x = i + std::min(from.x, to.x);
                cells.push_back({x, from.y + delphiRound(static_cast<double>((x - from.x) * (to.y - from.y)) / (to.x - from.x))});
            }
        } else {
            const int count = std::abs(to.y - from.y);
            for (int i = 0; i <= count; ++i) {
                const int y = i + std::min(from.y, to.y);
                cells.push_back({from.x + delphiRound(static_cast<double>((y - from.y) * (to.x - from.x)) / (to.y - from.y)), y});
            }
        }
        return cells;
    }

    std::vector<core::Point> rectangleCells(core::Point from, core::Point to) {
        std::vector<core::Point> cells;
        if (from.x < 0) {
            return cells;
        }
        const int width = std::abs(to.x - from.x) + 1;
        const int sides = std::max(0, std::abs(to.y - from.y) - 1);
        const int left = std::min(from.x, to.x);
        for (int i = 0; i < width; ++i) {
            cells.push_back({left + i, from.y});
            cells.push_back({left + i, to.y});
        }
        const int top = std::min(from.y, to.y);
        for (int i = 0; i < sides; ++i) {
            cells.push_back({from.x, top + 1 + i});
            cells.push_back({to.x, top + 1 + i});
        }
        return cells;
    }

    std::vector<core::Point> ellipseCells(core::Point from, core::Point to) {
        std::vector<core::Point> cells;
        const int width = std::abs(from.x - to.x);
        const int height = std::abs(from.y - to.y);
        const float originX = static_cast<float>(std::min(from.x, to.x));
        const float originY = static_cast<float>(std::min(from.y, to.y));
        const float centreX = originX + static_cast<float>(width) / 2;
        const float centreY = originY + static_cast<float>(height) / 2;
        std::vector<core::Point> columns(static_cast<std::size_t>(width) * 2);
        for (int i = 0; i < width; ++i) {
            const float x = static_cast<float>(i) + originX;
            const float t = static_cast<float>(std::fabs(1 - std::pow((x - centreX) / (static_cast<float>(width) / 2), 2)));
            const float low = centreY - static_cast<float>(std::sqrt(t)) * (static_cast<float>(height) / 2);
            const float high = centreY + static_cast<float>(std::sqrt(t)) * (static_cast<float>(height) / 2);
            columns[static_cast<std::size_t>(i)] = {delphiRound(x), delphiRound(low)};
            columns[static_cast<std::size_t>(width * 2 - 1 - i)] = {delphiRound(x), delphiRound(high)};
        }
        std::vector<core::Point> rows(static_cast<std::size_t>(height) * 2);
        for (int i = 0; i < height; ++i) {
            const float y = static_cast<float>(i) + originY;
            const float t = static_cast<float>(std::fabs(1 - std::pow((y - centreY) / (static_cast<float>(height) / 2), 2)));
            const float low = centreX - static_cast<float>(std::sqrt(t)) * (static_cast<float>(width) / 2);
            const float high = centreX + static_cast<float>(std::sqrt(t)) * (static_cast<float>(width) / 2);
            rows[static_cast<std::size_t>(i)] = {delphiRound(low), delphiRound(y)};
            rows[static_cast<std::size_t>(height * 2 - 1 - i)] = {delphiRound(high), delphiRound(y)};
        }
        cells = columns;
        cells.insert(cells.end(), rows.begin(), rows.end());
        return cells;
    }

}
