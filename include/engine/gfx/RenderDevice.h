#ifndef NORTH_ISLAND_GFX_RENDER_DEVICE_H
#define NORTH_ISLAND_GFX_RENDER_DEVICE_H

#include "core/Color.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "core/Size.h"
#include "formats/Image.h"
#include "formats/PixelArtScale.h"
#include "gfx/PixelArtPart.h"
#include "gfx/RenderMode.h"
#include "gfx/SdlDeleter.h"

#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace gfx {

    /// @brief How the window is shown: a window, a borderless window over its whole monitor, or the monitor switched to the window's size (exclusive fullscreen).
    enum class WindowDisplay { Windowed, Borderless, Fullscreen };

    /// @brief The game window and its renderer.
    /// @details The logical resolution (the game's pixels) is the window's size divided by the pixel scale, at least
    /// minWidth x minHeight; a window too small for that shows it shrunk. At a pixel scale above 1 the frame is drawn at the
    /// window's detail, which the HD pictures of the mod packs (Texture's detail) show.
    class RenderDevice {
        public:
            /// @brief What the window and renderer are created with.
            struct Settings {
                std::string title;
                int width = 800; // the game's screen width
                int height = 600; // the game's screen height
                bool resizable = false;
                bool maximized = false;
                bool minimized = false;
                WindowDisplay display = WindowDisplay::Windowed;
                int minWidth = 800; // the game's screen width
                int minHeight = 600; // the game's screen height
                RenderMode mode = RenderMode::Accelerated;
                bool vsync = false;
                int pixelScale = 1;
                std::string driver;
            };

            /// @brief Creates the window and renderer.
            /// @param settings The window and renderer settings.
            /// @return The device, or nothing when SDL cannot create them.
            static std::optional<RenderDevice> create(const Settings& settings);

            /// @brief The render drivers SDL has in this build ("direct3d11", "opengl", ...).
            /// @return The driver names.
            static std::vector<std::string> driverNames();

            /// @brief The SDL renderer.
            /// @return The renderer.
            SDL_Renderer* renderer() const { return m_renderer.get(); }

            /// @brief The SDL window.
            /// @return The window.
            SDL_Window* window() const { return m_window.get(); }

            /// @brief Whether the window is maximized.
            /// @return True when maximized.
            bool isMaximized() const;

            /// @brief The logical width in game pixels.
            /// @return The width.
            int width() const { return m_width; }

            /// @brief The logical height in game pixels.
            /// @return The height.
            int height() const { return m_height; }

            /// @brief The SDL renderer's name ("direct3d11", "opengl", "software", ...).
            /// @return The name.
            const std::string& backendName() const { return m_backendName; }

            /// @brief Whether reading the screen back is quick enough to do every frame (exact alpha fills, dimming).
            /// @return False for the Vulkan renderer, whose read back waits for the GPU (about 260 ms for 800 x 600).
            bool readsBackQuickly() const { return m_backendName != "vulkan"; }

            /// @brief Whether the renderer is GPU accelerated.
            /// @return True when accelerated.
            bool isAccelerated() const { return m_accelerated; }

            /// @brief How many window pixels across a game pixel covers (1: as the original).
            /// @return The pixel scale.
            int pixelScale() const { return m_pixelScale; }

            /// @brief Window pixels across a game pixel as drawn: the pixel scale, or less (not a whole number) when the window has no room for the least logical size at that scale.
            /// @return The draw scale.
            float drawScale() const { return m_drawScale; }

            /// @brief How many times as big the mouse cursor's pictures are made (a system cursor is in the screen's pixels).
            /// @return A game pixel's screen pixels; 1 at 800 x 600, 1x.
            int cursorScale() const;

            /// @brief Sets the pixel-art scaler put on pictures, sprites and fonts made into textures from now on.
            /// @param filter The filter.
            void setPixelArtFilter(formats::PixelArtFilter filter) { m_pixelArtFilter = filter; }

            /// @brief The pixel-art scaler in use.
            /// @return The filter.
            formats::PixelArtFilter pixelArtFilter() const { return m_pixelArtFilter; }

            /// @brief Turns the filter on (the default) or off for a part.
            /// @param part The kind of picture.
            /// @param on True to scale that part.
            void setPixelArtPart(PixelArtPart part, bool on) { m_pixelArtParts[static_cast<std::size_t>(part)] = on; }

            /// @brief The detail a part's picture is scaled to by the filter.
            /// @param part The kind of picture.
            /// @return The detail (1: not scaled).
            int pixelArtDetail(PixelArtPart part) const {
                return m_pixelArtFilter != formats::PixelArtFilter::None && m_pixelArtParts[static_cast<std::size_t>(part)] ? m_pixelScale : 1;
            }

            /// @brief The refresh rate of the monitor the window is on.
            /// @return Hz, or 0 when not known.
            int refreshRate() const;

            /// @brief The usable part of that monitor (without the taskbar).
            /// @return The size, 0 x 0 when not known.
            core::Size usableDesktop() const;

            /// @brief Tells the device the window's size changed; the logical size follows it.
            /// @param windowWidth New window width.
            /// @param windowHeight New window height.
            void resize(int windowWidth, int windowHeight);

            /// @brief Sets another pixel scale; the logical size follows, the window keeps its size.
            /// @param scale The pixel scale (1 to 3).
            void setPixelScale(int scale); // valid scales are 1 to 3

            /// @brief Asks for another window size (in the middle of its monitor) or a maximized window; the size-changed event follows.
            /// @param width Window width.
            /// @param height Window height.
            /// @param maximized True to maximize instead.
            void setWindowSize(int width, int height, bool maximized);

            /// @brief Switches between windowed, borderless and exclusive fullscreen; the size-changed event follows.
            /// @details Exclusive switches the monitor to the mode nearest width x height.
            /// @param display The display mode.
            /// @param width Wanted width.
            /// @param height Wanted height.
            /// @return False when SDL refused it (SDL_GetError).
            bool setDisplay(WindowDisplay display, int width, int height);

            /// @brief Whether the window covers its monitor (borderless or exclusive), so its size is not the windowed size to keep.
            /// @return True when fullscreen.
            bool isFullscreen() const;

            /// @brief Sets whether a present waits for the monitor's vertical blank.
            /// @param vsync True to wait.
            /// @return False when the renderer cannot change it.
            bool setVsync(bool vsync);

            /// @brief Clears the frame to a colour.
            /// @param color The colour.
            void clear(core::Color color = {0, 0, 0, 255}); // default: opaque black

            /// @brief Shows the drawn frame.
            void present();

            /// @brief Starts drawing off screen over an area of this many logical pixels from the buffer's top left.
            /// @details The area is the part of the canvas a zoom shows (bigger than the window when zoomed out).
            /// @param area Size of the area in logical pixels.
            void beginZoomedFrame(core::Size area) const;

            /// @brief Stretches a part of the off-screen area over the whole window, smoothed as pixel art.
            /// @details Original: StretchBufToScreen.
            /// @param source The part, in the area's pixels.
            void endZoomedFrame(const core::Rect& source) const;

            /// @brief Saves what has been drawn so far (call before present) as a .bmp file.
            /// @param bmpFile Path of the file to write.
            /// @return True when written.
            bool saveScreenshot(const std::string& bmpFile) const;

            /// @brief Reads what has been drawn so far (call before present), as logical pixels.
            /// @return The frame.
            formats::Image readFrame() const;

            /// @brief Reads the frame at the detail the window shows it: pixelScale pixels to a logical one at a whole scale, else logical pixels.
            /// @return The frame and its detail.
            std::pair<formats::Image, int> readFrameDetailed() const;

            /// @brief Reads the pixels now on the screen inside a rectangle of the logical screen.
            /// @param region The rectangle (clipped to the screen).
            /// @return The pixels.
            formats::Image readRegion(const core::Rect& region) const;

            /// @brief Restricts drawing to a rectangle.
            /// @param clip The clip rectangle, or nothing to remove the clip.
            void setClip(const std::optional<core::Rect>& clip) const;

        private:
            /// @brief Applies the logical size (window size / pixel scale, at least the minimum) to the renderer.
            void applyLogicalSize();

            SdlHandle<SDL_Window> m_window;
            SdlHandle<SDL_Renderer> m_renderer;
            mutable SdlHandle<SDL_Texture> m_zoomTarget;
            mutable int m_zoomTargetWidth = 0;
            mutable int m_zoomTargetHeight = 0;
            int m_width = 0;
            int m_height = 0;
            int m_windowWidth = 0;
            int m_windowHeight = 0;
            int m_minWidth = 0;
            int m_minHeight = 0;
            int m_pixelScale = 1;
            float m_drawScale = 1;
            formats::PixelArtFilter m_pixelArtFilter = formats::PixelArtFilter::None;
            std::array<bool, 3> m_pixelArtParts{true, true, true}; // one flag per PixelArtPart (Pictures, Sprites, Text)
            std::string m_backendName;
            bool m_accelerated = false;
    };

}

#endif
