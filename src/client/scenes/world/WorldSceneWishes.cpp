#include "client/scenes/world/WorldScene.h"

namespace client {

    namespace {

        constexpr int kBlissLamp = 34325; // item 34325: Bliss Lamp
        constexpr int kPassionBouquet = 34233; // item 34233: Passion Bouquet
        constexpr int kMessageMs = 2000; // message duration, ms

    }

    bool WorldScene::playerChosen(std::int32_t playerId) {
        if (m_insteadEcis.isOpen()) {
            if (playerId == 0) {
                return false;
            }
            m_insteadEcis.setName(speakerName(playerId));
        }
        if (m_blessBalloon.isOpen()) {
            if (playerId == 0) {
                return false;
            }
            m_blessBalloon.setName(speakerName(playerId));
        }
        return true;
    }

    BlessBalloonWindow::Hooks WorldScene::blessBalloonHooks() {
        BlessBalloonWindow::Hooks hooks;
        hooks.itemWindowBounds = [this] { return m_itemWindow.bounds(); };
        hooks.hideItemWindow = [this] { m_itemWindow.close(); };
        hooks.marksChanged = [this] { bagMarksChanged(); };
        hooks.inFight = [this] { return m_fightState != 0; };
        hooks.ownId = [this] { return m_player.id; };
        hooks.ownName = [this] { return m_player.name; };
        hooks.playerNamed = [this](const std::string& name) { return worldPlayerNamed(name); };
        hooks.askMessage = [this](int bagSlot, std::int32_t targetId) { m_blessMessage.open(bagSlot, targetId); };
        hooks.closeMessage = [this] { m_blessMessage.close(); };
        hooks.messageOpen = [this] { return m_blessMessage.isOpen(); };
        return hooks;
    }

    BlessMessageWindow::Hooks WorldScene::blessMessageHooks() {
        BlessMessageWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.closeBalloon = [this] { m_blessBalloon.close(); };
        return hooks;
    }

    LoveFlowerWindow::Hooks WorldScene::loveFlowerHooks() {
        LoveFlowerWindow::Hooks hooks;
        hooks.itemWindowBounds = [this] { return m_itemWindow.bounds(); };
        hooks.marksChanged = [this] { bagMarksChanged(); };
        hooks.inFight = [this] { return m_fightState != 0; };
        hooks.ownId = [this] { return m_player.id; };
        hooks.ownName = [this] { return m_player.name; };
        hooks.playerNamed = [this](const std::string& name) { return worldPlayerNamed(name); };
        hooks.askMessage = [this](int bagSlot, std::int32_t targetId) { m_loveFlowerMessage.open(bagSlot, targetId); };
        hooks.messageOpen = [this] { return m_loveFlowerMessage.isOpen(); };
        return hooks;
    }

    LoveFlowerMessageWindow::Hooks WorldScene::loveFlowerMessageHooks() {
        LoveFlowerMessageWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.releaseFlower = [this] { m_loveFlower.release(); };
        return hooks;
    }

    bool WorldScene::useWishItem(int itemId, int bagSlot) {
        if (itemId == kPassionBouquet) {
            if (m_fightState != 0) {
                m_messages.show("Can't be used during the battle", kMessageMs);
            } else {
                m_loveFlower.open(bagSlot);
            }
            return true;
        }
        if (itemId != kBlissLamp) {
            return false;
        }
        if (m_map && isHouseMap(m_map->id())) {
            m_messages.show("You cannot use this at home", kMessageMs);
        } else if (m_fightState != 0) {
            m_messages.show("Can't be used during the battle", kMessageMs);
        } else {
            m_blessBalloon.open(bagSlot);
        }
        return true;
    }

    void WorldScene::onBlessBalloonResult(int outcome, std::int32_t sender, std::int32_t target, const std::string& words) {
        if (outcome != 4) { // outcome 4: the balloon is put over the sender; 1 illegal words, 2 blacklist, 3 target in a fight
            if (outcome == 1) {
                m_messages.show("Don't use illegal words", kMessageMs);
            } else if (outcome == 2) { // outcome 2: sender is in the blacklist
                m_messages.show("You are in the blacklist Cannot use", kMessageMs);
            } else if (outcome == 3) { // outcome 3: target in a fight
                m_messages.show("The target is in a fight, cannot use", kMessageMs);
            }
            m_blessBalloon.release();
            m_blessMessage.setDone();
            return;
        }
        if (const auto character = characterOf(sender)) {
            const MapCharacter& blessed = character->get();
            m_wishEffects.startBless(sender, blessed.position, blessed.action == 0 ? std::optional<int>(blessed.direction) : std::nullopt, m_nowMs);
        }
        if (isOwn(sender)) {
            m_blessBalloon.release();
        }
        if (sender == target) {
            m_chat.add(0, "(System Prompt):" + speakerName(sender) + " " + words);
        } else {
            m_chat.add(0, "(System Prompt):" + speakerName(sender) + " told " + speakerName(target) + " " + words);
        }
    }

    void WorldScene::onLoveFlowerResult(int outcome, std::int32_t sender, std::int32_t target, const std::string& words) {
        if (outcome == 0) {
            if (sender == 1) {
                m_messages.show("You are in the blacklist Cannot use", kMessageMs);
            } else if (sender == 2) { // refusal reason 2: target in a fight (3 offline, 4 illegal words)
                m_messages.show("The target is in a fight, cannot use", kMessageMs);
            } else if (sender == 3) {
                m_messages.show("The target is offline.", kMessageMs);
            } else if (sender == 4) {
                m_messages.show("Don't use illegal words", kMessageMs);
            }
            m_loveFlowerMessage.setDone();
            m_loveFlower.release();
            return;
        }
        const std::string line = "(System Prompt):" + speakerName(sender) + " told " + speakerName(target) + " " + words;
        if (outcome == 1) {
            m_loveFlowerMessage.setDone();
            m_chat.add(0, line);
            m_loveFlower.release();
        } else if (outcome == 2) { // outcome 2: hearts over the target
            if (isOwn(target)) {
                m_wishEffects.startHearts(target, m_nowMs);
                m_chat.add(0, line);
            } else if (characterOf(target)) {
                m_wishEffects.startHearts(target, m_nowMs);
            }
        }
    }

}
