#include "client/scenes/world/WorldScene.h"

namespace client {

    namespace {

        constexpr std::size_t kMainBoxWrap = 40; // characters per line in the talk box
        constexpr int kCaveMazeMap = 13224; // map 13224: Maze in Cave (GPS refused)
        constexpr int kShortMs = 1500; // message display time, ms
        constexpr int kLongMs = 2000; // message display time, ms

    }

    std::optional<GpsWindow::Refusal> WorldScene::gpsRefusal(bool teleport) {
        const int mapId = m_map ? m_map->id() : 0;
        if (m_fightState != 0) {
            return GpsWindow::Refusal{"Unable to use in combat", kShortMs, true};
        }
        if (isNoTeleportMap(mapId) || mapId == kCaveMazeMap) {
            return GpsWindow::Refusal{teleport ? "Cannot teleport" : "Cannot lacation", kShortMs, false};
        }
        if (!teleport) {
            if (m_player.vehicle) {
                return GpsWindow::Refusal{"Using vehicle, cannot locate", kShortMs, false};
            }
            return std::nullopt;
        }
        if (m_eventRunner.isRunning()) {
            return GpsWindow::Refusal{"Doing quest, unable to teleport", kShortMs, false};
        }
        if (m_itemStall.isOpen() || m_petStall.isOpen()) {
            return GpsWindow::Refusal{"Setting a stall, unable to teleport", kShortMs, false};
        }
        if (m_player.vehicle) {
            return GpsWindow::Refusal{"Using vehicle, unable to teleport", kShortMs, false};
        }
        if (m_player.fishing) {
            return GpsWindow::Refusal{"You are fishing, unable to teleport", kLongMs, false};
        }
        if (m_itemTrade.isOpen() || m_petTrade.isOpen()) {
            return GpsWindow::Refusal{"Trading and unable to teleport", kLongMs, false};
        }
        if (m_teams.inTeam(m_player.id)) {
            return GpsWindow::Refusal{"Teaming up, unable to teleport", kLongMs, false};
        }
        return std::nullopt;
    }

    void WorldScene::castleTeleport() {
        if (const auto refusal = gpsRefusal(true)) {
            m_messages.show(refusal->text, refusal->ms);
            if (refusal->closes) {
                m_guildWindow.close();
            }
            return;
        }
        m_teamFormTeleport = true;
        m_gpsEffect.begin(m_camera.toScreen(m_player.position), m_nowMs);
        m_guildWindow.close();
    }

    GpsWindow::Hooks WorldScene::gpsHooks() {
        GpsWindow::Hooks hooks;
        hooks.itemWindowBounds = [this] { return m_itemWindow.bounds(); };
        hooks.refusal = [this](bool teleport) { return gpsRefusal(teleport); };
        hooks.closeItemWindow = [this] { m_itemWindow.close(); };
        hooks.startTeleport = [this] { m_gpsEffect.begin(m_camera.toScreen(m_player.position), m_nowMs); };
        hooks.fighting = [this] { return m_fightState != 0; };
        hooks.askMain = [this](const std::string& text, std::function<void()> onOk) {
            TalkDialog::Timing timing;
            timing.wrapChars = kMainBoxWrap;
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, std::move(timing));
        };
        hooks.askTimed = [this](const std::string& text, std::function<void()> onOk) {
            TalkDialog::Timing timing;
            timing.wrapChars = 32; // characters per line of the second talk box
            timing.timeoutMs = 10000; // ask box auto-close time in ms (ten seconds)
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, std::move(timing));
        };
        hooks.choose = [this](const std::string& text, const std::vector<std::string>& choices, std::function<void(int)> onChoice) {
            TalkDialog::Timing timing;
            timing.wrapChars = 32; // characters per line of the second talk box
            timing.exclusive = false;
            m_talk.choose(text, std::nullopt, choices, std::move(onChoice), [] {}, std::move(timing));
        };
        hooks.sceneName = [this](int mapId) -> std::optional<std::string> {
            const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
            if (!scene) {
                return std::nullopt;
            }
            return scene->name;
        };
        return hooks;
    }

    void WorldScene::onGpsMark(int point, int map, int x, int y) { m_gps.setMark(point, map, x, y); }

}
