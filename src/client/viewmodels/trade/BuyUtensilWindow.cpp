#include "client/viewmodels/trade/BuyUtensilWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/PriceColors.h"
#include "gfx/TextStyle.h"
#include "protocol/UtensilStallRequests.h"
#include "ui/DrawingArea.h"
#include "ui/TextWrap.h"

#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr int kPitch = 204; // horizontal distance between the piece memos of consecutive slots, px
        constexpr int kMemoChars = 17; // characters per line of an item memo before wrapping

        /// @brief Loads BuyUtensilWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded BuyUtensil window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BuyUtensilWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BuyUtensil");
        }

        /// @brief Reads the slot number from a "BuyUtensilSlotN" element name.
        /// @param name Element name.
        /// @return The slot number, or 0 when the name does not match.
        int slotOf(const std::string& name) {
            const std::string prefix = "BuyUtensilSlot";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

        /// @brief Centre of a piece's icon in the window.
        /// @param slot Piece slot (1..3).
        /// @return The centre point.
        core::Point slotCentre(int slot) { return {(slot - 1) * kPitch + 120, 162}; } // first piece's icon centre (x, y) in the window

    }

    BuyUtensilWindow::BuyUtensilWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const PlayerStatus& status,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        std::function<bool()> sportActive
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_status(status),
          m_messages(messages),
          m_info(info),
          m_sportActive(std::move(sportActive)),
          m_window(loadWindow(services, screen)),
          m_memoSpot(screen.get<ui::Spot>("BuyUtensilMemo")) {
        screen.get<ui::DrawingArea>("BuyUtensilPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            const gfx::TextStyle style{m_memoSpot.color(), gfx::TextEffect::Plain, std::nullopt};
            for (int slot = 1; slot <= static_cast<int>(m_goods.pieces.size()); ++slot) {
                const protocol::ItemInstance& item = m_goods.pieces[static_cast<std::size_t>(slot - 1)].item;
                if (const auto record = m_services.items.find(item.itemId)) {
                    const std::string text = ui::wrapTextLines(record->name + record->description, kMemoChars);
                    canvas.drawText(
                        {at.x + 46 + (slot - 1) * kPitch, at.y + 263},
                        text,
                        style,
                        m_memoSpot.size().width,
                        m_memoSpot.size().height
                    ); // first memo's top-left (x, y) in the area
                }
            }
            paint();
        });
        screen.on("onBuyUtensilSlotClick", [this](const ui::EventArgs& args) {
            m_clickedSlot = slotOf(args.source);
            m_info.hide();
            toggle(m_clickedSlot);
        });
        screen.on("onBuyUtensilBuy", [this](const ui::EventArgs&) { buy(); });
        screen.on("onBuyUtensilClose", [this](const ui::EventArgs&) { close(); });
    }

    ui::IconCell BuyUtensilWindow::pieceCell(const protocol::UtensilStallGoods::Piece& piece, bool marked) const {
        ui::IconCell cell = itemIconCell(m_services, piece.item, true, marked);
        cell.hint.clear();
        if (!marked) {
            cell.frame = {};
        }
        if (cell.picture.isValid()) {
            const gfx::Picture& icon = m_services.pictures.picture(cell.picture);
            cell.source = {0, 0, icon.width(), icon.height()};
        }
        return cell;
    }

    core::Size BuyUtensilWindow::cellsOf(int slot) const { return m_inventory.cellsOf(m_goods.pieces[static_cast<std::size_t>(slot - 1)].item.itemId); }

    void BuyUtensilWindow::show(const protocol::UtensilStallGoods& goods) {
        close();
        m_goods = goods;
        const int count = static_cast<int>(m_goods.pieces.size());
        const int layout = count >= 1 && count <= kSlots ? count : 1;
        m_window.setPicture("Form_Buy" + std::to_string(layout) + "Utensil_1", m_services.pictures);
        // x shift of the buttons for the two- and three-piece forms (wider pictures)
        const int shift = layout == 1 ? 0 : layout == 2 ? 95 : 206; // px; layout = number of pieces on offer (1..3)
        // Close button at (129 + shift, 453) and Buy button at (54 + shift, 453)
        m_screen.get<ui::Element>("BuyUtensilClose").setPosition({129 + shift, 453});
        m_screen.get<ui::Element>("BuyUtensilBuy").setPosition({54 + shift, 453});
        // small close button: x per form width (193, 399, 605), y 26
        m_screen.get<ui::Element>("BuyUtensilCloseSmall").setPosition({layout == 1 ? 193 : layout == 2 ? 399 : 605, 26});
        // x of the total and money edits per form width
        const int edits = layout == 1 ? 121 : layout == 2 ? 219 : 322;
        // y of the total edit (404) and the money edit (426)
        m_screen.get<ui::Element>("BuyUtensilTotal").setPosition({edits, 404});
        m_screen.get<ui::Element>("BuyUtensilMoney").setPosition({edits, 426});
        for (int slot = 1; slot <= kSlots; ++slot) {
            ui::IconGrid& grid = m_screen.get<ui::IconGrid>("BuyUtensilSlot" + std::to_string(slot));
            ui::TextBox& price = m_screen.get<ui::TextBox>("BuyUtensilPrice" + std::to_string(slot));
            grid.clearCells();
            price.setText("");
            if (slot > count) {
                continue;
            }
            const protocol::UtensilStallGoods::Piece& piece = m_goods.pieces[static_cast<std::size_t>(slot - 1)];
            const ui::IconCell cell = pieceCell(piece, false);
            const core::Point centre = slotCentre(slot);
            grid.setBounds({centre.x - cell.source.width / 2, centre.y - cell.source.height / 2, cell.source.width, cell.source.height});
            grid.setCell(1, cell);
            price.setText(std::to_string(piece.price));
            price.setFontColor(priceColor(price.text().size()));
        }
        markBag();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void BuyUtensilWindow::markBag() {
        m_marked.fill(false);
        m_reserved.fill(0);
        m_bag.clear();
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty()) {
                const core::Size cells = m_inventory.cellsOf(item.itemId);
                m_bag.claim(slot, cells.width, cells.height);
            }
        }
    }

    void BuyUtensilWindow::close() {
        for (int slot = 1; slot <= kSlots; ++slot) {
            m_screen.get<ui::IconGrid>("BuyUtensilSlot" + std::to_string(slot)).clearCells();
            m_screen.get<ui::TextBox>("BuyUtensilPrice" + std::to_string(slot)).setText("");
        }
        m_marked.fill(false);
        m_reserved.fill(0);
        m_goods = {};
        m_window.setVisible(false);
    }

    void BuyUtensilWindow::toggle(int slot) {
        if (slot < 1 || slot > static_cast<int>(m_goods.pieces.size())) {
            return;
        }
        const core::Size cells = cellsOf(slot);
        bool& marked = m_marked[static_cast<std::size_t>(slot)];
        if (!marked) {
            const auto place = m_bag.place(cells.width, cells.height);
            if (!place) {
                m_messages.show("It's full", 1200); // message display time in ms
                return;
            }
            marked = true;
            m_reserved[static_cast<std::size_t>(slot)] = *place;
        } else {
            marked = false;
            m_bag.free(m_reserved[static_cast<std::size_t>(slot)], cells.width, cells.height);
            m_reserved[static_cast<std::size_t>(slot)] = 0;
        }
        ui::IconGrid& grid = m_screen.get<ui::IconGrid>("BuyUtensilSlot" + std::to_string(slot));
        grid.setCell(1, pieceCell(m_goods.pieces[static_cast<std::size_t>(slot - 1)], marked));
    }

    void BuyUtensilWindow::buy() {
        if (m_sportActive()) {
            m_messages.show("This function is unavailable at present", 1200); // message display time in ms
            return;
        }
        std::int64_t total = 0;
        std::vector<protocol::UtensilPurchase> lines;
        for (int slot = 1; slot <= static_cast<int>(m_goods.pieces.size()); ++slot) {
            if (!m_marked[static_cast<std::size_t>(slot)]) {
                continue;
            }
            const protocol::UtensilStallGoods::Piece& piece = m_goods.pieces[static_cast<std::size_t>(slot - 1)];
            total += static_cast<std::int64_t>(piece.price) * piece.item.count;
            lines.push_back({piece.listPlace, piece.item.count});
        }
        if (lines.empty()) {
            return;
        }
        if (m_status.gold < total) {
            m_messages.show("Not enough money", 1200); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::buyUtensilsRequest(m_goods.ownerId, lines));
        close();
    }

    void BuyUtensilWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        const int hovered = m_screen.hoveredId();
        for (int slot = 1; slot <= static_cast<int>(m_goods.pieces.size()); ++slot) {
            const ui::IconGrid& grid = m_screen.get<ui::IconGrid>("BuyUtensilSlot" + std::to_string(slot));
            if (slot == m_clickedSlot && hovered != grid.id()) {
                m_clickedSlot = 0;
            }
            if (hovered == grid.id() && slot != m_clickedSlot) {
                const core::Point at = m_window.bounds().position() + grid.bounds().position();
                m_info.show(m_goods.pieces[static_cast<std::size_t>(slot - 1)].item, core::Rect{at.x, at.y, grid.bounds().width, grid.bounds().height});
            }
        }
    }

    void BuyUtensilWindow::paint() {
        std::int64_t total = 0;
        for (int slot = 1; slot <= static_cast<int>(m_goods.pieces.size()); ++slot) {
            if (m_marked[static_cast<std::size_t>(slot)]) {
                total += m_goods.pieces[static_cast<std::size_t>(slot - 1)].price;
            }
        }
        m_screen.get<ui::TextBox>("BuyUtensilTotal").setText(std::to_string(total));
        m_screen.get<ui::TextBox>("BuyUtensilMoney").setText(std::to_string(m_status.gold));
    }

}
