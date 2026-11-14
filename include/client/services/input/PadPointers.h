#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_POINTERS_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_PAD_POINTERS_H

#include "client/services/GameDirectory.h"
#include "gfx/RenderDevice.h"
#include "gfx/Texture.h"
#include "ui/Screen.h"

#include <optional>

namespace client {

    /// @brief The pointing hands a game pad's highlight is shown with: the game's own hand cursor, turned to point down and up.
    /// @details Original: cursor\cursor16.ani, a hand pointing left. Without the file the highlight falls back to a frame.
    class PadPointers {
        public:
            /// @brief Loads the hand and makes its two turned pictures.
            /// @param game Game directory to load from.
            /// @param device Device the pictures are made for.
            PadPointers(const GameDirectory& game, const gfx::RenderDevice& device);

            /// @brief Gives the hand that stands above a button and points down at it.
            /// @return The pointer; without a picture when the file could not be loaded.
            ui::PadPointer above() const { return {m_down ? &*m_down : nullptr, m_downTip, false}; }

            /// @brief Gives the hand that stands below a button and points up at it.
            /// @return The pointer; without a picture when the file could not be loaded.
            ui::PadPointer below() const { return {m_up ? &*m_up : nullptr, m_upTip, true}; }

        private:
            std::optional<gfx::Texture> m_down;
            std::optional<gfx::Texture> m_up;
            core::Point m_downTip; // the fingertip in the picture
            core::Point m_upTip;
    };

}

#endif
