#include "client/scenes/world/WorldScene.h"

#include "client/rendering/world/CampSprite.h"
#include "client/rules/ItemInfo.h"
#include "protocol/CastleRequests.h"
#include "protocol/MessageId.h"
#include "protocol/TentRequests.h"

namespace client {

    namespace {

        constexpr int kTent = 36002; // Tent
        constexpr int kTentPalette = 34022; // Tent Palette
        constexpr int kShapeClay = 34160; // Shape Clay
        constexpr int kCampItemType = 27; // item type 27: tents
        constexpr int kNoTents = 64; // scene flag bit 64: tents cannot be used on this map
        constexpr int kTrojanWarMaps[] = {10065, 10066, 10068}; // Trojan Round 1, Trojan Round 2, Trojan Round 3
        constexpr int kMessageMs = 1500; // message display time in ms

    }

    Camps::Hooks WorldScene::campsHooks() {
        Camps::Hooks hooks;
        hooks.isOwn = [this](std::int32_t id) { return isOwn(id); };
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.message = [this](const std::string& text, int durationMs) { m_messages.show(text, durationMs); };
        hooks.frameHeight = [this](int itemId) {
            const auto item = m_services.items.find(itemId);
            return item ? m_services.characters.campFrameHeight(item->lookIds[0]) : 0;
        };
        hooks.ownTentPitched = [this] { m_itemWindow.close(); };
        return hooks;
    }

