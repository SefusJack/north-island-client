#include "client/viewmodels/bank/WarehouseWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "protocol/ItemRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace client {

    namespace {

        constexpr int kNoWarehouseFlag = 32; // item record flag bit value 32: the item cannot be stored in the warehouse

        /// @brief Loads WarehouseWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "Warehouse" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("WarehouseWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Warehouse");
        }

    }

    WarehouseWindow::WarehouseWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& bag,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_bag(bag),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_itemGrid(screen.get<ui::IconGrid>("WarehouseItems")),
          m_bagGrid(screen.get<ui::IconGrid>("WarehouseBag")),
          m_chosenSpot(screen.get<ui::Spot>("WarehouseChosenCell")) {
        screen.get<ui::DrawingArea>("WarehouseChosen").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintChosen(canvas, at); });
        screen.on("onWarehouseItemPress", [this](const ui::EventArgs& args) { pressItem(args.index); });
        screen.on("onWarehouseBagPress", [this](const ui::EventArgs& args) { pressBag(args.index); });
        screen.on("onWarehouseDeposit", [this](const ui::EventArgs&) { deposit(); });
        screen.on("onWarehouseWithdraw", [this](const ui::EventArgs&) { withdraw(); });
        screen.on("onWarehouseClose", [this](const ui::EventArgs&) { close(); });
    }

    void WarehouseWindow::show(std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        m_itemChosen.fill(false);
        m_bagChosen.fill(false);
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void WarehouseWindow::close() {
        m_window.setVisible(false);
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    void WarehouseWindow::add(const protocol::BagItemRecord& record) {
        protocol::ItemInstance rest = record.item;
        for (int tries = 0; rest.count > 0 && tries <= kSlots; ++tries) {
            int slot = record.slot;
            if (slot == 0) {
                const auto stack = std::find_if(m_items.begin() + 1, m_items.end(), [&rest](const protocol::ItemInstance& there) {
                    return there.itemId == rest.itemId && there.count < Inventory::kMaxStack;
                });
                const auto empty = std::find_if(m_items.begin() + 1, m_items.end(), [](const protocol::ItemInstance& there) { return there.empty(); });
                const auto found = m_bag.isStackable(rest.itemId) && stack != m_items.end() ? stack : empty;
                if (found == m_items.end()) {
                    break;
                }
                slot = static_cast<int>(found - m_items.begin());
            }
            if (slot < 1 || slot > kSlots) {
                break;
            }
            protocol::ItemInstance& there = m_items[static_cast<std::size_t>(slot)];
            int added = 0;
            if (there.empty()) {
                there = rest;
                there.count = std::min(rest.count, Inventory::kMaxStack);
                added = there.count;
            } else if (there.itemId == rest.itemId) {
                added = std::min(rest.count, Inventory::kMaxStack - there.count);
                there.count += added;
            }
            rest.count -= added;
            if (record.slot != 0 || added == 0) {
                break;
            }
        }
        refresh();
    }

    void WarehouseWindow::clear(int slot) {
        if (slot >= 1 && slot <= kSlots) {
            m_items[static_cast<std::size_t>(slot)] = {};
            refresh();
        }
    }

    void WarehouseWindow::replaceItemIds(int first, int last, int to) {
        for (auto& item : m_items) {
            if (item.itemId >= first && item.itemId <= last) {
                item.itemId = to;
            }
        }
    }

    void WarehouseWindow::replace(int slot, const protocol::ItemInstance& item) {
        if (slot >= 1 && slot <= kSlots) {
            m_items[static_cast<std::size_t>(slot)] = item;
            refresh();
        }
    }

    void WarehouseWindow::refresh() {
        m_itemGrid.clearCells();
        m_bagGrid.clearCells();
        for (int slot = 1; slot <= kSlots; ++slot) {
            const std::size_t at = static_cast<std::size_t>(slot);
            if (!m_items[at].empty()) {
                m_itemGrid.setCell(slot, itemNameCell(m_services, m_items[at]));
            } else {
                m_itemChosen[at] = false;
            }
            const protocol::ItemInstance& item = m_bag.bagItem(slot);
            if (!item.empty()) {
                m_bagGrid.setCell(slot, itemNameCell(m_services, item));
            } else {
                m_bagChosen[at] = false;
            }
        }
    }

    void WarehouseWindow::pressItem(int slot) {
        if (slot >= 1 && slot <= kSlots) {
            m_itemChosen[static_cast<std::size_t>(slot)] = !m_itemChosen[static_cast<std::size_t>(slot)];
        }
    }

    void WarehouseWindow::pressBag(int slot) {
        if (slot < 1 || slot > kSlots) {
            return;
        }
        const auto record = m_services.items.find(m_bag.bagItem(slot).itemId);
        if (record && (record->flags & kNoWarehouseFlag) != 0) {
            m_messages.show("Unable to lay the item in the warehouse of the city.", 1200); // message display time in ms
            return;
        }
        m_bagChosen[static_cast<std::size_t>(slot)] = !m_bagChosen[static_cast<std::size_t>(slot)];
    }

    void WarehouseWindow::deposit() {
        std::vector<int> slots;
        for (int slot = 1; slot <= kSlots; ++slot) {
            if (m_bagChosen[static_cast<std::size_t>(slot)] && !m_bag.bagItem(slot).empty()) {
                slots.push_back(slot);
            }
        }
        if (!slots.empty()) {
            m_services.connection.send(protocol::warehouseDepositRequest(slots));
        }
        m_itemChosen.fill(false);
        m_bagChosen.fill(false);
    }

    void WarehouseWindow::withdraw() {
        std::vector<int> slots;
        for (int slot = 1; slot <= kSlots; ++slot) {
            if (m_itemChosen[static_cast<std::size_t>(slot)] && !m_items[static_cast<std::size_t>(slot)].empty()) {
                slots.push_back(slot);
            }
        }
        if (!slots.empty()) {
            m_services.connection.send(protocol::warehouseWithdrawRequest(slots));
        }
        m_itemChosen.fill(false);
        m_bagChosen.fill(false);
    }

    void WarehouseWindow::paintChosen(gfx::Canvas& canvas, core::Point at) {
        for (int slot = 1; slot <= kSlots; ++slot) {
            const std::size_t index = static_cast<std::size_t>(slot);
            if (m_itemChosen[index]) {
                const core::Point cell = m_itemGrid.bounds().position() + m_itemGrid.cellPosition(slot);
                canvas.fillRectAlpha({at.x + cell.x, at.y + cell.y, m_chosenSpot.size().width, m_chosenSpot.size().height}, m_chosenSpot.color());
            }
            if (m_bagChosen[index]) {
                const core::Point cell = m_bagGrid.bounds().position() + m_bagGrid.cellPosition(slot);
                canvas.fillRectAlpha({at.x + cell.x, at.y + cell.y, m_chosenSpot.size().width, m_chosenSpot.size().height}, m_chosenSpot.color());
            }
        }
    }

}
