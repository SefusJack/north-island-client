#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_CAMERA_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_CAMERA_H

#include "core/Point.h"
#include "core/Rect.h"

namespace client {

    /// @brief The part of the map the screen shows: the map pixel at the screen's top-left corner.
    /// @details Original: TGameMap.FViewX / FViewY. A bigger window shows more map around the 800 x 600 screen.
    class Camera {
        public:
            static constexpr int kScreenWidth = 800; // width of the game screen, px
            static constexpr int kScreenHeight = 600; // height of the game screen, px

            /// @brief Sets the window (in screen coordinates) the camera fills.
            /// @param view The window rectangle.
            void setView(const core::Rect& view) { m_view = view; }

            /// @brief The window the camera fills.
            /// @return The window rectangle.
            const core::Rect& view() const { return m_view; }

            /// @brief Puts a map point in the middle of the screen where the map allows it.
            /// @details Original: CenterMapView, with the game's 20-pixel shortfall at the right edge.
            /// @param mapPoint Map point to centre on.
            /// @param mapWidth Map width in pixels.
            /// @param mapHeight Map height in pixels.
            void centerOn(core::Point mapPoint, int mapWidth, int mapHeight);

            /// @brief Which axes a scroll moved.
            struct Scrolled {
                bool x = false;
                bool y = false;
            };

            /// @brief Follows the own player's step: an axis scrolls while the player is past the middle and the map goes on.
            /// @details Original: ScrollMapView.
            /// @param step The player's step in pixels.
            /// @param screenPosition The player's position on the screen.
            /// @param mapWidth Map width in pixels.
            /// @param mapHeight Map height in pixels.
            /// @return Which axes moved.
            Scrolled follow(core::Point step, core::Point screenPosition, int mapWidth, int mapHeight);

            /// @brief Sets the view at a map point.
            /// @details Original: used for a movie's map ground, MovieMap.SetViewPos.
            /// @param origin Map point at the screen's top-left corner.
            void setOrigin(core::Point origin) { m_origin = origin; }

            /// @brief The map point at the screen's top-left corner.
            /// @return The origin.
            core::Point origin() const { return m_origin; }

            /// @brief Converts a map point to screen coordinates.
            /// @param mapPoint Map point.
            /// @return Screen point.
            core::Point toScreen(core::Point mapPoint) const { return {mapPoint.x - m_origin.x, mapPoint.y - m_origin.y}; }

            /// @brief Converts a screen point to map coordinates.
            /// @param screenPoint Screen point.
            /// @return Map point.
            core::Point toMap(core::Point screenPoint) const { return {screenPoint.x + m_origin.x, screenPoint.y + m_origin.y}; }

        private:
            core::Point m_origin;
            core::Rect m_view{0, 0, kScreenWidth, kScreenHeight};
    };

}

#endif
