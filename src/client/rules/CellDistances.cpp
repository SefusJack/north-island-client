#include "client/rules/CellDistances.h"

#include "formats/MapFile.h"

#include <algorithm>
#include <deque>

namespace client {

    namespace {

        constexpr int kCell = formats::MapFile::kCellSize;
        const std::array<core::Point, 4> kNeighbours = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    }

    CellDistances::CellDistances(
        const WorldMap& map,
        core::Point start,
        int walkClass
    )
        : m_origin{start.x - kReach, start.y - kReach} {
        m_steps.fill(kUnreached);
        std::deque<core::Point> queue{start};
        m_steps[index(start)] = 0;
        while (!queue.empty()) {
            const core::Point cell = queue.front();
            queue.pop_front();
            for (const core::Point step : kNeighbours) {
                const core::Point next{cell.x + step.x, cell.y + step.y};
                if (inWindow(next) && at(next) == kUnreached && map.isWalkable({next.x * kCell, next.y * kCell}, walkClass)) {
                    m_steps[index(next)] = at(cell) + 1;
                    queue.push_back(next);
                }
            }
        }
    }

    bool CellDistances::inWindow(core::Point cell) const {
        const int x = cell.x - m_origin.x;
        const int y = cell.y - m_origin.y;
        return x >= 0 && y >= 0 && x < kWindow && y < kWindow;
    }

    std::size_t CellDistances::index(core::Point cell) const { return static_cast<std::size_t>((cell.y - m_origin.y) * kWindow + (cell.x - m_origin.x)); }

    int CellDistances::at(core::Point cell) const { return inWindow(cell) ? m_steps[index(cell)] : kUnreached; }

    std::vector<core::Point> CellDistances::trace(core::Point cell) const {
        std::vector<core::Point> cells{cell};
        while (at(cell) > 0) {
            for (const core::Point step : kNeighbours) {
                const core::Point next{cell.x + step.x, cell.y + step.y};
                if (at(next) == at(cell) - 1) {
                    cell = next;
                    break;
                }
            }
            cells.push_back(cell);
        }
        std::reverse(cells.begin(), cells.end());
        return cells;
    }

}
