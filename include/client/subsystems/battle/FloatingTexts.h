#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FLOATING_TEXTS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FLOATING_TEXTS_H

#include "core/Point.h"
#include "gfx/Canvas.h"

#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief Words and numbers that float off a fighter ("Miss", "Fail", HP or SP given at once).
    /// @details Original: TNumber, the fighter's FNumberList; AddFloatingNumber, UpdateFloatingNumbers, DrawNumberPopup. Each
    /// travels to its target point at its speed and is gone on arrival; one starting from the same height as the last waits
    /// 200 ms more.
    class FloatingTexts {
        public:
            /// @brief Adds a floating text.
            /// @details Original: AddFloatingNumber. Colour 8 is HP's yellow-red, 9 SP's blue (for numbers); kind 4 arcs on its way.
            /// @param text The word or number.
            /// @param kind The text kind.
            /// @param from Start point.
            /// @param to Target point.
            /// @param speed Speed in pixels per millisecond.
            /// @param colour Colour / font index.
            /// @param nowMs Current time in milliseconds.
            void add(
                const std::string& text,
                int kind,
                core::Point from,
                core::Point to,
                double speed,
                int colour,
                std::uint64_t nowMs
            );

            /// @brief Moves the texts and drops those that arrived.
            /// @details Original: UpdateFloatingNumbers.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Draws the texts.
            /// @details Original: DrawNumberPopup.
            /// @param canvas Target canvas.
            void draw(gfx::Canvas& canvas);

            /// @brief Tells whether no text is floating.
            /// @return True when empty.
            bool empty() const { return m_texts.empty(); }

        private:
            /// @brief One floating word or number.
            struct Text {
                std::string text;
                int kind = 0;
                int x = 0;
                int y = 0;
                int baseY = 0;
                int targetX = 0;
                int targetY = 0;
                double speed = 0.24;
                int colour = 0;
                int distance = 0;
                std::uint64_t moveMs = 0;
                int startDelay = 0;
                bool arrived = false;
                core::Point drawnAt;
                core::Point drawnFrom;
            };

            /// @brief Moves a text one step towards its target, arcing it when its kind says so.
            /// @details Original: MoveNumberPopup / ApplyNumberArc.
            /// @param text The text.
            /// @param nowMs Current time in milliseconds.
            static void move(Text& text, std::uint64_t nowMs);

            std::vector<Text> m_texts;
    };

}

#endif
