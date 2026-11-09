#include "client/viewmodels/events/PointStoreWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/StoreItemText.h"
#include "core/ByteReader.h"
#include "gfx/TextStyle.h"
#include "protocol/PointStoreRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kMostInPlace = 50; // most items of one stackable kind in a single cart slot
        constexpr int kMostAsked = 255; // most items one count-box entry may ask for (the server count is a u8)
        constexpr int kMessageMs = 1500; // notice display time, ms

        /// @brief Loads PointStoreWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "PointStore" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PointStoreWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PointStore");
        }

    }

    PointStoreWindow::PointStoreWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        PlayerMessages& messages,
        CountBox& countBox,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_messages(messages),
          m_countBox(countBox),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_buyButton(screen.get<ui::ImageButton>("PointStoreBuy")),
          m_list(screen.get<ui::BlockList>("PointStoreList")),
          m_cart(screen.get<ui::IconGrid>("PointStoreCart")),
          m_totalSpot(screen.get<ui::Spot>("PointStoreTotal")),
          m_pointsSpot(screen.get<ui::Spot>("PointStorePoints")),
          m_textColorSpot(screen.get<ui::Spot>("PointStoreTextColor")) {
        screen.get<ui::DrawingArea>("PointStorePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onPointStorePick", [this](const ui::EventArgs& args) { pick(args.index, args.ctrl); });
        screen.on("onPointStoreTakeBack", [this](const ui::EventArgs& args) { takeBack(args.index); });
        screen.on("onPointStoreBuy", [this](const ui::EventArgs&) { buy(); });
        screen.on("onPointStoreClose", [this](const ui::EventArgs&) { close(); });
    }

    void PointStoreWindow::show() {
        if (m_window.isVisible()) {
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        clearCart();
        m_rows.clear();
        m_list.clear();
        m_points = 0;
        m_hooks.askPoints();
    }

    void PointStoreWindow::close() {
        if (m_countBox.isOpen()) {
            m_countBox.close();
        }
        m_window.setVisible(false);
        m_hooks.closed();
    }

    void PointStoreWindow::clearCart() {
        m_lines.fill({});
        m_cartCount = 0;
        m_total = 0;
        m_pickedRow = 0;
        refreshCart();
    }

    void PointStoreWindow::refreshCart() {
        m_cart.clearCells();
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row != 0) {
                protocol::ItemInstance item;
                item.itemId = line.itemId;
                item.count = line.count;
                m_cart.setCell(slot, itemIconCell(m_services, item, true));
            }
        }
    }

    void PointStoreWindow::pointsArrived(std::int32_t points) {
        if (m_window.isVisible() && m_points == 0) {
            m_points = points;
        }
    }

    void PointStoreWindow::addRow(int itemId, std::int32_t price, int count) {
        const auto record = m_services.items.find(itemId);
        if (!record) {
            return;
        }
        using Align = ui::BlockList::Align;
        ui::BlockList::Row row;
        ui::BlockList::PictureBlock icon;
        icon.picture = m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{});
        // icon block 32 by 56 px; the row is 56 px high
        icon.area = {32, 56};
        icon.horizontal = Align::Centre;
        icon.vertical = Align::Centre;
        icon.count = count;
        row.pictures.push_back(icon);
        // text blocks as {x, y}, {width, height}: name {35,0} 72x56; price {292,0} 61x56; effect {113,3} 179x17
        row.texts.push_back({record->name, {35, 0}, {72, 56}, Align::Centre, Align::Centre, m_textColorSpot.color()});
        row.texts.push_back({std::to_string(price), {292, 0}, {61, 56}, Align::Centre, Align::Centre, m_textColorSpot.color()});
        row.texts.push_back({storeItemText(*record, true), {113, 3}, {179, 17}, Align::Start, Align::Start, m_textColorSpot.color()});
        m_list.addRow(std::move(row));
        m_rows.push_back({itemId, price, count});
    }

    void PointStoreWindow::receiveRows(const core::Bytes& payload) {
        show();
        core::ByteReader reader(payload, 2); // skip action byte and sub id
        const int count = reader.u8();
        for (int entry = 0; entry < count && !reader.failed(); ++entry) {
            const int itemId = reader.u16();
            const std::int32_t price = reader.i32();
            const int number = reader.u8();
            if (reader.failed()) {
                break;
            }
            addRow(itemId, price, number);
        }
    }

    void PointStoreWindow::receive(const core::Bytes& payload) {
        if (payload.size() < 2) { // action byte plus sub id
            return;
        }
        core::ByteReader reader(payload, 2); // skip action byte and sub id
        switch (payload[1]) { // sub id of server action 80
            case 1: receiveRows(payload); break; // 80/1: the store's rows (u8 count, then u16 item, i32 price, u8 number each)
            case 2: // 80/2: not enough points to trade
                m_messages.show("Unable to trade without enough points", kMessageMs);
                break;
            case 3: { // 80/3: one line bought (u16 item, i32 points spent, u8 count)
                const int itemId = reader.u16();
                const std::int32_t spent = reader.i32();
                const int count = reader.u8();
                if (reader.failed()) {
                    break;
                }
                const auto record = m_services.items.find(itemId);
                m_services.playSound("sound/wav0152.wav");
                // chat channel 0: the player's own
                m_hooks.chat(
                    0,
                    "Buy " + (record ? record->name : std::string()) + (count > 0 ? " " + std::to_string(count) + ", " : std::string()) +
                        " Points deducted successfully:" + std::to_string(spent)
                );
                m_points -= spent;
                m_hooks.chat(10, "Remaining points:" + std::to_string(m_points)); // chat channel 10: the system's
                break;
            }
            case 4: { // 80/4: buy result of a line (u16 item, i32 points), kept for 80/5
                reader.u16();
                m_boughtPoints = reader.i32();
                break;
            }
            case 5: // 80/5: points deducted for the kept buy result
                m_services.playSound("sound/wav0152.wav");
                m_hooks.chat(0, "Deduct points successfully:" + std::to_string(m_boughtPoints)); // chat channel 0: the player's own
                break;
            case 6: { // 80/6: points deducted (i32 points)
                const std::int32_t points = reader.i32();
                m_hooks.chat(0, "Deduct points successfully:" + std::to_string(reader.failed() ? 0 : points)); // chat channel 0: the player's own
                break;
            }
            case 7: // 80/7: not enough points
                m_messages.show("Not enough points", kMessageMs);
                break;
            case 8: // 80/8: the server is busy
                m_messages.show("The server is busy", kMessageMs);
                break;
            default: break; // other sub ids are ignored
        }
    }

    void PointStoreWindow::pick(int row, bool askCount) {
        if (row < 0 || row >= static_cast<int>(m_rows.size())) {
            return;
        }
        const Row& picked = m_rows[static_cast<std::size_t>(row)];
        const bool single = picked.count > 1;
        m_pickedRow = row + 1;
        if (m_countBox.isOpen()) {
            m_countBox.close();
        }
        if (m_inventory.isStackable(picked.itemId) && !single && askCount) {
            const int rowNumber = row + 1;
            m_countBox.ask([this, rowNumber, picked](int count) {
                if (!isOpen() || count > kMostAsked) {
                    return false;
                }
                m_pickedRow = rowNumber;
                addToCart(rowNumber, picked.itemId, count, picked.price, false);
                refreshCart();
                return true;
            });
            return;
        }
        addToCart(m_pickedRow, picked.itemId, picked.count, picked.price, single);
        refreshCart();
    }

    int PointStoreWindow::freeSlot() const {
        if (m_cartCount >= kCartSlots) {
            return 0;
        }
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            if (m_lines[static_cast<std::size_t>(slot)].row == 0) {
                return slot;
            }
        }
        return 0;
    }

    int PointStoreWindow::slotFor(int itemId, int count) const {
        if (!m_inventory.isStackable(itemId) || count > kMostInPlace) {
            return 0;
        }
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.itemId != 0 && line.itemId == itemId && line.count + count <= kMostInPlace) {
                return slot;
            }
        }
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            if (m_lines[static_cast<std::size_t>(slot)].row == 0) {
                return slot;
            }
        }
        return 0;
    }

    void PointStoreWindow::addToCart(
        int rowNumber,
        int itemId,
        int count,
        std::int32_t unitPrice,
        bool single
    ) {
        int slot = 0;
        if (single) {
            slot = freeSlot();
        } else if (m_inventory.isStackable(itemId)) {
            slot = slotFor(itemId, count);
        } else {
            slot = freeSlot();
        }
        if (slot == 0 || !m_services.items.find(itemId)) {
            return;
        }
        CartLine& line = m_lines[static_cast<std::size_t>(slot)];
        if (line.count == 0) {
            ++m_cartCount;
            line.row = rowNumber;
            line.itemId = itemId;
            line.unitPrice = unitPrice;
            line.single = single;
        }
        line.count += count;
        m_total += single ? unitPrice : count * unitPrice;
    }

    void PointStoreWindow::takeBack(int slot) {
        if (slot < 1 || slot > kCartSlots || m_cartCount <= 0 || m_lines[static_cast<std::size_t>(slot)].row == 0) {
            return;
        }
        const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
        m_total -= line.single ? line.unitPrice : line.unitPrice * line.count;
        --m_cartCount;
        m_lines[static_cast<std::size_t>(slot)] = {};
        refreshCart();
    }

    void PointStoreWindow::buy() {
        if (m_total > m_points) {
            m_messages.show("Unable to trade without enough points", kMessageMs);
            return;
        }
        if (m_cartCount <= 0) {
            return;
        }
        // \r in the text is the line break of the question box
        m_hooks.ask("Will consume points:" + std::to_string(m_total) + " points\rAre you sure you want to buy it?? ", [this] { send(); });
    }

    void PointStoreWindow::send() {
        if (m_total > m_points) {
            m_messages.show("Unable to trade without enough points", kMessageMs);
            return;
        }
        if (m_cartCount <= 0) {
            return;
        }
        std::vector<std::pair<int, int>> lines;
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row != 0) {
                lines.emplace_back(line.row, line.count);
            }
        }
        m_services.connection.send(protocol::pointStoreBuyRequest(lines));
        clearCart();
    }

    void PointStoreWindow::paint(gfx::Canvas& canvas, core::Point at) {
        m_buyButton.setBlocked(m_hooks.questionOpen());
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        // text boxes 200 wide and 20 high
        canvas.drawText({at.x + m_totalSpot.at().x, at.y + m_totalSpot.at().y}, std::to_string(m_total), style, 200, 20);
        canvas.drawText({at.x + m_pointsSpot.at().x, at.y + m_pointsSpot.at().y}, std::to_string(m_points), style, 200, 20);
    }

}
