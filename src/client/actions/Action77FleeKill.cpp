#include "client/actions/Action77FleeKill.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    namespace {

        /// @brief Reads the three prize name lists of 77/2 and tells each as a chat line.
        /// @details Original: RecvFleeKillPrizeList; each list is i32 count, then u8 length + name.
        /// @param reader Reader positioned after the action and sub id.
        /// @param context Message context with the event sink.
        /// @return True when the message was read completely.
        bool prizeLists(core::ByteReader& reader, MessageContext& context) {
            // three prize lists
            const char* const titles[3] = {"The first prize list:", "The second prize list:", "The third prize list:"};
            std::string lines[3]; // three prize lists
            for (int list = 0; list < 3; ++list) { // three prize lists
                std::string text = titles[list];
                const std::int32_t count = reader.i32();
                for (std::int32_t i = 0; i < count && !reader.failed(); ++i) {
                    text += reader.shortText() + ",";
                }
                lines[list] = text + "The total players are" + std::to_string(count);
            }
            if (reader.failed()) {
                return false;
            }
            for (const std::string& line : lines) {
                context.events.onChat(0, 0, line);
            }
            return true;
        }

    }

    void handleAction77FleeKill(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: { // 77/1: u8 code, a flee-kill event notice (RecvFleeKillEvent)
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onFleeKillEvent(code);
                    return;
                }
                break;
            }
            case 2: // 77/2: three prize name lists (RecvFleeKillPrizeList)
                if (prizeLists(reader, context)) {
                    return;
                }
                break;
            case 3: { // 77/3: u16 own survival victories (RecvSurvivalVictory)
                const int victories = reader.u16();
                context.events.onSurvivalVictory(reader.failed() ? 0 : victories);
                return;
            }
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
