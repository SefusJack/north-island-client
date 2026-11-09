#include "client/viewmodels/trade/UtensilTradeWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/PriceColors.h"
#include "protocol/UtensilTradeRequests.h"

#include <utility>
#include <vector>

namespace client {

    namespace {

        constexpr int kTradeMark = 2; // bag use-mark 2: the item is on the trade offer
        constexpr int kInstrumentType = 29; // item type 29: instruments / workshop machines (Auto Loom, Bench Saw, ...)
        constexpr int kFurnitureType = 30; // item type 30: furniture (ATM, Air Condition, Bathtub, ...)
        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, ...)
        constexpr int kCastleDefenseType = 53; // item type 53: castle defense items (Gold Eagle Totem, Torch, ...)
        constexpr int kNoTradeFlag = 2; // item flag bit 2: the item cannot be traded
        constexpr int kAgree = 1; // trade answer code 1: agree
        constexpr int kCancel = 2; // trade answer code 2: cancel

        /// @brief Loads UtensilTradeWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "UtensilTrade" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("UtensilTradeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("UtensilTrade");
        }

    }

    UtensilTradeWindow::UtensilTradeWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        const PlayerStatus& status,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_status(status),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_bag(screen.get<ui::IconGrid>("UtensilTradeBag")),
          m_partner(screen.get<ui::IconGrid>("UtensilTradePartner")),
          m_giveGold(screen.get<ui::TextBox>("UtensilTradeGiveGold")),
          m_partnerGold(screen.get<ui::TextBox>("UtensilTradePartnerGold")),
          m_okButton(screen.get<ui::Element>("UtensilTradeOk")),
          m_tradeButton(screen.get<ui::Element>("UtensilTradeTrade")),
          m_confirmIcon(screen.get<ui::Element>("UtensilTradeConfirmIcon")),
          m_tradeIcon(screen.get<ui::Element>("UtensilTradeTradeIcon")) {
        reset();
        screen.on("onUtensilTradeBagClick", [this](const ui::EventArgs& args) { toggle(args.index); });
        screen.on("onUtensilTradeOk", [this](const ui::EventArgs&) { confirmOffer(); });
        screen.on("onUtensilTradeTrade", [this](const ui::EventArgs&) { agree(); });
        screen.on("onUtensilTradeCancel", [this](const ui::EventArgs&) { cancel(); });
    }

    void UtensilTradeWindow::reset() {
        m_offered = 0;
        m_requestSent = false;
        m_partnerId = 0;
        m_partnerGrid.clear();
        m_partnerOffer.fill({});
        m_okButton.setVisible(true);
        m_tradeButton.setVisible(true);
        m_screen.get<ui::TextBox>("UtensilTradeMyName").setText("");
        m_giveGold.setText("0");
        m_screen.get<ui::TextBox>("UtensilTradePartnerName").setText("");
        m_partnerGold.setText("0");
    }

    void UtensilTradeWindow::open() {
        reset();
        m_screen.get<ui::TextBox>("UtensilTradeMyName").setText(m_hooks.ownName());
        m_tradeButton.setVisible(false);
        m_partnerGold.setFontColor(priceColor(0));
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    void UtensilTradeWindow::setPartner(std::int32_t partnerId, const std::string& name) {
        m_partnerId = partnerId;
        if (m_hooks.ownStall()) {
            m_messages.show("Operation canceled because you are keeping a stall", 2000); // message display time in ms
            return;
        }
        if (m_hooks.partnerStall(m_partnerId)) {
            m_messages.show("Operation canceled because the other party is keeping a stall", 2000); // message display time in ms
            return;
        }
        m_screen.get<ui::TextBox>("UtensilTradePartnerName").setText(name);
    }

    void UtensilTradeWindow::close() {
        m_marks.clearUseMarks(kTradeMark);
        m_window.setVisible(false);
        m_hooks.bagChanged();
    }

    void UtensilTradeWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        const int hovered = m_screen.hoveredId();
        const bool inBag = hovered == m_bag.id();
        if (!inBag && hovered != m_partner.id()) {
            return;
        }
        const ui::IconGrid& grid = inBag ? m_bag : m_partner;
        const core::Point origin = m_window.bounds().position() + grid.bounds().position();
        const int cell = grid.cellAt(m_screen.mouse() - origin);
        if (cell < 1 || cell > 50) { // 50 grid cells
            return;
        }
        const protocol::ItemInstance& item = inBag ? m_inventory.bagItem(cell) : m_partnerOffer[static_cast<std::size_t>(cell)];
        if (item.empty()) {
            return;
        }
        const core::Point at = origin + grid.cellPosition(cell);
        const core::Size cells = m_inventory.cellsOf(item.itemId);
        m_info.show(item, core::Rect{at.x, at.y, cells.width * 34 - 2, cells.height * 34 - 2}); // 34 px cell pitch, 2 px gap
    }

    void UtensilTradeWindow::refresh() {
        m_bag.clearCells();
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_bag.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
        m_partner.clearCells();
        for (int slot = 1; slot <= 50; ++slot) { // 50 partner grid cells
            const protocol::ItemInstance& item = m_partnerOffer[static_cast<std::size_t>(slot)];
            if (!item.empty()) {
                m_partner.setCell(slot, itemIconCell(m_services, item, true));
            }
        }
        layoutButtons();
    }

    void UtensilTradeWindow::layoutButtons() {
        m_confirmIcon.setVisible(!m_okButton.isVisible());
        m_tradeIcon.setVisible(!m_tradeButton.isVisible());
    }

    void UtensilTradeWindow::toggle(int bagSlot) {
        if (!m_okButton.isVisible() || bagSlot < 1 || bagSlot > Inventory::kBagSlots) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (item.empty() || !record) {
            return;
        }
        if (record->type != kInstrumentType && record->type != kFurnitureType && record->type != kVehicleType && record->type != kCastleDefenseType) {
            m_messages.show("This item cannot be traded", 2000); // message display time in ms
            return;
        }
        if (m_marks.vehicleState(bagSlot) != 0) {
            m_messages.show("Unable to perform this action while using a vehicle", 2000); // message display time in ms
            return;
        }
        if ((record->flags & kNoTradeFlag) != 0 || item.lookItemId != 0) {
            m_messages.show("Unable to trade the untranferred item", 2000); // message display time in ms
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, kTradeMark)) {
            m_messages.show(notice->text, notice->ms); // message display time in ms
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 1000); // message display time in ms
            return;
        }
        if (m_marks.useMark(bagSlot) == kTradeMark) {
            if (m_offered > 0) {
                --m_offered;
            }
            m_marks.setUseMark(bagSlot, 0);
        } else if (m_marks.useMark(bagSlot) == 0) {
            ++m_offered;
            m_marks.setUseMark(bagSlot, kTradeMark);
        }
        refresh();
        m_hooks.bagChanged();
    }

    bool UtensilTradeWindow::refused() {
        if (m_hooks.ownStall()) {
            m_messages.show("Operation canceled because you are keeping a stall", 2000); // message display time in ms
        } else if (m_hooks.partnerStall(m_partnerId)) {
            m_messages.show("Operation canceled because the other party is keeping a stall", 2000); // message display time in ms
        } else if (m_hooks.onVehicle()) {
            m_messages.show("Put away transportation vehicle before trading\xA1I", 1200); // message display time in ms
        } else {
            return false;
        }
        return true;
    }

    void UtensilTradeWindow::confirmOffer() {
        if (m_requestSent) {
            return;
        }
        if (m_giveGold.text().empty()) {
            m_giveGold.setText("0");
        }
        const long long gold = std::stoll(m_giveGold.text());
        if (gold > m_status.gold) {
            m_messages.show("Not enough money", 1200); // message display time in ms
            return;
        }
        if (refused()) {
            return;
        }
        std::vector<int> slots;
        for (int slot = 1; slot <= Inventory::kBagSlots && static_cast<int>(slots.size()) < m_offered; ++slot) {
            if (!m_inventory.bagItem(slot).empty() && m_marks.useMark(slot) == kTradeMark) {
                slots.push_back(slot);
            }
        }
        m_services.connection.send(protocol::utensilOfferRequest(static_cast<std::int32_t>(gold), slots));
        m_requestSent = true;
        m_okButton.setVisible(false);
        m_tradeButton.setVisible(true);
        layoutButtons();
    }

    void UtensilTradeWindow::agree() {
        if (m_hooks.sportActive()) {
            m_messages.show("This function is unavailable at present", 1200); // message display time in ms
            return;
        }
        if (refused()) {
            return;
        }
        m_services.connection.send(protocol::utensilAnswerRequest(kAgree));
        m_tradeButton.setVisible(false);
        layoutButtons();
    }

    void UtensilTradeWindow::cancel() { m_services.connection.send(protocol::utensilAnswerRequest(kCancel)); }

    void UtensilTradeWindow::showPartnerOffer(const protocol::TradeOffer& offer) {
        const std::string gold = std::to_string(offer.gold);
        m_partnerGold.setText(gold);
        m_partnerGold.setFontColor(priceColor(gold.size()));
        for (const protocol::ItemInstance& item : offer.items) {
            const core::Size cells = m_inventory.cellsOf(item.itemId);
            const auto slot = m_partnerGrid.place(cells.width, cells.height);
            if (!slot) {
                break;
            }
            m_partnerOffer[static_cast<std::size_t>(*slot)] = item;
        }
        refresh();
    }

    void UtensilTradeWindow::showResult(int code) {
        // result messages last 3000 ms (default 1000 ms)
        switch (code) {
            case 0: m_messages.show("The system is busy now. Please trade later.", 3000); break; // 25/42 code 0: system busy
            case 1: m_messages.show("Unable to trade, the target is unable to connect to the server.", 3000); break; // 25/42 code 1: target not connected
            case 2: m_messages.show("Unable to trade, the target is busy now.", 3000); break; // 25/42 code 2: target busy
            case 3: m_messages.show("The target has cancelled the trade.", 3000); break; // 25/42 code 3: target cancelled
            case 4: m_messages.show("Trade successful.", 3000); break; // 25/42 code 4: trade done
            case 5: m_messages.show("Unable to trade because you are in different scenes.", 3000); break; // 25/42 code 5: different scenes
            case 6: m_messages.show("Trade has been cancelled due to insufficient funds.", 3000); break; // 25/42 code 6: insufficient funds
            case 7: m_messages.show("Cancel the trade", 3000); break; // 25/42 code 7: trade cancelled
            case 8: m_messages.show("Unable to trade while in battle.", 3000); break; // 25/42 code 8: in battle
            case 9: m_messages.show("The target doesn't have enough space", 3000); break; // 25/42 code 9: target has no room
            case 10: m_messages.show("Not enough space", 3000); break; // 25/42 code 10: own bag has no room
            case 11: m_messages.show("Cancel the trade. Someone has the same pet.", 3000); break; // 25/42 code 11: same pet on the other side
            case 12: m_messages.show("0 amity with the pet, unable to trade", 3000); break; // 25/42 code 12: pet with 0 amity
            case 13:
                m_messages.show("Unable to trade cause the target closed the trade function", 3000); // message display time in ms
                break; // 25/42 code 13: target closed the trade function
            case 14: m_messages.show("Cannot trade when making an item", 3000); break; // 25/42 code 14: target is making an item
            default: m_messages.show("The system has interuppted the trade.", 1000); break; // 25/42 other code: system interrupted the trade
        }
        close();
    }

}
