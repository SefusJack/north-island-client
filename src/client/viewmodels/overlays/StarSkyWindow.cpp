#include "client/viewmodels/overlays/StarSkyWindow.h"

#include <algorithm>
#include <cmath>

#include "client/data/StarIntroductions.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        /// @brief Loads StarSkyWindow.xml into the screen and returns its layer.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "StarSkyLayer" element.
        ui::Element& loadLayer(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("StarSkyWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("StarSkyLayer");
        }

    }

    StarSkyWindow::StarSkyWindow(
        GameServices& services,
        ui::Screen& screen,
        StarSky& sky,
        RoleIntroWindow& intro
    )
        : m_screen(screen),
          m_sky(sky),
          m_intro(intro),
          m_layer(loadLayer(services, screen)),
          m_quit(screen.get<ui::ImageButton>("StarSkyQuit")) {
        screen.get<ui::DrawingArea>("StarSkyArea").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) {
            // the sky is made for the game's first screen size: in a larger window it is drawn larger about its middle until it
            // covers the window, and the mouse is taken back to where it points in the sky as made
            constexpr float kWidth = 800.0f; // the sky as made, px
            constexpr float kHeight = 600.0f;
            const core::Rect& view = m_screen.view();
            const float scale = std::max(1.0f, std::max(static_cast<float>(view.width) / kWidth, static_cast<float>(view.height) / kHeight));
            const core::Point middle{static_cast<int>(kWidth) / 2, static_cast<int>(kHeight) / 2};
            const core::Point mouse = m_screen.mouse();
            const core::Point inSky{
                middle.x + static_cast<int>(std::lround(static_cast<float>(mouse.x - middle.x) / scale)),
                middle.y + static_cast<int>(std::lround(static_cast<float>(mouse.y - middle.y) / scale))
            };
            if (scale > 1.0f) {
                canvas.beginScale(middle, scale);
            }
            m_sky.paint(canvas, nowMs, inSky);
            if (scale > 1.0f) {
                canvas.endScale();
            }
        });
        screen.on("onStarSkyUp", [this](const ui::EventArgs&) { openIntroduction(); });
        screen.on("onStarSkyRightUp", [this](const ui::EventArgs&) { m_sky.rightReleased(); });
        screen.on("onStarSkyQuit", [this](const ui::EventArgs&) { m_sky.quit(); });
    }

    void StarSkyWindow::update() { m_layer.setVisible(m_sky.active()); }

    void StarSkyWindow::setQuitShown(bool shown) { m_quit.setVisible(shown); }

    void StarSkyWindow::openIntroduction() {
        if (m_sky.phase() != StarSky::Phase::Sky || m_sky.fadeState() != 0 || m_sky.roleInfoShown()) {
            return;
        }
        const int star = m_sky.hoverStar();
        if (star == StarSky::kOwnStar) {
            m_intro.open();
        } else if (const std::string picture = starIntroduction(star); !picture.empty()) {
            m_intro.open(picture);
        } else {
            return;
        }
        m_sky.setRoleInfoShown(true);
        setQuitShown(false);
    }

}
