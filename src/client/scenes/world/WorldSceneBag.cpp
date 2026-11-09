#include "client/scenes/world/WorldScene.h"
#include "client/data/StarterRoles.h"

namespace client {

    BagRequests::Hooks WorldScene::bagRequestHooks() {
        BagRequests::Hooks hooks;
        hooks.swimming = [this] { return m_player.inShallowWater(); };
        hooks.groundCount = [this] { return m_ground.things().size(); };
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.inHouseMap = [this] { return m_map && isHouseMap(m_map->id()); };
        hooks.fighting = [this] { return m_fightState != 0; };
        hooks.level = [this] { return m_status.level; };
        hooks.enoughOnlineTime = [this] { return m_enoughOnlineTime; };
        hooks.campItemId = [this] { return m_camps.ownItem(); };
        hooks.confirm = [this](const std::string& text, std::function<void()> onOk) {
            TalkDialog::Timing timing;
            timing.wrapChars = 32; // characters per line of the second talk box
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, std::move(timing));
        };
        hooks.bottlePool = [this](int kind) { return kind == 3 ? m_status.hpBottle : m_status.spBottle; }; // kind 3: the HP bottle pool, else the SP pool
        hooks.petWorn = [this](int place, int slot) {
            const auto& pet = m_pets.at(place);
            return pet && slot >= 1 && slot <= 6 ? pet->worn[static_cast<std::size_t>(slot)] : protocol::ItemInstance{}; // pet equipment slots 1 to 6
        };
        hooks.roleIndex = [this] { return roleIndex(m_player.look.bodyType, m_player.look.head); };
        return hooks;
    }

}
