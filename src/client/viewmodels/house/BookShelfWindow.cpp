#include "client/viewmodels/house/BookShelfWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "protocol/BookShelfRequests.h"
#include "protocol/ItemInstance.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kCellSide = 32; // size of one item cell, px
        constexpr int kBlueprintSubType = 103; // item sub type 103: blueprints (original IsBlueprintItem); only these fit on the shelf

        /// @brief Loads BookShelfWindow.xml into the screen and returns the Bookcase window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded BookCaseWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BookShelfWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BookCaseWindow");
        }

    }

    BookShelfWindow::BookShelfWindow(
        GameServices& services,
        ui::Screen& screen,
        const BookShelves& shelves,
        const Inventory& inventory,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_shelves(shelves),
          m_inventory(inventory),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_case{loadWindow(services, screen), screen.get<ui::IconGrid>("BookCaseCells")},
          m_desk{screen.get<ui::Window>("BookDeskWindow"), screen.get<ui::IconGrid>("BookDeskCells")} {
        screen.on("onBookCaseClose", [this](const ui::EventArgs&) { m_case.window.setVisible(false); });
        screen.on("onBookDeskClose", [this](const ui::EventArgs&) { m_desk.window.setVisible(false); });
        screen.on("onBookCaseTake", [this](const ui::EventArgs& args) { take(false, args.index); });
        screen.on("onBookDeskTake", [this](const ui::EventArgs& args) { take(true, args.index); });
    }

    void BookShelfWindow::open(bool desk) {
        ui::Window& window = shelf(desk).window;
        window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(window.id());
        }
    }

    void BookShelfWindow::take(bool desk, int place) {
        const int itemId = m_shelves.item(desk, place);
        if (itemId == 0) {
            return;
        }
        if (m_inventory.freeSlotFor(itemId) == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::takeFromBookShelfRequest(desk, place));
    }

    void BookShelfWindow::drop(bool desk, int bagSlot, int place) {
        const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
        if (!record || record->subType != kBlueprintSubType) {
            m_messages.show("Cannot enter", 2000); // message display time in ms
            return;
        }
        const int empty = m_shelves.firstEmpty(desk);
        if (empty == 0) {
            m_messages.show("It's full", 2000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::putInBookShelfRequest(desk, bagSlot, m_shelves.item(desk, place) != 0 ? empty : place));
    }

    void BookShelfWindow::paint(bool desk) {
        Shelf& form = shelf(desk);
        form.cells.clearCells();
        for (int place = 1; place <= BookShelves::kPlaces; ++place) {
            const int itemId = m_shelves.item(desk, place);
            if (itemId != 0) {
                protocol::ItemInstance item;
                item.itemId = itemId;
                item.count = 1;
                ui::IconCell cell = itemIconCell(m_services, item, false);
                cell.hint.clear();
                form.cells.setCell(place, cell);
            }
        }
        if (m_screen.hoveredId() != form.cells.id()) {
            return;
        }
        const core::Point origin = form.window.bounds().position() + form.cells.bounds().position();
        const int place = form.cells.cellAt(m_screen.mouse() - origin);
        if (const int itemId = place != 0 ? m_shelves.item(desk, place) : 0; itemId != 0) {
            protocol::ItemInstance item;
            item.itemId = itemId;
            item.count = 1;
            const core::Point at = origin + form.cells.cellPosition(place);
            m_info.show(item, {at.x, at.y, kCellSide, kCellSide});
        }
    }

    void BookShelfWindow::update() {
        for (const bool desk : {false, true}) {
            if (!shelf(desk).window.isVisible()) {
                continue;
            }
            if (!m_hooks.inFloorRoom()) {
                shelf(desk).window.setVisible(false);
                continue;
            }
            paint(desk);
        }
    }

}
