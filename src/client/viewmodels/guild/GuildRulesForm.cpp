#include "client/viewmodels/guild/GuildRulesForm.h"

#include "protocol/GuildRequests.h"
#include "ui/ScrollBar.h"

namespace client {

    namespace {

        constexpr std::size_t kLongestRules = 200; // longest rules text, characters

        /// @brief Loads GuildRulesForm.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded GuildRulesForm window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GuildRulesForm.xml"), services.loadContext());
            return screen.get<ui::Window>("GuildRulesForm");
        }

    }

    GuildRulesForm::GuildRulesForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("GuildRulesText")),
          m_tabSpot(screen.get<ui::Spot>("GuildRulesFromTab")) {
        screen.get<ui::ScrollBar>("GuildRulesFormScroll").setRange(1, 1);
        screen.on("onGuildRulesOk", [this](const ui::EventArgs&) { save(); });
        screen.on("onGuildRulesCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onGuildRulesFormUp", [this](const ui::EventArgs&) { m_text.pageUp(); });
        screen.on("onGuildRulesFormDown", [this](const ui::EventArgs&) { m_text.pageDown(); });
        screen.on("onGuildRulesTyped", [this](const ui::EventArgs&) {
            if (m_text.text().size() >= kLongestRules) {
                m_messages.show("Sorry. Rules are too long.", 1200); // message display time in ms
            }
        });
    }

    void GuildRulesForm::toggle(core::Point guildTab, const std::string& rules) {
        if (m_window.isVisible()) {
            close();
            return;
        }
        const core::Rect bounds = m_window.bounds();
        m_window.setBounds({guildTab.x + m_tabSpot.at().x, guildTab.y + m_tabSpot.at().y, bounds.width, bounds.height});
        m_text.clear();
        m_text.appendLine(rules);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void GuildRulesForm::save() {
        if (m_text.text().empty()) {
            return;
        }
        close();
        m_services.connection.send(protocol::guildRulesRequest(m_text.text()));
    }

}
