#include "client/viewmodels/house/CarCaseWindow.h"

#include "client/rendering/painters/VehicleBoxPainter.h"
#include "protocol/GarageRequests.h"

namespace client {

    namespace {

        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, Hot-air Balloon, ...)
        const char* const kFull = "It's full";
        const char* const kLocked = "Unable to operate when the item is locked";

        /// @brief Loads CarCaseWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded CarCaseWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CarCaseWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CarCaseWindow");
        }

        /// @brief Gives the control name of a cell's drawing area.
        /// @param cell Cell number (1..4).
        /// @return The area name.
        std::string areaName(int cell) { return "CarCaseCell" + std::to_string(cell); }

    }

    CarCaseWindow::CarCaseWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        ShowPlatformWindow& platform,
        std::function<void(const core::Bytes&)> send
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_platform(platform),
          m_send(std::move(send)),
          m_window(loadWindow(services, screen)),
          m_areas{
              screen.get<ui::DrawingArea>("CarCasePage"),
              screen.get<ui::DrawingArea>(areaName(1)),
              screen.get<ui::DrawingArea>(areaName(2)),
              screen.get<ui::DrawingArea>(areaName(3)),
              screen.get<ui::DrawingArea>(areaName(4))
          },
          m_restrainSpot(screen.get<ui::Spot>("CarCaseRestrain")),
          m_restrainPicture(services.pictures.find("panel_restrainCarCase_1").value_or(gfx::PictureId{})) {
        m_areas[0].get().setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        for (int cell = 1; cell <= kCells; ++cell) {
            m_areas[static_cast<std::size_t>(cell)].get().setPainter([this, cell](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                const protocol::ItemInstance& car = m_images[static_cast<std::size_t>(cell)];
                if (!car.empty()) {
                    VehicleBoxPainter::paint(m_services, canvas, at, car);
                }
            });
        }
        screen.on("onCarCaseClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onCarCasePress", [this](const ui::EventArgs& args) { m_pressedCell = args.source.back() - '0'; });
        screen.on("onCarCaseTake", [this](const ui::EventArgs& args) {
            m_pressedCell = args.source.back() - '0';
            take(m_pressedCell);
        });
    }

    bool CarCaseWindow::pieceIsGarage() const {
        const int slot = m_house.utensils().slotOfPiece(m_furnIndex);
        return m_furnIndex != 0 && slot != 0 && m_house.utensils().utensil(slot).itemId == FurnitureCarCases::kGarageItem;
    }

    void CarCaseWindow::request(int furnIndex) {
        m_furnIndex = furnIndex;
        m_send(protocol::openGarageRequest(furnIndex)); // 62/51: open the Garage
    }

    void CarCaseWindow::shown(int restrained) {
        m_restrained = restrained;
        if (!pieceIsGarage()) {
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void CarCaseWindow::parked(const protocol::FurnitureCupboardItem& stored) {
        if (pieceIsGarage() && stored.place >= 1 && stored.place <= kCells) {
            m_images[static_cast<std::size_t>(stored.place)] = stored.item;
        }
    }

    void CarCaseWindow::left(int cell) {
        if (cell >= 1 && cell <= kCells) {
            m_images[static_cast<std::size_t>(cell)] = protocol::ItemInstance{};
        }
    }

    void CarCaseWindow::close() {
        m_window.setVisible(false);
        m_restrained = 0;
    }

    void CarCaseWindow::put(int bagSlot, int cell) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        if (m_house.carCases().emptyNumber() == 0) {
            m_messages.show(kFull, 2000); // message display time in ms
            return;
        }
        const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
        if (!record || record->type != kVehicleType) {
            m_messages.show("Only vehicles can be put in the garage", 2000); // message display time in ms
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show(kLocked, 1000); // message display time in ms
            return;
        }
        int target = cell;
        if (!m_house.carCases().keptUnder(cell).empty()) {
            target = m_house.carCases().emptyNumber();
            if (target == 0) {
                m_messages.show(kFull, 2000); // message display time in ms
                return;
            }
            // cells 3 and 4 only give back while the Garage is restrained (state 1)
            if ((target == 3 || target == 4) && m_restrained == 1) {
                return;
            }
            // cells 3 and 4 only give back while the Garage is restrained (state 1)
        } else if ((target == 3 || target == 4) && m_restrained == 1) {
            m_messages.show("Only withdrawal function can be used in this space", 2000); // message display time in ms
            return;
        }
        m_send(protocol::putInGarageRequest(m_furnIndex, bagSlot, target)); // 62/47: park the vehicle
    }

    void CarCaseWindow::take(int cell) {
        if (cell < 1 || cell > kCells || m_house.carCases().keptUnder(cell).empty()) {
            return;
        }
        if (m_inventory.freeSlotFor(m_house.carCases().keptUnder(cell).itemId) == 0) {
            m_messages.show(kFull, 2000); // message display time in ms
            return;
        }
        m_platform.garageTaking(cell);
        m_send(protocol::takeFromGarageRequest(m_furnIndex, cell)); // 62/48: take the vehicle back
    }

    void CarCaseWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_restrained == 1 && m_restrainPicture.isValid()) {
            canvas.drawPicture(m_restrainPicture, {at.x + m_restrainSpot.at().x, at.y + m_restrainSpot.at().y});
        }
    }

    void CarCaseWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!pieceIsGarage()) {
            close();
            return;
        }
        if (m_pressedCell != 0 && m_screen.hoveredId() != m_areas[static_cast<std::size_t>(m_pressedCell)].get().id()) {
            m_pressedCell = 0;
        }
        for (int cell = 1; cell <= kCells; ++cell) {
            const ui::DrawingArea& area = m_areas[static_cast<std::size_t>(cell)].get();
            const protocol::ItemInstance& car = m_images[static_cast<std::size_t>(cell)];
            if (m_screen.hoveredId() == area.id() && cell != m_pressedCell && !car.empty()) {
                const core::Point at = m_window.bounds().position() + area.bounds().position();
                m_info.show(car, {at.x, at.y, VehicleBoxPainter::kSide, VehicleBoxPainter::kSide});
            }
        }
    }

}
