#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_CURSOR_SET_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_CURSOR_SET_H

#include "client/services/GameDirectory.h"
#include "formats/AnimatedCursor.h"
#include "formats/PixelArtScale.h"
#include "gfx/MouseCursor.h"
#include "ui/CursorKind.h"

#include <array>
#include <cstdint>
#include <optional>

namespace client {

    /// @brief The game's animated system cursors, one per ui::CursorKind.
    /// @details Original: cursor\cursor1.ani normally, cursor4.ani over controls, plus the battle's own cursors.
    class CursorSet {
        public:
            /// @brief Loads the cursor files from the game's data folder and creates the system cursors.
            /// @param game Game directory to load from.
            explicit CursorSet(
                const GameDirectory& game
            );

            /// @brief Makes the cursors that many times as big (RenderDevice::cursorScale) with the pixel-art filter; remade only on a change.
            /// @param scale Integer cursor scale.
            /// @param filter Pixel-art scaling filter.
            /// @param nowMs Current time in milliseconds, to restart the shown cursor's animation.
            void setScale(int scale, formats::PixelArtFilter filter, std::uint64_t nowMs);

            /// @brief Switches to the cursor of a kind (no-op when it is already shown).
            /// @param kind Cursor kind to show.
            /// @param nowMs Current time in milliseconds.
            void show(ui::CursorKind kind, std::uint64_t nowMs);

            /// @brief Advances the animation of the shown cursor.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            /// @brief Looks up the system cursor of a kind.
            /// @param kind Cursor kind.
            /// @return The cursor slot (empty when its file could not be loaded).
            std::optional<gfx::MouseCursor>& cursorFor(ui::CursorKind kind);

            static constexpr std::size_t kKinds = 12; // cursor kinds (ui::CursorKind values)
            std::array<std::optional<formats::AnimatedCursor>, kKinds> m_sources;
            std::array<std::optional<gfx::MouseCursor>, kKinds> m_cursors;
            int m_scale = 1;
            formats::PixelArtFilter m_filter = formats::PixelArtFilter::None;
            std::optional<ui::CursorKind> m_current;
    };

}

#endif
