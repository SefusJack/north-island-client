#include "client/viewmodels/travel/VehicleRepairWindow.h"

#include "client/rendering/painters/VehicleBoxPainter.h"
#include "client/data/VehicleKinds.h"
#include "protocol/MallMessages.h"
#include "protocol/VehicleRequests.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, ...)
        const char* const kOnlyVehicles = "Only vehicles can be put in the garage";
        const char* const kLocked = "Unable to operate when the item is locked";

        /// @brief Loads VehicleRepairWindow.xml into the screen and returns its window.
        /// @param services Game services (layout files, load context).
        /// @param screen Screen to load into.
        /// @return The loaded repair window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("VehicleRepairWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("VehicleRepairWindow");
        }

    }

    VehicleRepairWindow::VehicleRepairWindow(
        GameServices& services,
        ui::Screen& screen,
        Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_costSpot(screen.get<ui::Spot>("VehicleRepairCost")) {
        screen.get<ui::DrawingArea>("VehicleRepairPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onVehicleRepairOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onVehicleRepairClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onVehicleRepairTake", [this](const ui::EventArgs&) { take(); });
        screen.on("onVehicleRepairDrop", [this](const ui::EventArgs& args) { put(args.index); });
    }

    void VehicleRepairWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.send(protocol::imPointsRequest(0));
    }

    void VehicleRepairWindow::close() {
        m_window.setVisible(false);
        if (!m_requestSent) {
            clear();
        }
    }

    void VehicleRepairWindow::clear() {
        if (m_slot != 0) {
            if (m_marks.useMark(m_slot) == BagMarks::kRepair) {
                m_marks.setUseMark(m_slot, 0);
            }
            if (m_marks.busyKind(m_slot) == 3) { // busy kind 3: repair in progress
                m_marks.setBusyKind(m_slot, 0);
            }
            m_hooks.marksChanged();
        }
        m_slot = 0;
        m_cost.clear();
    }

    void VehicleRepairWindow::take() {
        if (!m_requestSent) {
            m_services.playSound("sound/wav0154.wav");
            clear();
        }
    }

    void VehicleRepairWindow::put(int bagSlot) {
        if (m_requestSent || bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
        const int kind = VehicleKinds::kindOfItem(m_inventory.bagItem(bagSlot).itemId);
        if (!record || record->type != kVehicleType || kind == 0) {
            m_messages.show(kOnlyVehicles, 2000); // message display time in ms
        } else if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
        } else if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show(kLocked, 1000); // message display time in ms
        } else if (VehicleKinds::repairCost(kind) == 0) {
            m_messages.show("The vehicle can't be repaired", 1000); // message display time in ms
        } else {
            clear();
            m_services.playSound("sound/wav0154.wav");
            m_slot = bagSlot;
            m_cost = std::to_string(VehicleKinds::repairCost(kind));
            m_marks.setUseMark(bagSlot, BagMarks::kRepair);
            m_hooks.marksChanged();
        }
    }

    bool VehicleRepairWindow::checked() {
        const protocol::ItemInstance& stack = m_inventory.bagItem(m_slot);
        const auto record = m_services.items.find(stack.itemId);
        if (!record || record->type != kVehicleType || VehicleKinds::kindOfItem(stack.itemId) == 0) {
            m_messages.show(kOnlyVehicles, 2000); // message display time in ms
            return false;
        }
        if (const auto notice = m_marks.lockNotice(m_slot, BagMarks::kRepair)) {
            m_messages.show(notice->text, notice->ms);
            return false;
        }
        if (!m_marks.isAvailable(m_slot)) {
            m_messages.show(kLocked, 1000); // message display time in ms
            return false;
        }
        if (stack.usedAmount <= 0) {
            m_messages.show("The vehicle don't need to be reparied", 1000); // message display time in ms
            return false;
        }
        return true;
    }

    void VehicleRepairWindow::confirm() {
        if (m_slot < 1 || m_slot > Inventory::kBagSlots || !checked()) {
            return;
        }
        const auto record = m_services.items.find(m_inventory.bagItem(m_slot).itemId);
        m_hooks.confirm("[" + record->name + "] Repari required game points:" + m_cost + " Points", [this] { send(); });
    }

    void VehicleRepairWindow::send() {
        if (m_requestSent || m_slot < 1 || m_slot > Inventory::kBagSlots || m_inventory.bagItem(m_slot).empty() || !checked()) {
            return;
        }
        const int kind = VehicleKinds::kindOfItem(m_inventory.bagItem(m_slot).itemId);
        if (VehicleKinds::repairCost(kind) == 0) {
            m_messages.show("The vehicle can't be repaired", 1000); // message display time in ms
            return;
        }
        if (m_hooks.imPoints() - VehicleKinds::repairCost(kind) < 0) {
            m_messages.show("Not enough points", 1000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::repairVehicleRequest(m_slot));
        m_requestSent = true;
        m_window.setVisible(false);
    }

    void VehicleRepairWindow::result(int code, int value) {
        switch (code) {
            case 0: // 23/81 0: open the form and ask for the points
                open();
                break;
            case 1: // 23/81 1: the repair work starts (cell marked busy)
                if (m_slot >= 1 && m_slot <= Inventory::kBagSlots && m_marks.useMark(m_slot) == BagMarks::kRepair && m_marks.busyKind(m_slot) == 0) {
                    m_marks.setBusyKind(m_slot, 3); // busy kind 3: repair in progress
                    m_hooks.marksChanged();
                }
                break;
            case 2: // 23/81 2: repair done, value is the vehicle's new wear
                if (m_slot >= 1 && m_slot <= Inventory::kBagSlots) {
                    m_inventory.setUsedAmount(m_slot, value);
                }
                m_services.playSound("sound/wav0152.wav");
                m_requestSent = false;
                m_messages.show("Repair vehicleSuccessfully", 2000); // message display time in ms
                clear();
                close();
                break;
            case 3: // 23/81 3: repair failed
            case 4: // 23/81 4: not enough points
                m_requestSent = false;
                m_messages.show(code == 3 ? "Repair vehicleUnsuccessfully" : "Not enough points", 2000); // message display time in ms
                clear();
                close();
                break;
            default: // unknown code: ignored
                break;
        }
    }

    void VehicleRepairWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_slot != 0 && m_inventory.bagItem(m_slot).empty() && !m_requestSent) {
            clear();
        }
        if (!m_cost.empty()) {
            canvas.drawText(
                {at.x + m_costSpot.at().x, at.y + m_costSpot.at().y},
                "Repari required game points:" + m_cost,
                {core::Color::fromRgb565(31), gfx::TextEffect::Plain, std::nullopt},
                m_costSpot.rect().width,
                m_costSpot.rect().height
            ); // RGB565 31 = blue text
        }
        if (m_slot == 0) {
            return;
        }
        VehicleBoxPainter::paint(m_services, canvas, {at.x + 43, at.y + 68}, m_inventory.bagItem(m_slot)); // vehicle space offset (43, 68) in the form
    }

}
