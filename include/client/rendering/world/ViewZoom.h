#ifndef NORTH_ISLAND_CLIENT_RENDERING_WORLD_VIEW_ZOOM_H
#define NORTH_ISLAND_CLIENT_RENDERING_WORLD_VIEW_ZOOM_H

#include "core/Point.h"
#include "core/Rect.h"

#include <cstdint>

namespace client {

    /// @brief The zoom of the world view, qol's way: a level is the part of the canvas the camera shows, stretched over the window.
    /// @details Original: TForm1.FZoomLevel. The original cut a part of the drawn screen out (FZoomRect), stretched it over the window and let the mouse at an edge pan it. Here the window's middle with a fortieth of its size cut off every edge a step in (nine steps, as the original) or added a step out; the world is drawn over that part, centred on the own player (WorldScene); the names and the mouse go through the same stretch. Ctrl and the mouse wheel step it. With free frames each step glides over a beat instead of jumping.
    class ViewZoom {
        public:
            static constexpr int kMaxLevel = 9; // most steps in (nine, as the original)
            static constexpr int kMinLevel = -10; // most steps out (qol addition)
            static constexpr std::uint64_t kGlideMs = 30; // ms a glide between levels takes (one beat)

            /// @brief Turns gliding on (free frames: steps glide instead of jumping).
            /// @param on True to glide.
            void setGliding(bool on) { m_gliding = on; }

            /// @brief Sets the time of this frame, from which a step that comes now glides.
            /// @param nowMs Current time in milliseconds.
            void setNow(std::uint64_t nowMs) { m_nowMs = nowMs; }

            /// @brief Sets the window in the canvas's coordinates (the game's screen in its middle); the zoom starts again over it.
            /// @param window The window rectangle.
            void setWindow(const core::Rect& window);

            /// @brief The window the zoom works over.
            /// @return The window rectangle in the canvas's coordinates.
            const core::Rect& window() const { return m_window; }

            /// @brief Steps one level in.
            /// @details Original: ZoomViewIn.
            void zoomIn();

            /// @brief Steps one level out (if allowed).
            /// @details Original: ZoomViewOut.
            void zoomOut();

            /// @brief Jumps to the most zoomed level.
            /// @details Original: the movie filter 6.
            void zoomToMost();

            /// @brief Puts the level back to the whole window.
            /// @details Original: CloseFightField.
            void reset();

            /// @brief Sets whether a step out is allowed (not in a battle); when it stops being, a level out goes back to the whole window.
            /// @param allowed True when zooming out is allowed.
            void setOutAllowed(bool allowed);

            /// @brief The current zoom level.
            /// @return Level, 0 for the whole window, positive in, negative out.
            int level() const { return m_level; }

            /// @brief Tells whether the level is not the whole window.
            /// @return True when zoomed in or out.
            bool isZoomed() const { return m_level != 0; }

            /// @brief The part of the canvas a level shows.
            /// @param level Zoom level.
            /// @return The part rectangle in the canvas's coordinates.
            core::Rect part(int level) const;

            /// @brief The part shown at a time: the level's, or on the way to it while it glides.
            /// @param nowMs Time in milliseconds.
            /// @return The part rectangle in the canvas's coordinates.
            core::Rect shown(std::uint64_t nowMs) const;

            /// @brief Tells whether the part shown at a time is not the whole window.
            /// @param nowMs Time in milliseconds.
            /// @return True when zoomed.
            bool isShownZoomed(std::uint64_t nowMs) const;

            /// @brief Maps a point of a part to where the window shows it.
            /// @param point Point in the canvas's coordinates.
            /// @param part The zoomed part.
            /// @param window The window the part is stretched over.
            /// @return Point in the window.
            static core::Point toWindow(core::Point point, const core::Rect& part, const core::Rect& window);

            /// @brief Maps a point of the window back to the part.
            /// @param point Point in the window.
            /// @param part The zoomed part.
            /// @param window The window the part is stretched over.
            /// @return Point in the canvas's coordinates.
            static core::Point fromWindow(core::Point point, const core::Rect& part, const core::Rect& window);

        private:
            /// @brief Starts a glide from the part shown now (before a step changes the level).
            void glideFromHere();

            /// @brief Changes the level within its limits, gliding from what is shown.
            /// @param level The wanted level.
            void setLevel(int level);

            int m_level = 0;
            bool m_outAllowed = true;
            core::Rect m_window{0, 0, 800, 600}; // the 800 x 600 game screen until the window size is known
            bool m_gliding = false;
            std::uint64_t m_nowMs = 0;
            core::Rect m_from{0, 0, 800, 600}; // glide start part, the 800 x 600 screen at first
            std::uint64_t m_fromMs = 0;
    };

}

#endif
