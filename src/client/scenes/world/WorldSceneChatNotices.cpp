#include "client/scenes/world/WorldScene.h"

#include "client/rules/GmIds.h"

namespace client {

    void WorldScene::onSystemAnnounce(const SystemAnnounce& announce) {
        std::string source;
        std::string prize;
        if (announce.kind == 1 || announce.kind == 2) { // announce kinds 1 and 2 name an item as the source
            if (const auto item = m_services.items.find(announce.sourceId)) {
                source = item->name;
            }
        }
        if (announce.kind == 2) { // announce kind 2: the prize is an NPC/monster id, otherwise an item id
            if (const auto npc = m_services.npcs.find(announce.prizeId)) {
                prize = npc->name;
            }
        } else if (const auto item = m_services.items.find(announce.prizeId)) {
            prize = item->name;
        }
        const std::string line = announce.line(source, prize);
        if (!line.empty()) {
            m_chat.announce(line);
        }
    }

    void WorldScene::onChatBlocked() {
        if (!isGmId(m_player.id)) {
            m_chatBlockedMs = m_nowMs;
        }
    }

    void WorldScene::onChatLine(std::int32_t speakerId, const std::string& name, int flags, const std::string& text) {
        if (m_blacklist.contains(speakerId, name)) {
            return;
        }
        std::string body = flags == 1 ? text.substr(0, 60) : text; // chat kind 1 is the "(All)" channel, cut at 60 characters
        // flags above 7 go to channel 10 (the system channel)
        m_chat.add(flags <= 7 ? flags : 10, (flags == 1 ? "(All)" : "") + name + ":" + body, speakerId, false);
    }

    void WorldScene::onFlowerSent(int result) {
        constexpr int kSystemChannel = 10; // chat channel 10: system messages
        if (result == 1) {
            m_chat.add(kSystemChannel, "Your flower has been sent.", 0, false);
        } else if (result == 2) { // result 2: target inventory full (result 1 is success)
            m_chat.add(kSystemChannel, "The target inventory is full.", 0, false);
        } else if (result == 3) { // result 3: not enough money
            m_messages.show("Not enough money", 1500); // message display time in ms
        }
    }

    void WorldScene::onAutoCollecting(const std::vector<std::int32_t>& playerIds, bool collecting) {
        for (const std::int32_t id : playerIds) {
            if (id == 0) {
                continue;
            }
            if (id == m_player.id) {
                if (!collecting) {
                    m_autoCollecting = false;
                    m_autoCollectNpc = 0;
                }
            } else if (const auto other = m_others.find(id); other != m_others.end()) {
                other->second.autoCollecting = collecting;
            }
        }
    }

    void WorldScene::onFlowerReceived(std::int32_t senderId) {
        constexpr int kSystemChannel = 10; // chat channel 10: system messages
        if (const auto record = m_otherRecords.find(senderId); record != m_otherRecords.end()) {
            m_chat.add(kSystemChannel, "You receive" + record->second.name + "flower", 0, false);
        }
    }

    void WorldScene::onPlayerLevel(std::int32_t playerId, int level) {
        if (const auto record = m_otherRecords.find(playerId); record != m_otherRecords.end()) {
            record->second.level = level;
        }
    }

    void WorldScene::onChatSilenced(int kind, int mask, std::uint32_t minutes) {
        const std::string line = m_chatSilence.silence(kind, mask, minutes);
        if (!line.empty()) {
            onChat(0, 0, line);
        }
    }

    void WorldScene::onChatSilenceReleased(bool mailToo) { m_chatSilence.release(mailToo); }

    void WorldScene::onHelpRecover(std::int32_t casterId, std::int32_t targetId, std::int32_t hp, int sp) {
        const auto nameOf = [this](std::int32_t id) -> std::optional<std::string> {
            if (isOwn(id)) {
                return m_player.name;
            }
            if (const auto other = m_others.find(id); other != m_others.end()) {
                return other->second.name;
            }
            return std::nullopt;
        };
        const auto caster = nameOf(casterId);
        const auto target = nameOf(targetId);
        if (!caster || !target) {
            return;
        }
        const std::string who = casterId == targetId ? *caster : *caster + "help" + *target;
        if (hp != 0) {
            // 10: the system chat channel
            m_chat.add(10, who + "recover HP:" + std::to_string(hp), targetId);
        }
        if (sp != 0) {
            // 10: the system chat channel
            m_chat.add(10, who + "recover SP:" + std::to_string(sp), targetId);
        }
    }

