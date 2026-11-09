#include "client/viewmodels/login/RoleCarousel.h"

#include "client/data/StarterRoles.h"
#include "core/Point.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        /// @brief One place in the carousel: which role stands there, where its feet are and where its picture stops.
        struct Spot {
            int role;
            core::Point foot;
            int stopLeft;
        };

        // per spot: {role number, {foot x, foot y}, stop x of the picture's left edge}
        const std::array<Spot, kStarterRoleCount> kSpots = {{
            {11, {129, 456}, 79}, // 1st spot: role 11 (picture CreateRole11), feet at (129, 456), stops at x 79
            {1, {178, 477}, 51}, // 2nd spot: role 1 (CreateRole1), feet at (178, 477), stops at x 51
            {14, {248, 455}, 68}, // 3rd spot: role 14 (CreateRole14), feet at (248, 455), stops at x 68
            {7, {315, 456}, 68}, // 4th spot: role 7 (CreateRole7), feet at (315, 456), stops at x 68
            {9, {413, 457}, 57}, // 5th spot: role 9 (CreateRole9), feet at (413, 457), stops at x 57
            {13, {506, 456}, 50}, // 6th spot: role 13 (CreateRole13), feet at (506, 456), stops at x 50
            {10, {608, 464}, 55}, // 7th spot: role 10 (CreateRole10), feet at (608, 464), stops at x 55
            {2, {659, 465}, 48}, // 8th spot: role 2 (CreateRole2), feet at (659, 465), stops at x 48
            {6, {719, 454}, 64}, // 9th spot: role 6 (CreateRole6), feet at (719, 454), stops at x 64
            {8, {821, 470}, 62}, // 10th spot: role 8 (CreateRole8), feet at (821, 470), stops at x 62
            {3, {916, 484}, 78}, // 11th spot: role 3 (CreateRole3), feet at (916, 484), stops at x 78
            {5, {921, 456}, 69}, // 12th spot: role 5 (CreateRole5), feet at (921, 456), stops at x 69
            {12, {1026, 454}, 50}, // 13th spot: role 12 (CreateRole12), feet at (1026, 454), stops at x 50
            {4, {1111, 455}, 50}, // 14th spot: role 4 (CreateRole4), feet at (1111, 455), stops at x 50
        }};

        constexpr double kPixelsPerMs = 0.02; // scroll speed growth: step in px = ceil(0.02 * ms since the scroll began)
        constexpr int kWrapOverlap = 100; // px subtracted from the rightmost picture edge to get the width the strip wraps at
        constexpr int kArrowMargin = 10; // px the visible area reaches under the back/forward arrow buttons

    }

    RoleCarousel::RoleCarousel(
        ui::Screen& screen
    ) {
        for (int role = 1; role <= kStarterRoleCount; ++role) {
            m_pictures.push_back(screen.get<ui::ImageButton>("CreateRole" + std::to_string(role)));
        }
        int right = 0;
        for (const Spot& spot : kSpots) {
            ui::ImageButton& picture = m_pictures[static_cast<std::size_t>(spot.role - 1)];
            const core::Rect bounds = picture.bounds();
            picture.setPosition({spot.foot.x - bounds.width / 2, spot.foot.y + 15 - bounds.height}); // the foot point lies 15 px above the picture's lower edge
            right = std::max(right, picture.bounds().right());
        }
        m_wrapWidth = right - kWrapOverlap;
        m_visibleLeft = screen.get<ui::ImageButton>("CreateRolesBack").bounds().right() - kArrowMargin;
        m_visibleRight = screen.get<ui::ImageButton>("CreateRolesForward").bounds().x + kArrowMargin;
    }

    ui::ImageButton& RoleCarousel::pictureAt(int index) const {
        return m_pictures[static_cast<std::size_t>(kSpots[static_cast<std::size_t>(index - 1)].role - 1)];
    }

    void RoleCarousel::back() {
        m_index = m_index <= 1 ? kStarterRoleCount : m_index - 1;
        startScroll(Direction::Back);
    }

    void RoleCarousel::forward() {
        m_index = m_index >= kStarterRoleCount ? 1 : m_index + 1;
        startScroll(Direction::Forward);
    }

    void RoleCarousel::startScroll(Direction direction) {
        const int left = pictureAt(m_index).bounds().x;
        const int stop = kSpots[static_cast<std::size_t>(m_index - 1)].stopLeft;
        if (direction == Direction::Back) {
            m_distance = left > stop ? std::abs(stop + m_wrapWidth - left) : stop - left;
        } else {
            m_distance = left < stop ? -std::abs(m_wrapWidth - stop + left) : -std::abs(stop - left);
        }
        m_scrollStarted = false;
    }

    void RoleCarousel::update(std::uint64_t nowMs) {
        int step = 0;
        if (m_distance != 0) {
            if (!m_scrollStarted) {
                m_scrollStarted = true;
                m_scrollStartMs = nowMs;
            }
            step = static_cast<int>(std::ceil(kPixelsPerMs * static_cast<double>(nowMs - m_scrollStartMs)));
            step = std::min(step, std::abs(m_distance));
            if (m_distance < 0) {
                m_distance += step;
                step = -step;
            } else {
                m_distance -= step;
            }
        }
        for (ui::ImageButton& picture : m_pictures) {
            core::Rect bounds = picture.bounds();
            bounds.x += step;
            if (bounds.x > m_wrapWidth) {
                bounds.x -= m_wrapWidth;
            } else if (bounds.x < 0) {
                bounds.x += m_wrapWidth;
            }
            picture.setBounds(bounds);
            picture.setVisible(bounds.x >= m_visibleLeft && bounds.right() <= m_visibleRight);
        }
    }

}
