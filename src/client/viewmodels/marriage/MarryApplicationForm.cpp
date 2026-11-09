#include "client/viewmodels/marriage/MarryApplicationForm.h"

#include "client/rendering/painters/CouplePainter.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        /// @brief Loads MarryApplicationForm.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "MarryApplicationForm" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MarryApplicationForm.xml"), services.loadContext());
            return screen.get<ui::Window>("MarryApplicationForm");
        }

    }

    MarryApplicationForm::MarryApplicationForm(
        GameServices& services,
        ui::Screen& screen,
        Marriage& marriage
    )
        : m_services(services),
          m_screen(screen),
          m_marriage(marriage),
          m_window(loadWindow(services, screen)),
          m_marry(screen.get<ui::ImageButton>("MarryApplicationMarry")),
          m_waiting(screen.get<ui::Element>("MarryApplicationWaiting")) {
        screen.get<ui::DrawingArea>("MarryApplicationPeople").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            m_marry.setVisible(!m_marriage.accepted());
            m_waiting.setVisible(m_marriage.accepted());
            paintCouple(canvas, m_services, at, m_marriage.applicant(), m_marriage.partner());
        });
        screen.on("onMarryApplicationAccept", [this](const ui::EventArgs&) { m_marriage.acceptMarriage(); });
        screen.on("onMarryApplicationCancel", [this](const ui::EventArgs&) { m_marriage.cancelMarriage(); });
    }

    void MarryApplicationForm::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

}
