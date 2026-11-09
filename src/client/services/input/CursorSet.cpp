#include "client/services/input/CursorSet.h"

#include "core/Log.h"
#include "formats/AnimatedCursor.h"

#include <utility>

namespace client {

    namespace {

        /// @brief Loads one animated cursor file, logging a warning when it cannot be read.
        /// @param file Path of the .ani file.
        /// @return The cursor, empty on failure.
        std::optional<formats::AnimatedCursor> loadCursor(const std::filesystem::path& file) {
            auto source = formats::AnimatedCursor::load(file);
            if (!source) {
                core::logWarning("cannot read the cursor ", file.string());
            }
            return source;
        }

    }

    CursorSet::CursorSet(
        const GameDirectory& game
    )
        : m_sources{
              loadCursor(game.data("cursor/cursor1.ani")),
              loadCursor(game.data("cursor/cursor4.ani")),
              loadCursor(game.data("cursor/cursor3.ani")),
              loadCursor(game.data("cursor/cursor6.ani")),
              loadCursor(game.data("cursor/cursor8.ani")),
              loadCursor(game.data("cursor/cursor5.ani")),
              loadCursor(game.data("cursor/cursor11.ani")),
              loadCursor(game.data("cursor/cursor12.ani")),
              loadCursor(game.data("cursor/cursor13.ani")),
              loadCursor(game.data("cursor/cursor14.ani")),
              loadCursor(game.data("cursor/cursor18.ani")),
              loadCursor(game.data("cursor/cursor99.ani"))
          } {
        for (std::size_t i = 0; i < kKinds; ++i) {
            if (m_sources[i]) {
                m_cursors[i] = gfx::MouseCursor::create(*m_sources[i]);
            }
        }
    }

    void CursorSet::setScale(int scale, formats::PixelArtFilter filter, std::uint64_t nowMs) {
        if (scale == m_scale && (filter == m_filter || scale == 1)) {
            return;
        }
        m_scale = scale;
        m_filter = filter;
        core::logInfo("cursor ", scale, "x");
        for (std::size_t i = 0; i < kKinds; ++i) {
            m_cursors[i].reset();
            if (m_sources[i]) {
                m_cursors[i] = gfx::MouseCursor::create(*m_sources[i], scale, filter);
            }
        }
        if (const auto shown = std::exchange(m_current, std::nullopt)) {
            show(*shown, nowMs);
        }
    }

    std::optional<gfx::MouseCursor>& CursorSet::cursorFor(ui::CursorKind kind) { return m_cursors.at(static_cast<std::size_t>(kind)); }

    void CursorSet::show(ui::CursorKind kind, std::uint64_t nowMs) {
        if (m_current == kind) {
            return;
        }
        m_current = kind;
        if (auto& cursor = cursorFor(kind)) {
            cursor->activate(nowMs);
        }
    }

    void CursorSet::update(std::uint64_t nowMs) {
        if (!m_current) {
            return;
        }
        if (auto& cursor = cursorFor(*m_current)) {
            cursor->update(nowMs);
        }
    }

}
