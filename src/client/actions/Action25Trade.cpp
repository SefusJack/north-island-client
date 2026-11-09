#include "client/actions/Action25Trade.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/PetTradeLists.h"
#include "protocol/TradeLists.h"
#include "protocol/UtensilTradeRequests.h"

namespace client {

    void handleAction25Trade(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: { // 25/1: i32 partner: an item trade opens
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onItemTradeStart(playerId);
                    return;
                }
                break;
            }
            case 2: { // 25/2: u8 how the item trade ended
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onItemTradeResult(result);
                    return;
                }
                break;
            }
            case 3: // 25/3: the partner's item offer
                if (const auto offer = protocol::decodeTradeOffer(payload)) {
                    context.events.onItemTradeOffer(*offer);
                    return;
                }
                break;
            case 31: { // 25/31: result of sending a flower
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onFlowerSent(result);
                    return;
                }
                break;
            }
            case 32: { // 25/32: a flower was received (i32 sender)
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onFlowerReceived(playerId);
                    return;
                }
                break;
            }
            case 40: { // 25/40: i32 partner: a utensil trade opens
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onUtensilTradeStart(playerId);
                    return;
                }
                break;
            }
            case 41: // 25/41: the partner's utensil offer
                if (const auto offer = protocol::decodeUtensilOffer(payload)) {
                    context.events.onUtensilTradeOffer(*offer);
                    return;
                }
                break;
            case 42: { // 25/42: u8 how the utensil trade ended
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onUtensilTradeResult(code);
                    return;
                }
                break;
            }
            case 10: { // 25/10: i32 partner: a pet trade opens
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onPetTradeStart(playerId);
                    return;
                }
                break;
            }
            case 11: { // 25/11: u8 how the pet trade ended
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetTradeResult(result);
                    return;
                }
                break;
            }
            case 12: // 25/12: the partner's pet offer
                if (const auto offer = protocol::decodePetTradeOffer(payload)) {
                    context.events.onPetTradeOffer(*offer);
                    return;
                }
                break;
            case 20: { // 25/20: i32 pet kind, u8 state: express pet off or back
                const std::int32_t pet = reader.i32();
                const int state = reader.u8();
                if (!reader.failed()) {
                    context.events.onExpressState(pet, state);
                    return;
                }
                break;
            }
            case 21: { // 25/21: i32 pet kind, u8 mode: pet walk animation (HandlePetWalkAnimPacket)
                const std::int32_t pet = reader.i32();
                const int mode = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetWalk(pet, mode);
                    return;
                }
                break;
            }
            case 33: { // 25/33: i32 npc, u8 mode: NPC walks by (HandleNpcWalkByPacket)
                const std::int32_t npc = reader.i32();
                const int mode = reader.u8();
                if (!reader.failed()) {
                    context.events.onNpcWalkBy(npc, mode);
                    return;
                }
                break;
            }
            case 22: { // 25/22: why the express failed
                const int code = reader.u8();
                const int item = code == 1 ? reader.u16() : 0; // code 1 carries an item id
                if (!reader.failed()) {
                    context.events.onExpressFailed(code, item);
                    return;
                }
                break;
            }
            case 23: { // 25/23: an item sent by express (kind, other player, item, count)
                const int kind = reader.u8();
                const std::int32_t other = reader.i32();
                const int item = reader.u16();
                const int count = reader.u8();
                if (!reader.failed()) {
                    context.events.onExpressResult(kind, other, item, count);
                    return;
                }
                break;
            }
            case 43: { // 25/43: u8 why a trade was refused
                const int reason = reader.u8();
                if (!reader.failed()) {
                    context.events.onTradeRefused(reason);
                    return;
                }
                break;
            }
            case 44: { // 25/44: u8 1 / 2: not / long enough online to trade
                const int state = reader.u8();
                if (!reader.failed()) {
                    context.events.onTradeOnlineTime(state == 2); // state 2: online long enough
                    return;
                }
                break;
            }
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
