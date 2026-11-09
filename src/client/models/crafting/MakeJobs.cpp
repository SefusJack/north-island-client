#include "client/models/crafting/MakeJobs.h"

#include "client/models/house/Sandglass.h"

#include "core/ByteReader.h"

namespace client {

    namespace {

        constexpr int kMessageHead = 2;
        constexpr int kRecordSize = 8;

    }

    const MakeSlot& MakeJobs::slot(int index) const {
        static const MakeSlot empty;
        return index >= 1 && index <= kSlots ? m_slots[static_cast<std::size_t>(index)] : empty;
    }

    MakeSlot& MakeJobs::at(int index) { return m_slots[static_cast<std::size_t>(index)]; }

    int MakeJobs::freeSlot() const {
        for (int index = 1; index <= kSlots; ++index) {
            if (slot(index).productId == 0) {
                return index;
            }
        }
        return 0;
    }

    int MakeJobs::activeSlot() const {
        for (int index = 1; index <= kSlots; ++index) {
            if (slot(index).productId != 0 && slot(index).active) {
                return index;
            }
        }
        return 0;
    }

    void MakeJobs::clear() {
        m_slots = {};
        m_playerSlot = 0;
        m_petSlot = 0;
        m_petId = 0;
    }

    void MakeJobs::applyList(const core::Bytes& payload) {
        if (payload.size() < kMessageHead) {
            return;
        }
        core::ByteReader reader(payload);
        reader.skip(kMessageHead);
        for (std::size_t count = (payload.size() - kMessageHead) / kRecordSize; count > 0; --count) {
            const int index = reader.u8();
            MakeSlot record;
            record.productId = reader.u16();
            record.elapsedMs = static_cast<std::uint32_t>(reader.i32());
            record.batches = reader.u8();
            if (index >= 1 && index <= kSlots) {
                record.active = at(index).active;
                at(index) = record;
            }
        }
    }

    void MakeJobs::start(int slot, std::uint64_t nowMs) {
        if (slot < 1 || slot > kSlots) {
            return;
        }
        at(slot).active = true;
        m_playerSlot = slot;
        m_startMs = nowMs;
    }

    void MakeJobs::stop(int slot) {
        if (slot >= 1 && slot <= kSlots) {
            at(slot).active = false;
        }
        m_playerSlot = 0;
    }

    void MakeJobs::release(int slot) {
        if (slot >= 1 && slot <= kSlots) {
            at(slot) = {};
        }
        if (m_playerSlot == slot) {
            m_playerSlot = 0;
        }
    }

    void MakeJobs::fail(int slot, std::uint32_t fullMs) {
        if (slot >= 1 && slot <= kSlots) {
            at(slot).elapsedMs = fullMs;
            at(slot).active = false;
        }
        m_playerSlot = 0;
    }

    int MakeJobs::totalSeconds(int slotIndex, int recipeMinutes, int furnitureFlags, bool underSandglass) const {
        const int whole = recipeMinutes * 60 * slot(slotIndex).batches; // 60 seconds per minute
        if (underSandglass) {
            return Sandglass::jobSeconds(whole);
        }
        // 100 percent minus the savings of the furniture flags: bit 1 saves 15, bit 2 saves 2, bit 4 saves 3, bit 8 saves 3 percent
        const int percent = 100 - (furnitureFlags & 1) * 15 - ((furnitureFlags & 2) / 2) * 2 - ((furnitureFlags & 4) / 4) * 3 - ((furnitureFlags & 8) / 8) * 3;
        return percent * whole / 100; // percent
    }

    std::int64_t MakeJobs::workedSeconds(std::uint64_t nowMs) const {
        const std::int64_t sinceStart = nowMs >= m_startMs ? static_cast<std::int64_t>(nowMs - m_startMs) : 0;
        return (static_cast<std::int64_t>(slot(m_playerSlot).elapsedMs) + sinceStart) / 1000; // 1000 ms per second
    }

    int MakeJobs::barLength(int totalSeconds, std::uint64_t nowMs) const {
        const std::int64_t total = totalSeconds <= 0 ? 1 : totalSeconds;
        const std::int64_t worked = workedSeconds(nowMs);
        return total >= worked ? static_cast<int>(kBarLength * worked / total) : kBarLength;
    }

    int MakeJobs::secondsLeft(int totalSeconds, std::uint64_t nowMs) const {
        const std::int64_t total = totalSeconds <= 0 ? 1 : totalSeconds;
        return static_cast<int>(total - workedSeconds(nowMs));
    }

}
