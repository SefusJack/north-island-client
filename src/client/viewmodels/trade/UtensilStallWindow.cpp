#include "client/viewmodels/trade/UtensilStallWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/TextFilter.h"
#include "gfx/TextStyle.h"
#include "protocol/StallRequests.h"
#include "protocol/UtensilStallRequests.h"
#include "protocol/VehicleRequests.h"
#include "ui/DrawingArea.h"
#include "ui/TextWrap.h"

#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr int kPitch = 182; // horizontal distance between neighbouring stall places, px
        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, ...)
        constexpr int kNoTradeFlag = 2; // item flag bit 2: the item cannot be traded
        constexpr int kTeamPkMap1 = 12064; // map 12064: 2F, the Capitol (team PK, no stalls)
        constexpr int kTeamPkMap2 = 11073; // map 11073: 2F, Activity Centre (team PK, no stalls)
        constexpr int kMemoChars = 17; // characters per memo line when wrapping
        const char* const kDefaultName = "Personal shop";
        const char* const kWhileKept = "Stall keeping, operation unvailable";

        /// @brief Loads UtensilStallWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "UtensilStall" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("UtensilStallWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("UtensilStall");
        }

        /// @brief Extracts the stall place number from an element name such as "UtensilStallSlot2".
        /// @param name Element name.
        /// @return The place number, or 0 when the name has another prefix.
        int slotOf(const std::string& name) {
            const std::string prefix = "UtensilStallSlot";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

        /// @brief Gives the middle point of a stall place on the form.
        /// @param slot Stall place (1-based).
        /// @return Centre of the place in window coordinates.
        core::Point slotCentre(int slot) { return {(slot - 1) * kPitch + 306, 160}; } // first place centre (306,160), next ones kPitch apart

    }

    UtensilStallWindow::UtensilStallWindow(
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
          m_shopName(screen.get<ui::TextBox>("UtensilStallShopName")),
          m_selectGrid(screen.get<ui::IconGrid>("UtensilStallSelect")),
          m_memoSpot(screen.get<ui::Spot>("UtensilStallMemo")),
          m_selectedSpot(screen.get<ui::Spot>("UtensilStallSelected")),
          m_selectedMemoSpot(screen.get<ui::Spot>("UtensilStallSelectedMemo")) {
        screen.get<ui::DrawingArea>("UtensilStallPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onUtensilStallNext", [this](const ui::EventArgs&) { selectNext(); });
        screen.on("onUtensilStallPrevious", [this](const ui::EventArgs&) { selectPrevious(); });
        screen.on("onUtensilStallAdd", [this](const ui::EventArgs&) { add(); });
        screen.on("onUtensilStallRemove", [this](const ui::EventArgs&) { removeMarked(); });
        screen.on("onUtensilStallClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onUtensilStallEscape", [this](const ui::EventArgs&) { hide(); });
        screen.on("onUtensilStallKeep", [this](const ui::EventArgs&) { keep(); });
        screen.on("onUtensilStallRevoke", [this](const ui::EventArgs&) { revoke(); });
        screen.on("onUtensilStallSlotClick", [this](const ui::EventArgs& args) { toggleSlot(slotOf(args.source)); });
        m_shopName.setText(kDefaultName);
        for (int slot = 1; slot <= kSlots; ++slot) {
            price(slot).setText("0");
        }
    }

    ui::TextBox& UtensilStallWindow::price(int slot) { return m_screen.get<ui::TextBox>("UtensilStallPrice" + std::to_string(slot)); }

    std::int32_t UtensilStallWindow::priceOf(int slot) {
        const std::string& text = price(slot).text();
        return text.empty() ? 0 : static_cast<std::int32_t>(std::stol(text));
    }

    bool UtensilStallWindow::stallable(int bagSlot) const {
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (item.empty() || !record) {
            return false;
        }
        if (record->cellWidth <= 1 && record->cellHeight <= 1 && record->type != kVehicleType) {
            return false;
        }
        return (record->flags & kNoTradeFlag) == 0 && !m_marks.lockNotice(bagSlot, 0) && m_marks.isAvailable(bagSlot);
    }

    int UtensilStallWindow::findNext(int from) const {
        for (int slot = from + 1; slot <= Inventory::kBagSlots; ++slot) {
            if (stallable(slot)) {
                return slot;
            }
        }
        return from;
    }

    int UtensilStallWindow::findPrevious(int from) const {
        for (int slot = (from == 0 ? Inventory::kBagSlots + 1 : from) - 1; slot >= 1; --slot) {
            if (stallable(slot)) {
                return slot;
            }
        }
        return from;
    }

    void UtensilStallWindow::place(ui::IconGrid& grid, const ui::IconCell& cell, core::Point centre) {
        const int width = cell.source.width;
        const int height = cell.source.height;
        grid.setBounds({centre.x - width / 2, centre.y - height / 2, width, height});
        grid.clearCells();
        grid.setCell(1, cell);
    }

    void UtensilStallWindow::showSelected() {
        m_selectGrid.clearCells();
        if (m_selBagSlot > 0 && m_selBagSlot < Inventory::kBagSlots) {
            m_selected = m_inventory.bagItem(m_selBagSlot);
            ui::IconCell cell = itemIconCell(m_services, m_selected, true);
            cell.frame = {};
            place(m_selectGrid, cell, m_selectedSpot.at());
        } else {
            m_selected = {};
        }
    }

    void UtensilStallWindow::showSlot(int slot) {
        ui::IconGrid& grid = m_screen.get<ui::IconGrid>("UtensilStallSlot" + std::to_string(slot));
        grid.clearCells();
        const Slot& entry = m_slots[static_cast<std::size_t>(slot)];
        if (entry.bagSlot != 0) {
            ui::IconCell cell = itemIconCell(m_services, entry.item, true, m_marked[static_cast<std::size_t>(slot)]);
            if (!m_marked[static_cast<std::size_t>(slot)]) {
                cell.frame = {};
            }
            place(grid, cell, slotCentre(slot));
        }
    }

    void UtensilStallWindow::open() {
        m_selBagSlot = 0;
        showSelected();
        for (int slot = 1; slot <= kSlots; ++slot) {
            showSlot(slot);
        }
        m_shopName.setText(m_lastName.empty() ? kDefaultName : m_lastName);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void UtensilStallWindow::openFromMenu() {
        open();
        if (!m_hooks.keepsUtensilStall()) {
            m_selBagSlot = findNext(0);
        } else {
            open();
            return;
        }
        if (m_selBagSlot == 0) {
            m_messages.show("No utensils for keeping stall ", 2000); // message display time in ms
            hide();
            return;
        }
        showSelected();
    }

    void UtensilStallWindow::selectNext() {
        if (m_open) {
            m_messages.show(kWhileKept, 2000); // message display time in ms
            return;
        }
        m_selBagSlot = findNext(m_selBagSlot);
        showSelected();
    }

    void UtensilStallWindow::selectPrevious() {
        if (m_open) {
            m_messages.show(kWhileKept, 2000); // message display time in ms
            return;
        }
        m_selBagSlot = findPrevious(m_selBagSlot);
        showSelected();
    }

    void UtensilStallWindow::add() {
        if (m_selected.empty()) {
            return;
        }
        if (m_open) {
            m_messages.show(kWhileKept, 2000); // message display time in ms
            return;
        }
        if (m_count >= kSlots) {
            m_messages.show("No stall space available for so many items", 2000); // message display time in ms
            return;
        }
        if (m_marks.useMark(m_selBagSlot) == BagMarks::kStall) {
            return;
        }
        if (const auto notice = m_marks.lockNotice(m_selBagSlot, 0)) {
            if (!notice->text.empty()) {
                m_messages.show(notice->text, notice->ms);
            }
            return;
        }
        if (!m_marks.isAvailable(m_selBagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 2000); // message display time in ms
            return;
        }
        int slot = 0;
        for (int at = 1; at <= kSlots && slot == 0; ++at) {
            if (m_slots[static_cast<std::size_t>(at)].bagSlot == 0) {
                slot = at;
            }
        }
        if (slot == 0) {
            return;
        }
        ++m_count;
        m_slots[static_cast<std::size_t>(slot)] = {m_selBagSlot, m_inventory.bagItem(m_selBagSlot)};
        price(slot).setText("0");
        m_marks.setUseMark(m_selBagSlot, BagMarks::kStall);
        showSlot(slot);
        const int added = m_selBagSlot;
        m_selBagSlot = findNext(added);
        if (m_selBagSlot == added) {
            m_selBagSlot = findPrevious(added);
        }
        if (m_selBagSlot == added) {
            m_selBagSlot = 0;
        }
        showSelected();
        m_hooks.bagChanged();
    }

    void UtensilStallWindow::toggleSlot(int slot) {
        if (slot < 1 || slot > kSlots || m_slots[static_cast<std::size_t>(slot)].bagSlot == 0) {
            return;
        }
        if (m_open) {
            m_messages.show(kWhileKept, 2000); // message display time in ms
            return;
        }
        bool& marked = m_marked[static_cast<std::size_t>(slot)];
        marked = !marked;
        m_selSlot = marked ? slot : 0;
        showSlot(slot);
    }

    void UtensilStallWindow::removeMarked() {
        if (m_open) {
            m_messages.show(kWhileKept, 2000); // message display time in ms
            return;
        }
        if (m_selSlot == 0) {
            return;
        }
        Slot& entry = m_slots[static_cast<std::size_t>(m_selSlot)];
        m_marks.setUseMark(entry.bagSlot, 0);
        m_marked[static_cast<std::size_t>(m_selSlot)] = false;
        --m_count;
        m_selBagSlot = entry.bagSlot;
        entry = {};
        showSelected();
        showSlot(m_selSlot);
        m_selSlot = 0;
        m_hooks.bagChanged();
    }

    void UtensilStallWindow::reset() {
        m_selBagSlot = 0;
        m_count = 0;
        m_selSlot = 0;
        showSelected();
        for (int slot = 1; slot <= kSlots; ++slot) {
            Slot& entry = m_slots[static_cast<std::size_t>(slot)];
            m_marked[static_cast<std::size_t>(slot)] = false;
            if (entry.bagSlot != 0 && m_marks.useMark(entry.bagSlot) == BagMarks::kStall) {
                m_marks.setUseMark(entry.bagSlot, 0);
            }
            entry = {};
            showSlot(slot);
            price(slot).setText("0");
            price(slot).setReadOnly(false);
        }
        m_open = false;
        m_hooks.bagChanged();
    }

    void UtensilStallWindow::close() {
        if (!m_open) {
            reset();
        }
        hide();
    }

    void UtensilStallWindow::keep() {
        if (m_hooks.noStallScene()) {
            m_messages.show("Unable to keep a stall in this scenario", 2000); // message display time in ms
            return;
        }
        const int map = m_hooks.mapId();
        if (map == kTeamPkMap1 || map == kTeamPkMap2) {
            m_messages.show("Team PK, cannot set stall", 2000); // message display time in ms
            return;
        }
        if (m_hooks.remoteOn()) {
            m_messages.show("Operation unavailable while auto remote controller still in use", 2000); // message display time in ms
            return;
        }
        if (m_hooks.fishing()) {
            m_messages.show("Unable to set a stall while fishing", 2000); // message display time in ms
            return;
        }
        if (m_hooks.inTeam()) {
            m_messages.show("Unable to keep stall while in group", 2000); // message display time in ms
            return;
        }
        if (m_hooks.riding()) {
            m_messages.show("Riding, the action has been cancelled", 2000); // message display time in ms
            return;
        }
        if (m_hooks.usesVehicle()) {
            m_messages.show("Put away transport vehicle before opening stall\xA1I", 2000); // message display time in ms
            return;
        }
        if (m_hooks.keepsAnyStall()) {
            m_messages.show("One kind of stall every time", 2000); // message display time in ms
            return;
        }
        if (m_shopName.text().empty()) {
            m_messages.show("Please enter the name of the shop", 1200); // message display time in ms
            return;
        }
        if (!isNameClean(m_shopName.text(), false)) {
            m_messages.show(kIllegalWords, 2000); // message display time in ms
            return;
        }
        for (int slot = 1; slot <= kSlots; ++slot) {
            if (m_slots[static_cast<std::size_t>(slot)].bagSlot != 0) {
                if (priceOf(slot) == 0) {
                    m_messages.show("The amount should not be zero", 2000); // message display time in ms
                    return;
                }
            } else if (priceOf(slot) != 0) {
                price(slot).setText("0");
                return;
            }
        }
        if (m_count <= 0) {
            m_messages.show("Sell at least 1 item", 2000); // message display time in ms
            return;
        }
        std::vector<protocol::OwnStall::Price> prices;
        for (int slot = 1; slot <= kSlots; ++slot) {
            const Slot& entry = m_slots[static_cast<std::size_t>(slot)];
            if (entry.bagSlot == 0) {
                continue;
            }
            if (const auto record = m_services.items.find(entry.item.itemId);
                record && record->type == kVehicleType && m_marks.vehicleState(entry.bagSlot) == 1) { // vehicle state 1: put down on the map
                m_services.connection.send(protocol::useVehicleRequest(entry.bagSlot, entry.item.itemId));
            }
            prices.push_back({entry.bagSlot, priceOf(slot)});
        }
        m_services.connection.send(protocol::openUtensilStallRequest(m_shopName.text(), prices));
        m_open = true;
        hide();
        m_lastName = m_shopName.text();
    }

    void UtensilStallWindow::revoke() {
        if (!m_hooks.keepsUtensilStall()) {
            return;
        }
        m_services.connection.send(protocol::closeUtensilStallRequest());
        hide();
        reset();
        m_lastName = m_shopName.text();
    }

    void UtensilStallWindow::stallOpened() {
        for (int slot = 1; slot <= kSlots; ++slot) {
            price(slot).setReadOnly(true);
        }
        hide();
    }

    void UtensilStallWindow::showSale(const protocol::StallSale& sale) {
        if (sale.kind == 1) { // sale kind 1: an item sold
            const auto record = m_services.items.find(sale.itemId);
            m_hooks.chatLine("Sell " + (record ? record->name : std::string()) + " " + std::to_string(sale.count) + " Quantity");
        } else if (sale.kind == 2) { // sale kind 2: a recharge card sold
            m_hooks.chatLine("Sell Recharge Card of Wonderland Online");
        }
        for (int slot = 1; slot <= kSlots; ++slot) {
            Slot& entry = m_slots[static_cast<std::size_t>(slot)];
            if (entry.item.itemId == sale.itemId && entry.bagSlot == sale.bagSlot) {
                m_marked[static_cast<std::size_t>(slot)] = false;
                m_marks.setUseMark(entry.bagSlot, 0);
                if (m_inventory.bagItem(entry.bagSlot).empty()) {
                    m_marks.setPriced(entry.bagSlot, false);
                }
                entry = {};
                showSlot(slot);
                price(slot).setText("0");
            }
        }
        m_hooks.bagChanged();
    }

    void UtensilStallWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        const int hovered = m_screen.hoveredId();
        const auto describe = [&](ui::IconGrid& grid, const protocol::ItemInstance& item) {
            if (hovered != grid.id() || item.empty()) {
                return;
            }
            const core::Point at = m_window.bounds().position() + grid.bounds().position();
            m_info.show(item, core::Rect{at.x, at.y, grid.bounds().width, grid.bounds().height});
        };
        describe(m_selectGrid, m_selected);
        for (int slot = 1; slot <= kSlots; ++slot) {
            describe(m_screen.get<ui::IconGrid>("UtensilStallSlot" + std::to_string(slot)), m_slots[static_cast<std::size_t>(slot)].item);
        }
    }

    void UtensilStallWindow::paint(gfx::Canvas& canvas, core::Point at) {
        m_screen.get<ui::Element>("UtensilStallKeep").setVisible(!m_open);
        m_screen.get<ui::Element>("UtensilStallRevoke").setVisible(m_open);
        const gfx::TextStyle style{m_memoSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto memo = [&](core::Point where, const protocol::ItemInstance& item, bool leadingSpace) {
            const auto record = m_services.items.find(item.itemId);
            if (item.empty() || !record) {
                return;
            }
            canvas.drawText(
                where,
                ui::wrapTextLines(record->name + hintSuffix(*record, item) + (leadingSpace ? " " : "") + record->description, kMemoChars),
                style,
                m_memoSpot.size().width,
                m_memoSpot.size().height
            );
        };
        memo(at + m_selectedMemoSpot.at(), m_selected, true);
        for (int slot = 1; slot <= kSlots; ++slot) {
            // memo of the first place at (240,256), next ones kPitch apart
            memo({at.x + 240 + (slot - 1) * kPitch, at.y + 256}, m_slots[static_cast<std::size_t>(slot)].item, false);
        }
    }

}
