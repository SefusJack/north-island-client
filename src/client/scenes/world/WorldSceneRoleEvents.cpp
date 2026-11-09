#include "client/data/RoleOrder.h"
#include "client/rules/Vitality.h"
#include "client/scenes/world/WorldScene.h"

#include "protocol/PlayerId.h"
#include "protocol/PointStoreRequests.h"
#include "protocol/WorldRequests.h"

namespace client {

    void WorldScene::onRoleChange(std::int32_t playerId, int role) {
        const auto split = splitRoleIndex(role);
        if (!split) {
            return;
        }
        if (playerId == m_player.id || static_cast<std::uint32_t>(protocol::stripServerOffset(playerId)) == m_services.session.loginId) {
            m_player.look.bodyType = split->first;
            m_player.look.head = split->second;
            dressPlayer();
            return;
        }
        const auto record = m_otherRecords.find(playerId);
        const auto character = m_others.find(playerId);
        if (record == m_otherRecords.end() || character == m_others.end()) {
            return;
        }
        record->second.appearance.bodyType = split->first;
        record->second.appearance.head = split->second;
        character->second.look = CharacterLook::fromAppearance(record->second.appearance, m_services.items, record->second.job);
    }

    void WorldScene::onWaitEventIcon(std::int32_t playerId, int mode) {
        const auto character = characterOf(playerId);
        if (!character) {
            return;
        }
        if (mode == 1) { // mode 1: show the waiting icon
            character->get().waitEventSinceMs = m_nowMs;
        } else if (mode == 2) { // mode 2: hide the waiting icon
            character->get().waitEventSinceMs.reset();
        }
    }

    void WorldScene::onTimedLight() { m_timedLight.start(m_camera.toScreen(m_player.position), m_nowMs); }

    void WorldScene::onTreasureNpcJump(int npcIndex) { m_treasureJump.start(npcIndex, m_nowMs); }

    void WorldScene::updateTreasureJump(std::uint64_t nowMs) {
        const auto step = m_treasureJump.update(nowMs);
        if (!step) {
            return;
        }
        auto npc = findNpc(step->npcIndex);
        if (!npc) {
            m_treasureJump.cancel();
            m_services.connection.send(protocol::nextEventResultRequest());
            return;
        }
        npc->get().spriteOffsetY = step->offsetY;
        if (step->landed) {
            m_services.connection.send(protocol::nextEventResultRequest());
            m_treasureLights.start(npc->get().body.position, nowMs);
        }
    }

    void WorldScene::onEffectLights(std::int32_t playerId, const std::vector<int>& kinds) {
        if (const auto character = characterOf(playerId)) {
            m_effectLights.set(playerId, character->get().position, kinds, m_nowMs);
        }
    }

    void WorldScene::onBuyLevelAsked(bool pet) {
        int total = 0;
        int points = 0;
        std::optional<TalkDialog::Speaker> speaker;
        if (pet) {
            const auto& active = m_pets.at(m_pets.activePlace());
            if (!active) {
                return;
            }
            total = totalLevel(active->level, active->reborn) + 1;
            points = total * 2;
            speaker = TalkDialog::Speaker{active->info, active->name};
        } else {
            total = totalLevel(m_status.level, m_status.reborn) + 1;
            points = total;
            speaker = TalkDialog::Speaker{m_player.look, m_player.name};
        }
        const std::string text =
            "Want to spend #RGame Points " + std::to_string(points) + " points/#R Advanced to lvl " + std::to_string(total) + " Yes or No?";
        m_talk.ask(
            text,
            speaker,
            [this, pet] {
                if (m_battle.isOn()) {
                    return;
                }
                m_services.connection.send(protocol::pointStoreLevelRequest(pet));
                m_eventRunner.moveOn();
            },
            [this] {
                if (!m_battle.isOn()) {
                    m_eventRunner.moveOn();
                }
            }
        );
    }

    void WorldScene::onBuyLevelResult(int code) {
        if (code != 1) { // result code 1: the pet cannot go over the player's level
            return;
        }
        if (m_battle.isOn()) {
            m_eventRunner.moveOn();
        }
        TalkDialog::Timing timing;
        timing.modal = true;
        m_talk.say("Can't upgrade your pet, players' level should be more than pets'.", std::nullopt, [] {}, std::move(timing));
    }

}
