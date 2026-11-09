#include "client/scenes/world/WorldScene.h"

namespace client {

    namespace {

        constexpr int kFirstHouseMap = 0xF80D; // map id 63501: first player-house map id
        constexpr int kLastHouseMap = 0xFFDC; // map id 65500: last player-house map id
        constexpr int kNoTeleportScene = 4; // scene flag bit 4: no teleporting
        constexpr int kTeleportMessageMs = 1500; // message display time, ms
        constexpr int kBusyMessageMs = 2000; // message display time, ms

        /// @brief Tells whether a map id is one of the fixed no-teleport maps.
        /// @param mapId Map id to test.
        /// @return True when teleporting out of or into the map is not allowed.
        /// @details Original: IsNoTeleportMapId.
        bool noTeleportMapId(int mapId) {
            return mapId == 10004 || mapId == 10009 || mapId == 12064;
        } // maps where teleporting is not allowed: 10004 Prison, 10009 Maze, 12064 2F the Capitol

    }

    Instances::Hooks WorldScene::instancesHooks() {
        Instances::Hooks hooks;
        hooks.send = [this](const core::Bytes& bytes) { m_services.connection.send(bytes); };
        hooks.scene = [this](int mapId) { return m_scenes ? m_scenes->find(mapId) : std::nullopt; };
        hooks.scenes = [this] { return m_scenes ? m_scenes->instanceScenes() : std::vector<int>(); };
        hooks.isInstanceMap = [this](int mapId) { return m_scenes && m_scenes->isInstanceMap(mapId); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.ownName = [this] { return m_player.name; };
        hooks.ownMap = [this] { return m_map ? m_map->id() : 0; };
        hooks.fighting = [this] { return m_fightState != 0; };
        hooks.inGuild = [this] { return m_guild.exists(); };
        hooks.cannotTeleport = [this] { return cannotTeleport(); };
        hooks.playerName = [this](std::int32_t id) -> std::optional<std::string> {
            const auto mate = teamMate(id);
            return mate ? std::optional<std::string>(mate->name) : std::nullopt;
        };
        hooks.message = [this](const std::string& text, int durationMs) { m_messages.show(text, durationMs); };
        hooks.chat = [this](const std::string& text) { m_chat.add(0, "(System Prompt):" + text); };
        hooks.detailOpen = [this] { return m_instanceDetail.isOpen(); };
        hooks.openDetail = [this] { m_instanceDetail.open(); };
        hooks.closeDetail = [this] { m_instanceDetail.close(); };
        hooks.windowOpen = [this] { return m_instanceWindow.isOpen(); };
        hooks.showWindow = [this] { m_instanceWindow.open(m_instanceWindow.position()); };
        hooks.createListChanged = [this] { m_instanceCreate.listChanged(); };
        return hooks;
    }

    InstanceWindow::Hooks WorldScene::instanceWindowHooks() {
        InstanceWindow::Hooks hooks;
        hooks.inGuild = [this] { return m_guild.exists(); };
        hooks.showParty = [this](core::Point at) { m_groupWindow.showParty(at); };
        hooks.showGuild = [this](core::Point at) { m_groupWindow.showGuildAt(at); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.player = [this](std::int32_t id) { return teamMate(id); };
        hooks.whisper = [this](std::int32_t id) { m_inputBar.setWhisperTarget(id); };
        return hooks;
    }

    void WorldScene::onInstanceMessage(const core::Bytes& payload) { m_instances.receive(payload); }

    bool WorldScene::isHouseMap(int mapId) { return mapId >= kFirstHouseMap && mapId <= kLastHouseMap; }

    int WorldScene::mapFileOf(int mapId) const {
        if (isHouseMap(mapId)) {
            return houseMapFile(m_houseRoom);
        }
        const int scene = m_scenes ? m_scenes->instanceSceneOf(mapId) : 0;
        return scene != 0 ? scene : mapId;
    }

    bool WorldScene::isNoTeleportMap(int mapId) const {
        const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
        return noTeleportMapId(mapId) || (mapId >= kFirstHouseMap && mapId <= kLastHouseMap) || (scene && (scene->flags & kNoTeleportScene) != 0);
    }

    bool WorldScene::cannotTeleport() {
        const int mapId = m_map ? m_map->id() : 0;
        if (m_fightState != 0) {
            m_messages.show("Unable to use in combat", kTeleportMessageMs);
        } else if (isNoTeleportMap(mapId)) {
            m_messages.show("Cannot teleport", kTeleportMessageMs);
        } else if (m_eventRunner.isRunning()) {
            m_messages.show("Doing quest, unable to teleport", kTeleportMessageMs);
        } else if (m_itemStall.isOpen() || m_petStall.isOpen()) {
            m_messages.show("Setting a stall, unable to teleport", kTeleportMessageMs);
        } else if (m_player.vehicle) {
            m_messages.show("Using vehicle, unable to teleport", kTeleportMessageMs);
        } else if (m_player.fishing) {
            m_messages.show("You are fishing, unable to teleport", kBusyMessageMs);
        } else if (m_itemTrade.isOpen() || m_petTrade.isOpen()) {
            m_messages.show("Trading and unable to teleport", kBusyMessageMs);
        } else {
            return false;
        }
        return true;
    }

}
