#include "client/viewmodels/house/FurnitureBuyWindow.h"

#include "client/rules/PriceColors.h"
#include "gfx/TextStyle.h"
#include "protocol/FurnitureCaseMessages.h"
#include "protocol/FurnitureStallRequests.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kMostBought = 50; // most items that can be bought at once
        constexpr std::int32_t kDearPrice = 9999999; // unit price from which only 1 item can be bought
        constexpr int kTextWidth = 200; // text box width, px

        /// @brief Loads FurnitureBuyWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded FurnitureBuyWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("FurnitureBuyWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("FurnitureBuyWindow");
        }

        /// @brief Makes a plain (no effect) text style of a colour.
        /// @param color Text colour.
        /// @return The text style.
        gfx::TextStyle plain(const core::Color& color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

    }

    FurnitureBuyWindow::FurnitureBuyWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const Inventory& inventory,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_house(house),
          m_inventory(inventory),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_count(screen.get<ui::TextBox>("FurnitureBuyCount")),
          m_priceSpot(screen.get<ui::Spot>("FurnitureBuyPrice")),
          m_nameSpot(screen.get<ui::Spot>("FurnitureBuyName")),
          m_leftSpot(screen.get<ui::Spot>("FurnitureBuyLeft")),
          m_totalSpot(screen.get<ui::Spot>("FurnitureBuyTotal")) {
        screen.get<ui::DrawingArea>("FurnitureBuyPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onFurnitureBuyOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onFurnitureBuyClose", [this](const ui::EventArgs&) { close(); });
        m_count.setText("0");
    }

    void FurnitureBuyWindow::open(Kind kind, const StallInFurItem& item, const std::string& name) {
        m_kind = kind;
        m_name = name;
        m_furnIndex = item.furnIndex;
        m_itemCount = item.count;
        m_unitPrice = item.price;
        m_goodsSlot = item.posInFurn;
        m_stallIndex = item.stallIndex;
        m_count.setText("0");
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void FurnitureBuyWindow::reclaimed(int furnIndex, int posInFurn) {
        if (m_window.isVisible() && m_furnIndex == furnIndex && m_goodsSlot == posInFurn) {
            m_messages.show("Player has reclaimed the item.", 2000); // message display time in ms
            close();
        }
    }

    void FurnitureBuyWindow::caseChanged(const StallCaseChange& change) {
        if (change.own || !m_window.isVisible() || m_kind != Kind::Case) {
            return;
        }
        if (m_furnIndex == change.after.furnIndex && m_goodsSlot == change.after.posInFurn) {
            if (change.before.price != change.after.price) {
                m_messages.show("Price has been changed to " + std::to_string(change.after.price), 1000); // message display time in ms
                close();
            }
            if (change.before.count != change.after.count) {
                m_messages.show("Selling number has been changed to " + std::to_string(change.after.count), 1000); // message display time in ms
                m_itemCount = change.after.count;
            }
        }
        if (change.after.count == 0) {
            m_messages.show("Host reclaim items", 1000); // message display time in ms
            close();
        }
    }

    int FurnitureBuyWindow::count() const { return m_count.text().empty() ? 0 : std::atoi(m_count.text().c_str()); }

    void FurnitureBuyWindow::confirm() {
        const int slot = m_house.utensils().slotOfPiece(m_furnIndex);
        const int number = slot != 0 ? m_house.utensils().utensil(slot).contentSlots[static_cast<std::size_t>(m_goodsSlot)] : 0;
        const StallInFurItem& item = m_kind == Kind::Shelf     ? m_house.stalls().shelf(number)
                                     : m_kind == Kind::Vending ? m_house.stalls().vending(number)
                                                               : m_house.stalls().displayed(number);
        if (item.itemId != 0 && m_inventory.freeSlotFor(item.itemId) == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        if (static_cast<std::int64_t>(count()) * m_unitPrice > m_hooks.gold()) {
            m_messages.show("Not enough money", 2000); // message display time in ms
            return;
        }
        const std::int32_t owner = m_house.furniture().ownerId();
        if (m_kind == Kind::Case) {
            m_hooks.send(protocol::buyFromCaseRequest(owner, m_stallIndex, count(), item.price));
        } else {
            m_hooks.send(
                m_kind == Kind::Shelf ? protocol::buyFromShelfRequest(owner, m_stallIndex, count(), item.echoItem, item.echoPrice)
                                      : protocol::buyFromVendingRequest(owner, m_stallIndex, count(), item.echoItem, item.echoPrice)
            );
        }
        close();
    }

    void FurnitureBuyWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        const std::string price = std::to_string(m_unitPrice);
        canvas.drawText({at.x + m_priceSpot.at().x, at.y + m_priceSpot.at().y}, price, plain(priceColor(price.size())), kTextWidth);
        canvas.drawText({at.x + m_nameSpot.at().x, at.y + m_nameSpot.at().y}, m_name, plain(m_nameSpot.color()), kTextWidth);
        canvas.drawText({at.x + m_leftSpot.at().x, at.y + m_leftSpot.at().y}, std::to_string(m_itemCount), plain(m_leftSpot.color()), kTextWidth);
        if (!m_count.text().empty()) {
            if (m_unitPrice >= kDearPrice && count() > 1) {
                m_count.setText("1");
            }
            if (count() > m_itemCount) {
                m_count.setText(std::to_string(m_itemCount));
            }
            if (count() > kMostBought) {
                m_count.setText(std::to_string(kMostBought));
            }
            if (count() <= 0) {
                m_count.setText("0");
            }
            canvas.drawText(
                {at.x + m_totalSpot.at().x, at.y + m_totalSpot.at().y},
                std::to_string(static_cast<std::int64_t>(count()) * m_unitPrice),
                plain(m_totalSpot.color()),
                kTextWidth
            );
        }
    }

}
