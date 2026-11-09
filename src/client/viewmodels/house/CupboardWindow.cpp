#include "client/viewmodels/house/CupboardWindow.h"

#include "client/rules/FurnitureKinds.h"
#include "client/rendering/painters/ItemCells.h"
#include "protocol/FurnitureCupboardMessages.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kWardrobePlaces = 5; // places in a wardrobe (furniture kind 1)
        constexpr int kCabinetPlaces = 10; // places in a cabinet (furniture kind 2)
        constexpr int kNoStoreFlag = 32; // item flag bit 0x20: the item cannot be stored here
        constexpr int kCellSide = 32; // size of one item cell, px
        constexpr int kScreenWidth = 800; // game screen width, px (keeps the window on screen)
        constexpr int kScreenHeight = 600; // game screen height, px (keeps the window on screen)
        constexpr int kAllowedMark = 1; // bag use mark 1 (compound window mark) that lockNotice lets through

        /// @brief Loads CupboardWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded CupboardWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CupboardWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CupboardWindow");
        }

    }

    CupboardWindow::CupboardWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_fiveCells(screen.get<ui::IconGrid>("CupboardFiveCells")),
          m_tenCells(screen.get<ui::IconGrid>("CupboardTenCells")),
          m_close(screen.get<ui::ImageButton>("CupboardClose")),
          m_wardrobeCloseSpot(screen.get<ui::Spot>("CupboardWardrobeClose")),
          m_cabinetCloseSpot(screen.get<ui::Spot>("CupboardCabinetClose")) {
        screen.on("onCupboardClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onCupboardTake", [this](const ui::EventArgs& args) { take(args.index); });
    }

    ui::IconGrid& CupboardWindow::cells() const { return m_kind == 1 ? m_fiveCells : m_tenCells; }

    void CupboardWindow::open(int furnIndex) {
        const int slot = m_house.utensils().slotOfPiece(furnIndex);
        const int kind = FurnitureKinds::cupboardKind(m_services.items, m_house.utensils().utensil(slot).itemId);
        if (kind < 1 || kind > 2) { // cupboard kinds: 1 Wardrobe, 2 Cabinet
            return;
        }
        m_furnIndex = furnIndex;
        m_kind = kind;
        m_window.setPicture(kind == 1 ? "Form_Cupboard_1" : "Form_Cupboard_2", m_services.pictures);
        core::Rect area = m_window.bounds();
        area.x = std::min(area.x, kScreenWidth - area.width);
        area.y = std::min(area.y, kScreenHeight - area.height);
        m_window.setBounds(area);
        m_fiveCells.setVisible(kind == 1);
        m_tenCells.setVisible(kind == 2);
        m_close.setPosition(kind == 1 ? m_wardrobeCloseSpot.at() : m_cabinetCloseSpot.at());
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void CupboardWindow::take(int place) {
        const protocol::ItemInstance& item = m_house.cupboards().itemAt(m_furnIndex, place);
        if (item.empty()) {
            return;
        }
        if (m_inventory.freeSlotFor(item.itemId) == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::takeFromCupboardRequest(m_furnIndex, place));
    }

    void CupboardWindow::drop(int bagSlot, int place) {
        if (m_house.cupboards().emptyPlace(m_furnIndex) == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        if (record->cellWidth != 1 || record->cellHeight != 1) {
            m_messages.show("The item is too big to sell", 1000); // message display time in ms
            return;
        }
        if (const auto lock = m_marks.lockNotice(bagSlot, kAllowedMark)) {
            m_messages.show(lock->text, lock->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 1000); // message display time in ms
            return;
        }
        if ((record->flags & kNoStoreFlag) != 0) {
            m_messages.show("Cannot be stored", 2000); // message display time in ms
            return;
        }
        const int target = m_house.cupboards().itemAt(m_furnIndex, place).empty() ? place : m_house.cupboards().emptyPlace(m_furnIndex);
        m_hooks.send(protocol::putInCupboardRequest(m_furnIndex, bagSlot, target));
    }

    void CupboardWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        ui::IconGrid& grid = cells();
        grid.clearCells();
        const int places = m_kind == 1 ? kWardrobePlaces : kCabinetPlaces;
        for (int place = 1; place <= places; ++place) {
            const protocol::ItemInstance& item = m_house.cupboards().itemAt(m_furnIndex, place);
            if (!item.empty()) {
                ui::IconCell cell = itemIconCell(m_services, item, true);
                cell.hint.clear();
                grid.setCell(place, cell);
            }
        }
        if (m_screen.hoveredId() != grid.id()) {
            return;
        }
        const core::Point origin = m_window.bounds().position() + grid.bounds().position();
        const int place = grid.cellAt(m_screen.mouse() - origin);
        const protocol::ItemInstance& item = place != 0 ? m_house.cupboards().itemAt(m_furnIndex, place) : protocol::ItemInstance{};
        if (!item.empty()) {
            const core::Point at = origin + grid.cellPosition(place);
            m_info.show(item, {at.x, at.y, kCellSide, kCellSide});
        }
    }

}
