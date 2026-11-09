#include "client/services/input/ScreenPointFinder.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

namespace client {

    std::optional<core::Point> ScreenPointFinder::find(const core::Rect& area, const core::Rect& screen, const Hits& hits) {
        const core::Rect clip = area.intersected(screen);
        if (clip.isEmpty()) {
            return std::nullopt;
        }
        std::vector<char> known(static_cast<std::size_t>(clip.width) * static_cast<std::size_t>(clip.height), 0);
        const auto hit = [&](core::Point p) {
            if (!clip.contains(p)) {
                return hits(p);
            }
            char& slot = known[static_cast<std::size_t>(p.y - clip.y) * static_cast<std::size_t>(clip.width) + static_cast<std::size_t>(p.x - clip.x)];
            if (slot == 0) {
                slot = hits(p) ? 1 : 2; // cache codes: 1 hits, 2 does not hit
            }
            return slot == 1;
        };
        const core::Point middle{clip.x + clip.width / 2, clip.y + clip.height / 2};
        const int reach = std::max({middle.x - clip.left(), clip.right() - middle.x, middle.y - clip.top(), clip.bottom() - middle.y});
        for (const int margin : {3, 1, 0}) { // neighbour margins in pixels, widest first
            for (int ring = 0; ring <= reach; ++ring) {
                for (int dy = -ring; dy <= ring; ++dy) {
                    const int step = std::abs(dy) == ring ? 1 : std::max(2 * ring, 1);
                    for (int dx = -ring; dx <= ring; dx += step) {
                        const core::Point p{middle.x + dx, middle.y + dy};
                        if (!clip.contains(p) || !hit(p)) {
                            continue;
                        }
                        bool all = true;
                        for (int ny = -margin; ny <= margin && all; ++ny) {
                            for (int nx = -margin; nx <= margin && all; ++nx) {
                                all = hit({p.x + nx, p.y + ny});
                            }
                        }
                        if (all) {
                            return p;
                        }
                    }
                }
            }
        }
        return std::nullopt;
    }

}
