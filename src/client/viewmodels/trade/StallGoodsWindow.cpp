#include "client/viewmodels/trade/StallGoodsWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/PriceColors.h"
#include "core/DateTime.h"
#include "gfx/TextStyle.h"
#include "protocol/StallRequests.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kMostRows = 10; // most goods rows a stall lists
        constexpr int kMostInCart = 50; // most pieces of one goods line in the cart
        constexpr std::int32_t kDearest = 9999999; // price at or above which only 1 piece can be bought
        constexpr int kTotalsRight = 195; // x of the totals' right edge in the window, px
        const core::Color kTextColor = core::Color::fromRgb565(2113); // RGB565 0x0841: near-black text

        /// @brief Loads StallGoodsWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "StallGoods" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("StallGoodsWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("StallGoods");
        }

        /// @brief Draws a number so that it ends at the same place, 8 pixels a digit.
        /// @details Original: DrawControl.
        /// @param canvas Target canvas.
        /// @param at Window origin on the canvas.
        /// @param y Vertical offset inside the window.
        /// @param text The number text.
        void drawRightAligned(gfx::Canvas& canvas, core::Point at, int y, const std::string& text) {
            // up to 10 digits, 8 px per digit
            const int x = (10 - static_cast<int>(text.size())) * 8 + kTotalsRight;
            // text box 200x20
            canvas.drawText({at.x + x, at.y + y}, text, {kTextColor, gfx::TextEffect::Plain, std::nullopt}, 200, 20);
        }

    }

    StallGoodsWindow::StallGoodsWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const PlayerStatus& status,
        PlayerMessages& messages,
        CountBox& countBox
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_status(status),
          m_messages(messages),
          m_countBox(countBox),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("StallGoodsList")),
          m_cart(screen.get<ui::IconGrid>("StallGoodsCart")),
          m_ownerSpot(screen.get<ui::Spot>("StallGoodsOwner")) {
        screen.get<ui::DrawingArea>("StallGoodsPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onStallGoodsPick", [this](const ui::EventArgs& args) { pick(args.index, args.ctrl); });
        screen.on("onStallGoodsTakeBack", [this](const ui::EventArgs& args) { takeBack(args.index); });
        screen.on("onStallGoodsBuy", [this](const ui::EventArgs&) { buy(); });
        screen.on("onStallGoodsClose", [this](const ui::EventArgs&) { close(); });
    }

    void StallGoodsWindow::show(const protocol::StallGoods& goods, const std::string& ownerName, const std::string& stallName) {
        m_goods = goods;
        if (m_goods.items.size() > static_cast<std::size_t>(kMostRows)) {
            m_goods.items.resize(kMostRows);
        }
        m_left.clear();
        for (const protocol::StallGoods::Item& entry : m_goods.items) {
            m_left.push_back(entry.item.count);
        }
        m_lines.fill({});
        m_ownerName = ownerName;
        m_stallName = stallName;
        const double now = core::nowAsDateTime();
        m_list.clear();
        for (const protocol::StallGoods::Item& entry : m_goods.items) {
            const auto record = m_services.items.find(entry.item.itemId);
            if (!record) {
                break;
            }
            using Align = ui::BlockList::Align;
            ui::BlockList::Row row;
            ui::BlockList::PictureBlock icon;
            icon.picture = itemIconCell(m_services, entry.item, false).picture;
            icon.area = {32, 56}; // icon block size in px
            icon.horizontal = Align::Centre;
            icon.vertical = Align::Centre;
            icon.count = entry.item.count;
            row.pictures.push_back(icon);
            const std::string price = std::to_string(entry.price);
            // row text blocks as {x,y} and {w,h}: name (35,0) 72x56, description (110,3) 179x53, price (292,0) 61x56
            row.texts.push_back({stallItemName(m_services.items, entry.item), {35, 0}, {72, 56}, Align::Centre, Align::Centre, kTextColor});
            row.texts.push_back({describeStallItem(m_services.items, entry.item, now), {110, 3}, {179, 53}, Align::Centre, Align::Centre, kTextColor});
            row.texts.push_back({price, {292, 0}, {61, 56}, Align::Centre, Align::Centre, priceColor(price.size())});
            m_list.addRow(std::move(row));
        }
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void StallGoodsWindow::refresh() {
        for (int row = 0; row < m_list.rowCount() && row < static_cast<int>(m_left.size()); ++row) {
            m_list.row(row).pictures.front().count = m_left[static_cast<std::size_t>(row)];
        }
        m_cart.clearCells();
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row >= 0) {
                protocol::ItemInstance item = m_goods.items[static_cast<std::size_t>(line.row)].item;
                item.count = line.count;
                m_cart.setCell(slot, itemIconCell(m_services, item, true));
            }
        }
    }

    void StallGoodsWindow::pick(int row, bool askCount) {
        if (row < 0 || row >= static_cast<int>(m_goods.items.size())) {
            return;
        }
        if (!askCount) {
            addToCart(row, 1);
            refresh();
            return;
        }
        m_countBox.ask(m_left[static_cast<std::size_t>(row)], [this, row](int count) {
            if (!isOpen() || count > kMostInCart) {
                return false;
            }
            addToCart(row, count);
            refresh();
            return true;
        });
    }

    bool StallGoodsWindow::addToCart(int row, int count) {
        const std::size_t index = static_cast<std::size_t>(row);
        if (m_left[index] < count || (m_goods.items[index].price >= kDearest && count > 1)) {
            return false;
        }
        int slot = 0;
        for (int at = 1; at <= kCartSlots && slot == 0; ++at) {
            if (m_lines[static_cast<std::size_t>(at)].row == row) {
                slot = at;
            }
        }
        for (int at = 1; at <= kCartSlots && slot == 0; ++at) {
            if (m_lines[static_cast<std::size_t>(at)].row < 0) {
                slot = at;
            }
        }
        if (slot == 0) {
            return false;
        }
        CartLine& line = m_lines[static_cast<std::size_t>(slot)];
        const int added = std::min(count, kMostInCart - line.count);
        line.row = row;
        line.count += added;
        m_left[index] -= added;
        return true;
    }

    void StallGoodsWindow::takeBack(int cell) {
        if (cell < 1 || cell > kCartSlots || m_lines[static_cast<std::size_t>(cell)].row < 0) {
            return;
        }
        CartLine& line = m_lines[static_cast<std::size_t>(cell)];
        m_left[static_cast<std::size_t>(line.row)] += line.count;
        line = {};
        refresh();
    }

    std::int32_t StallGoodsWindow::total() const {
        std::int32_t sum = 0;
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row >= 0 && line.count != 0) {
                sum += line.count * m_goods.items[static_cast<std::size_t>(line.row)].price;
            }
        }
        return sum;
    }

    void StallGoodsWindow::buy() {
        Inventory trial = m_inventory;
        std::vector<protocol::StallPurchase> purchase;
        std::int32_t sum = 0;
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row < 0) {
                continue;
            }
            const protocol::StallGoods::Item& entry = m_goods.items[static_cast<std::size_t>(line.row)];
            const int place = trial.placeFor(entry.item.itemId, line.count);
            if (place == 0) {
                return;
            }
            protocol::ItemInstance item = entry.item;
            item.count = line.count;
            trial.addToBag(place, item);
            sum += entry.price * line.count;
            purchase.push_back({entry.stallSlot, line.count, entry.item.itemId, entry.echoKey});
        }
        if (m_status.gold < sum) {
            m_messages.show("Not enough money", 1200); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::buyFromStallRequest(m_goods.ownerId, purchase));
        close();
    }

    void StallGoodsWindow::paint(gfx::Canvas& canvas, core::Point at) {
        // total row y 445, gold row y 467
        drawRightAligned(canvas, at, 445, std::to_string(total()));
        drawRightAligned(canvas, at, 467, std::to_string(m_status.gold));
        if (!m_ownerName.empty()) {
            // caption at (50,45), text box 200x20
            canvas
                .drawText({at.x + 50, at.y + 45}, "[" + m_ownerName + "] " + m_stallName, {m_ownerSpot.color(), gfx::TextEffect::Plain, std::nullopt}, 200, 20);
        }
    }

}
