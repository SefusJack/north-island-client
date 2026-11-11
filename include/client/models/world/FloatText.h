#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_FLOAT_TEXT_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_FLOAT_TEXT_H

#include <cstdint>
#include <optional>
#include <string>

namespace client {

    /// @brief The word that rises over a character ("LevelUP", "ExpUp").
    /// @details Original: TBasePlayer.ShowFloatText / DrawFloatText. Rises ten pixels every 100 ms, gone once the rise passes 100.
    class FloatText {
        public:
            /// @brief One frame of the floating word.
            struct Shown {
                std::string text;
                int rise;
            };

            /// @brief Starts a word again from the foot of the sprite.
            /// @details Original: ShowFloatText.
            /// @param text The word to show.
            /// @param nowMs Current time in ms.
            void show(const std::string& text, std::uint64_t nowMs);

            /// @brief Advances the word's clock (DrawFloatText's clock); the frame that reaches a rise over 100 is the last.
            /// @param nowMs Current time in ms.
            /// @return The word and its rise for this frame, nothing when nothing floats.
            std::optional<Shown> update(std::uint64_t nowMs);

        private:
            std::string m_text;
            std::uint64_t m_lastMs = 0;
            int m_rise = 0;
    };

}

#endif
