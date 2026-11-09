#include "client/models/world/Camps.h"

#include "core/ByteReader.h"

#include <array>

namespace client {

    namespace {

        constexpr int kMostCamps = 300; // most camps on a map
        constexpr int kRecordSize = 16; // bytes per camp record in the camp list packet
        constexpr int kMessageMs = 1500; // on-screen message time, ms
        // sprite ids (bank 009) of the camp styles, indexed by style number; index 0 = no style, 9001 = plain tent
        // Tent Palette styles 1..11
        constexpr std::array<int, 12> kPaletteKinds = {0, 9001, 9018, 9020, 9022, 9024, 9026, 9028, 9030, 9032, 9034, 9036};
        // Shape Clay styles 1..18 (same sprite id scheme; 9071 and up are in bank 009_c01)
        constexpr std::array<int, 19> kClayKinds =
            {0, 9001, 9039, 9041, 9043, 9045, 9047, 9049, 9051, 9071, 9073, 9075, 9077, 9079, 9082, 9084, 9086, 9090, 9092};

    }

    Camps::Camps(
        const formats::ItemTable& items,
        Hooks hooks
    )
        : m_items(items),
          m_hooks(std::move(hooks)) {}

    int Camps::kindId(int index, int set) {
        if (set == 1 && index >= 1 && index <= 11) { // Tent Palette set (1) has styles 1..11
            return kPaletteKinds[static_cast<std::size_t>(index)];
        }
        if (set == 2 && index >= 1 && index <= 18) { // Shape Clay set (2) has styles 1..18
            return kClayKinds[static_cast<std::size_t>(index)];
        }
        return 0;
    }

    std::optional<int> Camps::slotOfOwner(std::int32_t ownerId) const {
        for (const auto& [slot, camp] : m_camps) {
            if (camp.ownerId == ownerId) {
                return slot;
            }
        }
        return std::nullopt;
    }

    int Camps::add(
        std::int32_t ownerId,
        int tentItemId,
        core::Point position,
        int kind1,
        int kind2
    ) {
        int slot = 1; // first camp place is 1
        while (slot <= kMostCamps && m_camps.count(slot) != 0) {
            ++slot;
        }
        if (slot > kMostCamps) {
            return 0;
        }
        Camp camp;
        camp.ownerId = ownerId;
        camp.tentItemId = tentItemId;
        camp.position = position;
        camp.kind1 = kind1;
        camp.kind2 = kind2;
        camp.frameHeight = m_hooks.frameHeight(tentItemId);
        m_camps[slot] = std::move(camp);
        if (m_hooks.isOwn(ownerId)) {
            m_ownMap = m_hooks.mapId();
            m_ownItem = tentItemId;
            m_hooks.ownTentPitched();
        }
        return slot;
    }

    void Camps::receiveAdded(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // reading starts at byte 2, after the action and sub id
        const std::int32_t owner = reader.i32();
        const int item = reader.u16();
        const int x = reader.i32();
        const int y = reader.i32();
        const int kind1 = reader.u8();
        const int kind2 = reader.u8();
        if (!reader.failed()) {
            add(owner, item, {x, y}, kind1, kind2);
        }
    }

    void Camps::receiveList(const core::Bytes& payload) {
        // a record count of 16-byte records follows a 1-byte header (payload.size() > 1 guards it)
        const std::size_t count = payload.size() > 1 ? (payload.size() - 1) / kRecordSize : 0;
        core::ByteReader reader(payload, 2); // reading starts at byte 2, after the action and sub id
        for (std::size_t i = 0; i < count; ++i) {
            const std::int32_t owner = reader.i32();
            const int item = reader.u16();
            const int x = reader.i32();
            const int y = reader.i32();
            const int kind1 = reader.u8();
            const int kind2 = reader.u8();
            if (reader.failed() || add(owner, item, {x, y}, kind1, kind2) == 0) {
                break;
            }
        }
    }

    void Camps::receiveTakenBack(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // reading starts at byte 2, after the action and sub id
        const std::int32_t owner = reader.i32();
        if (reader.failed()) {
            return;
        }
        if (!m_hooks.isOwn(owner)) {
            if (const auto slot = slotOfOwner(owner)) {
                m_camps.erase(*slot);
            }
            return;
        }
        if (m_ownMap == m_hooks.mapId()) {
            const auto slot = slotOfOwner(owner);
            if (!slot) {
                return;
            }
            m_camps.erase(*slot);
        }
        const auto item = m_items.find(m_ownItem);
        m_hooks.message((item ? item->name : std::string()) + " have been taken back already", kMessageMs);
        m_ownMap = 0;
        m_ownItem = 0;
    }

    void Camps::receiveStyle(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // reading starts at byte 2, after the action and sub id
        const std::int32_t owner = reader.i32();
        const int kind1 = reader.u8();
        const int kind2 = reader.u8();
        if (reader.failed()) {
            return;
        }
        for (auto& [slot, camp] : m_camps) {
            if (camp.ownerId == owner) {
                camp.kind1 = kind1;
                camp.kind2 = kind2;
            }
        }
        if (m_hooks.isOwn(owner)) {
            m_hooks.message("The style of tent has been changed", kMessageMs);
        }
    }

    void Camps::clear() { m_camps.clear(); }

}
