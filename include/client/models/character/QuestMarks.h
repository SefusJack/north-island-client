#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_QUEST_MARKS_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_QUEST_MARKS_H

#include "client/models/character/MarkSlot.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief The player's quest marks (server action 24): counter marks in 200 places and the flag bits.
    /// @details Counter marks are quests going on; flag bits are quests done and other switches. changed() tells the quest window to rebuild its list (FMarkListChanged / FMarkFlagsChanged).
    class QuestMarks {
        public:
            static constexpr int kSlots = 200; // counter mark slots (quests going on) 1..200
            static constexpr int kFlagBytes = 256; // bytes of flag bits (quests done and switches), 8 bits each

            /// @brief Adds to a counter mark.
            /// @details Original: AddMarkCount (server 24/1).
            /// @param markId Mark id.
            /// @param count Amount to add.
            /// @return The place it went to, 0 when it did not fit.
            int add(int markId, int count);

            /// @brief Subtracts from a counter mark; an emptied place is freed.
            /// @details Original: SubMarkCount (server 24/2).
            /// @param markId Mark id.
            /// @param count Amount to subtract.
            /// @return The place of the mark.
            int subtract(int markId, int count);

            /// @brief Deletes a counter mark.
            /// @details Original: DeleteMark (server 24/4).
            /// @param markId Mark id.
            /// @return The place that was freed.
            int remove(int markId);

            /// @brief Sets one place of the mark list.
            /// @details Original: RecvMarkList (server 24/6), (place, mark, count) records.
            /// @param slot Place number.
            /// @param markId Mark id.
            /// @param count Counter value.
            void setSlot(int slot, int markId, int count);

            /// @brief Sets or clears one flag bit.
            /// @details Original: SetMarkFlag (server 24/5).
            /// @param flagBit Flag bit number (1-based).
            /// @param on New value.
            void setFlag(int flagBit, bool on);

            /// @brief Sets a whole flag byte.
            /// @details Original: RecvMarkFlags (server 24/7), whole bytes, 1-based.
            /// @param byteIndex Byte number from 1.
            /// @param bits The eight flag bits.
            void setFlagByte(int byteIndex, int bits);

            /// @brief The place of a mark.
            /// @details Original: FindMarkSlot.
            /// @param markId Mark id.
            /// @return Place number, 0 for none.
            int slotOf(int markId) const;

            /// @brief One mark place.
            /// @param index Place number.
            /// @return The slot.
            const MarkSlot& slot(int index) const { return m_slots.at(static_cast<std::size_t>(index)); }

            /// @brief Whether a flag bit is set; bit 1 is the lowest bit of byte 1.
            /// @details Original: HasMarkFlag.
            /// @param flagBit Flag bit number (1-based).
            /// @return True when set.
            bool hasFlag(int flagBit) const;

            /// @brief Whether marks changed since markSeen().
            /// @return True when the quest window should rebuild.
            bool changed() const { return m_changed; }

            /// @brief Clears the changed flag.
            void markSeen() { m_changed = false; }

        private:
            /// @brief The first free mark place.
            /// @details Original: FindFreeMarkSlot.
            /// @return Place number, 0 for none.
            int freeSlot() const;

            std::array<MarkSlot, kSlots + 1> m_slots{};
            std::array<std::uint8_t, kFlagBytes + 1> m_flagBytes{};
            int m_count = 0;
            bool m_changed = true;
    };

}

#endif
