#include "client/viewmodels/marriage/MarryDivorceForm.h"

#include "client/rendering/painters/CouplePainter.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        /// @brief Loads MarryDivorceForm.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "MarryDivorceForm" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MarryDivorceForm.xml"), services.loadContext());
            return screen.get<ui::Window>("MarryDivorceForm");
        }

    }

    MarryDivorceForm::MarryDivorceForm(
        GameServices& services,
        ui::Screen& screen,
        Marriage& marriage
    )
        : m_services(services),
          m_screen(screen),
          m_marriage(marriage),
          m_window(loadWindow(services, screen)),
          m_divorce(screen.get<ui::ImageButton>("MarryDivorceDivorce")),
          m_waiting(screen.get<ui::Element>("MarryDivorceWaiting")),
          m_headingSpot(screen.get<ui::Spot>("MarryDivorceHeading")) {
        screen.get<ui::DrawingArea>("MarryDivorcePeople").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            m_divorce.setVisible(!m_marriage.accepted());
            m_waiting.setVisible(m_marriage.accepted());
            if (const auto heading = m_services.pictures.find("Form_Divorce")) {
                canvas.drawPicture(*heading, at + m_headingSpot.at(), core::Rect{0, 0, m_headingSpot.size().width, m_headingSpot.size().height});
            }
            paintCouple(canvas, m_services, at, m_marriage.applicant(), m_marriage.partner());
        });
        screen.on("onMarryDivorceAccept", [this](const ui::EventArgs&) { m_marriage.acceptDivorce(); });
        screen.on("onMarryDivorceCancel", [this](const ui::EventArgs&) { m_marriage.cancelDivorce(); });
    }

    void MarryDivorceForm::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

}
