#include "client/viewmodels/house/EggBuyWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/PriceColors.h"
#include "gfx/TextStyle.h"
#include "protocol/EggMachineRequests.h"

#include <string>

namespace client {

    namespace {

        constexpr int kPlaces = 10; // item places in the Fortune Egg machine

        /// @brief Loads EggBuyWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded EggBuyWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("EggBuyWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("EggBuyWindow");
        }

    }

    EggBuyWindow::EggBuyWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const Inventory& inventory,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_inventory(inventory),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_price(screen.get<ui::TextBox>("EggBuyPrice")),
          m_cells(screen.get<ui::IconGrid>("EggBuyCells")),
          m_eggCountSpot(screen.get<ui::Spot>("EggBuyCount")) {
        screen.get<ui::DrawingArea>("EggBuyPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onEggBuyClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onEggBuyOk", [this](const ui::EventArgs&) { play(); });
        m_price.setText("0");
    }

    const HouseUtensil& EggBuyWindow::utensil() const { return m_house.utensils().utensil(m_house.utensils().slotOfPiece(m_furnIndex)); }

    void EggBuyWindow::showPrice(std::int32_t price) {
        const std::string text = std::to_string(price);
        m_price.setText(text);
        m_price.setFontColor(priceColor(text.size()));
    }

    void EggBuyWindow::open(int furnIndex) {
        m_furnIndex = furnIndex;
        showPrice(utensil().salePrice);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.send(protocol::askEggMachineRequest(m_furnIndex, m_house.furniture().ownerId()));
    }

    void EggBuyWindow::priceChanged(int furnIndex, std::int32_t price) {
        if (m_window.isVisible() && m_furnIndex == furnIndex) {
            showPrice(price);
            m_messages.show("It is empty", 2000); // message display time in ms
            close();
        }
    }

    void EggBuyWindow::play() {
        const HouseUtensil& machine = utensil();
        const FurnitureStalls::Content& first = m_house.stalls().content(machine.contentSlots[1]);
        if (m_inventory.freeSlotFor(first.itemId) == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        if (machine.salePrice <= 0) {
            m_messages.show("It hasn't been used", 2000); // message display time in ms
            return;
        }
        if (machine.salePrice > m_hooks.gold()) {
            m_messages.show("Not enough money", 0); // message display time argument, 0 here
            return;
        }
        const std::int32_t price = machine.salePrice;
        m_hooks.askTimed("The price of playing once is" + std::to_string(price), [this, price] {
            m_hooks.send(protocol::playEggMachineRequest(m_house.furniture().ownerId(), m_furnIndex, price));
        });
    }

    void EggBuyWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        const HouseUtensil& machine = utensil();
        canvas.drawText(
            {at.x + m_eggCountSpot.at().x, at.y + m_eggCountSpot.at().y},
            machine.salePrice != 0 ? std::to_string(machine.eggCount) : "0",
            {m_eggCountSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            200
        ); // text box width in pixels
        m_cells.clearCells();
        for (int cell = 1; cell <= kPlaces; ++cell) {
            const FurnitureStalls::Content& stack = m_house.stalls().content(machine.contentSlots[static_cast<std::size_t>(cell)]);
            if (stack.itemId != 0) {
                protocol::ItemInstance item;
                item.itemId = stack.itemId;
                item.count = stack.count;
                ui::IconCell icon = itemIconCell(m_services, item, true);
                icon.hint.clear();
                m_cells.setCell(cell, icon);
            }
        }
    }

}
