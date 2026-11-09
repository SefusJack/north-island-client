#include "client/viewmodels/instances/InstanceWindow.h"

#include "client/rendering/hud/GameNumbers.h"
#include "client/rendering/hud/PageNumbers.h"
#include "client/rules/GmIds.h"

#include <algorithm>
#include <cstdlib>

namespace client {

    namespace {

        // all positions are offsets from the window origin, px, unless noted
        constexpr int kRowHeight = 60; // list row height, px
        constexpr int kRowSpacing = 10; // gap between list rows, px
        constexpr int kRowPitch = kRowHeight + kRowSpacing; // distance between row tops, px
        constexpr int kRowTextLeft = 3; // text inset inside a row, px
        constexpr int kHintGap = 3; // hint box sits this many px above the row top
        constexpr int kHintPad = 10; // padding inside the hint box, px
        constexpr int kHintLine = 20; // hint text line height, px
        constexpr int kScreenWidth = 800; // game window width, px
        constexpr int kGuildTabLeft = 85; // x of the guild tab
        constexpr int kInstancesTabWithGuild = 150; // x of the instances tab when the guild tab is shown
        constexpr int kInstancesTabAlone = 85; // x of the instances tab without the guild tab
        constexpr int kPageTabsLeft = 26; // x of the first page tab (Info / Members / Choose)
        constexpr int kInfoTabWidth = 44; // Info page tab width, px
        constexpr int kMembersTabWidth = 44; // Members page tab width, px
        constexpr int kMemberSlots = 5; // members shown per page
        constexpr int kMemberSlot = 1; // face slot 1: the small face bank picture
        constexpr int kKickAskMs = 5000; // "send away" question timeout, ms
        const TalkDialog::Timing kRequestBox{32, 0, nullptr, false}; // wrap at 32 chars, no timeout, no timeout callback, not exclusive
        // job icon picture per job number (index 0 = no job): 1 Assassin, 2 Samurai, 3 Knight, 4 Sage, 5 Clergyman, 6 Prophet
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};

