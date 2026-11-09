#include "client/viewmodels/house/ShowPlatformWindow.h"

#include "client/rendering/painters/VehicleBoxPainter.h"
#include "protocol/ShowPlatformRequests.h"

namespace client {

    namespace {

        /// @brief Loads ShowPlatformWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ShowPlatformWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ShowPlatformWindow");
        }

        /// @brief Builds the element name of a garage cell's drawing area.
        /// @param cell Cell 1..4.
        /// @return The element name.
        std::string areaName(int cell) { return "ShowPlatformCell" + std::to_string(cell); }

    }

    ShowPlatformWindow::ShowPlatformWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        ItemInfoPopup& info,
        std::function<void(const core::Bytes&)> send
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_info(info),
          m_send(std::move(send)),
          m_window(loadWindow(services, screen)),
          m_areas{
              screen.get<ui::DrawingArea>("ShowPlatformPage"),
              screen.get<ui::DrawingArea>(areaName(1)),
              screen.get<ui::DrawingArea>(areaName(2)),
              screen.get<ui::DrawingArea>(areaName(3)),
              screen.get<ui::DrawingArea>(areaName(4))
          },
          m_restrainSpot(screen.get<ui::Spot>("ShowPlatformRestrain")),
          m_displaySpot(screen.get<ui::Spot>("ShowPlatformDisplay")),
          m_restrainPicture(services.pictures.find("panel_restrainCarCase_1").value_or(gfx::PictureId{})),
          m_displayPicture(services.pictures.find("Form_Display").value_or(gfx::PictureId{})) {
        m_areas[0].get().setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        for (int cell = 1; cell <= kCells; ++cell) {
            m_areas[static_cast<std::size_t>(cell)].get().setPainter([this, cell](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                const protocol::ItemInstance& car = m_images[static_cast<std::size_t>(cell)];
                if (!car.empty()) {
                    VehicleBoxPainter::paint(m_services, canvas, at, car);
                }
            });
        }
        screen.on("onShowPlatformClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onShowPlatformPress", [this](const ui::EventArgs& args) {
            m_pressedCell = args.source.back() - '0';
            choose(m_pressedCell);
        });
        screen.on("onShowPlatformRelease", [this](const ui::EventArgs& args) {
            m_pressedCell = args.source.back() - '0';
            release(m_pressedCell);
        });
    }

    bool ShowPlatformWindow::pieceIsPlatform() const {
        const int slot = m_house.utensils().slotOfPiece(m_furnIndex);
        return m_furnIndex != 0 && slot != 0 && m_house.utensils().utensil(slot).itemId == FurnitureCarCases::kShowPlatformItem;
    }

    void ShowPlatformWindow::request(int furnIndex) {
        m_furnIndex = furnIndex;
        m_send(protocol::openShowPlatformRequest(furnIndex));
    }

    void ShowPlatformWindow::opened(int restrained, bool garage) {
        m_restrained = restrained;
        if (!garage || !pieceIsPlatform()) {
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ShowPlatformWindow::close() {
        m_window.setVisible(false);
        m_restrained = 0;
    }

    void ShowPlatformWindow::parked(const protocol::FurnitureCupboardItem& stored) {
        if (pieceIsPlatform() && stored.place >= 1 && stored.place <= kCells) {
            m_images[static_cast<std::size_t>(stored.place)] = stored.item;
        }
    }

    void ShowPlatformWindow::garageTaking(int cell) {
        if (cell == m_exhibitSlot) {
            m_exhibitSlot = 0;
            m_send(protocol::takeOffPlatformRequest(cell));
        }
    }

    void ShowPlatformWindow::choose(int cell) {
        if (cell == 0 || m_house.carCases().keptUnder(cell).empty()) {
            return;
        }
        m_exhibitSlot = cell;
        m_send(protocol::showOnPlatformRequest(m_furnIndex, cell));
    }

    void ShowPlatformWindow::release(int cell) {
        if (m_exhibitSlot == cell) {
            m_exhibitSlot = 0;
            m_send(protocol::takeOffPlatformRequest(cell));
        }
    }

    void ShowPlatformWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_displayPicture.isValid()) {
            canvas.drawPicture(
                m_displayPicture,
                {at.x + m_displaySpot.at().x, at.y + m_displaySpot.at().y},
                core::Rect{0, 0, m_displaySpot.size().width, m_displaySpot.size().height}
            );
        }
        if (m_restrained == 1 && m_restrainPicture.isValid()) { // restrain state 1: the Garage is restrained
            canvas.drawPicture(m_restrainPicture, {at.x + m_restrainSpot.at().x, at.y + m_restrainSpot.at().y});
        }
    }

    void ShowPlatformWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!pieceIsPlatform()) {
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
