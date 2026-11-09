#include "client/scenes/world/WorldScene.h"

#include "client/rules/ItemInfo.h"

#include <algorithm>
#include <iterator>

namespace client {

    namespace {

        // container codes of the 23/140/2 expiry packet
        constexpr int kBag = 1; // container 1: bag
        constexpr int kWorn = 2; // container 2: worn by the player
        constexpr int kPetWorn = 3; // container 3: worn by a pet
        constexpr int kWarehouse = 4; // container 4: storeroom
        constexpr int kOthersWorn = 5; // container 5: worn by another player
        constexpr int kFurniture = 6; // container 6: placed furniture
        constexpr int kBank = 7; // container 7: Item Depositor
        constexpr int kInnPet = 8; // container 8: worn by a pet kept at the inn
        constexpr int kItemBagCell = 9; // container 9: bonus space (item bag)
        constexpr int kExpiredWear = 250; // wear value 250: worn out completely (maximum)
        constexpr int kRepairTool = 130; // item sub type 130: time-limited items
        constexpr int kGoneMs = 3000; // message display time, ms
        // Tear Plier, Small Tear Plier, Teleport Scroll, Escape Button, Time Mantle, Stopwatch
        constexpr int kTimedItems[] = {34152, 34155, 34163, 34164, 34168, 34180};

        /// @brief Builds the item that takes the place of an expired one: a single piece, as worn out as can be.
        /// @param itemId Item id the server names as the replacement.
        /// @return The new item instance.
        protocol::ItemInstance replacement(int itemId) {
            protocol::ItemInstance item;
            item.itemId = itemId;
            item.count = 1;
            item.wear = kExpiredWear;
            return item;
        }

    }

