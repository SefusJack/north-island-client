#include "client/viewmodels/trade/ItemTradeWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/PriceColors.h"
#include "core/Log.h"
#include "protocol/TradeRequests.h"

#include <vector>

namespace client {

    namespace {

        constexpr int kTradeMark = 2; // bag use-mark 2: the bag slot is offered in the trade
        constexpr int kMostWidth = 5; // widest item that can be traded, bag cells
        constexpr int kMostHeight = 4; // tallest item that can be traded, bag cells
        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, Hot-air Balloon, ...)
        constexpr int kInstrumentType = 29; // item type 29: instruments (Assembly Basin, Auto Loom, ...)
        constexpr int kFurnitureType = 30; // item type 30: furniture (ATM, Air Condition, ...)
        constexpr int kNoTradeFlag = 2; // item flag bit 0x02: cannot be transferred
        constexpr int kAgree = 1; // trade answer 1: agree
        constexpr int kCancel = 2; // trade answer 2: cancel

        /// @brief Loads ItemTradeWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded ItemTrade window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ItemTradeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ItemTrade");
        }

    }

    ItemTradeWindow::ItemTradeWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        const PlayerStatus& status,
        PlayerMessages& messages,
        std::function<void()> bagChanged
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_status(status),
          m_messages(messages),
          m_bagChanged(std::move(bagChanged)),
          m_window(loadWindow(services, screen)),
          m_bag(screen.get<ui::IconGrid>("TradeBag")),
          m_mine(screen.get<ui::IconGrid>("TradeMine")),
          m_partner(screen.get<ui::IconGrid>("TradePartner")),
          m_giveGold(screen.get<ui::TextBox>("TradeGiveGold")),
          m_partnerGold(screen.get<ui::TextBox>("TradePartnerGold")) {
        m_giveGold.setText("0");
        m_partnerGold.setText("0");
        screen.on("onTradeBagClick", [this](const ui::EventArgs& args) { offerFromBag(args.index); });
        screen.on("onTradeTakeBack", [this](const ui::EventArgs& args) { takeBack(args.index); });
        screen.on("onTradeConfirm", [this](const ui::EventArgs&) { confirmOffer(); });
        screen.on("onTradeTrade", [this](const ui::EventArgs&) { agree(); });
        screen.on("onTradeCancel", [this](const ui::EventArgs&) { cancel(); });
    }

    void ItemTradeWindow::open(const std::string& partnerName) {
        m_screen.get<ui::TextBox>("TradePartnerName").setText(partnerName);
        m_partnerGold.setFontColor(priceColor(0));
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    core::Size ItemTradeWindow::cellsOf(int itemId) const { return m_inventory.cellsOf(itemId); }

    void ItemTradeWindow::refresh() {
        m_bag.clearCells();
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_bag.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
        m_mine.clearCells();
        m_partner.clearCells();
        for (int slot = 1; slot <= OfferGrid::kSlots; ++slot) {
            const Offered& mine = m_myOffer[static_cast<std::size_t>(slot)];
            if (mine.bagSlot != 0) {
                m_mine.setCell(slot, itemIconCell(m_services, mine.item, true));
            }
            const protocol::ItemInstance& theirs = m_partnerOffer[static_cast<std::size_t>(slot)];
            if (!theirs.empty()) {
                m_partner.setCell(slot, itemIconCell(m_services, theirs, true));
            }
        }
        layoutButtons();
    }

    void ItemTradeWindow::layoutButtons() {
        ui::Element& confirm = m_screen.get<ui::Element>("TradeConfirm");
        confirm.setVisible(!m_offerConfirmed && !m_agreed);
        m_screen.get<ui::Element>("TradeTrade").setVisible(m_partnerOfferReceived && !confirm.isVisible() && !m_agreed);
    }

    void ItemTradeWindow::offerFromBag(int bagSlot) {
        if (m_offerConfirmed) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (item.empty() || !record) {
            return;
        }
        if (record->cellWidth > kMostWidth || record->cellHeight > kMostHeight) {
            m_messages.show("Direct trade unavailable because your item is oversized", 2000); // message display time in ms
            return;
        }
        if (record->type == kVehicleType || record->type == kInstrumentType || record->type == kFurnitureType) {
            m_messages.show("It is not a item", 2000); // message display time in ms
            return;
        }
        if ((record->flags & kNoTradeFlag) != 0 || item.lookItemId != 0 || record->unlisted != 0 || m_offered >= OfferGrid::kSlots ||
            m_marks.useMark(bagSlot) != 0) {
            return;
        }
        const auto slot = m_myGrid.place(record->cellWidth, record->cellHeight);
        if (!slot) {
            return;
        }
        m_myOffer[static_cast<std::size_t>(*slot)] = {bagSlot, item};
        m_marks.setUseMark(bagSlot, kTradeMark);
        ++m_offered;
        refresh();
        m_bagChanged();
    }

    void ItemTradeWindow::takeBack(int cell) {
        if (m_offerConfirmed || cell < 1 || cell > OfferGrid::kSlots) {
            return;
        }
        const int slot = m_myGrid.ownerOf(cell);
        if (slot == 0) {
            return;
        }
        Offered& offered = m_myOffer[static_cast<std::size_t>(slot)];
        if (offered.bagSlot == 0 || m_marks.useMark(offered.bagSlot) != kTradeMark) {
            return;
        }
        const core::Size cells = cellsOf(offered.item.itemId);
        m_marks.setUseMark(offered.bagSlot, 0);
        m_myGrid.free(slot, cells.width, cells.height);
        offered = {};
        --m_offered;
        refresh();
        m_bagChanged();
    }

    void ItemTradeWindow::confirmOffer() {
        if (m_giveGold.text().empty()) {
            m_giveGold.setText("0");
        }
        const long long gold = std::stoll(m_giveGold.text());
        if (gold > m_status.gold) {
            m_messages.show("Not enough money", 1200); // message display time in ms
            return;
        }
        std::vector<int> slots;
        for (int slot = 1; slot <= OfferGrid::kSlots; ++slot) {
            if (m_myOffer[static_cast<std::size_t>(slot)].bagSlot != 0) {
                slots.push_back(m_myOffer[static_cast<std::size_t>(slot)].bagSlot);
            }
        }
        m_services.connection.send(protocol::tradeOfferRequest(static_cast<std::int32_t>(gold), slots));
        m_offerConfirmed = true;
        layoutButtons();
    }

    void ItemTradeWindow::agree() {
        if (!bagCanTake()) {
            m_messages.show("Unable to trade because your item column is full.", 2000); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::tradeAnswerRequest(kAgree));
        m_agreed = true;
        m_partnerGrid.clear();
        m_myGrid.clear();
        layoutButtons();
    }

    bool ItemTradeWindow::bagCanTake() const {
        constexpr int kColumns = 5; // bag columns
        constexpr int kRows = 10; // bag rows (50 places)
        std::array<bool, kColumns * kRows + 1> taken{};
        const auto claim = [&](int slot, core::Size cells) {
            const int column = (slot - 1) % kColumns;
            const int row = (slot - 1) / kColumns;
            for (int x = column; x < column + cells.width && x < kColumns; ++x) {
                for (int y = row; y < row + cells.height && y < kRows; ++y) {
                    taken[static_cast<std::size_t>(y * kColumns + x + 1)] = true;
                }
            }
        };
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty()) {
                claim(slot, cellsOf(item.itemId));
            }
        }
        for (int slot = 1; slot <= OfferGrid::kSlots; ++slot) {
            const protocol::ItemInstance& item = m_partnerOffer[static_cast<std::size_t>(slot)];
            if (item.empty()) {
                continue;
            }
            const core::Size cells = cellsOf(item.itemId);
            bool placed = false;
            for (int at = 1; at <= kColumns * kRows && !placed; ++at) {
                const int column = (at - 1) % kColumns;
                const int row = (at - 1) / kColumns;
                bool fits = column + cells.width <= kColumns && row + cells.height <= kRows;
                for (int x = column; fits && x < column + cells.width; ++x) {
                    for (int y = row; fits && y < row + cells.height; ++y) {
                        fits = !taken[static_cast<std::size_t>(y * kColumns + x + 1)];
                    }
                }
                if (fits) {
                    claim(at, cells);
                    placed = true;
                }
            }
            if (!placed) {
                return false;
            }
        }
        return true;
    }

    void ItemTradeWindow::cancel() {
        m_services.connection.send(protocol::tradeAnswerRequest(kCancel));
        m_screen.get<ui::TextBox>("TradePartnerName").setText("");
        m_partnerGold.setText("0");
        m_giveGold.setText("0");
        m_offerConfirmed = false;
        m_partnerOfferReceived = false;
        m_agreed = false;
        m_marks.clearUseMarks(kTradeMark);
        m_myOffer.fill({});
        m_partnerOffer.fill({});
        m_myGrid.clear();
        m_partnerGrid.clear();
        m_offered = 0;
        m_window.setVisible(false);
        m_bagChanged();
    }

    void ItemTradeWindow::showResult(int result) {
        // 3000 and 1000: message display time in ms
        switch (result) {
            case 0: m_messages.show("The system is busy now. Please trade later.", 3000); break; // 25/2 result 0: system busy
            case 1: m_messages.show("Unable to trade, the target is unable to connect to the server.", 3000); break; // 25/2 result 1: target not connected
            case 2: m_messages.show("Unable to trade, the target is busy now.", 3000); break; // 25/2 result 2: target busy
            case 3:
                m_messages.show(isOpen() ? "The target has cancelled the trade." : "Cancel the trade", 3000);
                break; // 25/2 result 3: cancelled by the target
            case 4: m_messages.show("Trade successful.", 3000); break; // 25/2 result 4: trade done
            case 5: m_messages.show("Unable to trade because you are in different scenes.", 3000); break; // 25/2 result 5: different scenes
            case 6: m_messages.show("Trade has been cancelled due to insufficient funds.", 3000); break; // 25/2 result 6: insufficient funds
            case 7: m_messages.show("Trade is cancelled because too many items were placed.", 3000); break; // 25/2 result 7: too many items
            case 8: m_messages.show("Unable to trade while in battle.", 3000); break; // 25/2 result 8: in battle
            case 9: m_messages.show("Unable to trade cause the target closed the trade function", 3000); break; // 25/2 result 9: target closed trading
            case 10: m_messages.show("Trade unavailable in stall mode", 3000); break; // 25/2 result 10: stall mode
            case 11: m_messages.show("Unable to compelete two deals at the same time", 3000); break; // 25/2 result 11: two deals at once
            case 14: m_messages.show("Cannot trade when making an item", 3000); break; // 25/2 result 14: making an item
            default: m_messages.show("The system has interuppted the trade.", 1000); break; // any other 25/2 result: trade interrupted
        }
        cancel();
    }

    void ItemTradeWindow::showPartnerOffer(const protocol::TradeOffer& offer) {
        const std::string gold = std::to_string(offer.gold);
        m_partnerGold.setText(gold);
        m_partnerGold.setFontColor(priceColor(gold.size()));
        for (const protocol::ItemInstance& item : offer.items) {
            const core::Size cells = cellsOf(item.itemId);
            if (cells.width > kMostWidth || cells.height > kMostHeight) {
                m_messages.show("Direct trade unvailable because the traded item is oversized.", 2000); // message display time in ms
                cancel();
                return;
            }
            const auto slot = m_partnerGrid.place(cells.width, cells.height);
            m_partnerOffer[static_cast<std::size_t>(slot.value_or(0))] = item;
        }
        m_partnerOfferReceived = true;
        refresh();
    }

}
