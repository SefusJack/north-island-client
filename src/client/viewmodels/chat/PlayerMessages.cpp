#include "client/viewmodels/chat/PlayerMessages.h"

#include "core/Log.h"

#include <algorithm>

namespace client {

    PlayerMessages::PlayerMessages(
        GameServices& services,
        ui::Screen& screen,
        const std::string& layout
    )
        : m_services(services),
          m_screen(screen),
          m_panel(dynamic_cast<ui::MessagePanel&>(screen.load(services.game.layout(layout), services.loadContext()))),
          m_centre(layout == "CentreMessages.xml") {}

    void PlayerMessages::show(const std::string& text, int durationMs) {
        core::logInfo("message: ", text);
        if (m_observer) {
            m_observer(text);
        }
        if (m_services.partyMessage) {
            m_services.partyMessage(m_centre, text, durationMs);
            return;
        }
        const std::uint64_t nowMs = m_screen.context().nowMs;
        if (m_grouping && m_panel.isVisible() && nowMs - m_shownMs <= kGroupMs) {
            if (std::find(m_group.begin(), m_group.end(), text) != m_group.end()) {
                return;
            }
            m_panel.append(text, durationMs, nowMs, kGroupedLines);
        } else {
            m_panel.show(text, durationMs, nowMs);
            m_group.clear();
        }
        m_group.push_back(text);
        m_shownMs = nowMs;
        m_screen.context().bringToFront(m_panel.id());
    }

    void PlayerMessages::showGrouped(const std::string& text, int durationMs) {
        m_grouping = true;
        show(text, durationMs);
    }

    void PlayerMessages::keepBelow(int minTop) { m_panel.keepBelow(minTop); }

}
