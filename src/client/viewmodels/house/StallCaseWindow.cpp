#include "client/viewmodels/house/StallCaseWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/PriceColors.h"
#include "gfx/TextStyle.h"
#include "protocol/FurnitureCaseMessages.h"

#include <algorithm>
#include <string>

namespace client {

    namespace {

        constexpr int kBoxes = 5; // boxes drawn in the case
        constexpr int kBoxStack = 50; // items one box shows
        constexpr int kMostInCase = 250; // most items the case holds (5 boxes x 50)
        constexpr std::int32_t kMostPrice = 9999999; // highest unit price
        constexpr int kHoldsFood = 23; // item type 23: cooked food and drinks (Apple Juice, ...); the case takes this type
        constexpr int kHoldsDrink = 32; // item type 32: ingredients (Beef, Banana, ...); the case takes this type too
        constexpr int kNoTradeFlag = 2; // item flag bit 0x02: the item cannot be sold
        constexpr int kAllowedMark = 0; // no bag use mark is let through by lockNotice
        constexpr int kTextWidth = 230; // text box width, px

        /// @brief Loads StallCaseWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("StallCaseWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("StallCaseWindow");
        }

        /// @brief Makes a plain text style in a colour.
        /// @param color Text colour.
        /// @return The text style.
        gfx::TextStyle plain(const core::Color& color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

    }

    StallCaseWindow::StallCaseWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_cells(screen.get<ui::IconGrid>("StallCaseCells")),
          m_nameSpot(screen.get<ui::Spot>("StallCaseName")),
          m_priceSpot(screen.get<ui::Spot>("StallCasePrice")) {
        screen.get<ui::DrawingArea>("StallCasePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onStallCaseClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onStallCaseSetup", [this](const ui::EventArgs&) {
            if (m_furnIndex != 0 && m_caseNo != 0 && m_itemId != 0) {
                m_hooks.openReprice(m_furnIndex, m_price);
            }
        });
        screen.on("onStallCaseTake", [this](const ui::EventArgs& args) { take(args.index); });
    }

    void StallCaseWindow::open(int furnIndex, int caseNo, int itemId, std::int32_t price) {
        m_furnIndex = furnIndex;
        m_caseNo = caseNo;
        m_itemId = itemId;
        m_price = price;
        show();
    }

    void StallCaseWindow::openForPiece(int furnIndex) {
        m_furnIndex = furnIndex;
        show();
    }

    void StallCaseWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void StallCaseWindow::close() {
        if (m_window.isVisible()) {
            m_hooks.closePrice();
        }
        m_window.setVisible(false);
        m_furnIndex = 0;
        m_caseNo = 0;
        m_itemId = 0;
        m_price = 0;
    }

    StallCaseWindow::Stack StallCaseWindow::stackAt(int box) const {
        const StallCaseRecord& stalled = m_house.stalls().ownCase(m_caseNo);
        if (stalled.itemId == 0 || box < 1 || box > kBoxes) {
            return {};
        }
        const int before = (box - 1) * kBoxStack;
        return {stalled.itemId, std::clamp(stalled.count - before, 0, kBoxStack)};
    }

    void StallCaseWindow::take(int box) {
        const Stack stack = stackAt(box);
        if (m_furnIndex == 0 || m_caseNo == 0 || stack.itemId == 0 || stack.count == 0) {
            return;
        }
        if (m_inventory.freeSlotFor(stack.itemId) == 0) {
            m_messages.show("There is not enough space in item column to recycle", 1000); // notice duration in ms
            return;
        }
        m_hooks.send(protocol::takeFromCaseRequest(m_furnIndex, m_caseNo, stack.count));
    }

    void StallCaseWindow::putItem(int bagSlot, bool setPrice, std::int32_t price) {
        if (bagSlot <= 0 || bagSlot > BagMarks::kSlots) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        if (item.itemId == 0 || m_furnIndex == 0) {
            return;
        }
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        if (record->type != kHoldsFood && record->type != kHoldsDrink) {
            m_messages.show("The item can't be stored", 1000); // notice duration in ms
            return;
        }
        if ((record->flags & kNoTradeFlag) != 0) {
            m_messages.show("Unavailable for expres", 2000); // notice duration in ms
            return;
        }
        if (const auto lock = m_marks.lockNotice(bagSlot, kAllowedMark)) {
            m_messages.show(lock->text, lock->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 1000); // notice duration in ms
            return;
        }
        if (setPrice) {
            if (price > 0 && price <= kMostPrice) {
                m_hooks.send(protocol::openCaseRequest(m_furnIndex, bagSlot, price));
            }
            return;
        }
        if (m_price == 0 || m_caseNo == 0) {
            return;
        }
        if (item.itemId != m_itemId) {
            m_messages.show("Items from different types can't' be put in", 1000); // notice duration in ms
            return;
        }
        const int held = m_house.stalls().ownCase(m_caseNo).count;
        const int room = held + item.count > kMostInCase ? kMostInCase - held : 0;
        m_hooks.send(protocol::addToCaseRequest(m_furnIndex, bagSlot, m_caseNo, room));
    }

    void StallCaseWindow::sendPrice(std::int32_t price) {
        if (m_house.stalls().ownCase(m_caseNo).price != price) {
            m_hooks.send(protocol::repriceCaseRequest(m_furnIndex, m_caseNo, price));
        }
    }

    void StallCaseWindow::changed(const StallCaseChange& change) {
        m_caseNo = change.caseNo;
        m_itemId = change.after.itemId;
        m_price = change.after.price;
        if (change.before.price != 0 && change.before.price != change.after.price) {
            m_messages.show("Price has been changed to " + std::to_string(change.after.price), 1000); // notice duration in ms
        }
        if (change.after.count == 0) {
            close();
        }
    }

    void StallCaseWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        const auto record = m_services.items.find(m_itemId);
        canvas.drawText({at.x + m_nameSpot.at().x, at.y + m_nameSpot.at().y}, record ? record->name : std::string(), plain(m_nameSpot.color()), kTextWidth);
        const std::string price = std::to_string(m_price);
        canvas.drawText({at.x + m_priceSpot.at().x, at.y + m_priceSpot.at().y}, price, plain(priceColor(price.size())), kTextWidth);
        m_cells.clearCells();
        for (int box = 1; box <= kBoxes; ++box) {
            const Stack stack = stackAt(box);
            if (stack.itemId != 0 && stack.count != 0) {
                protocol::ItemInstance item;
                item.itemId = stack.itemId;
                item.count = stack.count;
                ui::IconCell cell = itemIconCell(m_services, item, true);
                cell.hint.clear();
                m_cells.setCell(box, cell);
            }
        }
    }

}
