#include "client/scenes/world/WorldScene.h"

#include "client/models/house/Sandglass.h"
#include "protocol/SandglassMessages.h"

namespace client {

    namespace {

        constexpr int kNoticeMs = 2000; // message display time, ms

    }

    void WorldScene::askSandglass(int piece) {
        m_makeThing.close();
        m_makeInfo.close();
        m_sandglass.ask(piece);
        m_talk.ask("Do you want to use the sandglass", std::nullopt, [this] { confirmSandglass(); }, [] {});
    }

    void WorldScene::confirmSandglass() {
        if (m_sandglass.unused()) {
            m_messages.show("Some effects haven't been used and they can be used once only", kNoticeMs);
            return;
        }
        m_services.connection.send(protocol::sandglassRequest(m_sandglass.piece()));
        m_messages.show("Use sandglass to ", kNoticeMs);
    }

    void WorldScene::receiveSandglass(const core::Bytes& payload) {
        if (const auto used = protocol::decodeSandglassUsed(payload)) {
            if (used->piece != 0 && Sandglass::isSandglassItem(m_services.items, m_house.furniture().piece(used->piece).itemId)) {
                m_house.furniture().setUsedCount(used->piece, used->value);
            }
        } else if (const auto flags = protocol::decodeSandglassFlags(payload)) {
            m_sandglass.setFlags(*flags);
        }
    }

    void WorldScene::showSandglassEffects() {
        for (int slot = 1; slot <= MakeJobs::kSlots; ++slot) {
            if (!m_sandglass.slotActive(slot) || m_makeJobs.slot(slot).productId == 0) {
                continue;
            }
            const auto item = m_services.items.find(m_makeJobs.slot(slot).productId);
            if (item && !item->name.empty()) {
                m_messages.show("Use sandglass" + item->name, kNoticeMs);
            }
        }
        if (m_sandglass.unused()) {
            m_messages.show("Some effects haven't been used", kNoticeMs);
        }
    }

}
