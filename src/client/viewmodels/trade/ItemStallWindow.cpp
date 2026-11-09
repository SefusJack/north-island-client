#include "client/viewmodels/trade/ItemStallWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/Stalling.h"
#include "client/rules/TextFilter.h"
#include "client/models/world/StallSign.h"
#include "gfx/TextStyle.h"
#include "protocol/StallRequests.h"
#include "ui/DrawingArea.h"

#include <cstdio>
#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr int kItemSale = 1; // sale kind 1: an item was sold (other kinds are not an item sale)
        constexpr int kSigns = 14; // number of stall sign looks
        constexpr int kTeamPkMap1 = 12064; // map 12064: 2F, the Capitol (team PK; stalls not allowed)
        constexpr int kTeamPkMap2 = 11073; // map 11073: 2F, Activity Centre (team PK; stalls not allowed)
        constexpr std::size_t kLongestName = 14; // longest shop name, characters
        const char* const kDefaultName = "Personal shop";
        const char* const kNoTransfer = "This item cannot be transferred";

        /// @brief The part of a stall sign's picture the form shows.
        /// @details Original: DrawControl.
        /// @param sign Stall sign number (1..14).
        /// @return Source rectangle (x, y, width, height) in the sign picture.
        core::Rect signSource(int sign) {
            // the rectangles are the shown crop (x, y, width, height) of each sign's picture
            switch (sign) {
                case 1: return {97, 96, 90, 90}; // sign 1
                case 2: return {124, 96, 90, 90}; // sign 2
                case 3: return {107, 98, 55, 60}; // sign 3
                case 4: return {105, 122, 55, 60}; // sign 4
                case 5: return {40, 60, 140, 140}; // sign 5
                case 6: return {127, 60, 140, 140}; // sign 6
                case 7: return {50, 68, 145, 145}; // sign 7
                case 8: return {114, 68, 145, 145}; // sign 8
                case 9: return {35, 61, 150, 150}; // sign 9
                case 10: return {126, 61, 150, 150}; // sign 10
                case 11: return {80, 61, 150, 150}; // sign 11
                case 12: return {65, 61, 150, 150}; // sign 12
                case 13: return {108, 33, 150, 180}; // sign 13
                default: return {42, 33, 150, 180}; // sign 14 (the last one)
            }
        }

        /// @brief Loads ItemStallWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded ItemStall window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ItemStallWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ItemStall");
        }

        /// @brief Reads the row number from a "StallPriceN" element name.
        /// @param name Element name.
        /// @return The row number, or 0 when the name does not match.
        int rowOf(const std::string& name) {
            const std::string prefix = "StallPrice";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

    }

    ItemStallWindow::ItemStallWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
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
          m_bag(screen.get<ui::IconGrid>("StallBag")),
          m_rows(screen.get<ui::IconGrid>("StallItems")),
          m_name(screen.get<ui::TextBox>("StallName")),
          m_incomeSpot(screen.get<ui::Spot>("StallIncome")),
          m_signSpot(screen.get<ui::Spot>("StallSignCentre")) {
        screen.get<ui::DrawingArea>("StallPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onStallBagClick", [this](const ui::EventArgs& args) { addFromBag(args.index); });
        screen.on("onStallBagRemove", [this](const ui::EventArgs& args) { removeByBagSlot(args.index); });
        screen.on("onStallItemRemove", [this](const ui::EventArgs& args) { removeRow(args.index); });
        screen.on("onStallPreviousLook", [this](const ui::EventArgs&) { m_sign = m_sign > 1 ? m_sign - 1 : kSigns; });
        screen.on("onStallNextLook", [this](const ui::EventArgs&) { m_sign = m_sign < kSigns ? m_sign + 1 : 1; });
        screen.on("onStallClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onStallKeep", [this](const ui::EventArgs&) { keep(); });
        screen.on("onStallRevoke", [this](const ui::EventArgs&) { revoke(); });
        screen.on("onStallPriceTab", [this](const ui::EventArgs& args) { focusNextRow(rowOf(args.source)); });
        reset();
    }

    ui::TextBox& ItemStallWindow::price(int row) { return m_screen.get<ui::TextBox>("StallPrice" + std::to_string(row)); }

    std::int32_t ItemStallWindow::priceOf(int row) {
        const std::string& text = price(row).text();
        return text.empty() ? 0 : static_cast<std::int32_t>(std::stol(text));
    }

    void ItemStallWindow::toggle() {
        if (isOpen()) {
            hide();
        } else {
            open();
        }
    }

    void ItemStallWindow::open() {
        if (!m_hooks.keepsItemStall()) {
            reset();
            if (!m_lastName.empty()) {
                m_name.setText(m_lastName);
            }
            if (m_lastSign != 0) {
                m_sign = m_lastSign;
            }
        }
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ItemStallWindow::reset() {
        m_name.setText(kDefaultName);
        for (int row = 1; row <= kRows; ++row) {
            price(row).setText("0");
            price(row).setReadOnly(true);
            m_items[static_cast<std::size_t>(row)] = {};
        }
        m_sign = 1;
        m_income = 0;
    }

    void ItemStallWindow::clearRow(int row) {
        m_items[static_cast<std::size_t>(row)] = {};
        price(row).setText("0");
    }

    void ItemStallWindow::refresh() {
        const bool kept = m_hooks.keepsItemStall();
        for (int row = 1; row <= kRows; ++row) {
            Row& item = m_items[static_cast<std::size_t>(row)];
            if (item.bagSlot == 0) {
                continue;
            }
            const protocol::ItemInstance& now = m_inventory.bagItem(item.bagSlot);
            if (now.empty()) {
                if (!kept) {
                    m_marks.setUseMark(item.bagSlot, 0);
                    price(row).setReadOnly(true);
                }
                clearRow(row);
                continue;
            }
            item.item = now;
        }
        m_bag.clearCells();
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_bag.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
        m_rows.clearCells();
        for (int row = 1; row <= kRows; ++row) {
            const Row& item = m_items[static_cast<std::size_t>(row)];
            if (item.bagSlot != 0) {
                m_rows.setCell(row, itemIconCell(m_services, item.item, true));
            }
        }
        layoutButtons();
    }

    void ItemStallWindow::layoutButtons() {
        const bool kept = m_hooks.keepsItemStall();
        m_screen.get<ui::Element>("StallKeep").setVisible(!kept);
        m_screen.get<ui::Element>("StallRevoke").setVisible(kept);
        m_screen.get<ui::Element>("StallRevokeIcon").setVisible(!kept);
        m_screen.get<ui::Element>("StallKeepIcon").setVisible(kept);
    }

    void ItemStallWindow::addFromBag(int bagSlot) {
        if (m_hooks.keepsItemStall()) {
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            if (!notice->text.empty()) {
                m_messages.show(notice->text, notice->ms);
            }
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        if (item.empty()) {
            return;
        }
        switch (itemStallRefusal(m_services.items, item.itemId)) {
            case 1: // item cannot be transferred
                m_messages.show(kNoTransfer, 2000); // message display time in ms
                return;
            case 255: // item cannot be sold in the item stall
                m_messages.show("This item cannot be sold in the item stall", 2000); // message display time in ms
                return;
            default: // item may be sold
                break;
        }
        if (item.lookItemId != 0 || isNonTransferable(item.insertItemId)) {
            m_messages.show(kNoTransfer, 2000); // message display time in ms
            return;
        }
        for (int row = 1; row <= kRows; ++row) {
            Row& free = m_items[static_cast<std::size_t>(row)];
            if (free.bagSlot == 0) {
                free = {bagSlot, item};
                price(row).setText("0");
                price(row).setReadOnly(false);
                m_marks.setUseMark(bagSlot, BagMarks::kStall);
                m_hooks.bagChanged();
                return;
            }
        }
    }

    void ItemStallWindow::removeByBagSlot(int bagSlot) {
        if (m_hooks.keepsItemStall() || m_inventory.bagItem(bagSlot).empty() || m_marks.useMark(bagSlot) != BagMarks::kStall) {
            return;
        }
        for (int row = 1; row <= kRows; ++row) {
            if (m_items[static_cast<std::size_t>(row)].bagSlot == bagSlot) {
                removeRow(row);
            }
        }
    }

    void ItemStallWindow::removeRow(int row) {
        if (m_hooks.keepsItemStall() || row < 1 || row > kRows) {
            return;
        }
        const int bagSlot = m_items[static_cast<std::size_t>(row)].bagSlot;
        if (bagSlot == 0) {
            return;
        }
        m_marks.setUseMark(bagSlot, 0);
        clearRow(row);
        price(row).setReadOnly(true);
        m_hooks.bagChanged();
    }

    void ItemStallWindow::close() {
        if (!m_hooks.keepsItemStall()) {
            m_marks.clearUseMarks(BagMarks::kStall);
            reset();
            m_hooks.bagChanged();
        }
        hide();
    }

    bool ItemStallWindow::validate() {
        bool valid = false;
        int count = 0;
        for (int row = 1; row <= kRows; ++row) {
            const Row& item = m_items[static_cast<std::size_t>(row)];
            if (item.bagSlot == 0) {
                continue;
            }
            if (m_inventory.bagItem(item.bagSlot).empty()) {
                valid = false;
                break;
            }
            if (priceOf(row) == 0) {
                m_messages.show("The item price cannot be 0", 1200); // message display time in ms
                valid = false;
                break;
            }
            ++count;
            valid = true;
        }
        if (!valid || count <= 0) {
            return false;
        }
        if (m_name.text().empty()) {
            m_messages.show("Please enter the name of the shop", 1200); // message display time in ms
            return false;
        }
        if (m_name.text().size() > kLongestName) {
            m_messages.show("Shop name is too long", 1200); // message display time in ms
            return false;
        }
        return true;
    }

    void ItemStallWindow::keep() {
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
        if (m_hooks.keepsAnyStall()) {
            m_messages.show("One kind of stall every time", 2000); // message display time in ms
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
        if (m_name.text().empty()) {
            m_messages.show("Please enter the name of the shop", 1200); // message display time in ms
            return;
        }
        if (!isNameClean(m_name.text(), false)) {
            m_messages.show(kIllegalWords, 2000); // message display time in ms
            return;
        }
        if (!validate()) {
            return;
        }
        std::vector<protocol::OwnStall::Price> prices;
        for (int row = 1; row <= kRows; ++row) {
            const Row& item = m_items[static_cast<std::size_t>(row)];
            if (item.bagSlot != 0) {
                prices.push_back({item.bagSlot, priceOf(row)});
                m_marks.setUseMark(item.bagSlot, 0);
                price(row).setText(std::to_string(priceOf(row)));
            }
        }
        m_services.connection.send(protocol::openItemStallRequest(m_name.text(), m_sign, prices));
        m_lastName = m_name.text();
        m_lastSign = m_sign;
        m_hooks.bagChanged();
    }

    void ItemStallWindow::revoke() {
        if (!m_hooks.keepsItemStall()) {
            return;
        }
        m_services.connection.send(protocol::closeItemStallRequest());
        hide();
        m_lastName = m_name.text();
        m_lastSign = m_sign;
    }

    void ItemStallWindow::stallOpened(const protocol::OwnStall& stall) {
        (void)stall;
        for (int row = 1; row <= kRows; ++row) {
            price(row).setReadOnly(true);
        }
        hide();
    }

    void ItemStallWindow::stallClosed() {
        for (int row = 1; row <= kRows; ++row) {
            price(row).setReadOnly(false);
        }
    }

    void ItemStallWindow::showSale(const protocol::StallSale& sale) {
        if (sale.kind == kItemSale) {
            const auto record = m_services.items.find(sale.itemId);
            m_hooks.chatLine("Sell " + (record ? record->name : std::string()) + " " + std::to_string(sale.count) + " Quantity");
            if (m_inventory.bagItem(sale.bagSlot).empty()) {
                m_marks.setPriced(sale.bagSlot, false);
                for (int row = 1; row <= kRows; ++row) {
                    if (m_items[static_cast<std::size_t>(row)].bagSlot == sale.bagSlot) {
                        clearRow(row);
                        m_marks.setUseMark(sale.bagSlot, 0);
                    }
                }
            }
        } else {
            m_hooks.chatLine("Sell Recharge Card of Wonderland Online");
        }
        m_income += sale.money;
        m_hooks.bagChanged();
    }

    void ItemStallWindow::focusNextRow(int row) {
        for (int tries = 0; tries < kRows; ++tries) {
            row = row % kRows + 1;
            if (m_items[static_cast<std::size_t>(row)].bagSlot != 0) {
                m_screen.focus("StallPrice" + std::to_string(row));
                return;
            }
        }
    }

    void ItemStallWindow::paint(gfx::Canvas& canvas, core::Point at) {
        char income[16]; // buffer for the gold text (ten digits wide)
        std::snprintf(income, sizeof income, "%10d", static_cast<int>(m_income)); // income right-aligned in 10 characters
        // 200 x 20: size of the gold text box
        canvas
            .drawText({at.x + m_incomeSpot.at().x, at.y + m_incomeSpot.at().y}, income, {m_incomeSpot.color(), gfx::TextEffect::Plain, std::nullopt}, 200, 20);
        if (const auto sign = m_services.pictures.find(StallSign{StallSign::Kind::Items, {}, m_sign}.pictureName())) {
            const core::Rect source = signSource(m_sign);
            canvas.drawPicture(*sign, {at.x + m_signSpot.at().x - source.width / 2, at.y + m_signSpot.at().y - source.height / 2}, source);
        }
    }

}
