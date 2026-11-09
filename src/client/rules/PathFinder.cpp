#include "client/rules/PathFinder.h"

#include "client/rules/CellDistances.h"
#include "formats/MapFile.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kCell = formats::MapFile::kCellSize; // map cell size in px
        constexpr std::size_t kMaxPoints = 70; // most waypoints a found path may have

        /// @brief The cell a map pixel lies in.
        /// @param mapPixel A map point in pixels.
        /// @return The cell, not below 0.
        core::Point cellOf(core::Point mapPixel) { return {std::max(0, mapPixel.x / kCell), std::max(0, mapPixel.y / kCell)}; }

        /// @brief The point a walker stands on within a cell.
        /// @param cell The cell.
        /// @return The anchor point.
        core::Point anchorOf(core::Point cell) { return {cell.x * kCell + 2, cell.y * kCell + 15}; } // anchor offset inside the cell: x + 2, y + 15 pixels

        /// @brief The cells of a line, start and end included.
        /// @param from Start cell.
        /// @param to End cell.
        /// @return The cells in order.
        std::vector<core::Point> lineCells(core::Point from, core::Point to) {
            std::vector<core::Point> cells;
            const int dx = std::abs(to.x - from.x);
            const int dy = -std::abs(to.y - from.y);
            const int sx = from.x < to.x ? 1 : -1;
            const int sy = from.y < to.y ? 1 : -1;
            int error = dx + dy;
            core::Point at = from;
            while (true) {
                cells.push_back(at);
                if (at.x == to.x && at.y == to.y) {
                    break;
                }
                const int twice = 2 * error;
                if (twice >= dy) {
                    error += dy;
                    at.x += sx;
                }
                if (twice <= dx) {
                    error += dx;
                    at.y += sy;
                }
            }
            return cells;
        }

    }

    bool isLineClear(core::Point fromCell, core::Point toCell, const std::function<bool(core::Point)>& blocked) {
        const int dx = std::abs(fromCell.x - toCell.x);
        const int dy = std::abs(fromCell.y - toCell.y);
        const bool alongX = dx >= dy;
        const double slope = alongX ? dy / (dx + 0.01) : dx / (dy + 0.01); // 0.01: the original's fudge added to the longer side
        const core::Point left = fromCell.x > toCell.x ? toCell : fromCell;
        const core::Point right = fromCell.x > toCell.x ? fromCell : toCell;
        const bool goingDown = right.y > left.y;
        for (int i = 0; i <= (alongX ? dx : dy); ++i) {
            const int ceiled = static_cast<int>(std::ceil(i * slope));
            const int floored = static_cast<int>(std::floor(i * slope));
            for (const int offset : {ceiled, floored}) {
                const core::Point cell = alongX ? core::Point{left.x + i, goingDown ? left.y + offset : left.y - offset}
                                                : core::Point{left.x + offset, goingDown ? left.y + i : left.y - i};
                if (blocked(cell)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool isLineWalkable(const WorldMap& map, core::Point fromCell, core::Point toCell, int walkClass) {
        return isLineClear(fromCell, toCell, [&](core::Point cell) { return !map.isWalkable({cell.x * kCell, cell.y * kCell}, walkClass); });
    }

    std::vector<core::Point> findPath(const WorldMap& map, core::Point from, core::Point to, int walkClass) {
        const core::Point standing = cellOf(from);
        core::Point start = standing;
        core::Point target = cellOf(to);
        if (std::abs(target.x - start.x) > CellDistances::kReach || std::abs(target.y - start.y) > CellDistances::kReach) {
            return {};
        }
        if (!map.isWalkable({start.x * kCell, start.y * kCell}, walkClass)) {
            const std::vector<core::Point> ahead = lineCells(start, target);
            const auto walkable =
                std::find_if(ahead.begin(), ahead.end(), [&](core::Point cell) { return map.isWalkable({cell.x * kCell, cell.y * kCell}, walkClass); });
            if (walkable == ahead.end()) {
                return {};
            }
            start = *walkable;
        }
        const CellDistances distances(map, start, walkClass);
        if (distances.at(target) == CellDistances::kUnreached) {
            const std::vector<core::Point> back = lineCells(target, start);
            const auto reachable = std::find_if(back.begin(), back.end(), [&](core::Point cell) { return distances.at(cell) != CellDistances::kUnreached; });
            target = reachable != back.end() ? *reachable : start;
        }
        if (target.x == start.x && target.y == start.y) {
            const bool alreadyThere = target.x == standing.x && target.y == standing.y;
            return alreadyThere ? std::vector<core::Point>{} : std::vector<core::Point>{anchorOf(target)};
        }
        if (isLineWalkable(map, start, target, walkClass)) {
            return {anchorOf(target)};
        }

        const std::vector<core::Point> cells = distances.trace(target);
        const auto unreached = [&](core::Point cell) { return distances.inWindow(cell) && distances.at(cell) == CellDistances::kUnreached; };
        std::vector<core::Point> corners;
        std::size_t corner = 0;
        while (corner + 1 < cells.size() && corners.size() < kMaxPoints) {
            std::size_t farthest = corner + 1;
            for (std::size_t i = cells.size() - 1; i > corner + 1; --i) {
                if (isLineClear(cells[corner], cells[i], unreached)) {
                    farthest = i;
                    break;
                }
            }
            corners.push_back(anchorOf(cells[farthest]));
            corner = farthest;
        }
        return corners;
    }

}
