#include "gfx/RenderDevice.h"

#include "core/Log.h"

#include <algorithm>
#include <cmath>

namespace gfx {

    namespace {

        /// @brief Reads the renderer's pixels as an RGBA32 surface.
        /// @param renderer The renderer to read.
        /// @param area The area to read, or null for the whole output.
        /// @return The surface (converted from the renderer's own format), or null on failure.
        SdlHandle<SDL_Surface> readPixels(SDL_Renderer* renderer, const SDL_Rect* area) {
            SdlHandle<SDL_Surface> read(SDL_RenderReadPixels(renderer, area));
            if (!read || read->format == SDL_PIXELFORMAT_RGBA32) {
                return read;
            }
            return SdlHandle<SDL_Surface>(SDL_ConvertSurface(read.get(), SDL_PIXELFORMAT_RGBA32));
        }

    }

    std::optional<RenderDevice> RenderDevice::create(const Settings& settings) {
        RenderDevice device;
        device.m_windowWidth = settings.width;
        device.m_windowHeight = settings.height;
        device.m_minWidth = settings.resizable ? settings.minWidth : settings.width;
        device.m_minHeight = settings.resizable ? settings.minHeight : settings.height;
        device.m_pixelScale = std::clamp(settings.pixelScale, 1, 3); // pixel scale range 1..3
        const SDL_PropertiesID properties = SDL_CreateProperties();
        SDL_SetStringProperty(properties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, settings.title.c_str());
        SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, settings.width);
        SDL_SetNumberProperty(properties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, settings.height);
        SDL_SetNumberProperty(
            properties,
            SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER,
            static_cast<Sint64>((settings.resizable ? SDL_WINDOW_RESIZABLE : 0) | (settings.maximized ? SDL_WINDOW_MAXIMIZED : 0) |
                                 (settings.minimized ? SDL_WINDOW_MINIMIZED : 0))
        );
        device.m_window.reset(SDL_CreateWindowWithProperties(properties));
        SDL_DestroyProperties(properties);
        if (!device.m_window) {
            core::logError("cannot create the window: ", SDL_GetError());
            return std::nullopt;
        }
        if (settings.resizable) {
            SDL_SetWindowMinimumSize(device.m_window.get(), settings.minWidth, settings.minHeight);
        }
        if (settings.display != WindowDisplay::Windowed && !device.setDisplay(settings.display, settings.width, settings.height)) {
            core::logWarning("cannot show the window full screen: ", SDL_GetError());
        }

        const char* driver = settings.mode == RenderMode::Software ? "software" : settings.driver.empty() ? nullptr : settings.driver.c_str();
        device.m_renderer.reset(SDL_CreateRenderer(device.m_window.get(), driver));
        if (!device.m_renderer && settings.mode == RenderMode::Accelerated) {
            core::logWarning("no accelerated renderer (", SDL_GetError(), "), using the software renderer");
            device.m_renderer.reset(SDL_CreateRenderer(device.m_window.get(), "software"));
        }
        if (!device.m_renderer) {
            core::logError("cannot create a renderer: ", SDL_GetError());
            return std::nullopt;
        }
        if (settings.vsync) {
            SDL_SetRenderVSync(device.m_renderer.get(), 1);
        }

