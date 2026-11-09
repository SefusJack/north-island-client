#include "client/scenes/world/WorldScene.h"

#include "protocol/ItemRequests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kMagicMagnet = 34021; // item 34021: Magic Magnet
        constexpr int kCarnieMap = 11094; // map 11094: Carnie
        constexpr int kCaveMazeMap = 13224; // map 13224: Maze in Cave
        constexpr int kWaterScene = 14; // scene type 14: water scene
        constexpr int kUseMessageMs = 1500; // message display time, ms

    }

    std::optional<MagnetPlayer> WorldScene::magnetPlayer(const protocol::OtherPlayerRecord& record) const {
        const auto& worn = record.appearance.equipment;
        return MagnetPlayer{record.playerId, record.name, record.level, record.reborn, std::find(worn.begin(), worn.end(), kMagicMagnet) != worn.end()};
    }

    MagnetWindow::Hooks WorldScene::magnetHooks() {
        MagnetWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.playerNamed = [this](const std::string& name) -> std::optional<MagnetPlayer> {
            for (const auto& [id, record] : m_otherRecords) {
                if (record.name == name) {
                    return magnetPlayer(record);
                }
            }
            return std::nullopt;
        };
        hooks.playerWithId = [this](std::int32_t id) -> std::optional<MagnetPlayer> {
            const auto found = m_otherRecords.find(id);
            return found != m_otherRecords.end() ? magnetPlayer(found->second) : std::nullopt;
        };
        hooks.ownName = [this] { return m_player.name; };
        hooks.ownId = [this] { return m_player.id; };
        hooks.ownLevel = [this] { return m_status.level; };
        hooks.ownWearsMagnet = [this] { return m_inventory.equipmentIds()[5] == kMagicMagnet; }; // equipment slot 5 holds the magnet
        hooks.inFight = [this] { return m_fightState != 0; };
        hooks.onVehicle = [this] { return m_player.vehicle.has_value(); };
        hooks.stalled = [this] { return m_player.stall.has_value() || m_itemStall.isOpen() || m_petStall.isOpen(); };
        hooks.sceneBarsUse = [this] {
            const int mapId = m_map ? m_map->id() : 0;
            const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
            return isNoTeleportMap(mapId) || mapId == kCaveMazeMap || (scene && scene->type == kWaterScene);
        };
        hooks.sceneBarsAnswer = [this] { return isNoTeleportMap(m_map ? m_map->id() : 0); };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk, std::function<void()> onCancel, std::function<void()> onTimeout) {
            TalkDialog::Timing timing;
            timing.wrapChars = 32; // characters per line of the request box
            timing.timeoutMs = 10000; // 10 seconds to answer
            timing.onTimeout = std::move(onTimeout);
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), std::move(onCancel), std::move(timing));
        };
        hooks.closeAsk = [this] { m_talk.close(); };
        hooks.marksChanged = [this] { bagMarksChanged(); };
        return hooks;
    }

    void WorldScene::chooseMagnetUse(int bagSlot) {
        TalkDialog::Timing timing;
        timing.exclusive = false;
        m_talk.choose(
            "Please choose the funciton",
            std::nullopt,
            {"Use Magic Magnet", "wear  Magic Magnet"},
            [this, bagSlot](int choice) {
                if (choice == 0) {
                    m_magnet.open(bagSlot);
                } else {
                    m_itemWindow.wearBagItem(bagSlot);
                }
            },
            [] {},
            std::move(timing)
        );
    }

    void WorldScene::onMagnetNotice(int kind, std::int32_t id, int arg) {
        if (kind == 1) { // notice kind 1: someone asks (kind 3 and others: the result)
            m_magnet.asked(id);
        } else if (kind == 2) { // notice kind 2: waiting
            m_magnet.waiting(id, arg);
        } else {
            m_magnet.result(arg);
        }
    }

    bool WorldScene::itemUnusableHere() {
        const int mapId = m_map ? m_map->id() : 0;
        const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
        std::optional<std::string> refusal;
        if (m_teams.inTeam(m_player.id)) {
            refusal = "You can't use when you are in a team";
        } else if (m_fightState != 0) {
            refusal = "You can't use it during the battle";
        } else if (m_player.vehicle) {
            refusal = "Can't use it when you use vehicles.";
        } else if (mapId == kCarnieMap) {
            refusal = "Carnie can't be used";
        } else if (isNoTeleportMap(mapId)) {
            refusal = "Can't use it in the scene";
        } else if (m_activeSport != 0) {
            refusal = "Can't use during game";
        } else if (m_player.fishing || m_fishingRodSlot != 0) {
            refusal = "Can't use during fishing";
        } else if (m_player.stall || m_itemStall.isOpen() || m_petStall.isOpen()) {
            refusal = "Can't use during running the stall";
        } else if (m_eventRunner.isRunning()) {
            refusal = "Can't use during the event";
        } else if (scene && scene->type == kWaterScene) {
            refusal = "Can't use in the water scene";
        }
        if (refusal) {
            m_messages.show(*refusal, kUseMessageMs);
        }
        return refusal.has_value();
    }

    void WorldScene::useCarnieVoucher(int bagSlot) {
        if (!itemUnusableHere()) {
            m_services.connection.send(protocol::openBoxRequest(bagSlot, 0));
        }
    }

}
