#include "client/viewmodels/system/ScrolledRows.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kScrollStep = 8; // pixels the rows move per scroll step
        constexpr int kPageSteps = 31; // steps visible at once (scroll page size)
        constexpr int kWheelSteps = 2; // scroll steps per mouse wheel notch

    }

    ScrolledRows::ScrolledRows(
        ui::Screen& screen,
        const std::string& rows,
        const std::string& wheel,
        const std::string& bar
    )
        : m_rows(screen.get<ui::Element>(rows)),
          m_wheel(screen.get<ui::Element>(wheel)),
          m_bar(screen.get<ui::ScrollBar>(bar)) {}

    void ScrolledRows::setContentHeight(int height) {
        m_steps = std::max(kPageSteps, (height + kScrollStep - 1) / kScrollStep);
        m_bar.setRange(m_steps, kPageSteps);
        m_bar.setVisible(m_steps > kPageSteps);
        const int tall = m_steps * kScrollStep;
        const core::Rect rows = m_rows.bounds();
        m_rows.setBounds({rows.x, rows.y, rows.width, tall});
        const core::Rect area = m_wheel.bounds();
        m_wheel.setBounds({area.x, area.y, area.width, tall});
        scrollTo(m_bar.position());
    }

    void ScrolledRows::scrollTo(int position) {
        position = std::clamp(position, 0, m_steps - kPageSteps);
        m_bar.setPosition(position);
        m_rows.setPosition({0, -position * kScrollStep});
    }

    void ScrolledRows::wheel(int notches) { scrollTo(m_bar.position() - notches * kWheelSteps); }

}
