#include "client/viewmodels/trade/SaleMenuWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "gfx/TextStyle.h"
#include "protocol/ItemRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>
#include <vector>

namespace client {

    namespace {

        constexpr int kRequiredLevel = 20; // player level needed to open a stall
        constexpr int kNoSaleFlags = 35; // item flag mask 35 (bits 1, 2 and 32): the item cannot be put on sale
        constexpr int kNoSaleType = 53; // item type 53: castle defense items (Gold Eagle Totem, Torch, ...), cannot be sold
        constexpr std::int32_t kMostCount = 9999999; // largest item count per sale
        constexpr std::int32_t kMostGold = 2000000000; // largest price, gold

        /// @brief Loads SaleMenuWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "SaleMenu" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SaleMenuWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("SaleMenu");
        }

    }

    SaleMenuWindow::SaleMenuWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& bag,
        const ItemStore& store,
        const PlayerStatus& status,
        const std::int32_t& saleGold,
        std::uint8_t& requestPending,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_bag(bag),
          m_store(store),
          m_status(status),
          m_saleGold(saleGold),
          m_requestPending(requestPending),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_storeGrid(screen.get<ui::IconGrid>("SaleMenuStore")),
          m_bagGrid(screen.get<ui::IconGrid>("SaleMenuBag")),
          m_amount(screen.get<ui::TextBox>("SaleMenuAmount")),
          m_count(screen.get<ui::TextBox>("SaleMenuCount")),
          m_textSpot(screen.get<ui::Spot>("SaleMenuText")),
          m_chosenSpot(screen.get<ui::Spot>("SaleMenuChosenCell")) {
        screen.get<ui::DrawingArea>("SaleMenuPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintMoney(canvas, at); });
        screen.get<ui::DrawingArea>("SaleMenuChosen").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintChosen(canvas, at); });
        screen.on("onSaleMenuBagPress", [this](const ui::EventArgs& args) { pressBag(args.index); });
        screen.on("onSaleMenuStorePress", [this](const ui::EventArgs& args) { pressStore(args.index); });
        screen.on("onSaleMenuPut", [this](const ui::EventArgs&) { put(); });
        screen.on("onSaleMenuTakeBack", [this](const ui::EventArgs&) { takeBack(); });
        screen.on("onSaleMenuManage", [this](const ui::EventArgs&) { manage(); });
        screen.on("onSaleMenuSend", [this](const ui::EventArgs&) { send(); });
        screen.on("onSaleMenuLink", [](const ui::EventArgs&) {});
        screen.on("onSaleMenuDeposit", [this](const ui::EventArgs&) { deposit(); });
        screen.on("onSaleMenuWithdraw", [this](const ui::EventArgs&) { withdraw(); });
        screen.on("onSaleMenuAmountUp", [this](const ui::EventArgs&) { changeAmount(m_amount, 1, kMostGold); });
        screen.on("onSaleMenuAmountDown", [this](const ui::EventArgs&) { changeAmount(m_amount, -1, kMostGold); });
        screen.on("onSaleMenuCountUp", [this](const ui::EventArgs&) { changeAmount(m_count, 1, kMostCount); });
        screen.on("onSaleMenuCountDown", [this](const ui::EventArgs&) { changeAmount(m_count, -1, kMostCount); });
        screen.on("onSaleMenuClose", [this](const ui::EventArgs&) { close(); });
        m_amount.setText("0");
        m_count.setText("0");
    }

    void SaleMenuWindow::show(std::function<void()> onClosed) {
        m_onClosed = std::move(onClosed);
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void SaleMenuWindow::close() {
        m_window.setVisible(false);
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    void SaleMenuWindow::refresh() {
        m_bagGrid.clearCells();
        m_storeGrid.clearCells();
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_bag.bagItem(slot);
            if (!item.empty()) {
                m_bagGrid.setCell(slot, itemNameCell(m_services, item));
            } else {
                m_bagChosen[static_cast<std::size_t>(slot)] = false;
            }
        }
        for (int slot = 1; slot <= kStoreSlots; ++slot) {
            const protocol::ItemInstance& item = m_store.item(slot);
            if (!item.empty()) {
                m_storeGrid.setCell(slot, itemNameCell(m_services, item));
            } else {
                m_storeChosen[static_cast<std::size_t>(slot)] = false;
            }
        }
    }

    void SaleMenuWindow::pressBag(int slot) {
        if (slot < 1 || slot > kBagSlots || m_bag.bagItem(slot).empty()) {
            return;
        }
        const auto record = m_services.items.find(m_bag.bagItem(slot).itemId);
        if (record && ((record->flags & kNoSaleFlags) != 0 || record->type == kNoSaleType)) {
            m_messages.show("Unable to sell the item", 2000); // message display time in ms
            return;
        }
        m_bagChosen[static_cast<std::size_t>(slot)] = !m_bagChosen[static_cast<std::size_t>(slot)];
    }

    void SaleMenuWindow::pressStore(int slot) {
        if (slot >= 1 && slot <= kStoreSlots && !m_store.item(slot).empty()) {
            m_storeChosen[static_cast<std::size_t>(slot)] = !m_storeChosen[static_cast<std::size_t>(slot)];
        }
    }

    bool SaleMenuWindow::levelAllowed() {
        if (m_status.level < kRequiredLevel) {
            m_messages.show("Unable to use the function unless you've reached level 20.", 2000); // message display time in ms
            return false;
        }
        return m_requestPending == 0;
    }

    void SaleMenuWindow::put() {
        if (!levelAllowed()) {
            return;
        }
        std::vector<int> slots;
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            if (m_bagChosen[static_cast<std::size_t>(slot)]) {
                slots.push_back(slot);
            }
        }
        if (!slots.empty()) {
            m_services.connection.send(protocol::saleStorePutRequest(slots));
        }
    }

    void SaleMenuWindow::takeBack() {
        if (!levelAllowed()) {
            return;
        }
        std::vector<int> slots;
        for (int slot = 1; slot <= kStoreSlots; ++slot) {
            if (m_storeChosen[static_cast<std::size_t>(slot)]) {
                slots.push_back(slot);
            }
        }
        if (!slots.empty()) {
            m_services.connection.send(protocol::saleStoreTakeBackRequest(slots));
        }
    }

    void SaleMenuWindow::manage() {
        if (!levelAllowed()) {
            return;
        }
        m_requestPending = 2; // pending marker: manage page requested
        m_services.connection.send(protocol::saleManageRequest());
    }

    void SaleMenuWindow::send() {
        if (!levelAllowed()) {
            return;
        }
        m_requestPending = 1; // pending marker: sale list requested
        m_services.connection.send(protocol::saleSendRequest());
    }

    std::int32_t SaleMenuWindow::amountOf(const ui::TextBox& box) {
        const std::string& text = box.text();
        // at most 9 digits fit an int32
        if (text.empty() || text.size() > 9 || text.find_first_not_of("0123456789") != std::string::npos) {
            return 0;
        }
        return std::stoi(text);
    }

    void SaleMenuWindow::deposit() {
        if (!levelAllowed()) {
            return;
        }
        const std::int32_t gold = amountOf(m_count);
        if (gold <= 0) {
            m_count.setText("0");
            return;
        }
        m_services.connection.send(protocol::saleMoneyDepositRequest(gold));
        m_count.setText("0");
    }

    void SaleMenuWindow::withdraw() {
        if (!levelAllowed()) {
            return;
        }
        const std::int32_t gold = amountOf(m_amount);
        if (gold <= 0) {
            m_amount.setText("0");
            return;
        }
        m_services.connection.send(protocol::saleMoneyWithdrawRequest(gold));
        m_amount.setText("0");
    }

    void SaleMenuWindow::changeAmount(ui::TextBox& box, int step, std::int32_t most) {
        const std::int32_t value = amountOf(box);
        if (value + step >= 0 && value + step <= most) {
            box.setText(std::to_string(value + step));
        }
    }

    void SaleMenuWindow::paintMoney(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const std::string carried = std::to_string(m_status.gold);
        // right edge x 382 (carried) / 166 (account), y 90, 8 px per digit, text box 100x16
        canvas.drawText({at.x + 382 - static_cast<int>(carried.size()) * 8, at.y + 90}, carried, style, 100, 16);
        const std::string account = std::to_string(m_saleGold);
        canvas.drawText({at.x + 166 - static_cast<int>(account.size()) * 8, at.y + 90}, account, style, 100, 16);
    }

    void SaleMenuWindow::paintChosen(gfx::Canvas& canvas, core::Point at) {
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            if (m_bagChosen[static_cast<std::size_t>(slot)]) {
                const core::Point cell = m_bagGrid.bounds().position() + m_bagGrid.cellPosition(slot);
                canvas.fillRectAlpha({at.x + cell.x, at.y + cell.y, m_chosenSpot.size().width, m_chosenSpot.size().height}, m_chosenSpot.color());
            }
        }
        for (int slot = 1; slot <= kStoreSlots; ++slot) {
            if (m_storeChosen[static_cast<std::size_t>(slot)]) {
                const core::Point cell = m_storeGrid.bounds().position() + m_storeGrid.cellPosition(slot);
                canvas.fillRectAlpha({at.x + cell.x, at.y + cell.y, m_chosenSpot.size().width, m_chosenSpot.size().height}, m_chosenSpot.color());
            }
        }
    }

}
