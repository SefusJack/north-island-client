#include "client/viewmodels/travel/VoyageCompassWindow.h"

#include "core/Size.h"

#include <cmath>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // client screen width, px
        constexpr int kMarkerSlot = 4; // character action slot of the portrait used as the player marker
        constexpr int kFirstVoyageMap = 58001; // map 58001: Voyage Map
        constexpr int kSecondVoyageMap = 58002; // map 58002: The Mediterranean

        /// @brief Loads VoyageCompassWindow.xml into the screen and returns its window.
        /// @param services Game services (layout files, load context).
        /// @param screen Screen to load into.
        /// @return The loaded compass window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("VoyageCompassWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("VoyageCompassWindow");
        }

    }

    VoyageCompassWindow::VoyageCompassWindow(
        GameServices& services,
        ui::Screen& screen,
        const BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_minimize(screen.get<ui::ImageButton>("VoyageCompassMinimize")),
          m_placeSpot(screen.get<ui::Spot>("VoyageCompassPlace")),
          m_borderSpot(screen.get<ui::Spot>("VoyageCompassBorder")),
          m_foldedSpot(screen.get<ui::Spot>("VoyageCompassFolded")),
          m_buttonSpot(screen.get<ui::Spot>("VoyageCompassButton")) {
        screen.get<ui::DrawingArea>("VoyageCompassPaint").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t) { paint(canvas); });
        screen.on("onVoyageCompassClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onVoyageCompassMinimize", [this](const ui::EventArgs&) { toggle(); });
    }

    void VoyageCompassWindow::close() {
        m_window.setVisible(false);
        m_mode = 0;
        m_slot = 0;
        m_mapId = 0;
        m_scaleX = 0;
        m_scaleY = 0;
    }

    void VoyageCompassWindow::use(int bagSlot) {
        if (m_window.isVisible() && m_mode == 1) { // mode 1: open
            return;
        }
        if (m_marks.lockNotice(bagSlot, 0)) {
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 2000); // message display time in ms
            return;
        }
        const int mapId = m_hooks.mapId();
        if (mapId != kFirstVoyageMap && mapId != kSecondVoyageMap) {
            m_messages.show("The Compass can only be used in Voyage Map", 1500); // message display time in ms
            return;
        }
        m_slot = bagSlot;
        m_mapId = mapId;
        m_window.setPicture(std::to_string(mapId) + "_Mini_Form", m_services.pictures);
        const core::Rect size = m_window.bounds();
        if (size.width == 0) {
            close();
            return;
        }
        m_window.setPosition({kScreenWidth - size.width - m_placeSpot.at().x, m_buttonTop + m_placeSpot.at().y});
        const core::Point map = m_hooks.mapSize();
        m_scaleX = static_cast<double>(map.x) / (size.width - m_borderSpot.at().x);
        m_scaleY = static_cast<double>(map.y) / (size.height - m_borderSpot.at().x);
        m_minimize.setPicture(m_services.pictures.find("Btn_Minimize_1").value_or(gfx::PictureId{}));
        m_minimize.setBounds({size.width - m_buttonSpot.size().width, 0, m_buttonSpot.size().width, m_buttonSpot.size().height});
        m_buttonTop = 0;
        m_mode = 1; // mode 1: open
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void VoyageCompassWindow::toggle() {
        if (m_mode == 1) { // mode 1: open
            const core::Rect form = m_window.bounds();
            const core::Rect button = m_minimize.bounds();
            m_window.setPicture("Form_HotKey_Mini", m_services.pictures);
            m_window.setBounds(
                {form.x + button.x - (m_foldedSpot.rect().width - button.width) / 2,
                 form.y + button.y - (m_foldedSpot.rect().height - button.height) / 2,
                 m_foldedSpot.rect().width,
                 m_foldedSpot.rect().height}
            );
            m_minimize.setPicture(m_services.pictures.find("Btn_Window_1").value_or(gfx::PictureId{}));
            m_minimize.setBounds({m_foldedSpot.at().x, m_foldedSpot.at().x, m_buttonSpot.size().width, m_buttonSpot.size().height});
            m_buttonTop = m_foldedSpot.at().x;
            m_mode = 2; // mode 2: folded
        } else if (m_mode == 2) { // mode 2: folded
            use(m_slot);
        }
    }

    void VoyageCompassWindow::paint(gfx::Canvas& canvas) {
        if (m_slot < 1 || m_slot > BagMarks::kSlots || m_hooks.bagItem(m_slot) != kCompassItem || m_hooks.mapId() != m_mapId) {
            close();
            return;
        }
        if (m_mode != 1) { // mode 1: open
            return;
        }
        const core::Rect form = m_window.bounds();
        const core::Point position = m_hooks.playerPosition();
        const core::Point at{
            static_cast<int>(std::floor(position.x / m_scaleX + 0.5)) + form.x + m_borderSpot.at().y,
            static_cast<int>(std::floor(position.y / m_scaleY + 0.5)) + form.y + m_borderSpot.at().y
        };
        m_services.characters.drawPortrait(canvas, m_hooks.ownLook(), at, 0, kMarkerSlot);
    }

}
