#include "client/actions/Action50BattleActions.h"

#include "core/ByteReader.h"
#include "protocol/BattleLists.h"
#include "protocol/FighterCast.h"
#include "protocol/MessageId.h"

#include <algorithm>

namespace client {

    void handleAction50BattleActions(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 2: // 50/2: the fighters go back to their cells (RevAttackItem)
                context.events.onBattleItemRound();
                return;
            case 3: { // 50/3: a skill cast (RecvFightSkillCast)
                protocol::FighterCast cast;
                cast.casterRow = reader.u8();
                cast.casterCol = reader.u8();
                cast.targetRow = reader.u8();
                cast.targetCol = reader.u8();
                cast.skillId = reader.u16();
                cast.kind = reader.u8();
                cast.attribute = reader.u8();
                cast.negative = reader.u8();
                cast.amount = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onFighterCast(cast);
                return;
            }
            case 4: { // 50/4: fighter damage (RecvFighterDamage)
                const int kind = reader.u8();
                const int row = reader.u8();
                const int col = reader.u8();
                const std::int32_t damage = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onFighterDamage(kind, row, col, damage);
                return;
            }
            case 5: { // 50/5: fighter marks (RecvFighterMarks)
                const int count = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onFighterMarks(std::min<int>(count, static_cast<int>(reader.remaining())));
                return;
            }
            case 6: { // 50/6: fighter attack area (RecvFighterAttackArea)
                const int row = reader.u8();
                const int col = reader.u8();
                const int area = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onFighterAttackArea(row, col, area);
                return;
            }
            case 1: // 50/1: the actions played together (decoded after the switch)
                break;
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                return;
        }
        const auto actions = protocol::decodeBattleActions(payload);
        if (actions.empty()) {
            context.events.onUnhandled(id);
            return;
        }
        context.events.onBattleActions(actions);
    }

}
