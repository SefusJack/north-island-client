#include "client/scenes/world/WorldScene.h"

#include <algorithm>

namespace client {

    void WorldScene::onAtmForm(int form) {
        if (form == 1) {
            m_atmDeposit.show([this] { m_eventRunner.moveOn(); });
        } else if (form == 2) { // form 2: the transfer form
            m_atmTransfer.show([this] { m_eventRunner.moveOn(); });
        }
    }

    void WorldScene::onExchangeRate(std::int32_t rate) { m_pointExchange.open(rate); }

    void WorldScene::onExchangeGold(std::int32_t gold, std::int32_t gained) { m_pointExchange.goldReceived(gold, gained); }

    void WorldScene::onExchangeFee(int kind, std::int32_t playerId, std::int32_t value) { m_pointExchange.feeMessage(kind, playerId, value); }

    void WorldScene::onAtmBalance(int kind, std::int32_t amount, std::int32_t peerId) {
        constexpr std::uint64_t kMost = 400000000; // largest bank balance (400 million gold)
        constexpr std::int32_t kTransferSender = 10; // chat speaker id 10: the system sender used for transfer notices
        if (kind == 10) { // 26/10: the balance is set to amount
            m_atmGold = static_cast<std::uint32_t>(amount);
            return;
        }
        if (kind == 8) { // 26/8: amount is added (else 26/9: amount is taken off)
            m_atmGold = static_cast<std::uint32_t>(std::min<std::uint64_t>(static_cast<std::uint64_t>(m_atmGold) + static_cast<std::uint32_t>(amount), kMost));
        } else {
            m_atmGold -= static_cast<std::uint32_t>(amount);
        }
        if (peerId == 0 || speakerName(peerId).empty()) {
            return;
        }
        // 8 is a transfer received; the other kind is a transfer sent
        const std::string text = kind == 8 ? "Has received " + speakerName(peerId) + " Transfer money " + std::to_string(amount)
                                           : "Has " + std::to_string(amount) + " Transfer " + speakerName(peerId) + " Account";
        m_chat.add(0, text, kTransferSender);
    }
}
