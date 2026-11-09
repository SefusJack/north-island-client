#include "client/viewmodels/house/PledgeBoxWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "protocol/HouseRequests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game screen width, px (keeps the window on screen)
        constexpr int kScreenHeight = 600; // game screen height, px (keeps the window on screen)
        constexpr int kCellSide = 32; // size of one item cell, px
        constexpr int kFreeSearchItem = 10001; // item 10001: placeholder item ("question mark 2") used to look for a free bag slot
        constexpr int kPledgeType = 21; // item type 21: quest props (tooltip shows "Quest prop"; e.g. Christmas Bell, Pumpkin Seed)
        constexpr int kPutOption = 7; // 62/52 option 7: put an item into the Locker
        constexpr int kTakeOption = 8; // 62/52 option 8: take an item out of the Locker
        constexpr int kAllowedMark = 1; // bag use mark 1 (compound window mark) that lockNotice lets through

        /// @brief Loads PledgeBoxWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PledgeBoxWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PledgeBoxWindow");
        }

    }

    PledgeBoxWindow::PledgeBoxWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseBoxes& boxes,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_boxes(boxes),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_cells(screen.get<ui::IconGrid>("PledgeBoxCells")) {
        screen.on("onPledgeBoxClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onPledgeBoxTake", [this](const ui::EventArgs& args) { take(args.index); });
    }

    void PledgeBoxWindow::open(int piece) {
        m_piece = piece;
        core::Rect area = m_window.bounds();
        area.x = std::min(area.x, kScreenWidth - area.width);
        area.y = std::min(area.y, kScreenHeight - area.height);
        m_window.setBounds(area);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void PledgeBoxWindow::drop(int bagSlot, int place) {
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        // 50 = number of bag slots
        if (bagSlot <= 0 || bagSlot > 50 || item.empty()) {
            return;
        }
        if (m_marks.lockNotice(bagSlot, kAllowedMark)) {
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 1000); // notice duration in ms
            return;
        }
        const auto record = m_services.items.find(item.itemId);
        if (!record || record->type != kPledgeType) {
            m_messages.show("The item can't be stored", 1000); // notice duration in ms
            return;
        }
        if (place <= 0 || place > HouseBoxes::kLockerPlaces) {
            return;
        }
        int target = place;
        if (m_boxes.place(HouseBoxes::kLocker, place).itemId != 0) {
            target = m_boxes.firstFree(HouseBoxes::kLocker);
            if (target == 0) {
                return;
            }
        }
        m_hooks.send(protocol::houseBoxRequest(kPutOption, m_piece, target, bagSlot));
    }

    void PledgeBoxWindow::take(int place) {
        if (place <= 0 || place > HouseBoxes::kLockerPlaces) {
            return;
        }
        if (m_inventory.freeSlotFor(kFreeSearchItem) == 0) {
            m_messages.show("Not enough space", 1000); // notice duration in ms
            return;
        }
        m_hooks.send(protocol::houseBoxRequest(kTakeOption, m_piece, place));
    }

    void PledgeBoxWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        m_cells.clearCells();
        for (int place = 1; place <= HouseBoxes::kLockerPlaces; ++place) {
            const HouseBoxes::Place& held = m_boxes.place(HouseBoxes::kLocker, place);
            if (held.itemId != 0 && held.count != 0) {
                protocol::ItemInstance item;
                item.itemId = held.itemId;
                item.count = held.count;
                ui::IconCell cell = itemIconCell(m_services, item, true);
                cell.hint.clear();
                m_cells.setCell(place, cell);
            }
        }
        if (m_screen.hoveredId() != m_cells.id()) {
            return;
        }
        const core::Point origin = m_window.bounds().position() + m_cells.bounds().position();
        const int place = m_cells.cellAt(m_screen.mouse() - origin);
        const HouseBoxes::Place& held = place != 0 ? m_boxes.place(HouseBoxes::kLocker, place) : HouseBoxes::Place{};
        if (held.itemId != 0) {
            protocol::ItemInstance item;
            item.itemId = held.itemId;
            item.count = held.count;
            const core::Point at = origin + m_cells.cellPosition(place);
            m_info.show(item, {at.x, at.y, kCellSide, kCellSide});
        }
    }

}
