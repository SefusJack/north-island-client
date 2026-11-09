#include "client/actions/Action67Prizes.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <vector>

namespace client {

    namespace {

        constexpr int kShortMs = 2000; // notice display time, ms

    }

    void handleAction67Prizes(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 2: // 67/2: prize exchange refused (SPrizeFail_B)
            case 3: // 67/3: prize exchange refused (SPrizeFail_B)
                context.events.onNoticeMessage("Prize Exchange failed or have already exchanged them", kShortMs);
                return;
            case 4: // 67/4: same pet already owned, trade refused
                context.events.onNoticeMessage("You have the same pet, trade failed", kShortMs);
                return;
            case 5: { // 67/5: u16 count, then u16 skill ids the player forgot
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int count = reader.u16();
                std::vector<int> skills;
                for (int i = 0; i < count && !reader.failed(); ++i) {
                    skills.push_back(reader.u16());
                }
                if (!reader.failed()) {
                    context.events.onSkillsForgotten(skills);
                    return;
                }
                break;
            }
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
