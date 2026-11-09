#include "gfx/MouseCursor.h"

namespace gfx {

    std::optional<MouseCursor> MouseCursor::create(const formats::AnimatedCursor& source, int scale, formats::PixelArtFilter filter) {
        MouseCursor cursor;
        for (const formats::CursorFrame& frame : source.frames()) {
            const formats::Image image = scale > 1 ? formats::scalePixelArt(frame.image, filter, scale) : frame.image;
            SdlHandle<SDL_Surface> surface(SDL_CreateSurface(image.width(), image.height(), SDL_PIXELFORMAT_RGBA32));
            if (!surface) {
                return std::nullopt;
            }
            SDL_LockSurface(surface.get());
            for (int y = 0; y < image.height(); ++y) {
                Uint8* row = static_cast<Uint8*>(surface->pixels) + y * surface->pitch;
                for (int x = 0; x < image.width(); ++x) {
                    const core::Color c = image.at(x, y);
                    row[x * 4 + 0] = c.r; // RGBA32 surface: 4 bytes a pixel, offsets 0..3 are r, g, b, a
                    row[x * 4 + 1] = c.g;
                    row[x * 4 + 2] = c.b;
                    row[x * 4 + 3] = c.a;
                }
            }
            SDL_UnlockSurface(surface.get());
            SdlHandle<SDL_Cursor> sdlCursor(SDL_CreateColorCursor(surface.get(), frame.hotspot.x * scale, frame.hotspot.y * scale));
            if (!sdlCursor) {
                return std::nullopt;
            }
            cursor.m_frames.push_back(std::move(sdlCursor));
        }
        cursor.m_steps = source.steps();
        if (cursor.m_frames.empty() || cursor.m_steps.empty()) {
            return std::nullopt;
        }
        return cursor;
    }

    void MouseCursor::activate(std::uint64_t nowMs) {
        m_step = 0;
        m_stepStartMs = nowMs;
        SDL_SetCursor(m_frames[m_steps[0].frame].get());
    }

    void MouseCursor::update(std::uint64_t nowMs) {
        if (m_steps.size() < 2) {
            return;
        }
        const auto duration = static_cast<std::uint64_t>(m_steps[m_step].durationMs);
        if (nowMs - m_stepStartMs < duration) {
            return;
        }
        m_step = (m_step + 1) % m_steps.size();
        m_stepStartMs = nowMs;
        SDL_SetCursor(m_frames[m_steps[m_step].frame].get());
    }

}
