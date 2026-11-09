#include "client/scenes/world/WorldScene.h"

#include "client/rules/CharacterActions.h"
#include "protocol/EmotionRequests.h"

namespace client {

    namespace {

        constexpr int kPriestNpc = 14108; // npc 14108: Abba (the wedding priest)
        constexpr std::size_t kWeddingWrap = 32; // characters per line in the wedding talk box

    }

    Marriage::Hooks WorldScene::marriageHooks() {
        Marriage::Hooks hooks;
        hooks.send = [this](const core::Bytes& bytes) { m_services.connection.send(bytes); };
        hooks.player = [this](std::int32_t id) { return teamMate(id); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.message = [this](const std::string& text, int durationMs) { m_messages.show(text, durationMs); };
        hooks.chat = [this](const std::string& text) { m_chat.add(0, text, 10); }; // 10 = chat speaker id used for wedding lines
        hooks.speak = [this](const std::string& text, const std::optional<TeamMate>& person, int durationMs, bool indentWrapped) {
            weddingSays(text, person, durationMs, indentWrapped);
        };
        hooks.turn = [this](std::int32_t id, int direction) {
            if (const auto other = m_others.find(id); other != m_others.end()) {
                other->second.direction = direction;
            }
        };
        hooks.setGold = [this](std::int32_t gold) { m_status.gold = gold; };
        hooks.lockInput = [this](bool locked) { m_inputLocked = locked; };
        hooks.playMusic = [this](const std::string& name) {
            const std::filesystem::path file = m_services.game.data("Sound/" + name + ".wav");
            if (std::filesystem::exists(file)) {
                m_services.music.playLooped(file);
            }
        };
        hooks.weddingFlag = [this](bool on) { m_weddingFlag = on; };
        hooks.sceneName = [this](int mapId) {
            const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
            return scene ? scene->name : std::string();
        };
        hooks.openApplication = [this] { m_marryApplication.open(); };
        hooks.closeApplication = [this] { m_marryApplication.close(); };
        hooks.openDivorce = [this] { m_marryDivorce.open(); };
        hooks.closeDivorce = [this] { m_marryDivorce.close(); };
        hooks.eventMoveOn = [this] { m_eventRunner.moveOn(); };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk, std::function<void()> onCancel) {
            TalkDialog::Timing timing;
            timing.wrapChars = kWeddingWrap;
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), std::move(onCancel), std::move(timing));
        };
        hooks.playAction = [this](int slot) {
            startActionSlot(m_player, slot);
            m_services.connection.send(protocol::actionRequest(m_player.actionSlot()));
        };
        hooks.setPosition = [this](std::int32_t id, core::Point position) { onPlayerJumped(id, position); };
        hooks.hidePet = [this](std::int32_t id, bool hidden) { m_mapPets.setHidden(isOwn(id) ? m_player.id : id, hidden); };
        hooks.equip = [this](const protocol::ItemInstance& item) { onEquipment({item}); };
        return hooks;
    }

    MarryCardWindow::Hooks WorldScene::marryCardHooks() {
        MarryCardWindow::Hooks hooks;
        hooks.showFriends = [this](core::Point at) {
            m_socialWindow.openAt(at);
            m_socialWindow.showFriendTab();
        };
        hooks.showEmotions = [this](core::Point at) {
            m_socialWindow.openAt(at);
            m_socialWindow.showEmoteTab();
        };
        hooks.showSetup = [this](core::Point at) {
            m_socialWindow.openAt(at);
            m_socialWindow.showSetupTab();
        };
        hooks.mailSpouse = [this] { m_sendMail.openToSpouse(); };
        hooks.whisper = [this](std::int32_t id) { m_inputBar.setWhisperTarget(id); };
        return hooks;
    }

    void WorldScene::onMarriageMessage(const core::Bytes& payload) { m_marriage.receive(payload); }

    void WorldScene::weddingSays(const std::string& text, const std::optional<TeamMate>& person, int durationMs, bool indentWrapped) {
        std::optional<TalkDialog::Speaker> speaker;
        if (person) {
            speaker = TalkDialog::Speaker{person->look, person->name, true};
        } else if (const auto priest = m_services.npcs.find(kPriestNpc)) {
            speaker = TalkDialog::Speaker{*priest, priest->name, true};
        }
        m_talk.say(text, speaker, {}, {kWeddingWrap, static_cast<std::uint64_t>(durationMs), {}, false, indentWrapped});
    }

}
