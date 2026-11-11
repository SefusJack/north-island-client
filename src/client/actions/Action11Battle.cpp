#include "client/actions/Action11Battle.h"

#include "core/ByteReader.h"
#include "protocol/BattleLists.h"
#include "protocol/MessageId.h"

#include <iterator>

namespace client {

    void handleAction11Battle(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        switch (id.sub) {
            case 3: { // 11/3: why the player could not view or join a battle (u8 reason)
                static const char* const kReasons[] = {
                    "",
                    "Unable to view battle because the numbers are full.",
                    "There is no place for pets, so the pet cannot join the battle",
                    "There have been too many spectators",
                    "The battle is over"
                };
                const std::size_t reason = reader.u8();
                // reason 0 has no message, reasons 1.. index kReasons
                if (!reader.failed() && reason >= 1 && reason < std::size(kReasons)) {
                    context.events.onNoticeMessage(kReasons[reason], 1000); // notice duration in ms
                    return;
                }
                break;
            }
            case 250: // 11/250: the battle ground, the event's battle, the fighters (start)
                if (const auto start = protocol::decodeBattleStart(payload)) {
                    context.events.onBattleStart(*start);
                } else {
                    context.events.onUnhandled(id);
                }
                break;
            case 4: { // 11/4: fight state list (RevFightStateList)
                constexpr std::size_t kRecord = 8;
                const std::size_t count = (payload.size() - 2) / kRecord; // 2 = action and sub id bytes
                for (std::size_t i = 0; i < count; ++i) {
                    const int kind = reader.u8();
                    const std::int32_t playerId = reader.i32();
                    reader.u16();
                    const int state = reader.u8();
                    if (kind == 2) { // kind 2 = player
                        context.events.onPlayerFightState(playerId, state);
                    }
                }
                break;
            }
            case 5: // 11/5: fighters join the battle
                context.events.onFightersJoin(protocol::decodeBattleJoin(payload));
                break;
            case 251: // 11/251: the fighters' weapon looks
                context.events.onFighterWeaponLooks(protocol::decodeFighterWeaponLooks(payload));
                break;
            case 0: { // 11/0: i32 player, u16 code (0: the player left the battle)
                const std::int32_t playerId = reader.i32();
                const int code = reader.u16();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                if (code == 0) {
                    context.events.onPlayerLeftBattle(playerId);
                }
                break;
            }
            case 7: { // 11/7: u8 kind, u8 count, auto pill used (AddAutoPillUse)
                const int kind = reader.u8();
                const int count = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onAutoPillUsed(kind, count);
                break;
            }
            case 11: { // 11/11: i32 player, a fighter abstains (RevFightAbstain)
                const std::int32_t playerId = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onFighterAbstains(playerId);
                break;
            }
            case 9: { // 11/9: u8 times, u8 tell flag, auto heal potion
                const int times = reader.u8();
                const int tell = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                if (tell == 1) { // 1 = tell the player
                    context.events.onAutoHealPotion(times);
                }
                break;
            }
            case 10: { // 11/10: u8 battle type
                const int type = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onBattleType(type);
                break;
            }
            case 1: { // 11/1: u8 row, u8 col, that fighter runs off
                const int row = reader.u8();
                const int col = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onFighterLeaves(row, col);
                break;
            }
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