        /// @brief Loads the window's layout and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the form is loaded into.
        /// @return The instance window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("InstanceWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("InstanceWindow");
        }

        /// @brief Reads the number that follows a prefix in a control name.
        /// @param name Control name.
        /// @param prefix Expected prefix.
        /// @return The number, or 0 when the name does not start with the prefix.
        int numberAfter(const std::string& name, const std::string& prefix) {
            return name.compare(0, prefix.size(), prefix) == 0 ? std::atoi(name.c_str() + prefix.size()) : 0;
        }

        /// @brief Moves an element horizontally, keeping its other bounds.
        /// @param element Element moved.
        /// @param left New left edge.
        void moveLeft(ui::Element& element, int left) {
            const core::Rect bounds = element.bounds();
            element.setBounds({left, bounds.y, bounds.width, bounds.height});
        }

    }

    InstanceWindow::InstanceWindow(
        GameServices& services,
        ui::Screen& screen,
        Instances& instances,
        InstanceCreateForm& create,
        InstanceGoalsPainter& goals,
        TalkDialog& talk,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_instances(instances),
          m_create(create),
          m_talk(talk),
          m_hooks(std::move(hooks)),
          m_goals(goals),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::DrawingArea>("InstanceList")),
          m_hint(screen.get<ui::Panel>("InstanceHint")),
          m_tabGuild(screen.get<ui::ImageButton>("InstanceTabGuild")),
          m_tabInstances(screen.get<ui::ImageButton>("InstanceTabInstances")),
          m_pageInfo(screen.get<ui::ImageButton>("InstancePageInfo")),
          m_pageMembers(screen.get<ui::ImageButton>("InstancePageMembers")),
          m_pageChoose(screen.get<ui::ImageButton>("InstancePageChoose")),
          m_newInstance(screen.get<ui::ImageButton>("InstanceCreate")),
          m_previous(screen.get<ui::ImageButton>("InstancePrevious")),
          m_next(screen.get<ui::ImageButton>("InstanceNext")),
          m_membersPrevious(screen.get<ui::ImageButton>("InstanceMembersPrevious")),
          m_membersNext(screen.get<ui::ImageButton>("InstanceMembersNext")),
          m_enter(screen.get<ui::ImageButton>("InstanceEnter")),
          m_leave(screen.get<ui::ImageButton>("InstanceLeave")),
          m_artSpot(screen.get<ui::Spot>("InstanceArt")),
          m_orderNumbersSpot(screen.get<ui::Spot>("InstanceOrderNumbers")),
          m_memberNumbersSpot(screen.get<ui::Spot>("InstanceMemberNumbers")),
          m_startIconSpot(screen.get<ui::Spot>("InstanceStartIcon")),
          m_startNumberSpot(screen.get<ui::Spot>("InstanceStartNumber")),
          m_memberTitleSpot(screen.get<ui::Spot>("InstanceMemberTitle")),
          m_memberPanelSpot(screen.get<ui::Spot>("InstanceMemberPanel")),
          m_memberRowSpot(screen.get<ui::Spot>("InstanceMemberRow")),
          m_memberRectSpot(screen.get<ui::Spot>("InstanceMemberRect")),
          m_innerTextSpot(screen.get<ui::Spot>("InstanceInnerText")),
          m_innerDescriptionSpot(screen.get<ui::Spot>("InstanceInnerDescription")),
          m_textSpot(screen.get<ui::Spot>("InstanceText")),
          m_hoverBarSpot(screen.get<ui::Spot>("InstanceHoverBar")),
          m_hintTextSpot(screen.get<ui::Spot>("InstanceHintText")) {
        for (int i = 1; i <= Instances::kRowsPerPage; ++i) {
            m_joins.push_back(screen.get<ui::ImageButton>("InstanceJoin" + std::to_string(i)));
            m_kicks.push_back(screen.get<ui::ImageButton>("InstanceKick" + std::to_string(i)));
        }
        m_tabInstances.setSelected(true);
        screen.get<ui::DrawingArea>("InstanceTabArt").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { layout(canvas, at); });
        m_list.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintList(canvas, at); });
        screen.get<ui::DrawingArea>("InstanceTexts").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onInstanceClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onInstanceTabParty", [this](const ui::EventArgs&) {
            close();
            m_hooks.showParty(position());
        });
        screen.on("onInstanceTabGuild", [this](const ui::EventArgs&) {
            close();
            m_hooks.showGuild(position());
        });
        screen.on("onInstancePageInfo", [this](const ui::EventArgs&) { m_instances.selectTab(Instances::Tab::Info); });
        screen.on("onInstancePageMembers", [this](const ui::EventArgs&) { m_instances.selectTab(Instances::Tab::Members); });
        screen.on("onInstancePageChoose", [this](const ui::EventArgs&) { m_instances.selectTab(Instances::Tab::Choose); });
        screen.on("onInstanceCreate", [this](const ui::EventArgs&) {
            if (m_instances.mayCreate() && !m_create.isOpen()) {
                m_create.open();
            }
        });
        screen.on("onInstancePrevious", [this](const ui::EventArgs&) { m_instances.previousOrderPage(); });
        screen.on("onInstanceNext", [this](const ui::EventArgs&) { m_instances.nextOrderPage(); });
        screen.on("onInstanceMembersPrevious", [this](const ui::EventArgs&) { m_instances.previousMembersPage(); });
        screen.on("onInstanceMembersNext", [this](const ui::EventArgs&) { m_instances.nextMembersPage(); });
        screen.on("onInstanceJoin", [this](const ui::EventArgs& args) { m_instances.joinFromList(numberAfter(args.source, "InstanceJoin") - 1); });
        screen.on("onInstanceKick", [this](const ui::EventArgs& args) { askSendAway(numberAfter(args.source, "InstanceKick")); });
        screen.on("onInstanceListDown", [this](const ui::EventArgs& args) {
            const int row = hoverRow(args.position.y);
            if (row >= 0) {
                m_instances.showDetail(row);
            }
        });
        screen.on("onInstanceEnter", [this](const ui::EventArgs&) { askEnter(); });
        screen.on("onInstanceLeave", [this](const ui::EventArgs&) { askLeave(); });
        screen.on("onInstanceWindowDoubleClick", [this](const ui::EventArgs&) { doubleClicked(); });
    }

    void InstanceWindow::open(core::Point at) {
        const core::Rect bounds = m_window.bounds();
        m_window.setBounds({at.x, at.y, bounds.width, bounds.height});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_instances.windowShown();
    }

    void InstanceWindow::text(gfx::Canvas& canvas, const std::string& line, core::Point at) {
        canvas.drawText(at, line, {m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt}, m_textSpot.size().width, m_textSpot.size().height);
    }

    void InstanceWindow::layout(gfx::Canvas& canvas, core::Point at) {
        const bool inGuild = m_hooks.inGuild();
        m_tabGuild.setVisible(inGuild);
        moveLeft(m_tabGuild, kGuildTabLeft);
        moveLeft(m_tabInstances, inGuild ? kInstancesTabWithGuild : kInstancesTabAlone);
        if (m_instances.state() == 0) { // instance state 0: the player has no instance
            m_pageChoose.setVisible(true);
            moveLeft(m_pageChoose, kPageTabsLeft);
            m_pageInfo.setVisible(false);
            m_pageMembers.setVisible(false);
            applyTab(Instances::Tab::Choose);
        } else {
            m_pageInfo.setVisible(true);
            moveLeft(m_pageInfo, kPageTabsLeft);
            m_pageMembers.setVisible(true);
            moveLeft(m_pageMembers, kPageTabsLeft + kInfoTabWidth);
            m_pageChoose.setVisible(true);
            moveLeft(m_pageChoose, kPageTabsLeft + kInfoTabWidth + kMembersTabWidth);
            applyTab(m_instances.tab());
        }
        const char* art = "Form_DupChoose";
        if (m_instances.tab() == Instances::Tab::Info) {
            art = "Form_DupInnerInfo";
        } else if (m_instances.tab() == Instances::Tab::Members) {
            art = "Form_DupInnerMember";
        }
        if (const auto picture = m_services.pictures.find(art)) {
            canvas.drawPicture(*picture, {at.x + m_artSpot.at().x, at.y + m_artSpot.at().y});
        }
    }

    void InstanceWindow::applyTab(Instances::Tab tab) {
        const bool choose = tab == Instances::Tab::Choose;
        m_pageInfo.setSelected(tab == Instances::Tab::Info);
        m_pageMembers.setSelected(tab == Instances::Tab::Members);
        m_pageChoose.setSelected(choose);
        m_previous.setVisible(choose);
        m_next.setVisible(choose);
        m_newInstance.setVisible(choose);
        m_list.setVisible(choose);
        if (!choose) {
            m_hint.setVisible(false);
            for (ui::ImageButton& join : m_joins) {
                join.setVisible(false);
            }
        }
        m_leave.setVisible(tab == Instances::Tab::Info);
        m_enter.setVisible(tab == Instances::Tab::Info && m_instances.state() == 2 && !m_instances.here()); // instance state 2: started
        m_membersPrevious.setVisible(tab == Instances::Tab::Members);
        m_membersNext.setVisible(tab == Instances::Tab::Members);
        if (tab != Instances::Tab::Members) {
            for (ui::ImageButton& kick : m_kicks) {
                kick.setVisible(false);
            }
        }
        m_instances.selectTab(tab);
    }

    int InstanceWindow::hoverRow(int localY) const {
        if (m_screen.hoveredId() != m_list.id() || localY < 0) {
            return -1;
        }
        const int row = localY / kRowPitch;
        if ((row + 1) * kRowPitch - localY <= kRowSpacing || row >= Instances::kRowsPerPage || row >= static_cast<int>(m_instances.rows().size())) {
            return -1;
        }
        return row;
    }

    void InstanceWindow::paintList(gfx::Canvas& canvas, core::Point at) {
        const int row = hoverRow(m_screen.mouse().y - at.y);
        if (row >= 0) {
            canvas.fillRect({at.x, at.y - 2 + row * kRowPitch, m_list.bounds().width, kRowHeight}, m_hoverBarSpot.color()); // bar starts 2 px above the row
        }
    }

    void InstanceWindow::paint(gfx::Canvas& canvas, core::Point at) {
        switch (m_instances.tab()) {
            case Instances::Tab::Choose: // Choose Instance page: order page numbers and the open instances
                drawPageNumbersFrom(
                    canvas,
                    {at.x + m_orderNumbersSpot.at().x, at.y + m_orderNumbersSpot.at().y},
                    m_instances.orderPage(),
                    m_instances.orderPages()
                );
                paintRows(canvas, at);
                break;
            case Instances::Tab::Info: paintInner(canvas, at); break; // Inner Info page
            case Instances::Tab::Members: // Members page: member page numbers and the member rows
                drawPageNumbersFrom(
                    canvas,
                    {at.x + m_memberNumbersSpot.at().x, at.y + m_memberNumbersSpot.at().y},
                    m_instances.memberPage(),
                    m_instances.memberPages()
                );
                paintMembers(canvas, at);
                break;
        }
        if (const auto picture = m_services.pictures.find("Icon_StartDupNum")) {
            canvas.drawPicture(*picture, {at.x + m_startIconSpot.at().x, at.y + m_startIconSpot.at().y});
        }
        text(canvas, std::to_string(m_instances.startNumber()), {at.x + m_startNumberSpot.at().x, at.y + m_startNumberSpot.at().y});
    }

    void InstanceWindow::paintRows(gfx::Canvas& canvas, core::Point at) {
        const std::vector<InstanceRow>& rows = m_instances.rows();
        const int shown = std::min(static_cast<int>(rows.size()), Instances::kRowsPerPage);
        m_hint.setVisible(false);
        for (int i = 0; i < Instances::kRowsPerPage; ++i) {
            m_joins[static_cast<std::size_t>(i)].get().setVisible(i < shown);
        }
        const int left = at.x + m_list.bounds().x + kRowTextLeft;
        int top = at.y + m_list.bounds().y + 2; // rows start 2 px below the list top
        for (int i = 0; i < shown; ++i) {
            const InstanceRow& row = rows[static_cast<std::size_t>(i)];
            // text line offsets within a row: title -5, then 7, 18, 30 and 42 px below the row top
            text(canvas, row.title, {left, top - 5});
            text(canvas, "Level restricitons:" + row.level, {left, top + 7});
            text(canvas, "The current number of players: " + row.players, {left, top + 18});
            text(canvas, "Guild restrictions: " + row.guildLimit, {left, top + 30});
            text(canvas, row.message, {left, top + 42});
            top += kRowPitch;
        }
        const int hover = hoverRow(m_screen.mouse().y - (at.y + m_list.bounds().y));
        if (hover >= 0) {
            paintHint(canvas, at, hover);
        }
    }

    void InstanceWindow::paintHint(gfx::Canvas& canvas, core::Point at, int row) {
        const InstanceRow& r = m_instances.rows()[static_cast<std::size_t>(row)];
        std::string line = "Creator has left";
        if (!r.creator.empty()) {
            line = (r.guildLimit != "None" ? "Creates an organization:" : "Creator:") + r.creator;
        }
        const int glyph = 8; // width of one character, px
        const int width = static_cast<int>(line.size()) * glyph + 2 * kHintPad;
        int left = m_list.bounds().x + m_list.bounds().width;
        if (at.x + left + width > kScreenWidth) {
            left = m_list.bounds().x - width;
        }
        const int top = m_list.bounds().y - kHintGap + row * kRowPitch;
        m_hint.setBounds({left, top, width, kHintLine + 2 * kHintPad});
        m_hint.setVisible(true);
        canvas.drawText(
            {at.x + left + kHintPad, at.y + top + kHintPad},
            line,
            {m_hintTextSpot.color(), gfx::TextEffect::Shadow, std::nullopt},
            static_cast<int>(line.size()) * glyph,
            kHintLine
        );
    }

    void InstanceWindow::paintInner(gfx::Canvas& canvas, core::Point at) {
        const InstanceInfo& own = m_instances.own();
        const int x = at.x + m_innerTextSpot.at().x;
        int y = at.y + m_innerTextSpot.at().y;
        text(canvas, own.title, {x, y});
        y += m_innerTextSpot.pitch().y;
        // inner info offsets from the text origin: time limit (9, -1), member icon (113, -2), player count (186, 0)
        text(canvas, own.timeLimit, {x + 9, y - 1});
        if (const auto picture = m_services.pictures.find("Icon_DupMemberNum")) {
            canvas.drawPicture(*picture, {x + 113, y - 2});
        }
        text(canvas, std::to_string(own.members) + "/" + own.maxPlayers, {x + 186, y});
        y += m_innerTextSpot.pitch().y;
        if (m_instances.state() != 2) { // instance state 2: started
            text(canvas, "Hasn't started", {x - 75, y + 10}); // offset (-75, 10) of the "Hasn't started" text
        } else {
            const std::string time = Instances::minutesText(own.remainingMinutes);
            text(canvas, own.remainPrefix.empty() ? time : own.remainPrefix + time + " Clear", {x + 9, y - 2}); // offset (9, -2) of the remaining time
        }
        text(canvas, own.minLevel, {x + 188, y - 1}); // offset (188, -1) of the minimum level
        m_goals.paintInner(canvas, {x, y});
        text(canvas, own.description, {at.x + m_innerDescriptionSpot.at().x, at.y + m_innerDescriptionSpot.at().y});
    }

    void InstanceWindow::paintMembers(gfx::Canvas& canvas, core::Point at) {
        for (ui::ImageButton& kick : m_kicks) {
            kick.setVisible(false);
        }
        m_slotIds.fill(0);
        const InstanceInfo& own = m_instances.own();
        const int x = at.x + m_memberTitleSpot.at().x;
        const int y = at.y + m_memberTitleSpot.at().y;
        text(canvas, own.title, {x, y});
        if (const auto picture = m_services.pictures.find("Icon_DupMemberNum")) {
            canvas.drawPicture(*picture, {x, y + 17}); // member icon 17 px below the title
        }
        text(canvas, std::to_string(own.members) + "/" + own.maxPlayers, {x + 79, y + 19}); // player count offset (79, 19)
        int slot = 0;
        for (const std::int32_t id : m_instances.memberIds()) {
            if (++slot > kMemberSlots) {
                return;
            }
            const auto mate = m_hooks.player(id);
            if (!mate) {
                continue;
            }
            m_slotIds[static_cast<std::size_t>(slot - 1)] = id;
            const auto draw = [&](const std::string& name, core::Point where) {
                if (const auto picture = m_services.pictures.find(name)) {
                    canvas.drawPicture(*picture, where);
                }
            };
            if (mate->element >= 0 && mate->element <= 4) { // element ids 0..4 have an icon (0 none, 1 earth, 2 water, 3 fire, 4 wind)
                draw(
                    "Icon_Element_" + std::to_string(mate->element) + "_1",
                    {at.x + m_memberPanelSpot.at().x, at.y + m_memberPanelSpot.at().y + (slot - 1) * m_memberRowSpot.pitch().y}
                );
            }
            const core::Point row{at.x + m_memberRowSpot.at().x, at.y + m_memberRowSpot.at().y + m_memberRowSpot.pitch().y * slot};
            // member row offsets: portrait (31, 27), level digits (180, 13), name (67, 14), job icon (155, 13), nick icon (65, 34), nickname (110, 36)
            m_services.characters.drawPortrait(canvas, mate->look, {row.x + 31, row.y + 27}, 0, kMemberSlot);
            drawLevelDigits(canvas, mate->level, {row.x + 180, row.y + 13});
            text(canvas, mate->name, {row.x + 67, row.y + 14});
            if (mate->job >= 1 && mate->job < static_cast<int>(kJobIcons.size())) {
                draw(kJobIcons[static_cast<std::size_t>(mate->job)], {row.x + 155, row.y + 13});
            }
            draw("Icon_Nick", {row.x + 65, row.y + 34});
            text(canvas, mate->nickname, {row.x + 110, row.y + 36});
            // instance state 1: waiting (only then the creator may send members away)
            m_kicks[static_cast<std::size_t>(slot - 1)].get().setVisible(m_hooks.ownId() == own.creatorId && id != own.creatorId && m_instances.state() == 1);
        }
    }

    void InstanceWindow::askLeave() {
        m_talk.ask("Do you want to quit the instance?", std::nullopt, [this] { m_instances.leave(); }, nullptr, kRequestBox);
    }

    void InstanceWindow::askEnter() {
        m_talk.ask("Do you want to enter the current instance?", std::nullopt, [this] { m_instances.enter(); }, nullptr, kRequestBox);
    }

    void InstanceWindow::askSendAway(int slot) {
        if (slot < 1 || slot > kMemberSlots) {
            return;
        }
        const std::int32_t id = m_slotIds[static_cast<std::size_t>(slot - 1)];
        const auto mate = m_hooks.player(id);
        if (id == 0 || !mate || !m_instances.maySend(id)) {
            return;
        }
        m_talk.ask(
            "Are you sure to invite? " + mate->name + " Do you want to quit the instance?",
            std::nullopt,
            [this, id] { m_instances.sendAway(id); },
            nullptr,
            {40, kKickAskMs, nullptr}
        ); // 40: first field of the talk timing (wrap width in characters); kKickAskMs the answer timeout
    }

    void InstanceWindow::doubleClicked() {
        if (m_instances.tab() != Instances::Tab::Members) {
            return;
        }
        const core::Point mouse = m_screen.mouse();
        const core::Point origin = position();
        for (int slot = 1; slot <= kMemberSlots; ++slot) {
            const core::Rect rect{
                origin.x + m_memberRectSpot.rect().x,
                origin.y + m_memberRectSpot.rect().y + (slot - 1) * m_memberRowSpot.pitch().y,
                m_memberRectSpot.rect().width,
                m_memberRectSpot.rect().height
            };
            if (!rect.contains(mouse)) {
                continue;
            }
            const std::int32_t id = m_slotIds[static_cast<std::size_t>(slot - 1)];
            if (id != 0 && id != m_hooks.ownId() && !isGmId(id)) {
                m_hooks.whisper(id);
            }
            return;
        }
    }

}
