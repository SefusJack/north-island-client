#include "gfx/RenderDevice.h"

#include "core/Log.h"
#include "core/Speedups.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace gfx {

    namespace {

#ifdef _WIN32
        /// @brief Moves parts of the window's picture on the screen itself (the graphics card does it, also on Windows XP).
        /// @param window The window.
        /// @param parts The destination rectangles to fill with the picture that is now shift away from them, in an order
        /// in which no part is filled before another part has read from its place.
        /// @param shift How far the picture moves.
        /// @param lost Gets what could not be moved (its source was covered by another window or off the screen).
        /// @return False when nothing was moved.
        bool moveOnScreen(SDL_Window* window, const std::vector<SDL_Rect>& parts, core::Point shift, std::vector<SDL_Rect>& lost) {
            const HWND handle = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
            const HDC screen = handle != nullptr ? GetDC(handle) : nullptr;
            if (screen == nullptr) {
                return false;
            }
            for (const SDL_Rect& part : parts) {
                // the part and where its picture comes from; only the part itself is written
                RECT to{part.x, part.y, part.x + part.w, part.y + part.h};
                RECT both{std::min(to.left, to.left - shift.x), std::min(to.top, to.top - shift.y), std::max(to.right, to.right - shift.x),
                          std::max(to.bottom, to.bottom - shift.y)};
                RECT missing{0, 0, 0, 0};
                if (ScrollDC(screen, shift.x, shift.y, &both, &to, nullptr, &missing) == 0) {
                    lost.push_back(part);
                } else if (missing.right > missing.left && missing.bottom > missing.top) {
                    lost.push_back({missing.left, missing.top, missing.right - missing.left, missing.bottom - missing.top});
                }
            }
            ReleaseDC(handle, screen);
            return true;
        }
#endif

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
        if (!device.m_renderer && driver != nullptr && settings.mode == RenderMode::Accelerated) {
            // the chosen driver is not in this build or cannot start here (settings copied from another system): SDL's choice
            core::logWarning("the renderer ", settings.driver, " cannot start here (", SDL_GetError(), "), using the default renderer");
            device.m_renderer.reset(SDL_CreateRenderer(device.m_window.get(), nullptr));
        }
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

    bool RenderDevice::desktopIsComposited() {
#ifdef _WIN32
        // dwmapi.dll is not there before Windows Vista: asked for by name so the Windows XP build still starts
        using IsEnabled = HRESULT(WINAPI*)(BOOL*);
        static const IsEnabled isEnabled = [] {
            const HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
            return dwm != nullptr ? reinterpret_cast<IsEnabled>(GetProcAddress(dwm, "DwmIsCompositionEnabled")) : nullptr;
        }();
        BOOL enabled = FALSE;
        return isEnabled != nullptr && SUCCEEDED(isEnabled(&enabled)) && enabled != FALSE;
#else
        return false;
#endif
    }

    bool RenderDevice::driverStarts(const std::string& name) {
        SDL_Window* probe = SDL_CreateWindow("renderer probe", 64, 64, SDL_WINDOW_HIDDEN);
        if (probe == nullptr) {
            return true; // cannot try it: keep it listed, as before
        }
        SDL_Renderer* renderer = SDL_CreateRenderer(probe, name.c_str());
        if (renderer == nullptr) {
            core::logInfo("renderer ", name, " cannot start here: ", SDL_GetError());
        } else {
            SDL_DestroyRenderer(renderer);
        }
        SDL_DestroyWindow(probe);
        return renderer != nullptr;
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

    void RenderDevice::present() {
        // Speedups::softwareChangedAreas: the software renderer copies only the changed parts of the frame to the screen.
        SDL_Surface* frame = core::speedups().softwareChangedAreas && !m_accelerated ? SDL_GetWindowSurface(m_window.get()) : nullptr;
        if (frame == nullptr || SDL_BYTESPERPIXEL(frame->format) != 4 || SDL_MUSTLOCK(frame)) {
            SDL_RenderPresent(m_renderer.get());
            return;
        }
        SDL_FlushRenderer(m_renderer.get());
        constexpr int kTileWidth = 32; // px
        constexpr int kTileHeight = 16; // px
        const int width = frame->w;
        const int height = frame->h;
        const int pitch = frame->pitch;
        const auto* pixels = static_cast<const std::uint8_t*>(frame->pixels);
        const bool sized = m_shownWidth == width && m_shownHeight == height && m_shownPitch == pitch;
        if (!sized) {
            m_shownFrame.assign(static_cast<std::size_t>(pitch) * static_cast<std::size_t>(height), 0);
            m_shownWidth = width;
            m_shownHeight = height;
            m_shownPitch = pitch;
            m_presentAll = true;
        }
        // a scene that changes everywhere (the camera scrolls): the comparison finds nothing to leave out, so it is skipped
        // for a while and the frame is only remembered every so often to notice when the scene settles
        constexpr int kWholeLimit = 8; // changed-everywhere frames in a row before the comparison is skipped
        constexpr int kProbeEvery = 16; // then one frame in this many is compared again
        // Speedups::softwareScroll: when the scene moved (sceneMovedBy), the parts of the new frame that are the shown
        // frame moved by that much are moved on the screen by the graphics card, and only the rest is copied
        const bool scrollWanted = core::speedups().softwareScroll;
        const core::Point shift = m_sceneShift;
        m_sceneShift = {0, 0};
        int windowWidth = 0;
        int windowHeight = 0;
        SDL_GetWindowSize(m_window.get(), &windowWidth, &windowHeight);
        bool scrolled = false;
#ifdef _WIN32
        // a move of less than a tile: the parts are then moved in an order in which none reads what another has written
        scrolled = scrollWanted && sized && (shift.x != 0 || shift.y != 0) && std::abs(shift.x) < kTileWidth && std::abs(shift.y) < kTileHeight &&
                   windowWidth == width && windowHeight == height;
#endif
        if (scrolled && m_wholeFrames >= kWholeLimit && !m_presentAll) {
            // the shown frame was not being remembered (everything changed, as it does while the camera moves): this frame is
            // copied whole and remembered, the next one can be moved
            m_presentAll = true;
        }
        if (scrolled && !m_presentAll) {
            thread_local std::vector<SDL_Rect> moves;
            thread_local std::vector<SDL_Rect> copies;
            moves.clear();
            copies.clear();
            const auto add = [](std::vector<SDL_Rect>& list, int left, int top, int right, int bandHeight) {
                if (!list.empty() && list.back().x == left && list.back().w == right - left && list.back().y + list.back().h == top) {
                    list.back().h += bandHeight;
                } else {
                    list.push_back({left, top, right - left, bandHeight});
                }
            };
            long long copied = 0;
            for (int top = 0; top < height; top += kTileHeight) {
                const int bandHeight = std::min(kTileHeight, height - top);
                int runStart = 0;
                bool runMoves = false;
                for (int left = 0; left <= width; left += kTileWidth) {
                    bool same = false;
                    if (left < width) {
                        const int tileWidth = std::min(kTileWidth, width - left);
                        const std::size_t bytes = static_cast<std::size_t>(tileWidth) * 4;
                        const int fromLeft = left - shift.x;
                        const int fromTop = top - shift.y;
                        same = fromLeft >= 0 && fromTop >= 0 && fromLeft + tileWidth <= width && fromTop + bandHeight <= height;
                        for (int y = 0; y < bandHeight && same; ++y) {
                            const std::size_t at = static_cast<std::size_t>(top + y) * static_cast<std::size_t>(pitch) + static_cast<std::size_t>(left) * 4;
                            const std::size_t from = static_cast<std::size_t>(fromTop + y) * static_cast<std::size_t>(pitch) + static_cast<std::size_t>(fromLeft) * 4;
                            same = std::memcmp(pixels + at, m_shownFrame.data() + from, bytes) == 0;
                        }
                        if (!same) {
                            copied += static_cast<long long>(tileWidth) * bandHeight;
                        }
                    }
                    if (left == 0) {
                        runMoves = same;
                    } else if (left >= width || same != runMoves) {
                        if (runMoves) {
                            moves.push_back({runStart, top, std::min(left, width) - runStart, bandHeight});
                        } else {
                            add(copies, runStart, top, std::min(left, width), bandHeight);
                        }
                        runStart = left;
                        runMoves = same;
                    }
                }
            }
            std::memcpy(m_shownFrame.data(), pixels, m_shownFrame.size());
            const long long all = static_cast<long long>(width) * height;
            m_wholeFrames = copied * 10 >= all * 8 ? m_wholeFrames + 1 : 0;
#ifdef _WIN32
            thread_local std::vector<SDL_Rect> lost;
            lost.clear();
            // the picture moves down: the lowest parts first; to the right: the rightmost first
            std::sort(moves.begin(), moves.end(), [shift](const SDL_Rect& a, const SDL_Rect& b) {
                if (a.y != b.y) {
                    return shift.y > 0 ? a.y > b.y : a.y < b.y;
                }
                return shift.x > 0 ? a.x > b.x : a.x < b.x;
            });
            const bool movedIt = moves.empty() || moveOnScreen(m_window.get(), moves, shift, lost);
            if (!movedIt) {
                SDL_UpdateWindowSurface(m_window.get()); // could not move it: the whole frame
                return;
            }
            const SDL_Rect whole{0, 0, width, height};
            for (const SDL_Rect& part : lost) {
                SDL_Rect inside{};
                if (SDL_GetRectIntersection(&part, &whole, &inside)) {
                    copies.push_back(inside);
                }
            }
#endif
            if (!copies.empty()) {
                SDL_UpdateWindowSurfaceRects(m_window.get(), copies.data(), static_cast<int>(copies.size()));
            }
            return;
        }
        if (m_presentAll || (m_wholeFrames >= kWholeLimit && (m_wholeFrames - kWholeLimit) % kProbeEvery != 0)) {
            const bool remember = m_presentAll;
            if (remember) {
                std::memcpy(m_shownFrame.data(), pixels, m_shownFrame.size());
                m_wholeFrames = 0;
            } else {
                ++m_wholeFrames;
            }
            m_presentAll = false;
            SDL_UpdateWindowSurface(m_window.get());
            return;
        }
        thread_local std::vector<SDL_Rect> rects;
        rects.clear();
        long long changed = 0;
        for (int top = 0; top < height; top += kTileHeight) {
            const int bandHeight = std::min(kTileHeight, height - top);
            int runStart = -1;
            for (int left = 0; left <= width; left += kTileWidth) {
                bool dirty = false;
                if (left < width) {
                    const int tileWidth = std::min(kTileWidth, width - left);
                    const std::size_t bytes = static_cast<std::size_t>(tileWidth) * 4;
                    for (int y = top; y < top + bandHeight && !dirty; ++y) {
                        const std::size_t at = static_cast<std::size_t>(y) * static_cast<std::size_t>(pitch) + static_cast<std::size_t>(left) * 4;
                        dirty = std::memcmp(pixels + at, m_shownFrame.data() + at, bytes) != 0;
                    }
                    if (dirty) {
                        for (int y = top; y < top + bandHeight; ++y) {
                            const std::size_t at = static_cast<std::size_t>(y) * static_cast<std::size_t>(pitch) + static_cast<std::size_t>(left) * 4;
                            std::memcpy(m_shownFrame.data() + at, pixels + at, bytes);
                        }
                        changed += static_cast<long long>(tileWidth) * bandHeight;
                    }
                }
                if (dirty && runStart < 0) {
                    runStart = left;
                } else if (!dirty && runStart >= 0) {
                    // joined with the band above when it is the same span, so a tall thing is one copy
                    if (!rects.empty() && rects.back().x == runStart && rects.back().w == left - runStart && rects.back().y + rects.back().h == top) {
                        rects.back().h += bandHeight;
                    } else {
                        rects.push_back({runStart, top, std::min(left, width) - runStart, bandHeight});
                    }
                    runStart = -1;
                }
            }
        }
        const long long all = static_cast<long long>(width) * height;
        m_wholeFrames = changed * 10 >= all * 8 ? m_wholeFrames + 1 : 0; // 80 % or more changed
        if (!rects.empty()) {
            SDL_UpdateWindowSurfaceRects(m_window.get(), rects.data(), static_cast<int>(rects.size()));
        }
    }

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
