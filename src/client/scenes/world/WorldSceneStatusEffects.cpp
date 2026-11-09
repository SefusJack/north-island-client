#include "client/scenes/world/WorldScene.h"
#include "client/rules/CharacterActions.h"

#include "protocol/GpsRequests.h"
#include "protocol/PlayerId.h"
#include "protocol/StatusEffectRequests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kDoubleExpMs = 1500; // double exp message duration, ms
        constexpr int kLuckyGrassMs = 1300; // lucky grass message duration, ms

    }

    void WorldScene::onDoubleExp(int kind, std::optional<int> minutes) {
        const std::string text = m_statusEffects.doubleExp(kind, minutes);
        if (!text.empty()) {
            m_messages.show(text, kDoubleExpMs);
        }
    }

    void WorldScene::onLuckyGrass(int kind, int minutes) {
        const std::string text = m_statusEffects.luckyGrass(kind, minutes);
        if (!text.empty()) {
            m_messages.show(text, kLuckyGrassMs);
        }
    }

    void WorldScene::onDreamingPill(int kind) {
        if (kind == 1 || kind == 2) {
            m_statusEffects.setDreamingPill(true);
        } else if (kind == 3) { // kind 3: pill off
            m_statusEffects.setDreamingPill(false);
        }
    }

    void WorldScene::onSpaBuns(int kind, std::optional<int> value) {
        const std::string text = m_statusEffects.spaBuns(kind, value);
        if (!text.empty()) {
            m_messages.show(text, kDoubleExpMs);
        }
    }

    void WorldScene::onExpPotion(int kind, int first, int minutes, int rate) {
        std::string text;
        switch (kind) {
            case 1: // exp potion kind 1: the potion's icon (rate)
                text = m_statusEffects.expPotionIcon(first, rate);
                break;
            case 2: // exp potion kind 2: the potion is refused
                text = m_statusEffects.expPotionRefused(first);
                break;
            case 3: // exp potion kind 3: the time remaining
                text = m_statusEffects.expPotionRemaining(first, minutes, rate);
                break;
            default: // any other kind: the potion has expired
                text = m_statusEffects.expPotionExpired(first);
                break;
        }
        if (!text.empty()) {
            m_messages.show(text, kDoubleExpMs);
        }
    }

    std::string WorldScene::itemNameOf(int itemId) const {
        const auto item = m_services.items.find(itemId);
        return item ? item->name : std::string();
    }

    void WorldScene::onTrainingItem(int kind, int effect, int minutes, bool sound) {
        constexpr int kTrainingIslandMap = 13050; // map 13050: Training Island
        constexpr int kTrainingItem = 34181; // item 34181: Training Voucher
        constexpr int kLongMs = 5000; // long message duration, ms
        const bool fighting = m_fightState != 0 || (m_map && m_map->id() == kTrainingIslandMap);
        const StatusEffects::TrainingNotice notice = m_statusEffects.trainingItem(kind, effect, minutes, sound, fighting, itemNameOf(kTrainingItem));
        if (notice.text.empty()) {
            return;
        }
        if (notice.askTeleport) {
            constexpr int kAskMs = 10000; // answer timeout, ms
            m_talk.ask(
                notice.text,
                std::nullopt,
                [this] { confirmTrainingTeleport(); },
                [] {},
                TalkDialog::Timing{32, kAskMs, [] {}, false}
            ); // 32: line length of the team-style box
        } else {
            m_messages.show(notice.text, kind == 1 ? 3000 : kLongMs); // message duration in ms (kind 1: 3000, others: 5000)
        }
        if (notice.sound) {
            m_services.playSound("sound/wav0152.wav");
        }
    }

    void WorldScene::askTimeLeft(int tag) {
        switch (tag) {
            case 2: // time effect icon 2: asks its time left (tag + 12)
            case 3: // time effect icon 3: asks its time left (tag + 12)
            case 4: // time effect icon 4: asks its time left (tag + 12)
                m_services.connection.send(protocol::timeEffectRequest(tag + 12)); // tags 2..4 map to time effect request ids 14..16
                break;
            case 9: // icon 9: the Dreaming Pill
                m_services.connection.send(protocol::dreamingPillRequest());
                break;
            case 12: // icon 12: the Training Island item
                m_services.connection.send(protocol::trainingIslandRequest(1));
                break;
            case 14: // icon 14: the body shift
                useBodyShiftIcon();
                break;
            default: // icon 16 (and any other): the strength buff
                m_services.connection.send(protocol::strengthBuffRequest());
                break;
        }
    }

    bool WorldScene::inTrainingIslandMap() const {
        // the Training Island map ids (PkMapIds)
        constexpr std::array<int, 7> kMaps = {
            13051, // Beach Area
            13056, // 1st Forest Area
            13057, // 2nd Forest Area
            13052, // 1st Cave Area
            13053, // 2nd Cave Area
            13054, // 1st Ocean Area
            13055 // 2nd Ocean Area
        };
        return m_map && std::find(kMaps.begin(), kMaps.end(), m_map->id()) != kMaps.end();
    }

    void WorldScene::confirmTrainingTeleport() {
        if (m_fightState != 0) {
            return;
        }
        // the three messages below show for 2000 ms
        if (m_player.stall) {
            m_messages.show("set a stall... cancel", 2000);
        } else if (m_player.vehicle) {
            m_messages.show("Using vehicle,cancel", 2000);
        } else if (isNoTeleportMap(m_map ? m_map->id() : 0)) {
            m_messages.show("The current scene doesn't allow you to use the function", 2000);
        } else {
            m_services.connection.send(protocol::trainingIslandRequest(2));
        }
    }

    void WorldScene::onStrengthBuffTimes(const std::array<int, 5>& minutes) {
        const std::string line = m_statusEffects.strengthBuffTimes(minutes);
        if (!line.empty()) {
            m_messages.show(line, 2000); // message duration in ms
        }
    }

    void WorldScene::onStrengthBuffMask(std::uint32_t mask, bool sound) {
        for (const std::string& line : m_statusEffects.strengthBuffMask(mask)) {
            m_messages.show(line, 2000); // message duration in ms
        }
        if (sound) {
            m_services.playSound("sound/wav0152.wav");
        }
    }

    void WorldScene::onExpPause(int sub, bool paused) {
        constexpr int kStopwatch = 34180; // item 34180: Stopwatch
        const std::string text = m_statusEffects.expPause(sub, paused, itemNameOf(kStopwatch));
        if (!text.empty()) {
            m_messages.show(text, 1000); // message duration in ms
        }
    }

    void WorldScene::onStatEnhanced(int stat) {
        static const char* const kLines[] = {
            "Strength increases 1",
            "Constitution increases 1",
            "Intelligence increases 1",
            "Wisdom increases 1",
            "Agility increases 1",
            "The pet can't be enhanced anymore"
        };
        m_messages.show(kLines[stat], stat == 5 ? 1500 : 1000); // stat 5 is the pet's refusal line; message durations in ms
        if (stat <= 4) { // stats 0..4 are the five character stats (play the sound)
            m_services.playSound("sound/wav0152.wav");
        }
    }

    void WorldScene::onBodyShift(int sub, int value) {
        constexpr int kMs = 2000; // message duration, ms
        std::string text;
        if (sub == 1) {
            text = m_statusEffects.bodyShift(value);
        } else if (sub == 3) { // sub 3: RecvBodyShiftRefusal
            text = StatusEffects::bodyShiftRefusal(value);
        } else {
            text = m_statusEffects.bodyShiftAsk(value);
            if (!text.empty()) {
                m_talk.ask(
                    text,
                    std::nullopt,
                    [this] { toggleBodyShift(); },
                    [] {},
                    TalkDialog::Timing{32, 5000, [] {}, false}
                ); // 32: line length of the team-style box; 5000: answer timeout in ms
            }
            return;
        }
        if (!text.empty()) {
            m_messages.show(text, kMs);
        }
    }

    void WorldScene::onHumanTransform(std::int32_t playerId, int npcId) {
        const auto character = characterOf(playerId);
        if (!character) {
            return;
        }
        if (npcId == 0) {
            character->get().shape.reset();
            return;
        }
        if (const auto npc = m_services.npcs.find(npcId)) {
            character->get().shape = *npc;
            stopAction(character->get());
        }
    }

    void WorldScene::toggleBodyShift() {
        constexpr int kSceneA = 10085; // map 10085: Wedding Hall
        constexpr int kSceneB = 10079; // map 10079: Quiz Zone
        constexpr int kHouseCellar = 4; // house room 4: the cellar
        // body shift state 2 is the shift being on; 2000 below is the message duration in ms
        if (m_statusEffects.bodyShiftState() == 2 && m_map && (m_map->id() == kSceneA || m_map->id() == kSceneB || m_houseRoom == kHouseCellar)) {
            m_messages.show("The scene can't be used", 2000); // message duration in ms
            return;
        }
        m_services.connection.send(protocol::bodyShiftToggleRequest(m_statusEffects.bodyShiftState() != 1));
    }

    void WorldScene::onPlayerEquipChanged(std::int32_t playerId, int itemId, bool worn) {
        const auto record = m_otherRecords.find(playerId);
        const auto character = m_others.find(playerId);
        const auto item = m_services.items.find(itemId);
        if (record == m_otherRecords.end() || character == m_others.end() || !item) {
            return;
        }
        std::vector<std::uint16_t>& equipment = record->second.appearance.equipment;
        equipment.erase(
            std::remove_if(
                equipment.begin(),
                equipment.end(),
                [&](std::uint16_t worn_) {
                    const auto other = m_services.items.find(worn_);
                    return other && other->equipSlot == item->equipSlot;
                }
            ),
            equipment.end()
        );
        if (worn) {
            equipment.push_back(static_cast<std::uint16_t>(itemId));
        }
        character->second.look = CharacterLook::fromAppearance(record->second.appearance, m_services.items, record->second.job);
    }

    void WorldScene::onPlayerEquipList(std::int32_t playerId, const std::vector<int>& items) {
        const auto record = m_otherRecords.find(playerId);
        const auto character = m_others.find(playerId);
        if (record == m_otherRecords.end() || character == m_others.end()) {
            return;
        }
        std::vector<std::uint16_t>& equipment = record->second.appearance.equipment;
        equipment.clear();
        for (const int itemId : items) {
            if (itemId > 0 && m_services.items.find(itemId)) {
                equipment.push_back(static_cast<std::uint16_t>(itemId));
            }
        }
        character->second.look = CharacterLook::fromAppearance(record->second.appearance, m_services.items, record->second.job);
    }

    void WorldScene::onPlayerOnlineStatus(std::int32_t playerId, int status) {
        if (const auto record = m_otherRecords.find(playerId); record != m_otherRecords.end()) {
            record->second.onlineStatus = status;
        }
    }

    void WorldScene::onPlayerJob(std::int32_t playerId, int job) {
        if (const auto record = m_otherRecords.find(playerId); record != m_otherRecords.end()) {
            record->second.job = job;
        }
        if (const auto character = m_others.find(playerId); character != m_others.end()) {
            character->second.look.job = job;
        }
        if (playerId == m_player.id || static_cast<std::uint32_t>(protocol::stripServerOffset(playerId)) == m_services.session.loginId) {
            m_status.job = job;
            m_player.look.job = job;
            refreshHud();
        }
    }

}
