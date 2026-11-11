#include "client/actions/Action53Fighter.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction53Fighter(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 9 || id.sub == 10) { // 53/9: monster met (i32 npc); 53/10: monster book bits
            core::ByteReader book(payload, 2); // body starts after the 2-byte action/sub header
            if (id.sub == 10) { // 53/10: monster book bits
                context.events.onMonsterBits(payload);
            } else if (const std::int32_t npcId = book.i32(); !book.failed()) {
                context.events.onMonsterMet(npcId);
            } else {
                context.events.onUnhandled(id);
            }
            return;
        }
        if (id.sub == 11) { // 53/11: stick hint notice
            context.events.onNoticeMessage("A stick can help someone not to get beat down", 3000); // notice display time in ms
            return;
        }
        if (id.sub == 6) { // 53/6: fight help message
            core::ByteReader help(payload, 2); // body starts after the 2-byte action/sub header
            const int mode = help.u8();
            const std::int32_t helperId = help.i32();
            help.i32();
            const int kind = help.u8();
            if (!help.failed()) {
                context.events.onFightHelp(mode, helperId, kind);
            } else {
                context.events.onUnhandled(id);
            }
            return;
        }
        if (id.sub == 4) { // 53/4: an item is thrown
            core::ByteReader thrown(payload, 2); // body starts after the 2-byte action/sub header
            const int itemId = thrown.u16();
            const int sourceRow = thrown.u8();
            const int sourceCol = thrown.u8();
            std::vector<std::pair<int, int>> targets;
            while (thrown.remaining() >= 2) { // a target record is 2 bytes (row, column)
                const int row = thrown.u8();
                targets.emplace_back(row, thrown.u8());
            }
            if (thrown.failed()) {
                context.events.onUnhandled(id);
            } else {
                context.events.onItemThrown(itemId, sourceRow, sourceCol, targets);
            }
            return;
        }
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        const int row = reader.u8();
        const int col = reader.u8();
        if (id.sub == 1) { // 53/1: fighter status
            const int kind = reader.u8();
            const int skill = reader.u16();
            const int place = reader.u8();
            if (reader.failed()) {
                context.events.onUnhandled(id);
                return;
            }
            context.events.onFighterStatus(row, col, kind, skill, place);
            return;
        }
        if (id.sub == 7) { // 53/7: a fighter moves to another cell
            const int toRow = reader.u8();
            const int toCol = reader.u8();
            if (reader.failed()) {
                context.events.onUnhandled(id);
                return;
            }
            context.events.onFighterMoved(row, col, toRow, toCol);
            return;
        }
        if (reader.failed() || (id.sub != 3 && id.sub != 5)) { // 53/3 knocked out and 53/5 acted are the only ones left
            context.events.onUnhandled(id);
            return;
        }
        if (id.sub == 3) { // 53/3: knocked off the field
            context.events.onFighterKnockedOut(row, col);
        } else {
            context.events.onFighterActed(row, col);
        }
    }

}
