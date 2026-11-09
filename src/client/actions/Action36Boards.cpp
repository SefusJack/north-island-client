#include "client/actions/Action36Boards.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction36Boards(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 36/1: open the announcement form
                context.events.onAnnounceForm(true, std::string());
                return;
            case 2: // 36/2: the announcement text (rest of the message)
                context.events.onAnnounceForm(false, std::string(payload.begin() + 2, payload.end())); // 2: skip the action/sub header
                return;
            case 3: // 36/3: leave-word form opens (BK_LeaveWordForm.ShowCtrl)
            case 4: // 36/4: all leave words (RevAllLeaveWord)
            case 5: // 36/5: one leave word (RevLeaveWord)
            case 6: // 36/6: leave words cleared (RevClearLeaveWord)
            case 7: // 36/7: the leave-word request refusal/answer
                context.events.onLeaveWordMessage(payload);
                return;
            case 9: // 36/9: galaxy rank list (RecvGalaxyRank)
            case 10: // 36/10: galaxy rank form opens (TRE_GalaxyRankForm.ShowCtrl)
                context.events.onGalaxyRankMessage(payload);
                return;
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
