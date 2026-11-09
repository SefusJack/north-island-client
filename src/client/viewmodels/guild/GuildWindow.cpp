#include "client/viewmodels/guild/GuildWindow.h"

#include "client/models/character/CharacterLook.h"
#include "client/rendering/painters/EmblemPainter.h"
#include "client/rendering/hud/GameNumbers.h"
#include "client/rendering/hud/PageNumbers.h"
#include "core/Log.h"
#include "gfx/TextStyle.h"
#include "protocol/GuildRequests.h"
#include "ui/ScrollBar.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        constexpr int kRowsPerPage = 5; // members per page
        constexpr int kFaceSlot = 1; // portrait slot used for drawing the face
        constexpr int kRefusedTalkMs = 5000; // answer timeout of the talk box, ms
        constexpr int kNoticeMs = 2000; // notice display time, ms
        // online state icon per state 0..3: online, busy, hidden, offline
        const std::array<const char*, 4> kStateIcons = {"Icon_online_1", "Icon_Busy_1", "Icon_inline_1", "icon_offline_1"};
        // job icon picture per job number 1..6 (index 0 unused): Killer, Warrior, Knight, Wit, Priest, Seer
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};
        const std::array<const char*, 7> kJobNames = {"", "Killer ", "Warrior ", "Knight ", "Wit ", "Priest ", "Seer "}; // job names per job number 1..6
        constexpr int kOffline = 3; // online state 3: offline

        /// @brief Tells whether a job number has an icon (1..6).
        /// @param job Job number.
        /// @return True for a known job.
        bool validJob(int job) { return job >= 1 && job < static_cast<int>(kJobIcons.size()); }

        /// @brief Loads GuildWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded GuildWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GuildWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("GuildWindow");
        }

        /// @brief Reads the number that follows a name prefix.
        /// @param name Control name such as "GuildCheck3".
        /// @param prefix Expected name prefix.
        /// @return The number after the prefix, 0 when the name does not start with it.
        int numberAfter(const std::string& name, const std::string& prefix) { return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0; }

        /// @brief Gives the background picture name of a page.
        /// @param page Page number (0 info, 1 members, 2 insignia, 3 message).
        /// @return Picture name of the page.
        std::string pageName(int page) {
            switch (page) {
                case 0: return "Form_ArmyGrounpInfo_1"; // info page picture
                case 1: return "Form_ArmyGrounp_1"; // member page picture
                case 2: return "Form_Emblem_1"; // insignia (badge) page picture
                default: return "Form_OrgNotice"; // message (notice board) page picture
            }
        }

    }

    GuildWindow::GuildWindow(
        GameServices& services,
        ui::Screen& screen,
        Guild& guild,
        PlayerMessages& messages,
        TalkDialog& talk,
        NoticeBoard& board,
        NoticeReadForm& read,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_guild(guild),
          m_messages(messages),
          m_talk(talk),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_rules(screen.get<ui::Memo>("GuildRules")),
          m_badgeSpot(screen.get<ui::Spot>("GuildBadge")),
          m_rowSpot(screen.get<ui::Spot>("GuildMemberRow")),
          m_faceSpot(screen.get<ui::Spot>("GuildMemberFace")),
          m_levelSpot(screen.get<ui::Spot>("GuildMemberLevel")),
          m_careerSpot(screen.get<ui::Spot>("GuildMemberCareer")),
          m_elementSpot(screen.get<ui::Spot>("GuildMemberElement")),
          m_stateSpot(screen.get<ui::Spot>("GuildMemberState")),
          m_stateAreaSpot(screen.get<ui::Spot>("GuildMemberStateArea")),
          m_titleSpot(screen.get<ui::Spot>("GuildMemberTitle")),
          m_pageSpot(screen.get<ui::Spot>("GuildMemberPageNumbers")),
          m_textSpot(screen.get<ui::Spot>("GuildInfoText")),
          m_pageButtons{
              screen.get<ui::ImageButton>("GuildPageInfo"),
              screen.get<ui::ImageButton>("GuildPageMembers"),
              screen.get<ui::ImageButton>("GuildPageInsignia"),
              screen.get<ui::ImageButton>("GuildPageMessage")
          },
          m_notices(services, screen, guild, board, read),
          m_emblem(services, screen, guild, messages, m_hooks.ownId, m_hooks.ownBadge) {
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const std::string n = std::to_string(row);
            m_rows.push_back(
                {screen.get<ui::TextBox>("GuildName" + n),
                 screen.get<ui::TextBox>("GuildNickname" + n),
                 screen.get<ui::TextBox>("GuildRank" + n),
                 screen.get<ui::DrawingArea>("GuildCareer" + n),
                 screen.get<ui::ImageButton>("GuildMailIcon" + n),
                 screen.get<ui::Element>("GuildMailGray" + n),
                 screen.get<ui::ImageButton>("GuildCheck" + n)}
            );
            m_rows.back().career.get().setPainter([this, row](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                const int member = memberOfRow(row);
                if (member < 0 || m_page != Page::Members) {
                    return;
                }
                const int job = m_guild.at(member).record.job;
                if (validJob(job)) {
                    if (const auto icon = m_services.pictures.find(kJobIcons[static_cast<std::size_t>(job)])) {
                        canvas.drawPicture(*icon, at);
                    }
                }
            });
        }
        screen.get<ui::DrawingArea>("GuildPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            layoutInfo();
            layoutMembers();
            m_notices.layout(m_page == Page::Message);
            m_emblem.layout(m_page == Page::Insignia);
            if (m_page == Page::Info) {
                paintInfo(canvas, at);
            } else if (m_page == Page::Members) {
                paintMembers(canvas, at);
            } else if (m_page == Page::Message) {
                m_notices.paint(canvas, at);
            } else {
                m_emblem.paint(canvas, at);
            }
        });
        screen.on("onGuildWindowDoubleClick", [this](const ui::EventArgs&) {
            if (m_page == Page::Message) {
                m_notices.doubleClicked(position());
            }
        });
        screen.on("onGuildWindowClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onGuildTabParty", [this](const ui::EventArgs&) { m_hooks.showParty(); });
        screen.on("onGuildInstances", [this](const ui::EventArgs&) { m_hooks.showInstances(); });
        screen.on("onGuildPageInfo", [this](const ui::EventArgs&) { showPage(Page::Info); });
        screen.on("onGuildPageMembers", [this](const ui::EventArgs&) { showPage(Page::Members); });
        screen.on("onGuildPageInsignia", [this](const ui::EventArgs&) { showPage(Page::Insignia); });
        screen.on("onGuildPageMessage", [this](const ui::EventArgs&) { showPage(Page::Message); });
        screen.on("onGuildRulesUp", [this](const ui::EventArgs&) { m_rules.pageUp(); });
        screen.on("onGuildRulesDown", [this](const ui::EventArgs&) { m_rules.pageDown(); });
        screen.on("onGuildEndGuild", [this](const ui::EventArgs&) { askDisband(); });
        screen.on("onGuildEditRule", [this](const ui::EventArgs&) { m_hooks.editRules(position()); });
        screen.on("onGuildCastle", [this](const ui::EventArgs&) { m_hooks.castleTeleport(); });
        screen.on("onGuildCheck", [this](const ui::EventArgs& args) { toggleCheck(numberAfter(args.source, "GuildCheck")); });
        screen.on("onGuildMailMember", [this](const ui::EventArgs& args) { mailMember(numberAfter(args.source, "GuildMailIcon")); });
        screen.on("onGuildMailGroup", [this](const ui::EventArgs&) { mailChecked(); });
        screen.on("onGuildInvite", [this](const ui::EventArgs&) { m_hooks.inviteCursor(); });
        screen.on("onGuildLeave", [this](const ui::EventArgs&) { askLeave(); });
        screen.on("onGuildAbdicate", [this](const ui::EventArgs&) { abdicate(); });
        screen.on("onGuildDelMember", [this](const ui::EventArgs&) { fire(); });
        screen.on("onGuildMyAuthority", [this](const ui::EventArgs&) { m_hooks.toggleMyAuthority(); });
        screen.on("onGuildSetAuthority", [this](const ui::EventArgs&) { setAuthority(); });
        screen.on("onGuildNextPage", [this](const ui::EventArgs&) {
            if (m_memberPage < pageCount()) {
                ++m_memberPage;
            }
        });
        screen.on("onGuildPreviousPage", [this](const ui::EventArgs&) {
            if (m_memberPage > 1) {
                --m_memberPage;
            }
        });
        screen.get<ui::ScrollBar>("GuildRulesScroll").setRange(1, 1); // scroll range 1 to 1: no scrolling
        screen.get<ui::ImageButton>("GuildTabGuild").setSelected(true);
        showPage(Page::Info);
    }

    void GuildWindow::open(core::Point at) {
        const core::Rect bounds = m_window.bounds();
        m_window.setBounds({at.x, at.y, bounds.width, bounds.height});
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void GuildWindow::close() { m_window.setVisible(false); }

    void GuildWindow::refresh() { showRules(m_guild.rules()); }

    void GuildWindow::showRules(const std::string& rules) {
        m_rules.clear();
        m_rules.appendLine(rules);
    }

    void GuildWindow::showPage(Page page) {
        m_page = page;
        const int number = static_cast<int>(page);
        for (int i = 0; i < static_cast<int>(m_pageButtons.size()); ++i) {
            m_pageButtons[static_cast<std::size_t>(i)].get().setSelected(i == number);
        }
        m_window.setPicture(pageName(number), m_services.pictures);
        if (page == Page::Message) {
            m_notices.show();
        }
    }

    int GuildWindow::pageCount() const { return (m_guild.count() - 1) / kRowsPerPage + 1; }

    int GuildWindow::memberOfRow(int row) const {
        const int index = (m_memberPage - 1) * kRowsPerPage + row - 1;
        return index < m_guild.count() ? index : -1;
    }

    void GuildWindow::layoutInfo() {
        const bool shown = m_page == Page::Info;
        const bool lord = m_guild.isLord(m_hooks.ownId());
        const bool castle = !m_hooks.castleOwner().empty() && m_guild.name() == m_hooks.castleOwner();
        for (const char* name :
             {"GuildRules", "GuildRulesScroll", "GuildRulesUp", "GuildRulesDown", "GuildEndGuildGray", "GuildEditRuleGray", "GuildLeaderLabel"}) {
            m_screen.get<ui::Element>(name).setVisible(shown);
        }
        m_screen.get<ui::Element>("GuildEndGuild").setVisible(shown && lord);
        m_screen.get<ui::Element>("GuildEditRule").setVisible(shown && lord);
        m_screen.get<ui::Element>("GuildCastle").setVisible(shown && castle);
    }

    void GuildWindow::layoutMembers() {
        const bool shown = m_page == Page::Members;
        m_memberPage = std::min(m_memberPage, pageCount());
        const std::int32_t own = m_hooks.ownId();
        for (const char* name :
             {"GuildPreviousPage",
              "GuildNextPage",
              "GuildMailGroup",
              "GuildMailGroupGray",
              "GuildInviteGray",
              "GuildLeaveGray",
              "GuildAbdicateGray",
              "GuildDelMemberGray",
              "GuildMyAuthority",
              "GuildMyAuthorityGray",
              "GuildSetAuthorityGray"}) {
            m_screen.get<ui::Element>(name).setVisible(shown);
        }
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const Row& r = m_rows[static_cast<std::size_t>(row - 1)];
            const int member = shown ? memberOfRow(row) : -1;
            r.mailGray.get().setVisible(shown);
            r.name.get().setVisible(member >= 0);
            r.nickname.get().setVisible(member >= 0);
            r.rank.get().setVisible(member >= 0);
            r.career.get().setVisible(member >= 0 && validJob(m_guild.at(member).record.job));
            r.mail.get().setVisible(member >= 0);
            r.check.get().setVisible(member >= 0 && m_guild.at(member).record.id != own);
            if (member < 0) {
                r.career.get().setHint("");
                continue;
            }
            const GuildMember& m = m_guild.at(member);
            const bool isOwn = m.record.id == own;
            r.name.get().setText(m.record.name);
            r.nickname.get().setText(isOwn ? m_hooks.ownNickname() : m.record.nickname);
            r.rank.get().setText(member == 0 ? "OrgLeader" : member <= m_guild.viceCount() ? "Vice OrgLeader" : "");
            const int job = isOwn ? m_hooks.ownJob() : m.record.job;
            r.career.get().setHint(validJob(job) ? kJobNames[static_cast<std::size_t>(job)] : "");
            r.check.get().setPicture(m_services.pictures.find(m.checked ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}));
        }
        const bool lord = m_guild.isLord(own);
        const bool mayInvite = m_guild.mayInvite(own);
        const bool maySetAuthority = m_guild.maySetAuthority(own);
        const int ownIndex = m_guild.indexOf(own);
        const bool mayDelete = lord || (ownIndex >= 0 && ownIndex <= m_guild.viceCount());
        m_screen.get<ui::Element>("GuildInvite").setVisible(shown && mayInvite);
        m_screen.get<ui::Element>("GuildLeave").setVisible(shown && !lord);
        m_screen.get<ui::Element>("GuildAbdicate").setVisible(shown && lord);
        m_screen.get<ui::Element>("GuildDelMember").setVisible(shown && mayDelete);
        m_screen.get<ui::Element>("GuildSetAuthority").setVisible(shown && lord && maySetAuthority);
    }

    void GuildWindow::paintInfo(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        constexpr int kWidth = 150; // text box width, px
        if (m_guild.leaderId() != 0) {
            canvas.drawText({at.x + 96, at.y + 88}, m_guild.at(0).record.name, style, kWidth); // pixel position of the lord's name
        }
        canvas.drawText({at.x + 266, at.y + 103}, std::to_string(m_guild.count()), style, kWidth); // pixel position of the member count
        if (const auto badge = m_hooks.ownBadge()) {
            EmblemPainter::paint(canvas, *badge, at + m_badgeSpot.at());
        }
        if (!m_hooks.castleOwner().empty() && m_guild.name() == m_hooks.castleOwner()) {
            canvas.drawText({at.x + 195, at.y + 123}, "Castle", style, kWidth); // pixel position of the castle mark
        }
    }

    void GuildWindow::paintMembers(gfx::Canvas& canvas, core::Point at) {
        const std::int32_t own = m_hooks.ownId();
        const auto draw = [&](const std::string& name, core::Point where, const std::optional<core::Rect>& source = std::nullopt) {
            if (const auto id = m_services.pictures.find(name)) {
                canvas.drawPicture(*id, where, source);
            }
        };
        for (int row = 1; row <= kRowsPerPage; ++row) {
            const int y = (row - 1) * m_rowSpot.pitch().y;
            draw("Icon_OrgPosition", {at.x + m_titleSpot.at().x, at.y + m_titleSpot.at().y + y});
            const int member = memberOfRow(row);
            if (member < 0) {
                continue;
            }
            const protocol::GuildMemberRecord& record = m_guild.at(member).record;
            const bool isOwn = record.id == own;
            const int state = m_hooks.onlineState(isOwn ? own : record.id);
            draw(
                kStateIcons[static_cast<std::size_t>(std::clamp(state, 0, kOffline))],
                {at.x + m_stateSpot.at().x, at.y + m_stateSpot.at().y + y},
                m_stateAreaSpot.rect()
            );
            if (record.element >= 0 && record.element <= 4) { // element index 0..4 (0 none, 1 Earth, 2 Water, 3 Fire, 4 Wind; icons Icon_Element_0..4)
                draw("Icon_Element_" + std::to_string(record.element) + "_1", {at.x + m_elementSpot.at().x, at.y + m_elementSpot.at().y + y});
            }
            const core::Point corner{at.x + m_rowSpot.at().x, at.y + m_rowSpot.at().y + y};
            if (record.bodyType >= 1 && record.bodyType <= 4) { // valid body types 1..4
                const CharacterLook look = CharacterLook::fromColorCodes(record.bodyType, record.head, record.colorCode1, record.colorCode5, m_services.items);
                m_services.characters.drawPortrait(canvas, look, corner + m_faceSpot.at(), 0, kFaceSlot);
            }
            const int job = isOwn ? m_hooks.ownJob() : record.job;
            if (validJob(job)) {
                draw("Icon_Career", corner + m_careerSpot.at());
            }
            drawLevelDigits(canvas, isOwn ? m_hooks.ownLevel() : record.level, corner + m_levelSpot.at());
        }
        drawPageNumbers(canvas, {at.x + m_pageSpot.at().x, at.y + m_pageSpot.at().y}, m_memberPage, pageCount());
    }

    void GuildWindow::toggleCheck(int row) {
        if (m_hooks.authorityOpen()) {
            m_messages.show("Setting the ", 1200); // message display time in ms
            return;
        }
        const int member = memberOfRow(row);
        if (member < 0 || m_guild.at(member).record.id == m_hooks.ownId()) {
            return;
        }
        m_guild.at(member).checked = !m_guild.at(member).checked;
    }

    void GuildWindow::uncheckAll() { m_guild.uncheckAll(); }

    void GuildWindow::mailMember(int row) {
        const int member = memberOfRow(row);
        if (member < 0 || m_guild.at(member).record.id == m_hooks.ownId()) {
            return;
        }
        m_guild.at(member).checked = false;
        m_hooks.mailMember(m_guild.at(member).record.id);
    }

    void GuildWindow::mailChecked() {
        if (m_guild.checkedCount() == 0) {
            m_messages.show("Tick and select one of guild members", kNoticeMs);
            return;
        }
        m_hooks.mailChecked();
    }

    void GuildWindow::askLeave() {
        m_talk.ask(
            "Are you  sure to leave this guild?",
            std::nullopt,
            // talk options: line length of 40 characters, then the answer timeout in ms
            [this] { m_services.connection.send(protocol::leaveGuildRequest()); },
            nullptr,
            {40, kRefusedTalkMs, nullptr}
        );
    }

    void GuildWindow::askDisband() {
        m_talk.ask(
            "Are you sure you want to disband the guild?",
            m_hooks.ownSpeaker(),
            [this] { m_services.connection.send(protocol::disbandGuildRequest()); },
            nullptr
        );
    }

    void GuildWindow::abdicate() {
        const int checked = m_guild.checkedCount();
        if (checked == 0) {
            m_messages.show("Tick and select one of guild members", kNoticeMs);
            return;
        }
        if (checked > 1) {
            m_messages.show("Select one of guild members only", kNoticeMs);
            return;
        }
        for (int i = 0; i < m_guild.count(); ++i) {
            const GuildMember& member = m_guild.at(i);
            if (!member.checked) {
                continue;
            }
            if (member.onlineState == 2 || member.onlineState == 3) { // online state 2 hidden, 3 offline
                m_messages.show("The member you choose isn't online", kNoticeMs);
                return;
            }
            const std::int32_t id = member.record.id;
            m_talk.ask(
                "Sure to deliver the requirement for abdicating? ",
                std::nullopt,
                [this, id] { m_services.connection.send(protocol::abdicateGuildRequest(id)); },
                nullptr
            );
            break;
        }
        uncheckAll();
    }

    void GuildWindow::sendKick(std::int32_t memberId) { m_services.connection.send(protocol::kickGuildMemberRequest(memberId)); }

    void GuildWindow::fire() {
        const int checked = m_guild.checkedCount();
        if (checked == 0) {
            m_messages.show("Tick and select one of guild members", kNoticeMs);
            return;
        }
        if (checked > 1) {
            std::vector<std::int32_t> ids;
            for (int i = 0; i < m_guild.count(); ++i) {
                if (m_guild.at(i).checked) {
                    ids.push_back(m_guild.at(i).record.id);
                }
            }
            m_talk.ask(
                "Are you sure you want to remove these guild members?",
                std::nullopt,
                [this, ids] {
                    for (std::int32_t id : ids) {
                        sendKick(id);
                    }
                },
                nullptr
            );
            uncheckAll();
            return;
        }
        for (int i = 0; i < m_guild.count(); ++i) {
            if (m_guild.at(i).checked) {
                const std::int32_t id = m_guild.at(i).record.id;
                m_talk.ask("Are you sure you want to delete the teammate?", std::nullopt, [this, id] { sendKick(id); }, nullptr);
            }
        }
        uncheckAll();
    }

    void GuildWindow::setAuthority() {
        const int checked = m_guild.checkedCount();
        if (checked <= 0) {
            m_messages.show("Tick and select one of guild members", kNoticeMs);
            return;
        }
        if (m_guild.at(0).checked) {
            m_messages.show("The authorization of Guild Lord cannot be modified", kNoticeMs);
            return;
        }
        const std::int32_t own = m_hooks.ownId();
        for (int i = 0; i < m_guild.count(); ++i) {
            const GuildMember& member = m_guild.at(i);
            if (member.checked && !m_guild.isLord(own) && member.record.rights[0]) {
                m_messages.show("You have not enough authoriztion to modify" + member.record.name + "authoriztion of ", kNoticeMs);
                return;
            }
        }
        int selection = -checked;
        if (checked == 1) {
            for (int i = 0; i < m_guild.count(); ++i) {
                if (m_guild.at(i).checked) {
                    selection = i + 1;
                }
            }
        }
        m_hooks.toggleAuthority(selection);
    }

}
