#include "client/viewmodels/house/EggSellWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "gfx/TextStyle.h"
#include "protocol/EggMachineRequests.h"

#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kPlaces = 10; // item places in the Fortune Egg machine
        constexpr int kLeastStack = 10; // an item stack needs at least this many items to go in
        constexpr int kNoTradeFlag = 2; // item flag bit 0x02: the item cannot be sold
        constexpr int kAllowedMark = 1; // bag use mark 1 (compound window mark) that lockNotice lets through

        /// @brief Loads EggSellWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded EggSellWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("EggSellWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("EggSellWindow");
        }

    }

    EggSellWindow::EggSellWindow(
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
          m_price(screen.get<ui::TextBox>("EggSellPrice")),
          m_cells(screen.get<ui::IconGrid>("EggSellCells")),
          m_eggCountSpot(screen.get<ui::Spot>("EggSellCount")),
          m_noEggsSpot(screen.get<ui::Spot>("EggSellNoEggs")) {
        screen.get<ui::DrawingArea>("EggSellPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onEggSellClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onEggSellSetup", [this](const ui::EventArgs&) { setPrice(); });
        screen.on("onEggSellStop", [this](const ui::EventArgs&) { stopSelling(); });
        screen.on("onEggSellRetake", [this](const ui::EventArgs& args) { retake(args.index); });
        m_price.setText("0");
    }

    const HouseUtensil& EggSellWindow::utensil() const { return m_house.utensils().utensil(m_house.utensils().slotOfPiece(m_furnIndex)); }

    int EggSellWindow::price() const { return m_price.text().empty() ? 0 : std::atoi(m_price.text().c_str()); }

    void EggSellWindow::open(int furnIndex) {
        m_furnIndex = furnIndex;
        m_price.setText(std::to_string(utensil().salePrice));
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void EggSellWindow::priceChanged(int furnIndex, std::int32_t price) {
        if (m_window.isVisible() && m_furnIndex == furnIndex) {
            m_price.setText(std::to_string(price));
        }
    }

    int EggSellWindow::emptyPlace() const {
        for (int place = 1; place <= kPlaces; ++place) {
            if (utensil().contentSlots[static_cast<std::size_t>(place)] == 0) {
                return place;
            }
        }
        return 0;
    }

    bool EggSellWindow::allPlacesFull() const { return emptyPlace() == 0; }

    bool EggSellWindow::hasShortStack() const {
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_house.stalls().content(utensil().contentSlots[static_cast<std::size_t>(place)]).count < kLeastStack) {
                return true;
            }
        }
        return false;
    }

    void EggSellWindow::drop(int bagSlot, int place) {
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        const auto refuse = [this](const std::string& text, int ms) { m_messages.show(text, ms); };
        // the refusal messages below show for 2000 or 1000 ms
        if (emptyPlace() == 0) {
            refuse("It's full", 2000);
        } else if (item.wear != 0) {
            refuse("Item cannot be damaged", 2000);
        } else if (!m_inventory.isStackable(item.itemId)) {
            refuse("Cannot be used in it", 2000);
        } else if (item.count < kLeastStack) {
            refuse("Should be at least 10 items in the Fortune Egg machine.", 2000);
        } else if (record->cellWidth != 1 || record->cellHeight != 1) {
            refuse("The item is too big to sell", 1000);
        } else if (const auto lock = m_marks.lockNotice(bagSlot, kAllowedMark)) {
            refuse(lock->text, lock->ms);
        } else if (!m_marks.isAvailable(bagSlot)) {
            refuse("Unable to operate when the item is locked", 1000);
        } else if ((record->flags & kNoTradeFlag) != 0 || item.lookItemId != 0) {
            refuse("It cannot be sold", 2000);
        } else if (utensil().salePrice != 0) {
            refuse("Please stop selling firstly", 2000);
        } else if (utensil().contentSlots[static_cast<std::size_t>(place)] == 0) {
            m_hooks.send(protocol::putEggStackRequest(m_furnIndex, bagSlot, place));
        } else {
            m_hooks.send(protocol::putEggStackRequest(m_furnIndex, emptyPlace(), place));
        }
    }

    void EggSellWindow::setPrice() {
        if (!allPlacesFull()) {
            m_messages.show("Not enough items", 1000); // message display time in ms
            return;
        }
        if (utensil().salePrice != 0) {
            m_messages.show("Stop selling and change the price", 1000); // message display time in ms
            return;
        }
        if (price() <= 0) {
            m_messages.show("Wrong price. Please enter again", 1000); // message display time in ms
            return;
        }
        for (int place = 1; place <= kPlaces; ++place) {
            const FurnitureStalls::Content& stack = m_house.stalls().content(utensil().contentSlots[static_cast<std::size_t>(place)]);
            if (stack.count < kLeastStack) {
                m_messages.show("Should be at least 10 items in the Fortune Egg machine.", 2000); // message display time in ms
                return;
            }
            const auto record = m_services.items.find(stack.itemId);
            if (record && (record->flags & kNoTradeFlag) != 0) {
                return;
            }
        }
        m_hooks.askTimed("Do you want to set the price as" + std::to_string(price()), [this] { confirmPrice(); });
    }

    void EggSellWindow::confirmPrice() {
        const bool full = allPlacesFull();
        if (hasShortStack()) {
            m_messages.show("Should be at least 10 items in the Fortune Egg machine.", 2000); // message display time in ms
            return;
        }
        if (!full) {
            m_messages.show("Not enough items", 1000); // message display time in ms
            return;
        }
        close();
        m_hooks.send(protocol::setEggPriceRequest(m_furnIndex, price()));
        m_messages.show("Fortune Egg is set successfully", 1200); // message display time in ms
    }

    void EggSellWindow::stopSelling() { m_hooks.send(protocol::setEggPriceRequest(m_furnIndex, 0)); } // price 0 stops selling

    void EggSellWindow::retake(int place) {
        if (place < 1 || place > kPlaces) {
            return;
        }
        const FurnitureStalls::Content& stack = m_house.stalls().content(utensil().contentSlots[static_cast<std::size_t>(place)]);
        if (stack.itemId == 0) {
            return;
        }
        if (m_inventory.freeSlotFor(stack.itemId) == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        if (price() > 0) {
            m_messages.show("Stop selling and retake the item", 1000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::retakeEggStackRequest(m_furnIndex, place));
    }

    void EggSellWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        const HouseUtensil& machine = utensil();
        const ui::Spot& place = machine.salePrice != 0 ? m_eggCountSpot : m_noEggsSpot;
        canvas.drawText(
            {at.x + place.at().x, at.y + place.at().y},
            machine.salePrice != 0 ? std::to_string(machine.eggCount) : "0",
            {place.color(), gfx::TextEffect::Plain, std::nullopt},
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
