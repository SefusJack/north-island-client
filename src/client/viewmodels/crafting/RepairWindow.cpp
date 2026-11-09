#include "client/viewmodels/crafting/RepairWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "protocol/RepairRequests.h"

#include <algorithm>
#include <utility>

namespace client {

    namespace {

        constexpr int kIconSize = 32; // item icon size, px
        constexpr int kMaxItemType = 16; // item types 1..16 are wearable gear (weapons 1..11, then clothes, hairwear, cuffs, shoes, adornment)
        constexpr int kFirstEquipSlot = 1; // equipment slots 1..6 are the character's wearable slots
        constexpr int kLastEquipSlot = 6;
        constexpr int kRoleExclusiveGear = 149; // item sub type 149: role-exclusive gear, cannot be repaired
        // the "Broken ..." items, one per gear type, which cannot be repaired
        constexpr int kUnrepairable[] = {
            10062, // Broken Knife
            11098, // Broken Sword
            12069, // Broken Halberd
            13064, // Broken Bow
            14063, // Broken Wand
            15051, // Broken Claw
            17066, // Broken Ax
            18062, // Broken Gavel
            19055, // Broken Fan
            20039, // Broken Gun
            21120, // Broken Clothes
            22125, // Broken Hairwear
            23105, // Broken Cuffs
            24090, // Broken Shoes
            25044 // Broken Adornment
        };

        /// @brief Loads RepairWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("RepairWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("RepairWindow");
        }

        /// @brief Tells whether an item is in the list of gear that cannot be repaired.
        /// @param itemId Item id.
        /// @return True when it cannot be repaired.
        bool unrepairable(int itemId) { return std::find(std::begin(kUnrepairable), std::end(kUnrepairable), itemId) != std::end(kUnrepairable); }

    }

    RepairWindow::RepairWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_cell(screen.get<ui::IconGrid>("RepairCells")) {
        screen.on("onRepairOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onRepairClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onRepairTake", [this](const ui::EventArgs&) { removeItem(); });
    }

    void RepairWindow::removeItem() {
        m_itemId = 0;
        if (m_slot >= 1 && m_slot <= BagMarks::kSlots && m_marks.useMark(m_slot) == BagMarks::kItemRepair) {
            m_marks.setUseMark(m_slot, 0);
        }
        m_slot = 0;
        m_hooks.marksChanged();
    }

    void RepairWindow::close() {
        removeItem();
        if (m_toolSlot >= 1 && m_toolSlot <= BagMarks::kSlots && m_marks.useMark(m_toolSlot) == BagMarks::kItemRepair) {
            m_marks.setUseMark(m_toolSlot, 0);
        }
        m_toolSlot = 0;
        m_window.setVisible(false);
        m_hooks.marksChanged();
    }

    void RepairWindow::open(int toolSlot) {
        if (m_toolSlot >= 1 && m_toolSlot <= BagMarks::kSlots) {
            m_marks.setUseMark(m_toolSlot, 0);
        }
        m_toolSlot = toolSlot;
        m_marks.setUseMark(m_toolSlot, BagMarks::kItemRepair);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_slot = 0;
        m_itemId = 0;
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.marksChanged();
    }

    void RepairWindow::put(int bagSlot) {
        constexpr int kUnableMs = 1000; // "unable to lay the item" notice time, ms
        if (m_hooks.inFight()) {
            m_messages.show("Fighting, cannot repair the equipment", 1000); // message display time in ms
            return;
        }
        if (bagSlot < 1 || bagSlot > BagMarks::kSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        bool rejected = !record;
        if (m_marks.lockNotice(bagSlot, 0) || !m_marks.isAvailable(bagSlot)) {
            rejected = true;
        }
        if (record && (record->type < 1 || record->type > kMaxItemType || record->equipSlot < kFirstEquipSlot || record->equipSlot > kLastEquipSlot ||
                       record->maxUses != 0 || record->subType == kRoleExclusiveGear)) {
            rejected = true;
        }
        if (rejected || unrepairable(item.itemId)) {
            m_messages.show("Unable to lay the item", kUnableMs);
            return;
        }
        if (item.wear == 0) { // wear 0: no wear to repair
            m_messages.show("It's cannot be repaired", 2000); // message display time in ms
            return;
        }
        if (m_slot >= 1 && m_slot <= BagMarks::kSlots) {
            m_marks.setUseMark(m_slot, 0);
            m_itemId = 0;
        }
        m_slot = bagSlot;
        m_itemId = item.itemId;
        m_marks.setUseMark(bagSlot, BagMarks::kItemRepair);
        m_hooks.marksChanged();
    }

    void RepairWindow::confirm() {
        if (m_slot == 0) {
            return;
        }
        if (m_hooks.inFight()) {
            m_messages.show("Fighting, cannot repair the equipment", 1000); // message display time in ms
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(m_slot);
        if (item.wear == 0 || unrepairable(item.itemId)) {
            return;
        }
        m_hooks.send(protocol::repairItemRequest(m_itemId, m_slot, m_toolSlot));
        const auto record = m_services.items.find(item.itemId);
        m_messages.show((record ? record->name : std::string()) + " Repaired", 1000); // message display time in ms
        m_services.playSound("sound/wav0152.wav");
        removeItem();
    }

    void RepairWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        m_cell.clearCells();
        if (m_itemId == 0) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(m_slot);
        m_cell.setCell(1, itemIconCell(m_services, item, true));
        if (m_screen.hoveredId() != m_cell.id()) {
            return;
        }
        const core::Point at = m_window.bounds().position() + m_cell.bounds().position();
        m_info.show(item, {at.x, at.y, kIconSize, kIconSize});
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_info.id());
        }
    }

}
