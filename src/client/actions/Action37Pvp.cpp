#include "client/actions/Action37Pvp.h"

#include "client/rules/PvpNotice.h"
#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    namespace {

        /// @brief Reads the guild competition's top three places and tells each non-empty one in a chat line.
        /// @details Original: RecvGuildCompetitionRanks.
        /// @param reader Reader positioned after the message header.
        /// @param context Message context with the event sink.
        /// @return false when the message was truncated.
        bool guildRanks(core::ByteReader& reader, MessageContext& context) {
            std::string lists[3]; // 3 ranked places
            std::int32_t counts[3] = {0, 0, 0}; // 3 ranked places
            for (int place = 0; place < 3; ++place) { // 3 ranked places
                counts[place] = reader.i32();
                for (std::int32_t i = 0; i < counts[place] && !reader.failed(); ++i) {
                    lists[place] += "The " + std::to_string(reader.i32()) + "guild ";
                }
            }
            if (reader.failed()) {
                return false;
            }
            for (int place = 0; place < 3; ++place) { // 3 ranked places
                if (counts[place] > 0) {
                    context.events.onChat(0, 0, "The " + std::to_string(place + 1) + " place in the competition :  " + lists[place]);
                }
            }
            return true;
        }

    }

    void handleAction37Pvp(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: { // 37/1: i32 player, u16 victories (RecvPvpVictories)
                const std::int32_t player = reader.i32();
                const int wins = reader.u16();
                if (!reader.failed()) {
                    context.events.onPvpVictories(player, wins);
                    return;
                }
                break;
            }
            case 2: // 37/2: i32 player, u8 rank (RecvPvpRank)
            case 6: { // 37/6: i32 player, u8 arena victories (RecvArenaVictory)
                const std::int32_t player = reader.i32();
                const int value = reader.u8();
                if (!reader.failed()) {
                    if (id.sub == 2) { // 37/2 is the rank, 37/6 the arena victory
                        context.events.onPvpRank(player, value);
                    } else {
                        context.events.onArenaVictory(player, value);
                    }
                    return;
                }
                break;
            }
            case 3: { // 37/3: u8 arena notice code (RecvArenaNotice)
                const int code = reader.u8();
                if (!reader.failed()) {
                    if (const auto notice = arenaNotice(code)) {
                        context.events.onChat(0, 0, notice->text);
                    }
                    return;
                }
                break;
            }
            case 4: // 37/4: ignored
                return;
            case 5: // 37/5: guild competition ranks
                if (guildRanks(reader, context)) {
                    return;
                }
                break;
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
