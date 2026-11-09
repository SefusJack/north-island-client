#include "client/viewmodels/travel/ParkingWindow.h"

#include "client/rendering/painters/VehicleBoxPainter.h"
#include "client/data/VehicleKinds.h"
#include "protocol/VehicleRequests.h"

namespace client {

    namespace {

        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, ...)
        const char* const kFull = "It's full";
        const char* const kLocked = "Unable to operate when the item is locked";

        /// @brief Loads ParkingWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "ParkingWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ParkingWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ParkingWindow");
        }

        /// @brief Builds the element name of a parking place.
        /// @param cell Place (1..4).
        /// @return The name, e.g. "ParkingCell1".
        std::string areaName(int cell) { return "ParkingCell" + std::to_string(cell); }

    }

    ParkingWindow::ParkingWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        std::function<void(const core::Bytes&)> send
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_send(std::move(send)),
          m_window(loadWindow(services, screen)),
          m_areas{
              screen.get<ui::DrawingArea>("ParkingPage"),
              screen.get<ui::DrawingArea>(areaName(1)),
              screen.get<ui::DrawingArea>(areaName(2)),
              screen.get<ui::DrawingArea>(areaName(3)),
              screen.get<ui::DrawingArea>(areaName(4))
          },
          m_titleSpot(screen.get<ui::Spot>("ParkingTitle")),
          m_title(services.pictures.find("Form_Parkinglot").value_or(gfx::PictureId{})) {
        m_areas[0].get().setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        for (int cell = 1; cell <= kCells; ++cell) {
            m_areas[static_cast<std::size_t>(cell)].get().setPainter([this, cell](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                if (!m_cells[static_cast<std::size_t>(cell)].empty()) {
                    VehicleBoxPainter::paint(m_services, canvas, at, m_cells[static_cast<std::size_t>(cell)]);
                }
            });
        }
        screen.on("onParkingClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onParkingPress", [this](const ui::EventArgs& args) { m_pressedCell = args.source.back() - '0'; });
        screen.on("onParkingTake", [this](const ui::EventArgs& args) {
            m_pressedCell = args.source.back() - '0';
            redeem(m_pressedCell);
        });
    }

    void ParkingWindow::opened(int code) {
        if (code == 1) { // 23/157 code 1: show the form
            m_window.setVisible(true);
            if (m_screen.context().bringToFront) {
                m_screen.context().bringToFront(m_window.id());
            }
        } else if (code == 2) { // 23/157 code 2: the lot has no room
            m_messages.show("The space of parking lot is not enough", 1000); // message display time in ms
        }
    }

    void ParkingWindow::close() { m_window.setVisible(false); }

    void ParkingWindow::clear() { m_cells.fill(protocol::ItemInstance{}); }

    void ParkingWindow::setCell(const protocol::ParkedVehicle& cell) {
        if (cell.cell < 1 || cell.cell > kCells) {
            return;
        }
        protocol::ItemInstance& shown = m_cells[static_cast<std::size_t>(cell.cell)];
        shown = protocol::ItemInstance{};
        shown.itemId = cell.itemId;
        shown.count = 1;
        shown.usedAmount = cell.wear;
        shown.extraAmount = cell.fuel;
    }

    void ParkingWindow::setList(const std::vector<protocol::ParkedVehicle>& cells) {
        for (const protocol::ParkedVehicle& cell : cells) {
            setCell(cell);
        }
    }

    void ParkingWindow::result(int code, int itemId) {
        const auto record = m_services.items.find(itemId);
        const std::string name = record ? record->name : std::string();
        switch (code) {
            // result messages last 1000 ms
            case 1: m_messages.show(name + " Has been left in parking lot", 1000); break; // 23/155 code 1: vehicle parked
            case 2: m_messages.show("Parking lot used failed", 1000); break; // 23/155 code 2: parking failed
            case 3: // 23/155 code 3: vehicle redeemed (plays a sound)
                m_messages.show("Redeem from parking lot " + name + " Successfully", 1000); // message display time in ms
                m_services.playSound("sound/wav0152.wav");
                break;
            case 4: m_messages.show("The space of parking lot is not enough", 1000); break; // 23/155 code 4: lot has no room
            case 5: m_messages.show("Is using vehicle, the function can't be used", 1000); break; // 23/155 code 5: a vehicle is in use
            case 6: m_messages.show("Not enough points", 1000); break; // 23/155 code 6: not enough points
            default: break; // 23/155 other codes: ignored
        }
    }

    int ParkingWindow::freeCell() const {
        for (int cell = 1; cell <= kCells; ++cell) {
            if (m_cells[static_cast<std::size_t>(cell)].empty()) {
                return cell;
            }
        }
        return 0;
    }

    void ParkingWindow::put(int bagSlot, int cell) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const auto full = [this] {
            m_messages.show(kFull, 2000); // message display time in ms
            m_services.playSound("sound/wav0152.wav");
        };
        if (freeCell() == 0) {
            full();
            return;
        }
        const int itemId = m_inventory.bagItem(bagSlot).itemId;
        const auto record = m_services.items.find(itemId);
        if (!record || record->type != kVehicleType) {
            m_messages.show("Only vehicles can be put in the garage", 2000); // message display time in ms
            return;
        }
        if (VehicleKinds::isCapsule(itemId)) {
            m_messages.show("Unable to lay the item", 2000); // message display time in ms
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms); // message display time in ms
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show(kLocked, 1000); // message display time in ms
            return;
        }
        const int target = m_cells[static_cast<std::size_t>(cell)].empty() ? cell : freeCell();
        m_send(protocol::parkInLotRequest(bagSlot, target));
    }

    void ParkingWindow::redeem(int cell) {
        if (cell < 1 || cell > kCells || m_cells[static_cast<std::size_t>(cell)].empty()) {
            return;
        }
        if (m_inventory.freeSlotFor(m_cells[static_cast<std::size_t>(cell)].itemId) == 0) {
            m_messages.show(kFull, 2000); // message display time in ms
            return;
        }
        m_send(protocol::redeemFromLotRequest(cell));
    }

    void ParkingWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_title.isValid()) {
            canvas.drawPicture(
                m_title,
                {at.x + m_titleSpot.at().x, at.y + m_titleSpot.at().y},
                core::Rect{0, 0, m_titleSpot.rect().width, m_titleSpot.rect().height}
            );
        }
    }

    void ParkingWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (m_pressedCell != 0 && m_screen.hoveredId() != m_areas[static_cast<std::size_t>(m_pressedCell)].get().id()) {
            m_pressedCell = 0;
        }
        for (int cell = 1; cell <= kCells; ++cell) {
            const ui::DrawingArea& area = m_areas[static_cast<std::size_t>(cell)].get();
            if (m_screen.hoveredId() == area.id() && cell != m_pressedCell && !m_cells[static_cast<std::size_t>(cell)].empty()) {
                const core::Point at = m_window.bounds().position() + area.bounds().position();
                m_info.show(m_cells[static_cast<std::size_t>(cell)], {at.x, at.y, VehicleBoxPainter::kSide, VehicleBoxPainter::kSide});
            }
        }
    }

}