    CampStyleWindow::Hooks WorldScene::campStyleHooks() {
        CampStyleWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& bytes) { m_services.connection.send(bytes); };
        hooks.itemWindowBounds = [this] { return m_itemWindow.bounds(); };
        hooks.moveItemWindow = [this](int left) { m_itemWindow.moveLeft(left); };
        hooks.marksChanged = [this] { bagMarksChanged(); };
        return hooks;
    }

    void WorldScene::onTentMessage(const core::Bytes& payload) {
        switch (protocol::MessageId::of(payload).sub) {
            case 1: m_camps.receiveAdded(payload); break; // 65/1: a tent was pitched (RecvCampAdded)
            case 2:
                m_messages.show("This scenes tents have reached the upper limit, no more can be added!!!", 2000);
                break; // 65/2: scene tent limit reached (message 2000 ms)
            case 3: m_camps.receiveList(payload); break; // 65/3: the scene's tent list
            case 4: m_camps.receiveTakenBack(payload); break; // 65/4: a tent was taken back
            case 5: m_messages.show("The host has taken back his tent", 2000); break; // 65/5: the host took his tent back (message 2000 ms)
            case 6:
                m_messages.show("The tents of this server has reached the upper limit, no more can be added!!!", 2000);
                break; // 65/6: server tent limit reached (message 2000 ms)
            case 8: m_messages.show("It is forbidden to enter this tent!", kMessageMs); break; // 65/8: entering this tent is forbidden
            case 11: // 65/11: house room message (shared handler)
            case 12: // 65/12: house room message (shared handler)
            case 13: receiveHouseRoomMessage(payload); break; // 65/13: house room message (shared handler)
            case 14: m_camps.receiveStyle(payload); break; // 65/14: the tent's style
            default: break; // other sub ids are ignored
        }
    }

    void WorldScene::onTentStyleRefused(int code) {
        if (code == 1) {
            m_messages.show("Only has impact on ordinary tent", 1000); // message display time in ms
        } else if (code == 2) { // refusal code 2: the player has no tent
            m_messages.show("Has no tent", 1000); // message display time in ms
        }
        m_campStyle.close();
    }

    void WorldScene::onTentStyleChooser(int bagSlot, int set) { m_campStyle.open(bagSlot, set); }

    void WorldScene::refreshCampOwners() {
        for (auto& [slot, camp] : m_camps.camps()) {
            camp.own = isOwn(camp.ownerId);
            camp.ownerName.clear();
            camp.ownerTitle.clear();
            if (camp.own) {
                camp.ownerName = m_player.name;
                camp.ownerTitle = m_house.doorplates().text(camp.ownerId);
            } else if (const auto mate = teamMate(camp.ownerId)) {
                camp.ownerName = mate->name;
                if (m_others.find(camp.ownerId) != m_others.end()) {
                    camp.ownerTitle = m_house.doorplates().text(camp.ownerId);
                }
            }
        }
    }

    void WorldScene::onTrojanWarFlag(std::int32_t playerId, bool disguised) {
        if (const auto player = characterOf(playerId)) {
            player->get().trojanWar = disguised;
        }
    }

    bool WorldScene::trojanWarTarget(const MapCharacter& other) const {
        if (!onTrojanWarMap() || other.id == m_player.id || !other.trojanWar) {
            return false;
        }
        const auto leader = m_teams.leaderOf(other.id);
        const auto mine = m_teams.leaderOf(m_player.id);
        return !leader || *leader != mine.value_or(0);
    }

    bool WorldScene::onTrojanBattleMap() const {
        constexpr int kSecondCastle = 10066; // map 10066: Trojan Round 2
        constexpr int kThirdCastle = 10068; // map 10068: Trojan Round 3
        return m_map && (m_map->id() == kSecondCastle || m_map->id() == kThirdCastle);
    }

    void WorldScene::leaveTrojanWar() {
        if (m_teams.isMember(m_player.id)) {
            m_messages.show("Cannot use", 3000); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::trojanWarLeaveRequest());
    }

    void WorldScene::resetTrojanWarFlag() {
        if (m_fightState == 0 && m_player.trojanWar && !onTrojanBattleMap()) {
            m_player.trojanWar = false;
            m_services.connection.send(protocol::trojanWarFlagResetRequest());
        }
    }

    bool WorldScene::onTrojanWarMap() const {
        const int mapId = m_map ? m_map->id() : 0;
        for (const int map : kTrojanWarMaps) {
            if (mapId == map) {
                return true;
            }
        }
        return false;
    }

    std::optional<int> WorldScene::campAt(core::Point screenPoint) {
        std::optional<int> found;
        int foundY = 0;
        const bool trojan = onTrojanWarMap();
        const bool showTents = m_systemWindow.settings().humInfo[8] != 0; // index 8 of the human info settings: show other players' tents
        for (const auto& [slot, camp] : m_camps.camps()) {
            const bool ownLook = trojan ? camp.own : (showTents || camp.own);
            const auto sprite = campSprite(camp, ownLook, m_services.items);
            if (sprite && m_services.characters.campContains(sprite->bank, sprite->id, m_camera.toScreen(camp.position), screenPoint) &&
                (!found || camp.position.y >= foundY)) {
                found = slot;
                foundY = camp.position.y;
            }
        }
        return found;
    }

    void WorldScene::clickCamp(int slot) {
        const auto found = m_camps.camps().find(slot);
        if (found == m_camps.camps().end() || m_eventRunner.isRunning()) {
            return;
        }
        if (m_player.stall) {
            m_messages.show("Setting a stall, cannot use tent", kMessageMs);
            return;
        }
        if (m_player.fishing) {
            m_messages.show("You are fishing, cannot use tent", kMessageMs);
            return;
        }
        if (!standsOnFlatGround(true)) {
            m_messages.show("You are not at home, cannot use tent ", kMessageMs);
            return;
        }
        m_services.connection.send(protocol::enterTentRequest(found->second.ownerId));
        m_eventRunner.startedElsewhere();
    }

    void WorldScene::takeBackTent() {
        const int slot = m_inventory.nextSlotWith(m_camps.ownItem());
        if (m_camps.ownMap() != 0 && slot != 0) {
            m_services.connection.send(protocol::takeBackTentRequest(slot));
        }
    }

    bool WorldScene::useTentItem(int itemId, int bagSlot) {
        if (itemId == kTentPalette || itemId == kShapeClay) {
            m_campStyle.request(bagSlot, itemId == kTentPalette ? 1 : 2); // style set 1: Tent Palette, 2: Shape Clay
            return true;
        }
        if (itemId != kTent) {
            return false;
        }
        const auto name = [this](int id) {
            const auto item = m_services.items.find(id);
            return item ? item->name : std::string();
        };
        const int mapId = m_map ? m_map->id() : 0;
        const auto scene = m_scenes ? m_scenes->find(mapId) : std::nullopt;
        if (!m_player.stall) {
            m_ownTile = {m_player.position.x / formats::MapFile::kCellSize, m_player.position.y / formats::MapFile::kCellSize};
        }
        if (m_player.stall) {
            m_messages.show("Setting a stall, cannot use tent", kMessageMs);
        } else if (!standsOnFlatGround()) {
            m_messages.show("You are not at home, cannot use tent ", kMessageMs);
        } else if (scene && (scene->flags & kNoTents) != 0) {
            m_messages.show("Cannot use tent here", kMessageMs);
        } else if (isHouseMap(mapId)) {
            m_messages.show("Unable to use at home" + name(itemId), kMessageMs);
        } else if (m_camps.ownMap() != 0) {
            m_messages.show(name(m_camps.ownItem()) + " is already open", kMessageMs);
        } else {
            return false;
        }
        return true;
    }

    void WorldScene::rightClickItem(int itemId, int bagSlot) {
        const auto item = m_services.items.find(itemId);
        if (!item) {
            return;
        }
        if (itemId == 34021) { // item id of the Magic Magnet
            m_magnet.giveUp();
        }
        if (isFishingRod(*item) && m_player.fishing && m_fishingRodSlot == bagSlot) {
            stopFishing();
        }
        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, ...)
        const int under = m_map ? m_map->tileAttribute(m_ownTile) : 0;
        if (m_player.vehicle && itemId == m_player.vehicle->itemId && under != 0 && under != formats::MapFile::kShallowWater) {
            m_messages.show("Unable to cancel transportation vehicle here", kMessageMs);
            return;
        }
        if (item->type == kVehicleType) {
            rideVehicleItem(bagSlot);
            return;
        }
        if (item->type == kCampItemType) {
            if (m_camps.ownMap() != 0 && itemId == m_camps.ownItem()) {
                const int mapId = m_map ? m_map->id() : 0;
                if (isHouseMap(mapId) && m_house.furniture().ownerId() == m_player.id) {
                    m_messages.show("Unable to take back now" + item->name, kMessageMs);
                } else {
                    takeBackTent();
                }
            }
            return;
        }
        constexpr int kStoreTank = 34399; // Fuel Bank
        const int kind = bottleKind(*item);
        if (kind == 3 || kind == 4 || itemId == kStoreTank) { // bottle kinds 3 and 4 (HP and SP Bottle)
            m_itemWindow.raiseAbovePopups();
            m_rightClickMenu.open(bagSlot, m_itemWindow.bagCellPosition(bagSlot));
        }
    }

}
