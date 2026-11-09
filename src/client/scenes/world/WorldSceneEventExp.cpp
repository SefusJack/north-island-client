#include "client/scenes/world/WorldScene.h"
#include "client/rules/CharacterActions.h"
#include "protocol/PlayerId.h"

namespace client {

    namespace {

        constexpr int kShortMs = 2000; // message display time, ms

    }

    void WorldScene::onFloatText(bool pet, const std::string& text) {
        if (pet) {
            showPetFloatText(m_pets.activePlace(), text);
        } else {
            m_player.floatText.show(text, m_nowMs);
        }
    }

    void WorldScene::onTrainingOver(std::int32_t playerId, int flag) {
        if (static_cast<std::uint32_t>(protocol::stripServerOffset(playerId)) != m_services.session.loginId) {
            if (const auto other = characterOf(playerId)) {
                other->get().usedUtensil = 0;
                other->get().usedSeat = 0;
                stopAction(other->get());
            }
            return;
        }
        endTraining(m_player);
        if (flag == 2) { // 23/162/1 flag 2: the pet's training end
            showPetFloatText(m_pets.activePlace(), "ExpUp");
        }
        if (flag == 1 || flag == 2) { // flag 1: own training end, flag 2: pet training end
            m_messages.show("Training is over", kShortMs);
        }
    }

    void WorldScene::onEventExp(std::int32_t value) {
        m_chat.add(0, m_player.name + " obtain Exp:" + std::to_string(value), m_player.id);
        m_player.floatText.show("ExpUP", m_nowMs);
    }

    void WorldScene::onContestLevelUp(bool pet, std::int32_t level) {
        std::string name = m_player.name;
        if (pet) {
            const auto& active = m_pets.at(m_pets.activePlace());
            if (!active) {
                return;
            }
            name = active->name;
            showPetFloatText(m_pets.activePlace(), "LevelUP");
        } else {
            m_player.floatText.show("LevelUP", m_nowMs);
        }
        m_services.playSound("sound/wav0153.wav");
        m_messages.show(name + " level upgrade to " + std::to_string(level) + " level", kShortMs);
    }

}
