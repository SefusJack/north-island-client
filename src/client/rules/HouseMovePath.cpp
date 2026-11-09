#include "client/rules/HouseMovePath.h"

#include "client/models/house/HouseGrid.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kColumnShift = HouseGrid::kGridColumn - 1;
        constexpr int kRowShift = HouseGrid::kGridRow - 1;
        constexpr int kFirstPassable = 241; // house cell values 241..248 count as walkable like an empty cell (0); original [0, 241..248]
        constexpr int kLastPassable = 248; // last walkable house cell value
        constexpr int kMostSteps = 100; // cap on path smoothing rounds

    }

    HouseMovePath::HouseMovePath(
        const HomeFurniture& furniture,
        int room,
        const core::Rect& view
    )
        : m_furniture(furniture),
          m_room(room),
          m_view(view),
          m_nodes(static_cast<std::size_t>(kNodes * kNodes)) {}

    core::Point HouseMovePath::absolute(core::Point cell) { return {cell.x + kColumnShift, cell.y + kRowShift}; }

    core::Point HouseMovePath::relative(core::Point cell) { return {cell.x - kColumnShift, cell.y - kRowShift}; }

    bool HouseMovePath::free(core::Point cell) const {
        const int held = m_furniture.houseCell(cell.x, cell.y, 1, m_room);
        return held == 0 || (held >= kFirstPassable && held <= kLastPassable);
    }

    bool HouseMovePath::inRoom(core::Point cell) const { return m_furniture.room(m_room).contains(cell.x, cell.y); }

    bool HouseMovePath::check(core::Point cell) const {
        const core::Point on = absolute(cell);
        if (!inRoom(on)) {
            return false;
        }
        const core::Point screen = HouseGrid::cellToScreen(on.x, on.y, 1);
        return !(m_view.left() > screen.x || m_view.top() > screen.y || m_view.right() < screen.x || m_view.bottom() < screen.y);
    }

    bool HouseMovePath::lineOk(core::Point from, core::Point to, core::Point& last) const {
        last = {-1, -1};
        if (from == to) {
            return true;
        }
        if (!inRoom(to)) {
            return false;
        }
        const int across = std::abs(from.x - to.x);
        const int down = std::abs(from.y - to.y);
        int x = from.x;
        int y = from.y;
        int previousX = from.x;
        int previousY = from.y;
        bool clear = true;
        if (across >= down) {
            double slope = static_cast<double>(down) / across;
            if (from.y > to.y) {
                slope = -slope;
            }
            for (int step = 1; step <= across && clear; ++step) {
                previousX = x;
                previousY = y;
                x = from.x + (from.x > to.x ? -step : step);
                y = static_cast<int>(step * slope + (from.y + 0.5));
                if (y != previousY && (!free({previousX, y}) || !free({x, previousY}))) {
                    clear = false;
                } else if (!free({x, y})) {
                    clear = false;
                }
            }
        } else {
            double slope = static_cast<double>(across) / down;
            if (from.x > to.x) {
                slope = -slope;
            }
            for (int step = 1; step <= down && clear; ++step) {
                previousX = x;
                previousY = y;
                x = static_cast<int>(step * slope + (from.x + 0.5));
                y = from.y + (from.y > to.y ? -step : step);
                if (x != previousX && (!free({previousX, y}) || !free({x, previousY}))) {
                    clear = false;
                } else if (!free({x, y})) {
                    clear = false;
                }
            }
        }
        if (!clear) {
            last = {previousX, previousY};
        }
        return clear;
    }

    double HouseMovePath::estimate(int index) const {
        const Node& open = m_nodes[static_cast<std::size_t>(index)];
        return open.g + open.h;
    }

    void HouseMovePath::sortOpen() {
        if (!m_open.empty()) {
            quickSort(0, static_cast<int>(m_open.size()) - 1);
        }
    }

    void HouseMovePath::quickSort(int low, int high) {
        const auto compare = [this](int first, int second) {
            const double a = estimate(first);
            const double b = estimate(second);
            return a < b ? -1 : a > b ? 1 : 0;
        };
        int left = low;
        int i = 0;
        do {
            i = left;
            int j = high;
            const int pivot = m_open[static_cast<std::size_t>((left + high) >> 1)];
            do {
                while (compare(m_open[static_cast<std::size_t>(i)], pivot) < 0) {
                    ++i;
                }
                while (compare(m_open[static_cast<std::size_t>(j)], pivot) > 0) {
                    --j;
                }
                if (i <= j) {
                    std::swap(m_open[static_cast<std::size_t>(i)], m_open[static_cast<std::size_t>(j)]);
                    ++i;
                    --j;
                }
            } while (i <= j);
            if (left < j) {
                quickSort(left, j);
            }
            left = i;
        } while (i < high);
    }

    void HouseMovePath::expand(core::Point cell, core::Point start, core::Point goal) {
        if (node(cell).closed != 0) {
            return;
        }
        const core::Point neighbors[] = {{cell.x, cell.y - 1}, {cell.x + 1, cell.y}, {cell.x, cell.y + 1}, {cell.x - 1, cell.y}};
        for (const core::Point next : neighbors) {
            if (!check(next)) {
                continue;
            }
            if (!free(absolute(next))) {
                node(next).closed = 1;
                continue;
            }
            const double toGoal = std::sqrt(std::pow(goal.x - next.x, 2) + std::pow(goal.y - next.y, 2));
            const double fromStart = std::sqrt(std::pow(start.x - next.x, 2) + std::pow(start.y - next.y, 2));
            const int steps = (node(cell).g + 1) & 0xFF; // the game keeps the step count in one byte
            bool better = true;
            if (node(next).g != 0) {
                const double oldCost = node(next).h + node(next).cost;
                const double newCost = toGoal + fromStart;
                if (oldCost <= newCost) {
                    better = false;
                }
            }
            if (!better) {
                continue;
            }
            Node& reached = node(next);
            reached.h = toGoal;
            reached.cost = fromStart;
            reached.parent = cell;
            reached.pos = next;
            reached.g = steps;
            if (reached.closed == 0) {
                m_open.push_back(next.x * kNodes + next.y);
                sortOpen();
                const double oldCost = reached.h + reached.cost;
                const double newCost = m_best.h + m_best.cost;
                if (oldCost < newCost) {
                    m_best = reached;
                } else if (oldCost == newCost && reached.h < m_best.h) {
                    m_best = reached;
                }
            }
        }
        node(cell).closed = 1;
    }

    void HouseMovePath::pop(core::Point start, core::Point goal) {
        while (!m_open.empty()) {
            const core::Point next = m_nodes[static_cast<std::size_t>(m_open.front())].pos;
            m_open.erase(m_open.begin());
            if (next == goal) {
                m_open.clear();
                break;
            }
            expand(next, start, goal);
        }
    }

    bool HouseMovePath::smooth(core::Point start) {
        core::Point last;
        int stepNo = 0;
        const int originalCount = m_length;
        if (m_length <= 2) {
            return false;
        }
        int fromIndex = 1;
        core::Point from = m_path[1];
        int pathLength = 1;
        int toIndex = fromIndex + 1;
        if (lineOk(absolute(from), absolute(start), last)) {
            for (int k = 2; k <= m_length; ++k) {
                m_path[static_cast<std::size_t>(k)] = {};
            }
            ++pathLength;
            m_path[static_cast<std::size_t>(pathLength)] = start;
            m_length = pathLength;
            return true;
        }
        while (stepNo <= m_length && fromIndex != 0) {
            ++stepNo;
            toIndex = fromIndex + 1;
            const int count = m_length;
            for (int j = fromIndex + 1; j <= count; ++j) {
                core::Point to = m_path[static_cast<std::size_t>(j)];
                if (lineOk(absolute(from), absolute(to), last)) {
                    toIndex = j;
                } else {
                    to = m_path[static_cast<std::size_t>(toIndex)];
                    for (int k = fromIndex + 1; k <= toIndex; ++k) {
                        m_path[static_cast<std::size_t>(k)] = {};
                    }
                    ++pathLength;
                    m_path[static_cast<std::size_t>(pathLength)] = to;
                    fromIndex = toIndex;
                    from = to;
                    if (lineOk(absolute(from), absolute(start), last)) {
                        for (int k = fromIndex + 1; k <= count; ++k) {
                            m_path[static_cast<std::size_t>(k)] = {};
                        }
                        ++pathLength;
                        m_path[static_cast<std::size_t>(pathLength)] = start;
                        if (m_length == pathLength) {
                            return false;
                        }
                        m_length = pathLength;
                        return true;
                    }
                    break;
                }
                if (toIndex == count) {
                    to = m_path[static_cast<std::size_t>(toIndex)];
                    for (int k = fromIndex + 1; k <= toIndex; ++k) {
                        m_path[static_cast<std::size_t>(k)] = {};
                    }
                    ++pathLength;
                    m_path[static_cast<std::size_t>(pathLength)] = to;
                    fromIndex = 0;
                    break;
                }
                if (fromIndex != 1 && lineOk(absolute(to), absolute(start), last)) {
                    const core::Point corner = m_path[static_cast<std::size_t>(j)];
                    for (int k = fromIndex + 1; k <= j; ++k) {
                        m_path[static_cast<std::size_t>(k)] = {};
                    }
                    ++pathLength;
                    m_path[static_cast<std::size_t>(pathLength)] = corner;
                    for (int k = j + 1; k <= count; ++k) {
                        m_path[static_cast<std::size_t>(k)] = {};
                    }
                    if (corner != to) {
                        ++pathLength;
                        m_path[static_cast<std::size_t>(pathLength)] = to;
                    }
                    ++pathLength;
                    m_path[static_cast<std::size_t>(pathLength)] = start;
                    fromIndex = 0;
                    break;
                }
            }
        }
        m_length = pathLength;
        return originalCount != m_length;
    }

    void HouseMovePath::walk(core::Point start, core::Point goal) {
        const core::Point first = relative(start);
        core::Point end = relative(goal);
        if (first == end) {
            return;
        }
        core::Point last;
        if (lineOk(absolute(first), absolute(end), last)) {
            m_path[static_cast<std::size_t>(++m_length)] = first;
            m_path[static_cast<std::size_t>(++m_length)] = end;
            return;
        }
        Node& origin = node(first);
        origin.pos = first;
        origin.g = 0;
        origin.h = std::abs(end.x - first.x) + std::abs(end.y - first.y);
        m_best = origin;
        expand(first, first, end);
        pop(first, end);
        if (!inRoom(absolute(end)) && (m_best.g != 0 || m_best.h != 0)) {
            if (m_best.pos == first) {
                return;
            }
            end = m_best.pos;
        }
        if (node(end).parent.x == 0 && (m_best.g != 0 || m_best.h != 0)) {
            if (m_best.pos == first) {
                return;
            }
            end = m_best.pos;
        }
        m_path[static_cast<std::size_t>(++m_length)] = first;
        core::Point cell = end;
        m_length = std::min(node(cell).g + 1, kPathMost);
        for (int j = m_length; j >= 2; --j) {
            m_path[static_cast<std::size_t>(j)] = node(cell).pos;
            cell = node(cell).parent;
        }
        int rounds = 0;
        while (smooth(end) && rounds < kMostSteps) {
            ++rounds;
        }
    }

    bool HouseMovePath::boxedIn(core::Point cell) const {
        for (const core::Point around :
             {cell, core::Point{cell.x - 1, cell.y}, core::Point{cell.x + 1, cell.y}, core::Point{cell.x, cell.y - 1}, core::Point{cell.x, cell.y + 1}}) {
            if (inRoom(around) && free(around)) {
                return false;
            }
        }
        return true;
    }

    void HouseMovePath::search(core::Point from, core::Point to) {
        core::Point last;
        if (boxedIn(from) && inRoom(to) && free(to)) {
            m_path[static_cast<std::size_t>(++m_length)] = relative(from);
            m_path[static_cast<std::size_t>(++m_length)] = relative(to);
        } else if (!lineOk(from, to, last)) {
            if (last.x == -1 && last.y == -1) {
                walk(from, to);
            } else {
                walk(last, to);
                if (m_length == 0) {
                    m_length = 1;
                    m_path[1] = relative(last);
                }
                for (int j = m_length; j >= 1; --j) {
                    m_path[static_cast<std::size_t>(j + 1)] = m_path[static_cast<std::size_t>(j)];
                }
                ++m_length;
                m_path[1] = relative(from);
            }
        } else {
            m_path[static_cast<std::size_t>(++m_length)] = relative(from);
            m_path[static_cast<std::size_t>(++m_length)] = relative(to);
        }
    }

    std::vector<core::Point> HouseMovePath::find(core::Point from, core::Point to) {
        const HouseGrid::Cell start = HouseGrid::screenToCell(from);
        const HouseGrid::Cell goal = HouseGrid::screenToCell(to);
        m_length = 0;
        search({start.x, start.y}, {goal.x, goal.y});
        if (m_length == 2 && m_path[1] == m_path[2]) {
            return {};
        }
        for (int i = 2; i <= m_length; ++i) {
            if (!check(m_path[static_cast<std::size_t>(i)])) {
                m_length = 0;
                break;
            }
        }
        std::vector<core::Point> points;
        for (int i = 1; i <= m_length; ++i) {
            const core::Point cell = absolute(m_path[static_cast<std::size_t>(i)]);
            core::Point screen = HouseGrid::cellToScreen(cell.x, cell.y, 1);
            screen.y += HouseGrid::kCellHeight / 2;
            points.push_back(screen);
        }
        return points;
    }

}
