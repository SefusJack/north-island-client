#include "client/viewmodels/trade/NpcStoreWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/StoreItemText.h"
#include "gfx/TextStyle.h"
#include "protocol/ShopRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kMostInPlace = 50; // most of one goods item in the cart (a stack limit)
        constexpr int kMostAsked = 255; // largest count accepted from the quantity box
        constexpr int kTotalsRight = 195; // x of the left edge of the total/gold number fields, px
        constexpr int kDiscountCard = 135; // item sub type 135: a card worn in the special slot that halves the price
        constexpr int kCardSlot = 6; // equip slot 6: the special slot (accessories, cards)
        const core::Color kTextColor = core::Color::fromRgb565(2113); // RGB565 0x0841: near black

        /// @brief Loads NpcStoreWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded NpcStore window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NpcStoreWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("NpcStore");
        }

        /// @brief Draws a number right-aligned in a ten digit field.
        /// @details Original: DrawControl, 8 pixels a digit.
        /// @param canvas Canvas to draw on.
        /// @param at Top-left of the drawing area.
        /// @param y Top of the field relative to the area.
        /// @param text The number text.
        void drawRightAligned(gfx::Canvas& canvas, core::Point at, int y, const std::string& text) {
            // 10: digits in the field, 8: pixels a digit
            const int x = (10 - static_cast<int>(text.size())) * 8 + kTotalsRight;
            // 200 x 20: size of the text box
            canvas.drawText({at.x + x, at.y + y}, text, {kTextColor, gfx::TextEffect::Plain, std::nullopt}, 200, 20);
        }

    }

    NpcStoreWindow::NpcStoreWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const PlayerStatus& status,
        PlayerMessages& messages,
        PlayerMessages& centreMessages,
        CountBox& countBox
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_status(status),
          m_messages(messages),
          m_centreMessages(centreMessages),
          m_countBox(countBox),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("NpcStoreList")),
          m_cart(screen.get<ui::IconGrid>("NpcStoreCart")) {
        screen.get<ui::DrawingArea>("NpcStorePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onNpcStorePick", [this](const ui::EventArgs& args) { pick(args.index, args.ctrl); });
        screen.on("onNpcStoreTakeBack", [this](const ui::EventArgs& args) { takeBack(args.index); });
        screen.on("onNpcStoreBuy", [this](const ui::EventArgs&) { buy(); });
        screen.on("onNpcStoreClose", [this](const ui::EventArgs&) { close(); });
    }

    void NpcStoreWindow::show(Kind kind, const std::vector<int>& itemIds, std::function<void()> onClosed) {
        m_kind = kind;
        m_onClosed = std::move(onClosed);
        m_window.setPicture(kind == Kind::Equipment ? "Form_EquipStore_1" : "Form_ThingStore_1", m_services.pictures);
        m_itemIds.clear();
        m_list.clear();
        for (const int itemId : itemIds) {
            const auto record = m_services.items.find(itemId);
            if (!record) {
                continue;
            }
            using Align = ui::BlockList::Align;
            ui::BlockList::Row row;
            ui::BlockList::PictureBlock icon;
            icon.picture = m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{});
            icon.area = {32, 56}; // icon block size (width, height)
            icon.horizontal = Align::Centre;
            icon.vertical = Align::Centre;
            row.pictures.push_back(icon);
            // text blocks (position, size): name at (35, 0) 72 x 56; price at (292, 0) 61 x 56; description at (113, 3) 179 x 17
            row.texts.push_back({record->name, {35, 0}, {72, 56}, Align::Centre, Align::Centre, kTextColor});
            row.texts.push_back({std::to_string(record->price), {292, 0}, {61, 56}, Align::Centre, Align::Centre, kTextColor});
            row.texts.push_back({storeItemText(*record, kind == Kind::Equipment), {113, 3}, {179, 17}, Align::Start, Align::Start, kTextColor});
            m_list.addRow(std::move(row));
            m_itemIds.push_back(itemId);
        }
        m_lines.fill({});
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void NpcStoreWindow::close() {
        m_window.setVisible(false);
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    void NpcStoreWindow::refresh() {
        m_cart.clearCells();
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row >= 0) {
                protocol::ItemInstance item;
                item.itemId = m_itemIds[static_cast<std::size_t>(line.row)];
                item.count = line.count;
                m_cart.setCell(slot, itemIconCell(m_services, item, true));
            }
        }
    }

    void NpcStoreWindow::pick(int row, bool askCount) {
        if (row < 0 || row >= static_cast<int>(m_itemIds.size())) {
            return;
        }
        if (m_kind == Kind::Goods && askCount) {
            if (m_countBox.isOpen()) {
                m_countBox.close();
            }
            m_countBox.ask([this, row](int count) {
                if (!isOpen() || count > kMostAsked) {
                    return false;
                }
                addToCart(row, count);
                refresh();
                return true;
            });
            return;
        }
        addToCart(row, 1);
        refresh();
    }

    void NpcStoreWindow::addToCart(int row, int count) {
        int slot = 0;
        if (m_kind == Kind::Goods) {
            if (count > kMostInPlace) {
                return;
            }
            for (int at = 1; at <= kCartSlots && slot == 0; ++at) {
                const CartLine& line = m_lines[static_cast<std::size_t>(at)];
                if (line.row >= 0 && m_itemIds[static_cast<std::size_t>(line.row)] == m_itemIds[static_cast<std::size_t>(row)] &&
                    line.count + count < kMostInPlace) {
                    slot = at;
                }
            }
        }
        for (int at = 1; at <= kCartSlots && slot == 0; ++at) {
            if (m_lines[static_cast<std::size_t>(at)].row < 0) {
                slot = at;
            }
        }
        if (slot == 0) {
            return;
        }
        CartLine& line = m_lines[static_cast<std::size_t>(slot)];
        if (line.count == 0) {
            line.row = row;
        }
        line.count += count;
    }

    void NpcStoreWindow::takeBack(int cell) {
        if (cell < 1 || cell > kCartSlots || m_lines[static_cast<std::size_t>(cell)].row < 0) {
            return;
        }
        m_lines[static_cast<std::size_t>(cell)] = {};
        refresh();
    }

    std::int32_t NpcStoreWindow::total() const {
        std::int32_t sum = 0;
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row >= 0) {
                const auto record = m_services.items.find(m_itemIds[static_cast<std::size_t>(line.row)]);
                sum += record ? record->price * line.count : 0;
            }
        }
        return sum;
    }

    void NpcStoreWindow::buy() {
        std::vector<std::pair<int, int>> lines;
        for (int slot = 1; slot <= kCartSlots; ++slot) {
            const CartLine& line = m_lines[static_cast<std::size_t>(slot)];
            if (line.row >= 0) {
                lines.emplace_back(line.row + 1, line.count);
            }
        }
        if (lines.empty()) {
            return;
        }
        const auto card = m_services.items.find(m_inventory.equipped(kCardSlot).itemId);
        const bool discount = card && card->subType == kDiscountCard;
        if (m_status.gold < (discount ? total() / 2 : total())) { // the discount card halves the price
            m_messages.show("Not enough cash at hand", 2000); // message display time in ms
            return;
        }
        if (discount) {
            m_centreMessages.show("Show" + card->name + " Purchasing", 2000); // message display time in ms
        }
        m_services.connection.send(protocol::npcStoreBuyRequest(lines));
        close();
    }

    void NpcStoreWindow::bought(int result) {
        static const char* const kResults[] = {
            nullptr,
            "Sorry, you don't have enough money.",
            "Sorry, your inventory is full.",
            "Sorry, you can't put all items you buy in your inventory.",
            "Sorry, trade has failed."
        };
        // 27/1 results 1..4 are failures; 2000 is the message display time in ms
        if (result >= 1 && result <= 4) {
            m_messages.show(kResults[result], 2000);
        }
    }

    void NpcStoreWindow::paint(gfx::Canvas& canvas, core::Point at) {
        drawRightAligned(canvas, at, 446, std::to_string(total())); // y of the total field
        drawRightAligned(canvas, at, 468, std::to_string(m_status.gold)); // y of the gold field
    }

}
