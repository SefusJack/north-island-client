#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_EMOTE_HOTKEYS_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_EMOTE_HOTKEYS_H

#include "client/models/social/EmoteSlot.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace client {

    /// @brief The emote tab's Alt+1..8 slots and Ctrl+1..5 phrases, kept per character in the hotkey file.
    /// @details Original: user/<id>hotkey (LoadHotkeyFile / SaveHotkeyFile): eight i32 number + i32 kind; five phrases of 30 bytes
    /// (a length byte and 29 characters); one byte, the heart radar switch.
    class EmoteHotkeys {
        public:
            static constexpr int kSlots = 8; // Alt+1..8 emote slots
            static constexpr int kPhrases = 5; // Ctrl+1..5 phrases
            static constexpr std::size_t kPhraseLength = 29; // characters of a phrase (the file keeps 30 bytes with the length byte)

            /// @brief Clears the hotkeys (original ClearEmotionHotkeys), then reads the file; without one, slot 1 holds action 13 (sitting).
            /// @param file Hotkey file path.
            /// @return True when there was a file.
            bool load(const std::filesystem::path& file);

            /// @brief Writes the hotkeys to the file (original SaveHotkeyFile).
            /// @param file Hotkey file path.
            void save(const std::filesystem::path& file) const;

            /// @brief Gets an Alt+n slot.
            /// @param index Slot 1..8.
            /// @return The slot.
            const EmoteSlot& slot(int index) const { return m_slots[static_cast<std::size_t>(index - 1)]; }

            /// @brief Sets an Alt+n slot.
            /// @param index Slot 1..8.
            /// @param slot New slot content.
            void setSlot(int index, EmoteSlot slot) { m_slots[static_cast<std::size_t>(index - 1)] = slot; }

            /// @brief Gets a Ctrl+n phrase.
            /// @param index Phrase 1..5.
            /// @return The phrase text.
            const std::string& phrase(int index) const { return m_phrases[static_cast<std::size_t>(index - 1)]; }

            /// @brief Sets a Ctrl+n phrase, cut to the longest phrase.
            /// @param index Phrase 1..5.
            /// @param text New text.
            void setPhrase(int index, const std::string& text);

            /// @brief Tells whether the setup tab's heart radar switch is on (kept in the same file).
            /// @return True when on.
            bool heartRadar() const { return m_heartRadar == 1; }

            /// @brief Sets the heart radar switch.
            /// @param on New state.
            void setHeartRadar(bool on) { m_heartRadar = on ? 1 : 0; }

        private:
            std::array<EmoteSlot, kSlots> m_slots{};
            std::array<std::string, kPhrases> m_phrases{};
            std::uint8_t m_heartRadar = 0;
    };

}

#endif
