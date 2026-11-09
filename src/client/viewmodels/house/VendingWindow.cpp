#include "client/viewmodels/house/VendingWindow.h"

#include "client/rules/FurnitureKinds.h"
#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/PriceColors.h"
#include "core/DateTime.h"

#include <string>

namespace client {

    namespace {

        constexpr int kPlaces = 5; // item rows in the vending machine

        /// @brief Loads VendingWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("VendingWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("VendingWindow");
        }

    }

    VendingWindow::VendingWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("VendingList")),
          m_placeSpot(screen.get<ui::Spot>("VendingPlace")),
          m_textSpot(screen.get<ui::Spot>("VendingText")) {
        screen.get<ui::DrawingArea>("VendingDrop").setPainter([this](gfx::Canvas&, core::Point, std::uint64_t) { paint(); });
        screen.on("onVendingClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onVendingPick", [this](const ui::EventArgs& args) {
            if (m_hooks.ownId() == m_house.furniture().ownerId()) {
                return;
            }
            if (const StallInFurItem& item = itemOf(args.index); item.itemId != 0) {
                m_hooks.buy(item);
            }
        });
        screen.on("onVendingStop", [this](const ui::EventArgs& args) {
            if (m_hooks.ownId() != m_house.furniture().ownerId()) {
                return;
            }
            if (const StallInFurItem& item = itemOf(args.index); item.itemId != 0) {
                m_hooks.askStop(item.stallIndex);
            }
        });
    }

    void VendingWindow::open(int furnIndex) {
        m_furnIndex = furnIndex;
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    const StallInFurItem& VendingWindow::itemOf(int row) const {
        static const StallInFurItem none;
        const int slot = m_house.utensils().slotOfPiece(m_furnIndex);
        if (slot == 0 || row < 0 || row >= kPlaces) {
            return none;
        }
        return m_house.stalls().vending(m_house.utensils().utensil(slot).contentSlots[static_cast<std::size_t>(row + 1)]);
    }

    int VendingWindow::rowUnderMouse() const {
        const int y = m_screen.mouse().y - m_window.bounds().position().y;
        for (int row = 1; row <= kPlaces; ++row) {
            if ((row - 1) * m_placeSpot.pitch().y + m_placeSpot.at().y <= y && row * m_placeSpot.pitch().y + m_placeSpot.at().y >= y) {
                return row;
            }
        }
        return 0;
    }

    void VendingWindow::refresh() {
        using Align = ui::BlockList::Align;
        m_list.clear();
        const int slot = m_house.utensils().slotOfPiece(m_furnIndex);
        if (slot == 0 || !FurnitureKinds::isVendingMachine(m_services.items, m_house.utensils().utensil(slot).itemId)) {
            return;
        }
        const double now = core::nowAsDateTime();
        for (int place = 0; place < kPlaces; ++place) {
            ui::BlockList::Row row;
            if (const StallInFurItem& stalled = itemOf(place); stalled.itemId != 0 && m_services.items.find(stalled.itemId)) {
                protocol::ItemInstance shown = stalled.item;
                shown.itemId = stalled.itemId;
                shown.count = stalled.count;
                ui::BlockList::PictureBlock icon;
                icon.picture = itemIconCell(m_services, shown, false).picture;
                icon.area = {32, 56}; // icon area width x height in pixels
                icon.horizontal = Align::Centre;
                icon.vertical = Align::Centre;
                icon.count = stalled.count;
                row.pictures.push_back(icon);
                const std::string price = std::to_string(stalled.price);
                // text blocks per row as {offset x, y}, {area width, height}: name at x 35 (72 wide), description at x 110 (179 wide), price at x 292 (61 wide)
                row.texts.push_back({stallItemName(m_services.items, shown), {35, 0}, {72, 56}, Align::Centre, Align::Centre, m_textSpot.color()});
                row.texts.push_back({describeStallItem(m_services.items, shown, now), {110, 3}, {179, 53}, Align::Centre, Align::Centre, m_textSpot.color()});
                row.texts.push_back({price, {292, 0}, {61, 56}, Align::Centre, Align::Centre, priceColor(price.size())});
            }
            m_list.addRow(std::move(row));
        }
    }

    void VendingWindow::paint() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        refresh();
    }

}
