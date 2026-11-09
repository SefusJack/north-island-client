#include "client/viewmodels/overlays/RoleIntroWindow.h"

#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kOverlayBody = 4; // body type that gets the extra BRole_4_3 overlay
        constexpr int kOverlayHead = 3; // head that gets the extra BRole_4_3 overlay

        /// @brief Loads RoleIntroWindow.xml into the screen and returns its layer.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "RoleIntroLayer" element.
        ui::Element& loadLayer(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("RoleIntroWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("RoleIntroLayer");
        }

    }

    RoleIntroWindow::RoleIntroWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_layer(loadLayer(services, screen)),
          m_overlaySpot(screen.get<ui::Spot>("RoleIntroOverlay")) {
        screen.get<ui::DrawingArea>("RoleIntroArea").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t) { paint(canvas); });
        screen.on("onRoleIntroBack", [this](const ui::EventArgs&) { close(); });
        screen.on("onRoleIntroRightUp", [this](const ui::EventArgs&) { close(); });
    }

    bool RoleIntroWindow::isOpen() const { return m_layer.isVisible(); }

    void RoleIntroWindow::open(const std::string& picture) {
        const int body = m_hooks.bodyType();
        const int head = m_hooks.head();
        m_picture = picture.empty() ? "JRole_" + std::to_string(body) + "_" + std::to_string(head) : picture;
        m_overlay = picture.empty() && body == kOverlayBody && head == kOverlayHead ? "BRole_4_3" : "";
        m_layer.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_layer.id());
        }
    }

    void RoleIntroWindow::close() {
        if (!m_layer.isVisible()) {
            return;
        }
        m_layer.setVisible(false);
        m_hooks.closed();
    }

    void RoleIntroWindow::paint(gfx::Canvas& canvas) {
        if (const auto jpg = m_services.pictures.find(m_picture)) {
            canvas.drawPicture(*jpg, {0, 0});
        }
        if (!m_overlay.empty()) {
            if (const auto overlay = m_services.pictures.find(m_overlay)) {
                canvas.drawPicture(*overlay, m_overlaySpot.at());
            }
        }
    }

}