    void WorldScene::onRecoverNotice(int petPlace, std::int32_t hp, int sp) {
        std::string name = m_player.name;
        if (petPlace != 0) {
            const auto& pet = m_pets.at(petPlace);
            if (!pet) {
                return;
            }
            name = pet->name;
        }
        if (hp != 0) {
            // 10: the system chat channel
            m_chat.add(10, name + "recover HP:" + std::to_string(hp), m_player.id);
        }
        if (sp != 0) {
            // 10: the system chat channel
            m_chat.add(10, name + "recover SP:" + std::to_string(sp), m_player.id);
        }
    }

    void WorldScene::onPraise(int grade) {
        static const char* const kPraises[] = {"Oh, quite well!", "Oh, quite well!", "En, very well!", "Wow, great!"};
        const std::string text = kPraises[grade >= 1 && grade <= 3 ? grade : 0]; // grades 1 to 3 are valid, else the first text
        TalkDialog::Timing timing;
        timing.timeoutMs = 3000; // talk box time in ms (3 seconds)
        m_talk.say(text, ownSpeaker(true), {}, std::move(timing));
        m_services.playSound("sound/9911.wav");
    }

    void WorldScene::onOwnSystemLine(const std::string& text) { m_chat.add(0, text, m_player.id); }

    void WorldScene::onSoundEffect(const std::string& relative) { m_services.playSound(relative); }

    void WorldScene::onEventAnnounce() {
        constexpr int kArenaNpcId = 15162; // npc 15162 (not in the npc name table): shows the Arena Competition notice
        constexpr int kArenaDoor = 8; // door index 8: shows the Arena Competition notice
        const int index = m_eventRunner.eventNpc();
        auto npc = findNpc(index);
        const char* const arena = "There is an Arena Competition from 7:00pm to 9:00pm on every Sunday, Wednesday and Friday.";
        if (npc && npc->get().info.npcId == kArenaNpcId) {
            m_messages.show(arena, 2000); // message display time in ms
        } else if (index == 4) { // event NPC index 4: Team PK Competition
            m_messages.show("There is Team PK Competition from 7:00pm-9:00 every Sunday.", 2000); // message display time in ms
        } else if (index == 3) { // event NPC index 3: Wednesday Arena Competition
            m_messages.show("There is an Arena Competition from 7:00pm to 9:00pm on every Wednesday.", 2000); // message display time in ms
        } else if (index == 5) { // event NPC index 5: Challenge Arena Competition
            m_messages.show("There is Challenge Arena Competition from 7:00pm to 9:00pm on every Friday.", 2000); // message display time in ms
        } else if (index == 14) { // event NPC index 14: prizes wait for the Challenge Arena to end
            m_messages.show("Prizes can only be exchanged after Friday's Challenge Arena ends!",
                            2000); // message display time in ms
        } else if (m_eventRunner.eventDoor() == kArenaDoor) {
            m_messages.show(arena, 2000); // message display time in ms
        }
    }

    void WorldScene::onCortonPlace(int place, std::int32_t playerId, const std::string& name) {
        m_chat.add(0, "Congratulations! Capture the Evil Corton:The " + std::to_string(place) + " place in the competition " + name, m_player.id);
        if (isOwn(playerId)) {
            m_messages.show(
                "Congratulations. You have obtained The " + std::to_string(place) + " place in the competition ",
                2000
            ); // message display time in ms
        }
    }

    void WorldScene::onTalkNotice(int talkId, int durationMs) { m_messages.show(m_services.talks.text(talkId), durationMs); }

    void WorldScene::onVoucherItem(int itemId) {
        const auto record = m_services.items.find(itemId);
        m_centreMessages.show("obtain 1 " + (record ? record->name : std::string()), 1500); // message display time in ms
    }

    void WorldScene::onSkillsForgotten(const std::vector<int>& skills) {
        std::string names;
        for (const int id : skills) {
            if (const auto skill = m_services.skills.find(id)) {
                names += (names.empty() ? "" : "\r") + skill->name;
            }
        }
        if (!names.empty()) {
            m_messages.show("The following skills are forgotten: \r" + names, 3000); // message display time in ms
        }
    }

}
