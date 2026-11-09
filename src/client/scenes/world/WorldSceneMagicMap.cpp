#include "client/scenes/world/WorldScene.h"

namespace client {

    MagicMapWindow::Hooks WorldScene::magicMapHooks() {
        MagicMapWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inTeam = [this] { return m_teams.inTeam(m_player.id); };
        hooks.refusal = [this] { return magicMapRefusal(); };
        hooks.inHouse = [this] {
            return isHouseMap(m_map ? m_map->id() : 0) || (m_houseRoom >= 2 && m_houseRoom <= 4);
        }; // house rooms 2 to 4 count as in the house
        hooks.cannotLeaveScene = [this] { return isNoTeleportMap(m_map ? m_map->id() : 0); };
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk) {
            TalkDialog::Timing timing;
            timing.exclusive = false;
            m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, timing);
        };
        return hooks;
    }

    std::string WorldScene::magicMapRefusal() {
        if (m_player.stall) {
            return "Can't be used when you are setting a stall";
        }
        if (m_eventRunner.isRunning()) {
            return "Can't be used during the event";
        }
        if (m_makeJobs.playerSlot() != 0 && m_makeJobs.petSlot() == 0) {
            return "Can't be used during making";
        }
        if (m_fightState != 0) {
            return "Can't be used during the battle";
        }
        if (m_activeSport != 0) {
            return "Can't be used during the game";
        }
        return {};
    }

    void WorldScene::useMagicMapItem(int bagSlot, int mode) {
        if (m_magicMap.use(bagSlot, mode)) {
            m_itemWindow.close();
        }
    }

    void WorldScene::onThingWear(int container, int slot, std::int32_t wear, int petPlace) {
        constexpr int kBag = 1;
        constexpr int kWorn = 2;
        constexpr int kPetWorn = 3;
        if (container == kBag) {
            m_inventory.setBagWear(slot, wear);
        } else if (container == kWorn) {
            m_inventory.setEquipmentWear(slot, std::min(wear, 250)); // maximum wear value shown
        } else if (container == kPetWorn) {
            auto& pet = m_pets.at(petPlace);
            if (pet && slot >= 1 && slot <= 6 && pet->worn[static_cast<std::size_t>(slot)].itemId != 0) { // pet worn slots 1 to 6
                pet->worn[static_cast<std::size_t>(slot)].wear = std::min(wear, 250); // maximum wear value shown
            }
        }
        inventoryChanged();
    }

    void WorldScene::onExhibitState(int cell, int itemId) {
        m_house.exhibit().setCell(cell);
        m_house.exhibit().setItem(itemId);
        m_showPlatform.setExhibitSlot(cell);
    }

    void WorldScene::onMagicMapRelease(int bagSlot) { m_magicMap.release(bagSlot); }

}
