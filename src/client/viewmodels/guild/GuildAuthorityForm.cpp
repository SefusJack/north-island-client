#include "client/viewmodels/guild/GuildAuthorityForm.h"

#include "protocol/GuildRequests.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/TextBox.h"

namespace client {

    namespace {

        constexpr int kRights = 5;
        constexpr int kRightSetAuthority = 1;
        constexpr int kRightInvite = 2;
        constexpr int kRightEmblem = 5;
        const std::string kCheckPrefix = "GuildAuthorityCheck";

        /// @brief Loads GuildAuthorityForm.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "GuildAuthority" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GuildAuthorityForm.xml"), services.loadContext());
            return screen.get<ui::Window>("GuildAuthority");
        }

    }

    GuildAuthorityForm::GuildAuthorityForm(
        GameServices& services,
        ui::Screen& screen,
        Guild& guild,
        std::function<std::int32_t()> ownId
    )
        : m_services(services),
          m_screen(screen),
          m_guild(guild),
          m_ownId(std::move(ownId)),
          m_window(loadWindow(services, screen)) {
        screen.get<ui::DrawingArea>("GuildAuthorityUpdate").setPainter([this](gfx::Canvas&, core::Point, std::uint64_t) {
            m_screen.get<ui::Element>("GuildAuthorityVice").setVisible(m_guild.checkedCount() == 1);
        });
        screen.on("onGuildAuthorityOk", [this](const ui::EventArgs&) { apply(); });
        screen.on("onGuildAuthorityClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onGuildAuthorityVice", [this](const ui::EventArgs&) { toggleVice(); });
        screen.on("onGuildAuthorityCheck", [this](const ui::EventArgs& args) { toggleRight(std::stoi(args.source.substr(kCheckPrefix.size()))); });
    }

    void GuildAuthorityForm::toggle(int selection) {
        if (m_window.isVisible()) {
            close();
        } else {
            open(selection);
        }
    }

    void GuildAuthorityForm::showCheck(int right, bool checked) {
        m_checks[static_cast<std::size_t>(right - 1)] = checked;
        m_screen.get<ui::ImageButton>(kCheckPrefix + std::to_string(right))
            .setPicture(m_services.pictures.find(checked ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}));
    }

    void GuildAuthorityForm::showVice(bool checked) {
        m_viceChecked = checked;
        m_screen.get<ui::ImageButton>("GuildAuthorityVice")
            .setPicture(m_services.pictures.find(checked ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}));
    }

    void GuildAuthorityForm::open(int selection) {
        ui::TextBox& name = m_screen.get<ui::TextBox>("GuildAuthorityName");
        name.setText("");
        for (int right = 1; right <= kRights; ++right) {
            showCheck(right, false);
        }
        showVice(false);
        if (selection > 0) {
            const int member = selection - 1;
            const protocol::GuildMemberRecord& target = m_guild.at(member).record;
            name.setText(target.name);
            showVice(member >= 1 && member <= m_guild.viceCount());
            const int own = m_guild.indexOf(m_ownId());
            if (own >= 0) {
                const protocol::GuildMemberRecord& mine = m_guild.at(own).record;
                showCheck(kRightSetAuthority, m_guild.isLord(m_ownId()) && target.rights[kRightSetAuthority - 1]);
                // rights 3 and 4 are the third and fourth rights
                for (int right : {kRightInvite, 3, 4, kRightEmblem}) {
                    const auto index = static_cast<std::size_t>(right - 1);
                    showCheck(right, mine.rights[index] && target.rights[index]);
                }
            }
        }
        if (selection > 0) {
            const int own = m_guild.indexOf(m_ownId());
            const bool emblem = own >= 0 && m_guild.at(own).record.rights[kRightEmblem - 1];
            m_screen.get<ui::Element>(kCheckPrefix + std::to_string(kRightInvite)).setVisible(m_guild.mayInvite(m_ownId()));
            m_screen.get<ui::Element>(kCheckPrefix + std::to_string(kRightEmblem)).setVisible(emblem);
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void GuildAuthorityForm::toggleVice() {
        if (m_guild.checkedCount() >= 2) {
            showVice(false);
        } else if (m_guild.checkedCount() == 1 && m_guild.isLord(m_ownId())) {
            showVice(!m_viceChecked);
        }
    }

    void GuildAuthorityForm::toggleRight(int right) {
        const int own = m_guild.indexOf(m_ownId());
        if (own >= 0) {
            if (right == kRightSetAuthority && !m_guild.isLord(m_ownId())) {
                return;
            }
            if (right > kRightSetAuthority && !m_guild.at(own).record.rights[static_cast<std::size_t>(right - 1)]) {
                return;
            }
        }
        showCheck(right, !m_checks[static_cast<std::size_t>(right - 1)]);
    }

    void GuildAuthorityForm::apply() {
        int selected = 0;
        int selectedIndex = -1;
        for (int i = 0; i < m_guild.count(); ++i) {
            GuildMember& member = m_guild.at(i);
            if (!member.checked) {
                continue;
            }
            ++selected;
            selectedIndex = i;
            m_services.connection.send(protocol::guildRightsRequest(member.record.id, m_checks));
            member.checked = false;
        }
        if (selected == 1 && m_guild.isLord(m_ownId())) {
            const std::int32_t id = m_guild.at(selectedIndex).record.id;
            const bool isVice = selectedIndex >= 1 && selectedIndex <= m_guild.viceCount();
            if (m_viceChecked && !isVice) {
                m_services.connection.send(protocol::viceOfferAnswerRequest(id, 1)); // 39/14 with answer 1: appoint the vice lord
            } else if (!m_viceChecked && isVice) {
                m_services.connection.send(protocol::deposeViceRequest(id));
            }
        }
        close();
    }

}
