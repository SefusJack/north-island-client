#include "client/models/character/QuestMarks.h"

namespace client {

    namespace {

        constexpr int kMostCount = 255; // largest stack count one mark slot can hold (one byte)

    }

    int QuestMarks::slotOf(int markId) const {
        for (int slot = 1; slot <= kSlots; ++slot) {
            if (m_slots[static_cast<std::size_t>(slot)].markId == markId) {
                return slot;
            }
        }
        return 0;
    }

    int QuestMarks::freeSlot() const {
        if (m_count == kSlots) {
            return 0;
        }
        return slotOf(0);
    }

    int QuestMarks::add(int markId, int count) {
        int slot = slotOf(markId);
        if (slot == 0) {
            slot = freeSlot();
            if (slot == 0) {
                return 0;
            }
            m_slots[static_cast<std::size_t>(slot)] = {markId, count};
            ++m_count;
        } else {
            MarkSlot& place = m_slots[static_cast<std::size_t>(slot)];
            if (kMostCount - place.count < count) {
                return 0;
            }
            place.count += count;
        }
        m_changed = true;
        return slot;
    }

    int QuestMarks::subtract(int markId, int count) {
        const int slot = slotOf(markId);
        if (slot == 0 || m_slots[static_cast<std::size_t>(slot)].count < count) {
            return 0;
        }
        MarkSlot& place = m_slots[static_cast<std::size_t>(slot)];
        place.count -= count;
        if (place.count == 0 && m_count > 0) {
            --m_count;
            place.markId = 0;
        }
        m_changed = true;
        return slot;
    }

    int QuestMarks::remove(int markId) {
        const int slot = slotOf(markId);
        if (slot == 0) {
            return 0;
        }
        --m_count;
        m_slots[static_cast<std::size_t>(slot)] = {};
        m_changed = true;
        return slot;
    }

    void QuestMarks::setSlot(int slot, int markId, int count) {
        if (slot < 1 || slot > kSlots) {
            return;
        }
        m_slots[static_cast<std::size_t>(slot)] = {markId, count};
        ++m_count;
        m_changed = true;
    }

    void QuestMarks::setFlag(int flagBit, bool on) {
        // 8 flag bits per byte; bit 1 is the lowest bit of byte 1
        const int byteIndex = (flagBit + 7) / 8;
        if (flagBit < 1 || byteIndex > kFlagBytes) {
            return;
        }
        const auto bit = static_cast<std::uint8_t>(1 << (flagBit - (byteIndex - 1) * 8 - 1));
        std::uint8_t& bits = m_flagBytes[static_cast<std::size_t>(byteIndex)];
        bits = on ? static_cast<std::uint8_t>(bits | bit) : static_cast<std::uint8_t>(bits & ~bit);
        m_changed = true;
    }

    void QuestMarks::setFlagByte(int byteIndex, int bits) {
        if (byteIndex < 1 || byteIndex > kFlagBytes) {
            return;
        }
        m_flagBytes[static_cast<std::size_t>(byteIndex)] = static_cast<std::uint8_t>(bits);
        m_changed = true;
    }

    bool QuestMarks::hasFlag(int flagBit) const {
        // 8 flag bits per byte; bit 1 is the lowest bit of byte 1
        const int byteIndex = (flagBit + 7) / 8;
        if (flagBit < 1 || byteIndex > kFlagBytes) {
            return false;
        }
        return (m_flagBytes[static_cast<std::size_t>(byteIndex)] >> (flagBit - (byteIndex - 1) * 8 - 1) & 1) != 0;
    }

}
