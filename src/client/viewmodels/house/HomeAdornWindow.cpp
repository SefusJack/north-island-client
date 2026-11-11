#include "client/viewmodels/house/HomeAdornWindow.h"

#include "client/models/house/HouseGrid.h"
#include "protocol/PillarRequests.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kPictureDrop = 15; // the carried furniture picture is drawn this many px below its cell origin

        /// @brief Loads HomeAdornWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("HomeAdornWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("HomeAdornWindow");
        }

    }

    HomeAdornWindow::HomeAdornWindow(
        GameServices& services,
        ui::Screen& screen,
        HousePillars& pillars,
        Hooks hooks
    )
        : m_screen(screen),
          m_pillars(pillars),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)) {
        // startEdit modes: 1 add, 2 remove, 3 move (editMode values)
        screen.on("onHomeAdornAdd", [this](const ui::EventArgs&) { m_pillars.startEdit(1, m_hooks.floor()); });
        screen.on("onHomeAdornMove", [this](const ui::EventArgs&) { m_pillars.startEdit(3, m_hooks.floor()); }); // mode 3: move
        screen.on("onHomeAdornRemove", [this](const ui::EventArgs&) { m_pillars.startEdit(2, m_hooks.floor()); }); // mode 2: remove
        screen.on("onHomeAdornClose", [this](const ui::EventArgs&) { close(); });
    }

    void HomeAdornWindow::toggle(int furnIndex) {
        m_furnIndex = furnIndex;
        if (m_window.isVisible()) {
            close();
        } else {
            show();
        }
    }

    void HomeAdornWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_pillars.startEdit(1, m_hooks.floor()); // edit mode 1: add
    }

    void HomeAdornWindow::hide() { m_window.setVisible(false); }

    void HomeAdornWindow::close() {
        if (m_pillars.editMode() == 0) {
            return;
        }
        const int floor = m_hooks.floor();
        if (!m_pillars.stopEdit(floor)) {
            hide();
            return;
        }
        m_hooks.ask(
            "Are you sure you want to change? ",
            [this, floor] {
                if (const core::Bytes body = m_pillars.changeBody(m_furnIndex, floor); !body.empty()) {
                    m_hooks.send(protocol::pillarChangesRequest(body));
                }
                hide();
            },
            [this, floor] {
                m_pillars.revert(floor);
                hide();
            }
        );
    }

    core::Point HomeAdornWindow::cellAt(core::Point mapPoint) {
        const HouseGrid::Cell cell = HouseGrid::screenToCell(mapPoint);
        const core::Point at{cell.x - HouseGrid::kGridColumn + 1, cell.y - HouseGrid::kGridRow + 1};
        if (at.x < 1 || at.x > HousePillars::kColumns || at.y < 1 || at.y > HousePillars::kRows) {
            return {};
        }
        return at;
    }

    bool HomeAdornWindow::press(core::Point mouse, const Camera& camera) {
        if (!m_hooks.inFloorRoom() || m_pillars.editMode() == 0) {
            return false;
        }
        const int floor = m_hooks.floor();
        switch (m_pillars.editMode()) {
            case 1: // edit mode 1: add a pillar in the clicked cell
                if (const core::Point cell = cellAt(camera.toMap(mouse)); cell != core::Point{} && !m_pillars.has(cell.x, cell.y, floor)) {
                    m_pillars.add(cell.x, cell.y, floor);
                }
                break;
            case 2: // edit mode 2: remove the lit pillar
                if (const core::Point lit = m_pillars.lit(); lit != core::Point{}) {
                    m_pillars.remove(lit.x, lit.y, floor);
                }
                break;
            case 3: // edit mode 3: pick up the lit pillar to move it
                if (const core::Point lit = m_pillars.lit(); lit != core::Point{}) {
                    m_pillars.setCarried(lit);
                    m_pillars.setCarriedTo(lit);
                    m_grabMouse = mouse;
                }
                break;
            default: break; // other modes: nothing to do
        }
        return true;
    }

    core::Point HomeAdornWindow::carriedCell(core::Point mouse) const {
        const core::Point home = HousePillars::worldOf(m_pillars.carried().x, m_pillars.carried().y);
        return cellAt({home.x + (mouse.x - m_grabMouse.x), home.y + kPictureDrop + (mouse.y - m_grabMouse.y)});
    }

    void HomeAdornWindow::drag(core::Point mouse) {
        if (m_pillars.carried() != core::Point{}) {
            m_pillars.setCarriedTo(carriedCell(mouse));
        }
    }

    void HomeAdornWindow::release(core::Point mouse) {
        const core::Point from = m_pillars.carried();
        if (from == core::Point{}) {
            return;
        }
        const core::Point to = carriedCell(mouse);
        m_pillars.setCarried({});
        m_pillars.setCarriedTo({});
        if (m_hooks.locked() || to == core::Point{} || to == from) {
            return;
        }
        m_pillars.move(from.x, from.y, to.x, to.y, m_hooks.floor());
    }

    void HomeAdornWindow::update() {
        if (m_window.isVisible() && !m_hooks.inFloorRoom()) {
            m_pillars.stopEdit(m_hooks.floor());
            hide();
        }
    }

}
