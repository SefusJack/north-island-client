#include "client/actions/Action82Notices.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/NoticeLists.h"

namespace client {

    namespace {

        constexpr int kShortMs = 1300; // notice display time, ms
        constexpr int kDeletedMs = 2500; // display time of the "deleted" notice, ms

    }

    void handleAction82Notices(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: // 82/1: public board, the notice rows
            case 11: // 82/11: guild board, the notice rows
                context.events.onNoticeRows(
                    id.sub == 11, // sub 11 = guild board, 1 = public board
                    protocol::decodeNoticeRows(payload)
                );
                return;
            case 2: // 82/2: public board, one notice's content
            case 12: // 82/12: guild board, one notice's content
                if (const auto content = protocol::decodeNoticeContent(payload)) {
                    context.events.onNoticeContent(id.sub == 12, *content);
                    return;
                }
                break;
            case 3: // 82/3: public board, page info
            case 4: // 82/4: public board, page info
            case 13: // 82/13: guild board, page info
            case 14: // 82/14: guild board, page info
                if (const auto info = protocol::decodeNoticePageInfo(payload)) {
                    // subs 13 and up are the guild board; subs 3 and 13 are the first page info
                    context.events.onNoticePages(id.sub >= 13, *info, id.sub == 3 || id.sub == 13);
                    return;
                }
                break;
            case 5: // 82/5: cannot add information
                context.events.onNoticeMessage("New information can't be added", kShortMs);
                return;
            case 6: // 82/6: board capacity full
                context.events.onNoticeMessage("The capacity of message board is full", kShortMs);
                return;
            case 7: // 82/7: file not found
                context.events.onNoticeMessage("File not found", kShortMs);
                return;
            case 8: { // 82/8: u8 kind, opens the write form
                const int kind = reader.u8();
                if (!reader.failed()) {
                    context.events.onNoticeWriteForm(kind != 1);
                    return;
                }
                break;
            }
            case 9: // 82/9: the board was opened
                context.events.onNoticeBoardOpened();
                return;
            case 10: // 82/10: not allowed during the language forbidden period
                context.events.onNoticeMessage("Can't be used during language forbidden period", kShortMs);
                return;
            case 15: { // 82/15: guild notice deleted (name, title)
                const std::string name = reader.longText();
                const std::string title = reader.longText();
                if (!reader.failed()) {
                    context.events.onNoticeMessage("has deleted " + name + "[" + title + "]" + " message", kDeletedMs);
                    return;
                }
                break;
            }
            case 16: // 82/16: no right to delete the letter
                context.events.onNoticeMessage("Has no right to delete the letter", kShortMs);
                return;
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
