#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_MESSAGE_ZOOM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_MESSAGE_ZOOM_H

#include "core/Rect.h"

#include <cstdint>

namespace client {

    /// @brief The opening animation of a talk box (AnimMsgOpenZoom, FAnimMode 1 of TSe_ShortMsgPanel).
    /// @details Original: a thin bar widens in four column steps, then grows in four row steps; a step is one millisecond of FStepInterval.
    class MessageZoom {
        public:
            /// @brief The margins and the longest line's and the whole text's size the steps are cut from.
            /// @details Original: CalcMsgPanelSize (FColStepPx, FRowStepPx).
            struct Shape {
                int marginLeft = 0;
                int marginRight = 0;
                int marginTop = 0;
                int marginBottom = 0;
                int longestLineWidth = 0;
                int textHeight = 0;
            };

            MessageZoom() = default;

            /// @brief Starts an opening.
            /// @param shape Margins and text size the steps are cut from.
            /// @param startMs Start time in ms.
            MessageZoom(
                const Shape& shape,
                std::uint64_t startMs
            );

            /// @brief One frame of the opening.
            /// @param nowMs Current time in ms.
            /// @param target The box that ends up (FTargetLeft, FTargetTop, FTargetWidth, FTargetHeight).
            /// @return The box to show now.
            /// @details Original: AnimMsgOpenZoom.
            core::Rect advance(std::uint64_t nowMs, const core::Rect& target);

            /// @brief Whether the opening is done and text and figure show.
            /// @return True once SetMsgTextVisible(1) was reached.
            bool textVisible() const { return m_textVisible; }

        private:
            static constexpr int kColSteps = 4;
            static constexpr int kRowSteps = 4;

            Shape m_shape;
            std::uint64_t m_startMs = 0;
            int m_colStep = 0;
            int m_rowStep = 0;
            int m_width = 0;
            int m_height = 0;
            bool m_textVisible = false;
    };

}

#endif
