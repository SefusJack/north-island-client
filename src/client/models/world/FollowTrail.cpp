#include "client/models/world/FollowTrail.h"

#include <cstdlib>

namespace client {

    void FollowTrail::scatter(core::Point at, std::mt19937& random) {
        constexpr int kSpread = 30; // size in px of the square (x and y) the trail points are scattered in
        std::uniform_int_distribution<int> offset(0, kSpread - 1);
        for (core::Point& point : m_points) {
            point = {at.x + offset(random), at.y + offset(random)};
        }
    }

    void FollowTrail::update(core::Point at) {
        constexpr int kStep = 20; // px the owner must move (on x or y) before a new trail point is recorded
        if (std::abs(m_points[0].x - at.x) < kStep && std::abs(m_points[0].y - at.y) < kStep) {
            return;
        }
        for (std::size_t i = m_points.size() - 1; i > 0; --i) {
            m_points[i] = m_points[i - 1];
        }
        m_points[0] = at;
    }

}
