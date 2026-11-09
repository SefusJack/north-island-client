#include "client/scenes/world/WorldScene.h"

#include "client/rules/GmIds.h"

#include <algorithm>
#include <iterator>

namespace client {

    namespace {

        constexpr int kNoFunctionMaps[] = {10085, 12766}; // map 10085: Wedding Hall, map 12766: Altar (function buttons refused)
        constexpr int kCaveMazeMap = 13224; // map 13224: Maze in Cave (teleport refused)

    }

    ActivitiesWindow::Hooks WorldScene::activitiesHooks() {
        ActivitiesWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.sceneName = [this](int mapId) {
            const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
            return scene ? scene->name : std::string();
        };
        hooks.ownLevel = [this] { return m_status.level; };
        hooks.teleportRefused = [this] { return activityTeleportRefused(); };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk) {
            TalkDialog::Timing timing;
            timing.timeoutMs = 5000; // ask box auto-close time in ms
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, timing);
        };
        hooks.blink = [this](bool blinking) { m_functionButtons.setScheduleBlinking(blinking); };
        return hooks;
    }

    bool WorldScene::functionButtonAllowed() {
        const int mapId = m_map ? m_map->id() : 0;
        if (std::find(std::begin(kNoFunctionMaps), std::end(kNoFunctionMaps), mapId) != std::end(kNoFunctionMaps)) {
            m_messages.show("Cannot use this function", 2000); // message display time in ms
            return false;
        }
        if (isGmId(m_player.id)) {
            m_messages.show("Unable to use the function", 1200); // message display time in ms
            return false;
        }
        return true;
    }

    void WorldScene::toggleActivities() {
        if (functionButtonAllowed()) {
            m_activities.toggle();
        }
    }

    void WorldScene::openLottery() {
        if (functionButtonAllowed()) {
            m_lottery.open();
        }
    }

    bool WorldScene::activityTeleportRefused() {
        const int mapId = m_map ? m_map->id() : 0;
        const char* const cannot = "Cannot teleport";
        const char* const stall = "Setting a stall, unable to teleport";
        const char* const trading = "Trading and unable to teleport";
        const auto refuse = [this](const char* text, int ms) {
            m_messages.show(text, ms);
            return true;
        };
        if (isNoTeleportMap(mapId) || mapId == kNoFunctionMaps[0] || mapId == kCaveMazeMap) {
            return refuse(cannot, 1500); // message display time in ms
        }
        if (m_fightState != 0) {
            return refuse("You can't use it during the battle", 2000); // message display time in ms
        }
        if (m_eventRunner.isRunning()) {
            return refuse("Can't use during the event", 2000); // message display time in ms
        }
        if (m_player.stall) {
            return refuse(stall, 1500); // message display time in ms
        }
        if (m_player.vehicle) {
            return refuse("Using vehicle, unable to teleport", 1500); // message display time in ms
        }
        if (m_player.fishing) {
            return refuse("You are fishing, unable to teleport", 2000); // message display time in ms
        }
        if (m_itemTrade.isOpen() || m_petTrade.isOpen()) {
            return refuse(trading, 2000); // message display time in ms
        }
        if (m_teams.inTeam(m_player.id)) {
            return refuse("Teaming up, unable to teleport", 2000); // message display time in ms
        }
        return false;
    }

    void WorldScene::onActivityList(const protocol::ActivityList& list) { m_activities.scheduleReceived(list); }

    void WorldScene::onActivityStates(const protocol::ActivityStates& states) { m_activities.statesReceived(states); }

    void WorldScene::onActivityNotice(int kind) { m_activities.noticeReceived(kind); }

    void WorldScene::onLotteryInfo(const protocol::LotteryInfo& info) { m_lottery.infoReceived(info); }

    void WorldScene::onLotteryResult(const protocol::LotteryResult& result) { m_lottery.resultReceived(result); }

}