        const char* name = SDL_GetRendererName(device.m_renderer.get());
        device.m_backendName = name != nullptr ? name : "unknown";
        device.m_accelerated = device.m_backendName != "software";
        SDL_SetDefaultTextureScaleMode(device.m_renderer.get(), SDL_SCALEMODE_NEAREST);
        device.applyLogicalSize();
        SDL_SetRenderDrawBlendMode(device.m_renderer.get(), SDL_BLENDMODE_BLEND);
        core::logInfo("renderer: ", device.m_backendName, device.m_accelerated ? " (GPU accelerated)" : " (software)");
        return device;
    }

    std::vector<std::string> RenderDevice::driverNames() {
        std::vector<std::string> names;
        for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
            if (const char* name = SDL_GetRenderDriver(i)) {
                names.emplace_back(name);
            }
        }
        return names;
    }

    void RenderDevice::setWindowSize(int width, int height, bool maximized) {
        if (maximized) {
            SDL_MaximizeWindow(m_window.get());
            return;
        }
        if (isMaximized()) {
            SDL_RestoreWindow(m_window.get());
        }
        SDL_SetWindowSize(m_window.get(), width, height);
        const SDL_DisplayID display = SDL_GetDisplayForWindow(m_window.get());
        SDL_SetWindowPosition(m_window.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display));
    }

    bool RenderDevice::setDisplay(WindowDisplay display, int width, int height) {
        SDL_Window* window = m_window.get();
        if (display == WindowDisplay::Windowed) {
            return SDL_SetWindowFullscreen(window, false);
        }
        if (display == WindowDisplay::Borderless) {
            return SDL_SetWindowFullscreenMode(window, nullptr) && SDL_SetWindowFullscreen(window, true);
        }
        SDL_DisplayMode mode{};
        if (!SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window), width, height, 0.0f, true, &mode)) {
            return false;
        }
        return SDL_SetWindowFullscreenMode(window, &mode) && SDL_SetWindowFullscreen(window, true);
    }

    bool RenderDevice::isFullscreen() const { return (SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_FULLSCREEN) != 0; }

    bool RenderDevice::setVsync(bool vsync) { return SDL_SetRenderVSync(m_renderer.get(), vsync ? 1 : 0); }

    bool RenderDevice::isMaximized() const { return (SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_MAXIMIZED) != 0; }

    void RenderDevice::clear(core::Color color) {
        SDL_SetRenderDrawColor(m_renderer.get(), color.r, color.g, color.b, color.a);
        SDL_RenderClear(m_renderer.get());
    }

    void RenderDevice::present() { SDL_RenderPresent(m_renderer.get()); }

    void RenderDevice::resize(int windowWidth, int windowHeight) {
        if (windowWidth == m_windowWidth && windowHeight == m_windowHeight) {
            return;
        }
        m_windowWidth = windowWidth;
        m_windowHeight = windowHeight;
        applyLogicalSize();
    }

    void RenderDevice::setPixelScale(int scale) {
        scale = std::clamp(scale, 1, 3); // pixel scale range 1..3
        if (scale == m_pixelScale) {
            return;
        }
        m_pixelScale = scale;
        applyLogicalSize();
    }

    void RenderDevice::applyLogicalSize() {
        m_width = std::max(m_windowWidth / m_pixelScale, m_minWidth);
        m_height = std::max(m_windowHeight / m_pixelScale, m_minHeight);
        m_drawScale = static_cast<float>(m_pixelScale);
        const bool whole = m_pixelScale > 1 && m_windowWidth >= m_width * m_pixelScale && m_windowHeight >= m_height * m_pixelScale;
        if (m_pixelScale > 1 && !whole) {
            const float fit = std::min(
                static_cast<float>(m_windowWidth) / static_cast<float>(m_minWidth),
                static_cast<float>(m_windowHeight) / static_cast<float>(m_minHeight)
            );
            m_drawScale = std::max(fit, 1.0f);
            m_width = std::max(static_cast<int>(static_cast<float>(m_windowWidth) / m_drawScale), m_minWidth);
            m_height = std::max(static_cast<int>(static_cast<float>(m_windowHeight) / m_drawScale), m_minHeight);
        }
        SDL_SetRenderLogicalPresentation(
            m_renderer.get(),
            m_width,
            m_height,
            whole ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE : SDL_LOGICAL_PRESENTATION_LETTERBOX
        );
        m_zoomTarget.reset();
        m_zoomTargetWidth = 0;
        m_zoomTargetHeight = 0;
    }

    int RenderDevice::cursorScale() const {
        // follows the game's pixels (Pixel Scale), not the desktop's scaling
        const float density = SDL_GetWindowPixelDensity(m_window.get());
        const float scale = m_drawScale * (density > 0.0f ? density : 1.0f);
        return std::clamp(static_cast<int>(std::lround(scale)), 1, 6); // cursor scale range 1..6
    }

    int RenderDevice::refreshRate() const {
        const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(m_window.get()));
        return mode != nullptr ? static_cast<int>(mode->refresh_rate + 0.5f) : 0; // rounds the refresh rate to the nearest Hz
    }

    core::Size RenderDevice::usableDesktop() const {
        SDL_Rect area{};
        return SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(m_window.get()), &area) ? core::Size{area.w, area.h} : core::Size{};
    }

    void RenderDevice::beginZoomedFrame(core::Size area) const {
        const int width = static_cast<int>(std::ceil(static_cast<float>(area.width) * m_drawScale));
        const int height = static_cast<int>(std::ceil(static_cast<float>(area.height) * m_drawScale));
        if (!m_zoomTarget || width > m_zoomTargetWidth || height > m_zoomTargetHeight) {
            m_zoomTargetWidth = std::max(width, m_zoomTargetWidth);
            m_zoomTargetHeight = std::max(height, m_zoomTargetHeight);
            m_zoomTarget.reset(SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, m_zoomTargetWidth, m_zoomTargetHeight));
            if (m_zoomTarget && !SDL_SetTextureScaleMode(m_zoomTarget.get(), SDL_SCALEMODE_PIXELART)) {
                SDL_SetTextureScaleMode(m_zoomTarget.get(), SDL_SCALEMODE_LINEAR);
            }
        }
        if (m_zoomTarget && SDL_SetRenderTarget(m_renderer.get(), m_zoomTarget.get())) {
            SDL_SetRenderScale(m_renderer.get(), m_drawScale, m_drawScale);
            SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 255); // opaque black
            SDL_RenderClear(m_renderer.get());
        }
    }

    void RenderDevice::endZoomedFrame(const core::Rect& source) const {
        if (SDL_GetRenderTarget(m_renderer.get()) == nullptr) {
            return;
        }
        SDL_SetRenderTarget(m_renderer.get(), nullptr);
        const float scale = m_drawScale;
        const SDL_FRect part{
            static_cast<float>(source.x) * scale,
            static_cast<float>(source.y) * scale,
            static_cast<float>(source.width) * scale,
            static_cast<float>(source.height) * scale
        };
        SDL_RenderTexture(m_renderer.get(), m_zoomTarget.get(), &part, nullptr);
    }

    bool RenderDevice::saveScreenshot(const std::string& bmpFile) const {
        SdlHandle<SDL_Surface> read(SDL_RenderReadPixels(m_renderer.get(), nullptr));
        SdlHandle<SDL_Surface> surface(read ? SDL_ConvertSurface(read.get(), SDL_PIXELFORMAT_BGR24) : nullptr);
        if (!surface) {
            core::logWarning("cannot read the screen: ", SDL_GetError());
            return false;
        }
        return SDL_SaveBMP(surface.get(), bmpFile.c_str());
    }

    formats::Image RenderDevice::readFrame() const {
        SDL_FRect shown{};
        SDL_GetRenderLogicalPresentationRect(m_renderer.get(), &shown);
        const SDL_Rect area{static_cast<int>(shown.x), static_cast<int>(shown.y), static_cast<int>(shown.w), static_cast<int>(shown.h)};
        SdlHandle<SDL_Surface> surface = readPixels(m_renderer.get(), area.w > 0 && area.h > 0 ? &area : nullptr);
        formats::Image frame(m_width, m_height);
        if (!surface) {
            core::logWarning("cannot read the screen: ", SDL_GetError());
            return frame;
        }
        const int width = surface->w;
        const int height = surface->h;
        for (int y = 0; y < m_height; ++y) {
            const Uint8* row = static_cast<const Uint8*>(surface->pixels) + (y * height / m_height) * surface->pitch;
            for (int x = 0; x < m_width; ++x) {
                const Uint8* pixel = row + (x * width / m_width) * 4; // RGBA32: 4 bytes a pixel
                frame.at(x, y) = {pixel[0], pixel[1], pixel[2], 255}; // opaque alpha
            }
        }
        return frame;
    }

    std::pair<formats::Image, int> RenderDevice::readFrameDetailed() const {
        SDL_FRect shown{};
        SDL_GetRenderLogicalPresentationRect(m_renderer.get(), &shown);
        const SDL_Rect area{static_cast<int>(shown.x), static_cast<int>(shown.y), static_cast<int>(shown.w), static_cast<int>(shown.h)};
        if (m_pixelScale == 1 || area.w != m_width * m_pixelScale || area.h != m_height * m_pixelScale) {
            return {readFrame(), 1};
        }
        SdlHandle<SDL_Surface> surface = readPixels(m_renderer.get(), &area);
        formats::Image frame(area.w, area.h);
        if (!surface) {
            core::logWarning("cannot read the screen: ", SDL_GetError());
            return {std::move(frame), m_pixelScale};
        }
        for (int y = 0; y < std::min(area.h, surface->h); ++y) {
            const Uint8* row = static_cast<const Uint8*>(surface->pixels) + y * surface->pitch;
            for (int x = 0; x < std::min(area.w, surface->w); ++x) {
                // RGBA32: 4 bytes a pixel, offsets 0..2 are r, g, b; alpha is opaque
                frame.at(x, y) = {row[x * 4], row[x * 4 + 1], row[x * 4 + 2], 255};
            }
        }
        return {std::move(frame), m_pixelScale};
    }

    formats::Image RenderDevice::readRegion(const core::Rect& region) const {
        const int left = std::max(region.x, 0);
        const int top = std::max(region.y, 0);
        const int right = std::min(region.right(), m_width);
        const int bottom = std::min(region.bottom(), m_height);
        if (left >= right || top >= bottom) {
            return {};
        }
        int outputWidth = 0;
        int outputHeight = 0;
        SDL_GetRenderOutputSize(m_renderer.get(), &outputWidth, &outputHeight);
        if (outputWidth != m_width || outputHeight != m_height) {
            SDL_FRect shown{};
            SDL_GetRenderLogicalPresentationRect(m_renderer.get(), &shown);
            const int shownX = static_cast<int>(shown.x);
            const int shownY = static_cast<int>(shown.y);
            const int shownWidth = static_cast<int>(shown.w);
            const int shownHeight = static_cast<int>(shown.h);
            formats::Image part(right - left, bottom - top);
            if (shownWidth <= 0 || shownHeight <= 0) {
                return part;
            }
            const int firstColumn = left * shownWidth / m_width;
            const int firstRow = top * shownHeight / m_height;
            const SDL_Rect area{
                shownX + firstColumn,
                shownY + firstRow,
                (right - 1) * shownWidth / m_width - firstColumn + 1,
                (bottom - 1) * shownHeight / m_height - firstRow + 1
            };
            SdlHandle<SDL_Surface> surface = readPixels(m_renderer.get(), &area);
            if (!surface) {
                core::logWarning("cannot read the screen: ", SDL_GetError());
                return part;
            }
            for (int y = top; y < bottom; ++y) {
                const int row = std::min(y * shownHeight / m_height - firstRow, surface->h - 1);
                const Uint8* line = static_cast<const Uint8*>(surface->pixels) + row * surface->pitch;
                for (int x = left; x < right; ++x) {
                    const Uint8* pixel = line + std::min(x * shownWidth / m_width - firstColumn, surface->w - 1) * 4; // RGBA32: 4 bytes a pixel
                    part.at(x - left, y - top) = {pixel[0], pixel[1], pixel[2], 255}; // opaque alpha
                }
            }
            return part;
        }
        formats::Image part(right - left, bottom - top);
        const SDL_Rect area{left, top, right - left, bottom - top};
        SdlHandle<SDL_Surface> surface = readPixels(m_renderer.get(), &area);
        if (!surface) {
            core::logWarning("cannot read the screen: ", SDL_GetError());
            return part;
        }
        for (int y = 0; y < std::min(area.h, surface->h); ++y) {
            const Uint8* row = static_cast<const Uint8*>(surface->pixels) + y * surface->pitch;
            for (int x = 0; x < std::min(area.w, surface->w); ++x) {
                // RGBA32: 4 bytes a pixel, offsets 0..2 are r, g, b; alpha is opaque
                part.at(x, y) = {row[x * 4], row[x * 4 + 1], row[x * 4 + 2], 255};
            }
        }
        return part;
    }

    void RenderDevice::setClip(const std::optional<core::Rect>& clip) const {
        if (!clip) {
            SDL_SetRenderClipRect(m_renderer.get(), nullptr);
            return;
        }
        const SDL_Rect rect{clip->x, clip->y, clip->width, clip->height};
        SDL_SetRenderClipRect(m_renderer.get(), &rect);
    }

}
