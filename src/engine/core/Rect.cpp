#include "core/Rect.h"

#include <algorithm>

namespace core {

    Rect Rect::intersected(const Rect& other) const {
        const int l = std::max(left(), other.left());
        const int t = std::max(top(), other.top());
        const int r = std::min(right(), other.right());
        const int b = std::min(bottom(), other.bottom());
        if (r <= l || b <= t) {
            return {};
        }
        return fromEdges(l, t, r, b);
    }

}
