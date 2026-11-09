#include "client/viewmodels/trade/NpcSaleWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/Stalling.h"
#include "gfx/TextStyle.h"
#include "protocol/ShopRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>
#include <vector>

namespace client {

    namespace {

        const core::Color kTextColor = core::Color::fromRgb565(2113); // RGB565 0x0841: near black

        /// @brief Loads NpcSaleWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded NpcSale window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NpcSaleWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("NpcSale");
        }

        /// @brief Draws a number right-aligned in a ten digit field.
        /// @details Original: Format('%10s'), 8 pixels a digit.
        /// @param canvas Canvas to draw on.
        /// @param at Top-left of the field.
        /// @param text The number text.
        void drawRightAligned(gfx::Canvas& canvas, core::Point at, const std::string& text) {
            // 10: digits in the field, 8: pixels a digit
            const int x = (10 - static_cast<int>(text.size())) * 8;
            // 200 x 20: size of the text box
            canvas.drawText({at.x + x, at.y}, text, {kTextColor, gfx::TextEffect::Plain, std::nullopt}, 200, 20);
        }

    }

    NpcSaleWindow::NpcSaleWindow(
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
          m_bag(screen.get<ui::IconGrid>("NpcSaleBag")),
          m_ok(screen.get<ui::ImageButton>("NpcSaleOk")),
          m_close(screen.get<ui::ImageButton>("NpcSaleClose")) {
        screen.get<ui::DrawingArea>("NpcSalePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onNpcSaleToggle", [this](const ui::EventArgs& args) { toggle(args.index); });
        screen.on("onNpcSaleUnmark", [this](const ui::EventArgs& args) { unmark(args.index); });
        screen.on("onNpcSaleOk", [this](const ui::EventArgs&) { sell(); });
        screen.on("onNpcSaleClose", [this](const ui::EventArgs&) { close(); });
    }

    void NpcSaleWindow::show(bool goods, std::function<void()> onClosed) {
        m_goods = goods;
        m_onClosed = std::move(onClosed);
        m_window.setPicture(goods ? "Form_SaleThing_1" : "Form_SaleEquip_1", m_services.pictures);
        const int buttonsY = goods ? 461 : 471; // y of the OK and Close buttons for the goods and equipment forms
        m_ok.setPosition({54, buttonsY}); // x of the OK button
        m_close.setPosition({129, buttonsY}); // x of the Close button
        clearMarks();
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void NpcSaleWindow::close() {
        m_window.setVisible(false);
        clearMarks();
        const std::function<void()> handler = std::move(m_onClosed);
        m_onClosed = nullptr;
        if (handler) {
            handler();
        }
    }

    void NpcSaleWindow::clearMarks() {
        m_marks.clearUseMarks(BagMarks::kSale);
        m_bagChanged();
    }

    void NpcSaleWindow::refresh() {
        m_bag.clearCells();
        for (int slot = 1; slot <= BagMarks::kSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_bag.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
    }

    void NpcSaleWindow::toggle(int slot) {
        if (slot < 1 || slot > BagMarks::kSlots) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(slot);
        if (item.empty()) {
            return;
        }
        const int refusal = npcSaleRefusal(m_services.items, item.itemId, !m_goods);
        if (refusal == 1) {
            m_messages.show("This item cannot be sold to NPC", 2000); // message display time in ms
            return;
        }
        if (refusal != 0) {
            m_messages.show("This NPC don't accept this kind of item", 2000); // message display time in ms
            return;
        }
        if (m_marks.isPriced(slot)) {
            m_messages.show("This item is selected for other purposes, cannot be traded. ", 2000); // message display time in ms
            return;
        }
        const int mark = m_marks.useMark(slot);
        if (mark == 0) {
            m_marks.setUseMark(slot, BagMarks::kSale);
        } else if (mark == BagMarks::kSale) {
            m_marks.setUseMark(slot, 0);
        }
        m_bagChanged();
    }

    void NpcSaleWindow::unmark(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == BagMarks::kSale) {
            m_marks.setUseMark(slot, 0);
            m_bagChanged();
        }
    }

    void NpcSaleWindow::sell() {
        std::vector<int> slots;
        for (int slot = 1; slot <= BagMarks::kSlots; ++slot) {
            if (!m_inventory.bagItem(slot).empty() && m_marks.useMark(slot) == BagMarks::kSale) {
                slots.push_back(slot);
            }
        }
        if (slots.empty()) {
            return;
        }
        m_services.connection.send(protocol::npcSaleRequest(slots, m_goods ? 1 : 0));
        clearMarks();
    }

    void NpcSaleWindow::sold(int result) {
        static const char* const kResults[] = {
            nullptr,
            "Sorry, your bank account is full.",
            "Sorry, this item is not in your inventory.",
            "Sorry, you can't sell so many items.",
            "Sorry, trade has failed."
        };
        // 27/2 results 1..4 are failures; 2000 is the message display time in ms
        if (result >= 1 && result <= 4) {
            m_messages.show(kResults[result], 2000);
        }
    }

    void NpcSaleWindow::paint(gfx::Canvas& canvas, core::Point at) {
        std::int32_t sum = 0;
        for (int slot = 1; slot <= BagMarks::kSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && m_marks.useMark(slot) == BagMarks::kSale) {
                if (const auto record = m_services.items.find(item.itemId)) {
                    sum += record->sellPrice * item.count;
                }
            }
        }
        // top-left of the sum field for the goods and equipment forms
        const core::Point total = m_goods ? core::Point{111, 413} : core::Point{118, 419};
        drawRightAligned(canvas, {at.x + total.x, at.y + total.y}, std::to_string(sum));
        drawRightAligned(canvas, {at.x + total.x, at.y + total.y + 22}, std::to_string(m_status.gold)); // the gold field is 22 pixels below the sum
    }

}
