#include "client/scenes/world/WorldScene.h"

namespace client {

    std::optional<protocol::PuppetPerson> WorldScene::puppetPersonOf(std::int32_t playerId) const {
        const auto found = m_otherRecords.find(playerId);
        if (found == m_otherRecords.end()) {
            return std::nullopt;
        }
        const protocol::OtherPlayerRecord& record = found->second;
        protocol::PuppetPerson person;
        person.id = record.playerId;
        person.name = record.name;
        person.title = record.nickname;
        person.level = record.level;
        person.job = record.job;
        person.element = record.element;
        person.bodyType = record.appearance.bodyType;
        person.head = record.appearance.head;
        person.colorCode1 = record.appearance.colorCode1;
        person.colorCode5 = record.appearance.colorCode2;
        return person;
    }

    InsteadEcisWindow::Hooks WorldScene::insteadEcisHooks() {
        InsteadEcisWindow::Hooks hooks;
        hooks.itemWindowBounds = [this] { return m_itemWindow.bounds(); };
        hooks.marksChanged = [this] { bagMarksChanged(); };
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFight = [this] { return m_fightState != 0; };
        hooks.ownLevel = [this] { return static_cast<int>(m_status.level); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.ownName = [this] { return m_player.name; };
        hooks.enoughOnlineTime = [this] { return m_enoughOnlineTime; };
        hooks.playerNamed = [this](const std::string& name) { return worldPlayerNamed(name); };
        hooks.playerKnown = [this](std::int32_t id) { return m_otherRecords.count(id) != 0; };
        hooks.worldPerson = [this](std::int32_t id) { return puppetPersonOf(id); };
        hooks.invited =
            [this](std::int32_t askerId, const std::string& text, std::function<void()> onOk, std::function<void()> onCancel, std::function<void()> onTimeout) {
                m_talk.ask(
                    text,
                    speakerOf(askerId),
                    std::move(onOk),
                    std::move(onCancel),
                    TalkDialog::Timing{32, 5000, std::move(onTimeout)}
                ); // 32 chars per line, 5000 ms timeout
            };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk) {
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, TalkDialog::Timing{32, 5000, [] {}, false}); // 32 chars per line, 5000 ms timeout
        };
        hooks.askExpand = [this](const std::string& text, std::function<void()> onOk) {
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, TalkDialog::Timing{32, 0, {}, false}); // 32 chars per line, no timeout
        };
        hooks.mailTo = [this](const protocol::PuppetPerson& person) { m_sendMail.openToPuppet(person); };
        hooks.whisper = [this](std::int32_t id) { m_inputBar.setWhisperTarget(id); };
        return hooks;
    }

    MemCardWindow::Hooks WorldScene::memCardHooks() {
        MemCardWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFight = [this] { return m_fightState != 0; };
        hooks.ownId = [this] { return m_player.id; };
        hooks.playerKnown = [this](std::int32_t id) { return m_otherRecords.count(id) != 0; };
        hooks.worldPerson = [this](std::int32_t id) { return puppetPersonOf(id); };
        hooks.request = [this](const std::string& text, std::function<void()> onOk, std::function<void()> onCancel, std::function<void()> onTimeout) {
            m_talk.ask(
                text,
                std::nullopt,
                std::move(onOk),
                std::move(onCancel),
                TalkDialog::Timing{32, 5000, std::move(onTimeout)}
            ); // 32 chars per line, 5000 ms timeout
        };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk) {
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, TalkDialog::Timing{32, 5000, [] {}, false}); // 32 chars per line, 5000 ms timeout
        };
        hooks.mailTo = [this](const protocol::PuppetPerson& person) { m_sendMail.openToPuppet(person); };
        hooks.whisper = [this](std::int32_t id) { m_inputBar.setWhisperTarget(id); };
        hooks.clearPartner = [this] { m_insteadEcis.clearPartner(); };
        return hooks;
    }

    void WorldScene::showPuppetForm() {
        if (m_puppetState.mode == 1) { // mode 1: the requester's form
            m_insteadEcis.showTraining();
        } else if (m_puppetState.mode == 2) { // mode 2: the helper's Memory Card form
            m_memCard.show();
        }
    }

    void WorldScene::onPuppetPacket(const protocol::PuppetPacket& packet) {
        switch (packet.option) {
            case 5: // 23/104 option 5: to the Memory Card form
            case 8: // 23/104 option 8: to the Memory Card form
            case 9: // 23/104 option 9: to the Memory Card form
            case 10: // 23/104 option 10: to the Memory Card form
            case 13: // 23/104 option 13: to the Memory Card form
            case 16: // 23/104 option 16: to the Memory Card form
            case 17: // 23/104 option 17: to the Memory Card form
                m_memCard.received(packet);
                break;
            case 6: // 23/104 option 6: RecvInsteadEcisEnd, the training ends
                if (m_player.id == packet.id) {
                    m_insteadEcis.end(packet.slot);
                    m_memCard.close();
                }
                break;
            case 15: // 23/104 option 15: to both forms
            case 21: // 23/104 option 21: to both forms
                m_insteadEcis.received(packet);
                m_memCard.received(packet);
                break;
            default: // other options: to the requester's form
                m_insteadEcis.received(packet);
                break;
        }
    }

}
