#include "client/actions/Action69PvpEvents.h"

#include "client/rules/PvpNotice.h"
#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    namespace {

        /// @brief Shows a PvP notice either as a chat line or as a timed notice message.
        /// @param notice The notice to show.
        /// @param context Message context with the event sink.
        void show(const PvpNotice& notice, MessageContext& context) {
            if (notice.chat) {
                context.events.onChat(0, 0, notice.text);
            } else {
                context.events.onNoticeMessage(notice.text, notice.durationMs);
            }
        }

        /// @brief Reads the three prize name lists of 69/2 and tells each as a chat line.
        /// @details Original: RecvPrizeLists; each list is i32 count, then i32 length + name (the length is stored in a Byte).
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
                    const auto length = static_cast<std::uint8_t>(reader.i32());
                    text += reader.text(length) + ",";
                }
                lines[list] = text + "The total players are" + std::to_string(count);
            }
            if (reader.failed()) {
                return false;
            }
            for (const std::string& line : lines) {
                context.events.onChat(0, 0, " " + line);
            }
            return true;
        }

    }

    void handleAction69PvpEvents(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: // 69/1: u8 code, PvP event notice (ShowPvpEventNotice)
            case 3: { // 69/3: u8 code, four-player PK notice (Show4PlayerPkNotice)
                const int code = reader.u8();
                if (!reader.failed()) {
                    const auto notice = id.sub == 1 ? pvpEventNotice(code) : fourPlayerPkNotice(code);
                    if (notice) {
                        show(*notice, context);
                    }
                    return;
                }
                break;
            }
            case 2: // 69/2: three prize name lists (RecvPrizeLists)
                if (prizeLists(reader, context)) {
                    return;
                }
                break;
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
