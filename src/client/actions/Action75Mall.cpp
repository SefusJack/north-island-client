#include "client/actions/Action75Mall.h"

#include "core/ByteReader.h"
#include "protocol/MallMessages.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction75Mall(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: // 75/1: the catalog
                context.events.onMallCatalog(protocol::decodeMallCatalog(payload));
                return;
            case 2: // 75/2: the catalog again (changed)
                context.events.onMallCatalogUpdate(protocol::decodeMallCatalog(payload));
                return;
            case 3: { // 75/3: i32 points left, i32 spent, u16 item, u8 count
                const std::int32_t points = reader.i32();
                const std::int32_t spent = reader.i32();
                const int itemId = reader.u16();
                const int count = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMallPointsSpent(points, spent, itemId, count);
                return;
            }
            case 4: { // 75/4: u16 item, u8 category, u8 count, u16 key, u8 bought flag
                const int itemId = reader.u16();
                const int category = reader.u8();
                const int count = reader.u8();
                const int key = reader.u16();
                const int ok = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMallBought(itemId, category, count, key, ok != 0);
                return;
            }
            case 5: { // 75/5: u8 reason a purchase was refused
                const int reason = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMallRefused(reason);
                return;
            }
            case 6: { // 75/6: forge result (u8 outcome, u8 bag place, u8 fusion level)
                const int outcome = reader.u8();
                const int slot = reader.u8();
                const int level = reader.u8();
                if (outcome == 0) {
                    break;
                }
                context.events.onMallForgeResult(outcome, slot, level);
                return;
            }
            case 8: { // 75/8: u16 bonus points
                const int points = reader.u16();
                if (reader.failed()) {
                    break;
                }
                context.events.onMallBonusPoints(points);
                return;
            }
            case 9: { // 75/9: bonus purchase (i32 points, i32 spent, u16 item, u8 count)
                const std::int32_t points = reader.i32();
                const std::int32_t spent = reader.i32();
                const int itemId = reader.u16();
                const int count = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMallBonusSpent(points, spent, itemId, count);
                return;
            }
            case 10: // 75/10: RecvShopListB, bonus shop list, not shown
            case 11: // 75/11: RecvShopListUpdateB, bonus shop list update, not shown
                return;
            case 7: { // 75/7: u8 state, 1 opens the slot machines' tab
                const int state = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMallGamesTab(state == 1);
                return;
            }
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
