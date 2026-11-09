#include "client/viewmodels/house/GasStorageWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "gfx/TextStyle.h"
#include "protocol/HouseRequests.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace client {

    namespace {

        constexpr int kGasItem = 47009; // item 47009: Gas (item type 38)
        constexpr int kNoticeMs = 2000; // notice display time, ms
        constexpr int kTankWidth = 83; // tank picture width, px

        /// @brief Per-tank constants: layout file, element prefix, use mark, capacity and tank picture placement.
        struct Layout {
            const char* file;
            const char* prefix;
            int mark;
            int capacity;
            const char* picture;
            int pictureHeight;
            int pictureLeft;
            int pictureTop;
        };

        // fields: use mark, capacity (fuel units), tank picture height, left and top (px)
        constexpr Layout kBig{"GasCanWindow.xml", "GasCan", 19, 6000, "GasCanCap", 100, 213, 225}; // big gas can: mark 19, holds 6000
        constexpr Layout kSmall{"SmallGasWindow.xml", "SmallGas", 20, 250, "SmallGasCan", 75, 212, 233}; // small gas can: mark 20, holds 250

        /// @brief Picks the layout constants of a tank.
        /// @param tank Which tank.
        /// @return The Layout of that tank.
        const Layout& layoutOf(GasStorageWindow::Tank tank) { return tank == GasStorageWindow::Tank::Big ? kBig : kSmall; }

        /// @brief Builds a tank's element name from its prefix and a part name.
        /// @param tank Which tank.
        /// @param part Name suffix such as "Window" or "Cells".
        /// @return The full element name.
        std::string nameOf(GasStorageWindow::Tank tank, const char* part) { return std::string(layoutOf(tank).prefix) + part; }

        /// @brief Loads the tank's layout file and returns its window.
        /// @param tank Which tank.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GasStorageWindow::Tank tank, GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout(layoutOf(tank).file), services.loadContext());
            return screen.get<ui::Window>(nameOf(tank, "Window"));
        }

    }

    GasStorageWindow::GasStorageWindow(
        Tank tank,
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_tank(tank),
          m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(tank, services, screen)),
          m_cells(screen.get<ui::IconGrid>(nameOf(tank, "Cells"))),
          m_fuelSpot(screen.get<ui::Spot>(nameOf(tank, "Fuel"))),
          m_slashSpot(screen.get<ui::Spot>(nameOf(tank, "Slash"))),
          m_capacitySpot(screen.get<ui::Spot>(nameOf(tank, "Capacity"))) {
        screen.get<ui::DrawingArea>(nameOf(tank, "Paint")).setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("on" + nameOf(tank, "Close"), [this](const ui::EventArgs&) { close(); });
        screen.on("on" + nameOf(tank, "Fill"), [this](const ui::EventArgs&) { fill(); });
        screen.on("on" + nameOf(tank, "Mark"), [this](const ui::EventArgs& args) { mark(args.index); });
        screen.on("on" + nameOf(tank, "Unmark"), [this](const ui::EventArgs& args) { unmark(args.index); });
    }

    void GasStorageWindow::open() {
        unmarkAll();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.closeMakeInfo();
    }

    void GasStorageWindow::close() {
        m_window.setVisible(false);
        unmarkAll();
    }

    void GasStorageWindow::unmarkAll() {
        m_marks.clearUseMarks(layoutOf(m_tank).mark);
        m_marked.fill(false);
        m_hooks.marksChanged();
    }

    void GasStorageWindow::mark(int slot) {
        if (slot < 1 || slot > kCells || m_inventory.bagItem(slot).empty()) {
            return;
        }
        if (m_inventory.bagItem(slot).itemId != kGasItem) {
            m_messages.show("This item cannot be stored", kNoticeMs);
            return;
        }
        if (const auto notice = m_marks.lockNotice(slot, layoutOf(m_tank).mark)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(slot)) {
            m_messages.show("Unable to operate when the item is locked", kNoticeMs);
            return;
        }
        if (m_marks.useMark(slot) != 0) {
            return;
        }
        m_marks.setUseMark(slot, layoutOf(m_tank).mark);
        m_marked[static_cast<std::size_t>(slot)] = true;
        m_hooks.marksChanged();
    }

    void GasStorageWindow::unmark(int slot) {
        if (slot < 1 || slot > kCells || m_marks.useMark(slot) != layoutOf(m_tank).mark) {
            return;
        }
        m_marks.setUseMark(slot, 0);
        m_marked[static_cast<std::size_t>(slot)] = false;
        m_hooks.marksChanged();
    }

    void GasStorageWindow::fill() {
        if (m_fuel >= layoutOf(m_tank).capacity) {
            unmarkAll();
            m_messages.show("Gas capacity is full", kNoticeMs);
            return;
        }
        std::vector<int> slots;
        for (int slot = 1; slot <= kCells; ++slot) {
            if (m_marked[static_cast<std::size_t>(slot)]) {
                slots.push_back(slot);
            }
        }
        if (slots.empty()) {
            m_messages.show("Choose the items that you want to deposit on the left", kNoticeMs);
            return;
        }
        m_hooks.send(protocol::gasDepositRequest(m_tank == Tank::Small, slots));
    }

    void GasStorageWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        m_cells.clearCells();
        for (int slot = 1; slot <= kCells; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_cells.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
    }

    void GasStorageWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const Layout& layout = layoutOf(m_tank);
        const gfx::TextStyle style{m_fuelSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const std::string fuel = std::to_string(m_fuel);
        // the fuel text's x is its right edge, 8 = digit width
        canvas.drawText(
            {at.x + m_fuelSpot.at().x - (static_cast<int>(fuel.size()) - 1) * 8, at.y + m_fuelSpot.at().y},
            fuel,
            style,
            m_fuelSpot.size().width,
            m_fuelSpot.size().height
        );
        canvas.drawText({at.x + m_slashSpot.at().x, at.y + m_slashSpot.at().y}, "/", style, m_slashSpot.size().width, m_slashSpot.size().height);
        canvas.drawText(
            {at.x + m_capacitySpot.at().x, at.y + m_capacitySpot.at().y},
            std::to_string(layout.capacity),
            style,
            m_capacitySpot.size().width,
            m_capacitySpot.size().height
        );
        // fuel units per picture pixel: 60 for the Big tank, 3.33 for the Small one
        const int full = m_tank == Tank::Big ? m_fuel / 60 : static_cast<int>(std::ceil(m_fuel / 3.33));
        const int top = std::clamp(layout.pictureHeight - full, 0, layout.pictureHeight);
        if (const auto picture = m_services.pictures.find(layout.picture); picture && top < layout.pictureHeight) {
            canvas
                .drawPicture(*picture, {at.x + layout.pictureLeft, at.y + layout.pictureTop + top}, core::Rect{0, top, kTankWidth, layout.pictureHeight - top});
        }
    }

}
