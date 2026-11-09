#include "client/models/world/Camera.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kRightShortfall = 20; // px the view stops short of the map's right edge when centring

        /// @brief Tells whether a map narrower than a bigger-than-game window lies in the window's middle.
        /// @param mapSize Map size on this axis.
        /// @param screenSize Window size on this axis.
        /// @param gameSize The game's own screen size on this axis.
        /// @return True when the map is centred in the window.
        bool mapInMiddle(int mapSize, int screenSize, int gameSize) { return screenSize > gameSize && mapSize < screenSize; }

        /// @brief One axis of CenterMapView: the view origin that centres a point.
        /// @param point Map point on this axis.
        /// @param half Half the window size.
        /// @param mapSize Map size on this axis.
        /// @param screenSize Window size on this axis.
        /// @param shortfall Pixels left short at the far edge.
        /// @param gameSize The game's own screen size on this axis.
        /// @return The origin on this axis.
        int centeredOrigin(
            int point,
            int half,
            int mapSize,
            int screenSize,
            int shortfall,
            int gameSize
        ) {
            if (mapInMiddle(mapSize, screenSize, gameSize)) {
                return -(screenSize - mapSize) / 2;
            }
            if (point - half <= 0) {
                return 0;
            }
            if (point + half <= mapSize) {
                return point - half;
            }
            return mapSize > screenSize ? mapSize - screenSize - shortfall : 0;
        }

        /// @brief One axis of ScrollMapView: the view origin after a step.
        /// @param origin Current origin on this axis.
        /// @param step The player's step on this axis.
        /// @param screenPosition The player's screen position on this axis.
        /// @param half Half the window size.
        /// @param mapSize Map size on this axis.
        /// @param screenSize Window size on this axis.
        /// @param gameSize The game's own screen size on this axis.
        /// @return The new origin on this axis.
        int scrolledOrigin(
            int origin,
            int step,
            int screenPosition,
            int half,
            int mapSize,
            int screenSize,
            int gameSize
        ) {
            if (mapInMiddle(mapSize, screenSize, gameSize)) {
                return origin;
            }
            if (step < 0 && screenPosition <= half) {
                return std::max(0, origin + step);
            }
            if (step > 0 && screenPosition >= half) {
                return std::max(origin, std::min(origin + step, mapSize - screenSize));
            }
            return origin;
        }

    }

    void Camera::centerOn(core::Point mapPoint, int mapWidth, int mapHeight) {
        m_origin.x = centeredOrigin(mapPoint.x, m_view.width / 2, mapWidth, m_view.width, kRightShortfall, kScreenWidth) - m_view.x;
        m_origin.y = centeredOrigin(mapPoint.y, m_view.height / 2, mapHeight, m_view.height, 0, kScreenHeight) - m_view.y;
    }

    Camera::Scrolled Camera::follow(core::Point step, core::Point screenPosition, int mapWidth, int mapHeight) {
        const core::Point before = m_origin;
        m_origin.x =
            scrolledOrigin(m_origin.x + m_view.x, step.x, screenPosition.x - m_view.x, m_view.width / 2, mapWidth, m_view.width, kScreenWidth) - m_view.x;
        m_origin.y =
            scrolledOrigin(m_origin.y + m_view.y, step.y, screenPosition.y - m_view.y, m_view.height / 2, mapHeight, m_view.height, kScreenHeight) - m_view.y;
        return {m_origin.x != before.x, m_origin.y != before.y};
    }

}
