#include "protocol/BattleLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 4; // bytes before the records: action, sub, ground, event battle
        constexpr std::size_t kFighterRecord = 30; // bytes per fighter: u8 state + 29-byte record
        constexpr std::size_t kJoinRecord = 29; // bytes per joining fighter (state is shared, sent once)

        /// @brief Reads a fighter record after its state byte (u8 kind, i32 id, u16 map NPC, i32 owner, u8 row, u8 col, i32 max HP, u16 max SP, i32 HP, u16 SP, u8 level, u8 element, u8 reborn, u8 job).
        /// @param reader Reader positioned after the state byte.
        /// @param state The fighter's state, read by the caller.
        /// @return The fighter.
        FighterRecord readFighter(core::ByteReader& reader, int state) {
            FighterRecord fighter;
            fighter.state = state;
            fighter.kind = reader.u8();
            fighter.id = reader.i32();
            fighter.mapNpc = reader.u16();
            fighter.ownerId = reader.i32();
            fighter.row = reader.u8();
            fighter.col = reader.u8();
            fighter.maxHp = reader.i32();
            fighter.maxSp = reader.u16();
            fighter.hp = reader.i32();
            fighter.sp = reader.u16();
            fighter.level = reader.u8();
            fighter.element = reader.u8();
            fighter.reborn = reader.u8();
            fighter.job = reader.u8();
            return fighter;
        }

    }

    std::optional<BattleStart> decodeBattleStart(const core::Bytes& payload) {
        if (payload.size() < kHeader) {
            return std::nullopt;
        }
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        BattleStart start;
        start.ground = reader.u8();
        start.eventBattle = reader.u8();
        const std::size_t count = (payload.size() - kHeader) / kFighterRecord;
        for (std::size_t i = 0; i < count; ++i) {
            const int state = reader.u8();
            start.fighters.push_back(readFighter(reader, state));
        }
        return start;
    }

    std::vector<FighterRecord> decodeBattleJoin(const core::Bytes& payload) {
        std::vector<FighterRecord> fighters;
        // 3 = action, sub and the shared state byte; the reader starts after the first 2
        if (payload.size() < 3) {
            return fighters;
        }
        core::ByteReader reader(payload, 2);
        const int state = reader.u8();
        const std::size_t count = (payload.size() - 3) / kJoinRecord;
        for (std::size_t i = 0; i < count; ++i) {
            fighters.push_back(readFighter(reader, state));
        }
        return fighters;
    }

    std::vector<FighterWeaponLook> decodeFighterWeaponLooks(const core::Bytes& payload) {
        constexpr std::size_t kRecord = 12; // bytes per record: i32 id, i32 owner, u8 row, u8 col, u16 weapon item
        std::vector<FighterWeaponLook> looks;
        // 3 = action, sub and the leading u8 0
        if (payload.size() < 3) {
            return looks;
        }
        core::ByteReader reader(payload, 3);
        const std::size_t count = (payload.size() - 3) / kRecord;
        for (std::size_t i = 0; i < count; ++i) {
            FighterWeaponLook look;
            look.id = reader.i32();
            look.ownerId = reader.i32();
            look.row = reader.u8();
            look.col = reader.u8();
            look.itemId = reader.u16();
            looks.push_back(look);
        }
        return looks;
    }

    std::vector<BattleAction> decodeBattleActions(const core::Bytes& payload) {
        std::vector<BattleAction> actions;
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        while (reader.remaining() > 0) {
            const std::size_t length = reader.u16();
            const std::size_t end = reader.position() + length;
            BattleAction action;
            action.row = reader.u8();
            action.col = reader.u8();
            action.skillId = reader.u16();
            reader.u8();
            const int count = reader.u8();
            for (int i = 0; i < count; ++i) {
                BattleTarget target;
                target.row = reader.u8();
                target.col = reader.u8();
                target.hit = reader.u8() != 0;
                target.guard = reader.u8();
                const int values = reader.u8();
                for (int j = 0; j < values; ++j) {
                    BattleValue value;
                    value.attribute = reader.u8();
                    value.amount = reader.i32();
                    value.negative = reader.u8();
                    target.values.push_back(value);
                }
                action.targets.push_back(target);
            }
            if (reader.failed() || reader.position() > end) {
                return {};
            }
            reader.seek(end);
            actions.push_back(action);
        }
        return actions;
    }

}