    void WorldScene::onThingExpired(
        int container,
        int slot,
        int petPlace,
        int newItemId,
        int itemId,
        const std::string& owner
    ) {
        const auto nameOf = [this](int id) {
            const auto record = m_services.items.find(id);
            return record ? record->name : std::string();
        };
        const auto say = [this](const std::string& line) { m_chat.add(0, line, m_player.id); };
        const auto isPeriodItem = [this](int id) {
            const auto record = m_services.items.find(id);
            return record && usePeriod(*record) != 0;
        };
        switch (container) {
            case kBag: { // 23/140/2 container 1: an item of the bag
                if (slot < 1 || slot > Inventory::kBagSlots || m_inventory.bagItem(slot).empty() || m_inventory.isStackable(m_inventory.bagItem(slot).itemId)) {
                    return;
                }
                const protocol::ItemInstance item = m_inventory.bagItem(slot);
                const std::string name = nameOf(item.itemId);
                if (isPeriodItem(item.itemId)) {
                    say("Take along [" + name + "] The usage period is over, the item has been destroyed");
                }
                const auto record = m_services.items.find(item.itemId);
                const bool timed = record && (record->subType == kRepairTool ||
                                              std::find(std::begin(kTimedItems), std::end(kTimedItems), item.itemId) != std::end(kTimedItems));
                if (timed) {
                    m_messages.show(name + " Damage is gone", kGoneMs);
                    say("Take along [" + name + "]The time for using it is over");
                }
                m_inventory.takeFromBag(slot, item.count);
                if (newItemId != 0) {
                    m_inventory.addToBag(slot, replacement(newItemId));
                }
                inventoryChanged();
                return;
            }
            case kWorn: { // 23/140/2 container 2: an item the player wears
                if (slot < 1 || slot > Inventory::kEquipmentSlots || m_inventory.equipped(slot).empty() ||
                    m_inventory.isStackable(m_inventory.equipped(slot).itemId)) {
                    return;
                }
                const int worn = m_inventory.equipped(slot).itemId;
                if (isPeriodItem(worn)) {
                    say("Equipped[" + nameOf(worn) + "] The usage period is over, the item has been destroyed");
                }
                m_inventory.clearEquipment(slot);
                if (newItemId != 0) {
                    m_inventory.equip(replacement(newItemId));
                }
                dressPlayer();
                inventoryChanged();
                return;
            }
            case kPetWorn: { // 23/140/2 container 3: an item a pet wears
                auto& pet = m_pets.at(petPlace);
                // pets have equipment slots 1 to 6
                if (!pet || slot < 1 || slot > 6 || pet->worn[static_cast<std::size_t>(slot)].empty() ||
                    m_inventory.isStackable(pet->worn[static_cast<std::size_t>(slot)].itemId)) {
                    return;
                }
                const int worn = pet->worn[static_cast<std::size_t>(slot)].itemId;
                if (isPeriodItem(worn)) {
                    say(pet->name + "Equipped[" + nameOf(worn) + "] The usage period is over, the item has been destroyed");
                }
                m_pets.clearEquipment(petPlace, slot);
                if (newItemId != 0) {
                    m_pets.equip(petPlace, replacement(newItemId));
                }
                inventoryChanged();
                return;
            }
            case kWarehouse: { // 23/140/2 container 4: an item of the storeroom
                if (slot < 1 || slot > Inventory::kBagSlots || m_warehouse.item(slot).empty() || m_inventory.isStackable(m_warehouse.item(slot).itemId)) {
                    return;
                }
                const int stored = m_warehouse.item(slot).itemId;
                if (isPeriodItem(stored)) {
                    say("Storeroom's [" + nameOf(stored) + "] The usage period is over, the item has been destroyed");
                }
                m_warehouse.clear(slot);
                if (newItemId != 0) {
                    m_warehouse.replace(slot, replacement(newItemId));
                }
                return;
            }
            case kOthersWorn: // 23/140/2 container 5: an item another player wears (only told)
                say(owner + "Equipped[" + nameOf(itemId) + "] The usage period is over, the item has been destroyed");
                return;
            case kFurniture: // 23/140/2 container 6: an item placed in the furniture (only told)
                say("Placed in the furniture[" + nameOf(itemId) + "] The usage period is over, the item has been destroyed");
                return;
            case kBank: { // 23/140/2 container 7: an item kept at the Item Depositor
                if (slot < 1 || slot > Inventory::kBagSlots || m_bank.bagItem(slot).empty() || m_inventory.isStackable(m_bank.bagItem(slot).itemId)) {
                    return;
                }
                const protocol::ItemInstance item = m_bank.bagItem(slot);
                if (isPeriodItem(item.itemId)) {
                    say("Kept at the Item Depositor's [" + nameOf(item.itemId) + "] The usage period is over, the item has been destroyed");
                }
                m_bank.takeFromBag(slot, item.count);
                if (newItemId != 0) {
                    m_bank.addToBag(slot, replacement(newItemId));
                }
                inventoryChanged();
                return;
            }
            case kInnPet: { // 23/140/2 container 8: an item worn by a pet kept at the inn
                const int worn = m_petStorage.wornBy(petPlace, slot);
                if (worn == 0) {
                    return;
                }
                if (isPeriodItem(worn)) {
                    say(m_petStorage.nameOf(petPlace) + "Equipped[" + nameOf(worn) + "] The usage period is over, the item has been destroyed");
                }
                m_petStorage.replaceWorn(petPlace, slot, newItemId);
                return;
            }
            case kItemBagCell: { // 23/140/2 container 9: an item of the bonus space (item bag)
                if (slot < 1 || slot > 25 || // the item bag has 25 cells
                    m_itemBag.item(slot).empty() || m_inventory.isStackable(m_itemBag.item(slot).itemId)) {
                    return;
                }
                const int stored = m_itemBag.item(slot).itemId;
                if (isPeriodItem(stored)) {
                    say("Kept in the bonus space[" + nameOf(stored) + "] The usage period is over, the item has been destroyed");
                }
                m_itemBag.replace(slot, newItemId != 0 ? replacement(newItemId) : protocol::ItemInstance{});
                return;
            }
            default: // other containers are ignored
                return;
        }
    }

}
