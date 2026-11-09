#include "client/scenes/world/WorldScene.h"

#include "client/rules/CharacterActions.h"
#include "client/rules/Walking.h"
#include "protocol/EmotionRequests.h"
#include "protocol/FishingRequests.h"

namespace client {

    namespace {

        constexpr int kMessageMs = 2000; // message display time, ms
        constexpr int kFishingRodSubType = 107; // item sub type 107: fishing rods
        constexpr int kFishingPose = 46; // character action group 46: the fishing / sitting pose
        constexpr int kReachBefore = 6; // water search reach: cells before the player's cell (x and y)
        constexpr int kReachAfter = 7; // water search reach: cells after the player's cell (x and y)

    }

    bool WorldScene::isFishingRod(const formats::ItemRecord& item) { return item.subType == kFishingRodSubType && item.rank != 0; }

    void WorldScene::useFishingRod(int bagSlot, int rodKind) {
        if (const auto notice = m_bagMarks.lockNotice(bagSlot, 1)) { // allowed use mark 1 (BagMarks::kCompound)
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_bagMarks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 1000); // message display time in ms
            return;
        }
        if (m_player.stall) {
            m_messages.show("Please close the stall before fishing", kMessageMs);
            return;
        }
        if (!m_player.fishing) {
            m_player.rodKind = rodKind;
            m_fishingRodSlot = bagSlot;
            startFishing();
            m_itemWindow.close();
            return;
        }
        stopFishing();
    }

    void WorldScene::startFishing() {
        const auto refuse = [this](const char* text) { m_messages.show(text, kMessageMs); };
        if (m_fightState != 0) {
            refuse("Unable to fish during combat");
            return;
        }
        if (m_teams.inTeam(m_player.id)) {
            refuse("Please leave the team before fishing");
            return;
        }
        if (m_player.walking) {
            refuse("Please stop to fishing");
            return;
        }
        if (m_player.stall) {
            refuse("Please close the stall before fishing");
            return;
        }
        if (m_player.inShallowWater() || !m_map) {
            return;
        }
        const int cell = formats::MapFile::kCellSize;
        const core::Point own{m_player.position.x / cell, m_player.position.y / cell};
        const bool boatOnWater = m_player.vehicle && VehicleKinds::classOf(m_player.vehicle->kind) == VehicleKinds::kWaterClass &&
                                 m_map->tileAttribute(m_ownTile) == formats::MapFile::kDeepWater;
        if (m_player.vehicle && !boatOnWater) {
            refuse("Please take back the vehicle before fishing");
            return;
        }
        if (m_player.riding) {
            dismountOwnPet();
        }
        if (m_player.fishing) {
            refuse("Fishing");
            return;
        }
        if (m_map->tileAttribute(own) == 0) {
            std::optional<core::Point> water;
            for (int x = own.x - kReachBefore; x <= own.x + kReachAfter && !water; ++x) {
                for (int y = own.y - kReachBefore; y <= own.y + kReachAfter && !water; ++y) {
                    if (m_map->tileAttribute({x - 1, y - 1}) == formats::MapFile::kDeepWater) {
                        water = core::Point{(x - 1) * cell, (y - 1) * cell};
                    }
                }
            }
            if (water) {
                startAction(m_player, kFishingPose);
                m_services.connection.send(protocol::actionRequest(m_player.actionSlot()));
                m_player.direction = directionTowards(m_player.position, *water, m_player.direction);
                m_player.actionOffset = m_player.direction;
                m_services.connection.send(protocol::actionRequest(m_player.actionSlot()));
                m_player.fishing = true;
                m_bagMarks.setUseMark(m_fishingRodSlot, BagMarks::kFishingRod);
                m_services.connection.send(protocol::startFishingRequest(m_fishingRodSlot, m_player.rodKind));
            } else {
                refuse("No fishing area around here");
                clearFishing();
            }
        }
        if (boatOnWater) {
            m_player.fishing = true;
            m_bagMarks.setUseMark(m_fishingRodSlot, BagMarks::kFishingRod);
            m_services.connection.send(protocol::startFishingRequest(m_fishingRodSlot, m_player.rodKind));
        }
    }

    void WorldScene::clearFishing() {
        if (m_bagMarks.useMark(m_fishingRodSlot) == BagMarks::kFishingRod) {
            m_bagMarks.setUseMark(m_fishingRodSlot, 0);
        }
        m_player.rodKind = 0;
        m_fishingRodSlot = 0;
        m_player.fishing = false;
    }

    void WorldScene::stopFishing() {
        m_services.connection.send(protocol::stopFishingRequest());
        clearFishing();
    }

    void WorldScene::onFishingStarted(std::int32_t playerId, int rodKind) {
        const auto fisher = characterOf(playerId);
        if (!fisher) {
            return;
        }
        fisher->get().fishing = true;
        fisher->get().rodKind = rodKind;
        if (isOwn(playerId) && !fisher->get().vehicle) {
            startAction(m_player, kFishingPose);
            m_services.connection.send(protocol::actionRequest(m_player.actionSlot()));
        }
    }

    void WorldScene::onFishingStopped(std::int32_t playerId) {
        if (isOwn(playerId)) {
            clearFishing();
        } else if (const auto fisher = characterOf(playerId)) {
            fisher->get().fishing = false;
        }
    }

}
