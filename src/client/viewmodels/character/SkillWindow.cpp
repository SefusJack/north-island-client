#include "client/viewmodels/character/SkillWindow.h"

#include "client/rules/ItemInfo.h"
#include "client/rules/Experience.h"
#include "client/rules/SkillTree.h"
#include "client/rules/Skills.h"
#include "gfx/TextStyle.h"
#include "protocol/PetRequests.h"
#include "ui/TextWrap.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace client {

    namespace {

        constexpr int kTreeTab = 5; // tab number (1..5) that shows the skill tree instead of the list
        constexpr std::array<const char*, 5> kTabNames = {"SkillTab1", "SkillTab2", "SkillTab3", "SkillTab4", "SkillTab5"};
        constexpr std::array<const char*, 5> kStatNames = {"STR", "CON", "INT", "WIS", "AGI"};

        /// @brief Loads SkillWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "SkillWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SkillWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("SkillWindow");
        }

        /// @brief Plain (no effect) text style of a colour.
        /// @param color Text colour.
        /// @return The style.
        gfx::TextStyle plain(core::Color color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

        /// @brief Right-aligns text in a field of width characters.
        /// @param text Text to align.
        /// @param width Field width in characters.
        /// @return The padded text.
        std::string padLeft(const std::string& text, std::size_t width) { return text.size() >= width ? text : std::string(width - text.size(), ' ') + text; }

    }

    SkillWindow::SkillWindow(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        const PetTeam& pets,
        const MapCharacter& player,
        PlayerMessages& messages
    )
        : m_services(services),
          m_screen(screen),
          m_status(status),
          m_pets(pets),
          m_player(player),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::DrawingArea>("SkillList")),
          m_scrollBar(screen.get<ui::ScrollBar>("SkillScroll")),
          m_scrollUp(screen.get<ui::ImageButton>("SkillScrollUp")),
          m_scrollDown(screen.get<ui::ImageButton>("SkillScrollDown")),
          m_paint(screen.get<ui::DrawingArea>("SkillPaint")),
          m_rowsSpot(screen.get<ui::Spot>("SkillRows")),
          m_treeCellSpot(screen.get<ui::Spot>("SkillTreeCell")),
          m_textSpot(screen.get<ui::Spot>("SkillTextColor")),
          m_infoSpot(screen.get<ui::Spot>("SkillInfoColor")),
          m_levelSpot(screen.get<ui::Spot>("SkillLevelColor")),
          m_tabs{
              screen.get<ui::ImageButton>(kTabNames[0]),
              screen.get<ui::ImageButton>(kTabNames[1]),
              screen.get<ui::ImageButton>(kTabNames[2]),
              screen.get<ui::ImageButton>(kTabNames[3]),
              screen.get<ui::ImageButton>(kTabNames[4])
          },
          m_learn{
              screen.get<ui::ImageButton>("SkillLearn1"),
              screen.get<ui::ImageButton>("SkillLearn2"),
              screen.get<ui::ImageButton>("SkillLearn3"),
              screen.get<ui::ImageButton>("SkillLearn4"),
              screen.get<ui::ImageButton>("SkillLearn5"),
              screen.get<ui::ImageButton>("SkillLearn6"),
              screen.get<ui::ImageButton>("SkillLearn7"),
              screen.get<ui::ImageButton>("SkillLearn8")
          } {
        screen.get<ui::DrawingArea>("SkillPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        m_list.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintList(canvas, at); });
        screen.on("onSkillWindowClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onSkillTab", [this](const ui::EventArgs& args) {
            for (std::size_t i = 0; i < kTabNames.size(); ++i) {
                if (args.source == kTabNames[i]) {
                    selectTab(static_cast<int>(i) + 1);
                }
            }
        });
        screen.on("onSkillPreviousOwner", [this](const ui::EventArgs&) { switchOwner(false); });
        screen.on("onSkillNextOwner", [this](const ui::EventArgs&) { switchOwner(true); });
        screen.on("onSkillLearn", [this](const ui::EventArgs& args) { learnSkill(m_top + (args.source.back() - '1')); });
        screen.on("onSkillScrolled", [this](const ui::EventArgs& args) { scrollTo(args.index); });
        screen.on("onSkillScrollUp", [this](const ui::EventArgs&) { scrollTo(m_top - 1); });
        screen.on("onSkillScrollDown", [this](const ui::EventArgs&) { scrollTo(m_top + 1); });
        screen.on("onSkillListWheel", [this](const ui::EventArgs& args) { scrollTo(m_top - args.index); });
        screen.on("onSkillListClick", [this](const ui::EventArgs&) { clicked(); });
        screen.on("onSkillListPress", [this](const ui::EventArgs&) { pressed(); });
        screen.on("onSkillListDrop", [this](const ui::EventArgs& args) {
            if (args.target == "HotKeySlots" && m_putOnBar) {
                m_putOnBar(args.targetIndex, args.index);
            } else if (args.target.rfind("PlusKey", 0) == 0 && m_putOnPlus) {
                m_putOnPlus(std::stoi(args.target.substr(7)), args.index); // length of the "PlusKey" prefix; the rest is the slot number
            }
        });
        m_tabs[0].get().setSelected(true);
    }

    void SkillWindow::open() {
        fillList();
        scrollTo(m_tabTops[static_cast<std::size_t>(m_tab)]);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void SkillWindow::close() {
        m_tabTops[static_cast<std::size_t>(m_tab)] = m_top;
        m_window.setVisible(false);
    }

    void SkillWindow::toggle() {
        if (isOpen()) {
            close();
        } else {
            open();
        }
    }

    std::optional<std::reference_wrapper<const Pet>> SkillWindow::shownPet() const {
        const int place = m_pets.placeByOrder(m_owner);
        if (m_owner == 0 || place == 0 || !m_pets.at(place)) {
            return std::nullopt;
        }
        return std::cref(*m_pets.at(place));
    }

    void SkillWindow::switchOwner(bool forward) {
        const int old = m_owner;
        if (forward) {
            m_owner = m_owner == m_pets.count() ? 0 : m_owner + 1;
        } else {
            m_owner = m_owner == 0 ? m_pets.count() : m_owner - 1;
        }
        if (old != m_owner) {
            m_tabTops = {};
            m_top = 0;
        }
        fillList();
        scrollTo(m_tabTops[static_cast<std::size_t>(m_tab)]);
    }

    void SkillWindow::refresh() {
        if (m_owner != 0 && !shownPet()) {
            m_owner = 0;
            m_tabTops = {};
            m_top = 0;
        }
        if (isOpen()) {
            const int top = m_top;
            fillList();
            scrollTo(top);
        }
    }

    void SkillWindow::selectTab(int tab) {
        m_tabTops[static_cast<std::size_t>(m_tab)] = m_top;
        m_tab = tab;
        for (std::size_t i = 0; i < m_tabs.size(); ++i) {
            m_tabs[i].get().setSelected(static_cast<int>(i) + 1 == tab);
        }
        const bool list = tab != kTreeTab;
        m_scrollBar.setVisible(list);
        m_scrollUp.setVisible(list);
        m_scrollDown.setVisible(list);
        fillList();
        scrollTo(m_tabTops[static_cast<std::size_t>(m_tab)]);
    }

    void SkillWindow::fillList() {
        m_rows.clear();
        if (m_tab == kTreeTab && !shownPet()) {
            return;
        }
        const auto add = [this](int skillId, int level) {
            const auto row = std::find_if(m_rows.begin(), m_rows.end(), [skillId](const Row& r) { return r.skillId == skillId; });
            if (row != m_rows.end()) {
                row->level = level;
            } else if (shownSkill(skillId)) {
                m_rows.push_back({skillId, level});
            }
        };
        if (m_tab == 1) { // tab 1 also lists the two built-in actions
            add(kNormalAttack, 1);
            add(kDefense, 1);
        }
        if (const auto pet = shownPet()) {
            for (std::size_t i = 0; i < pet->get().info.skills.size(); ++i) {
                const auto record = m_services.skills.find(pet->get().info.skills[i]);
                if (record && m_tab != kTreeTab && petSkillOnTab(m_tab, record->skillType)) {
                    add(record->id, pet->get().skillLevels[i]);
                }
            }
            return;
        }
        for (const auto& [key, skill] : m_status.skills) {
            if (skill.level == 0) {
                continue;
            }
            const auto record = m_services.skills.findByKey(key);
            if (!record) {
                continue;
            }
            const auto shown = shownSkill(record->id);
            if (shown && skillOnTab(m_tab, shown->id, shown->skillType)) {
                add(record->id, skill.level);
            }
        }
    }

    int SkillWindow::visibleRows() const { return m_list.bounds().height / m_rowsSpot.pitch().y; }

    void SkillWindow::scrollTo(int top) {
        const int count = static_cast<int>(m_rows.size());
        m_top = std::clamp(top, 0, std::max(0, count - visibleRows()));
        m_scrollBar.setRange(count, visibleRows());
        m_scrollBar.setPosition(m_top);
    }

    void SkillWindow::updateLearnButtons() {
        for (std::size_t button = 0; button < m_learn.size(); ++button) {
            const std::size_t row = static_cast<std::size_t>(m_top) + button;
            bool shown = false;
            if (m_tab != kTreeTab && row < m_rows.size() && m_rows[row].level == 0) {
                const auto record = m_services.skills.find(m_rows[row].skillId);
                shown = record && record->learnPoints <= m_status.skillPoints;
            }
            m_learn[button].get().setVisible(shown);
        }
    }

    void SkillWindow::learnSkill(int row) {
        if (row >= 0 && row < static_cast<int>(m_rows.size()) && m_send) {
            m_send(protocol::learnSkillRequest(m_rows[static_cast<std::size_t>(row)].skillId, m_rows[static_cast<std::size_t>(row)].level));
        }
    }

    int SkillWindow::hoveredRow() const {
        if (m_screen.hoveredId() != m_list.id()) {
            return -1;
        }
        const core::Point origin = m_window.bounds().position() + m_list.bounds().position();
        const int y = m_screen.mouse().y - (origin.y - 2); // the rows start 2 px above the list area
        if (y < 0) {
            return -1;
        }
        const int row = m_top + y / m_rowsSpot.pitch().y;
        return row < static_cast<int>(m_rows.size()) && y / m_rowsSpot.pitch().y < visibleRows() ? row : -1;
    }

    std::optional<formats::SkillRecord> SkillWindow::shownSkill(int skillId) const {
        if (skillId == kRoleSkillPlaceholder) {
            return m_services.skills.find(roleSkillId(m_player.look.bodyType, m_player.look.head));
        }
        return m_services.skills.find(skillId);
    }

    std::optional<protocol::LearnedSkill> SkillWindow::learned(int skillId) const {
        const auto record = m_services.skills.find(isRoleSkill(skillId) ? kRoleSkillPlaceholder : skillId);
        if (!record) {
            return std::nullopt;
        }
        const auto found = m_status.skills.find(record->key);
        if (found == m_status.skills.end() || found->second.level == 0) {
            return std::nullopt;
        }
        return found->second;
    }

    void SkillWindow::pressed() {
        const int row = hoveredRow();
        if (row < 0) {
            return;
        }
        const int skillId = m_rows[static_cast<std::size_t>(row)].skillId;
        const auto skill = shownSkill(skillId);
        const auto icon = skill ? m_services.pictures.find("Icon_sk" + std::to_string(skill->iconId) + "s") : std::nullopt;
        if (!icon) {
            return;
        }
        constexpr int kIconSize = 24; // dragged skill icon size, px
        m_screen.context().drag = ui::DragItem{m_list.id(), skillId, *icon, {0, 0, kIconSize, kIconSize}, {kIconSize / 2, kIconSize / 2}};
    }

    void SkillWindow::clicked() {
        const int row = hoveredRow();
        if (row < 0) {
            return;
        }
        if (const auto taken = m_battlePick ? m_battlePick(m_rows[static_cast<std::size_t>(row)].skillId) : std::nullopt) {
            if (*taken) {
                close();
            }
            return;
        }
        const auto skill = shownSkill(m_rows[static_cast<std::size_t>(row)].skillId);
        if (!skill) {
            return;
        }
        const auto pet = shownPet();
        if (pet && std::find(pet->get().info.skills.begin(), pet->get().info.skills.end(), skill->id) == pet->get().info.skills.end()) {
            return;
        }
        if ((pet ? pet->get().sp : m_status.sp) < skill->spCost) {
            m_messages.show("not enough SP", 1200); // message display time in ms
            return;
        }
        constexpr int kBattleOnly = 17011; // skill 17011: Najas's Memory (battle only)
        if (skill->id == kBattleOnly) {
            m_messages.show("The skill can only be used in battle.", 1200); // message display time in ms
        }
    }

    void SkillWindow::paint(gfx::Canvas& canvas, core::Point at) {
        updateLearnButtons();
        const auto pet = shownPet();
        const std::string& name = pet ? pet->get().name : m_player.name;
        // name centred (8 px per character, 4 px nudge) at y 40; box 230 x 18 px
        canvas.drawText({at.x + m_paint.bounds().width / 2 - static_cast<int>(name.size()) * 8 / 2 - 4, at.y + 40}, name, plain(m_infoSpot.color()), 230, 18);
        if (m_tab == kTreeTab) {
            paintTree(canvas, at);
        } else {
            paintHovered(canvas, at);
        }
    }

    void SkillWindow::paintList(gfx::Canvas& canvas, core::Point at) {
        const int hovered = hoveredRow();
        const int rows = std::min(visibleRows(), static_cast<int>(m_rows.size()) - m_top);
        if (hovered >= 0) {
            canvas.fillRect(
                {at.x, at.y - 2 + (hovered - m_top) * m_rowsSpot.pitch().y, m_list.bounds().width, m_rowsSpot.pitch().y},
                m_rowsSpot.color()
            ); // rows start 2 px above the list area
        }
        for (int r = 0; r < rows; ++r) {
            const Row& row = m_rows[static_cast<std::size_t>(m_top + r)];
            const auto skill = shownSkill(row.skillId);
            if (!skill) {
                continue;
            }
            const int y = at.y - 2 + r * m_rowsSpot.pitch().y; // rows start 2 px above the list area
            if (const auto icon = m_services.pictures.find("Icon_sk" + std::to_string(skill->iconId) + "s")) {
                canvas.drawPicture(*icon, {at.x, y});
            }
            std::string text;
            if (row.skillId == kNormalAttack) {
                text = "Normal attack";
            } else if (row.skillId == kDefense) {
                text = skill->name;
            } else {
                text = skill->name + " " + std::to_string(skill->spCost) + " sp";
            }
            // name at x 30, y +7 (width 170 px); level number at y +13 (width 20 px) over the icon
            canvas.drawText({at.x + 30, y + 7}, text, plain(m_textSpot.color()), 170);
            canvas.drawText({at.x, y + 13}, std::to_string(row.level), {m_levelSpot.color(), gfx::TextEffect::Outline, std::nullopt}, 20);
        }
    }

    void SkillWindow::paintHovered(gfx::Canvas& canvas, core::Point at) {
        const int row = hoveredRow();
        if (row < 0) {
            return;
        }
        const int skillId = m_rows[static_cast<std::size_t>(row)].skillId;
        const auto skill = shownSkill(skillId);
        if (!skill) {
            return;
        }
        if (skillId == kNormalAttack || skillId == kDefense) {
            // description area at (31, 339), 230 x 18 px
            canvas.drawText({at.x + 31, at.y + 339}, skillId == kNormalAttack ? "Normal attack" : "Defense", plain(m_infoSpot.color()), 230, 18);
            return;
        }
        // description at (31, 339), wrapped at 29 characters, 232 x 18 px
        canvas.drawText({at.x + 31, at.y + 339}, ui::wrapTextLines(skill->description, 29), plain(m_infoSpot.color()), 232, 18);
        int level = 0;
        double percent = 0.0;
        bool atMost = false;
        if (const auto pet = shownPet()) {
            const auto& skills = pet->get().info.skills;
            const auto place = std::find(skills.begin(), skills.end(), skill->id);
            const std::size_t index = static_cast<std::size_t>(place - skills.begin());
            if (place != skills.end() && pet->get().skillLevels[index] > 0) {
                level = pet->get().skillLevels[index];
                const std::int64_t reached = totalSkillLevelExp(m_services.formula, level);
                const std::int64_t next = totalSkillLevelExp(m_services.formula, level + 1);
                if (next > reached) {
                    percent = std::min(
                        100.0,
                        static_cast<double>(pet->get().skillExp[index] - reached) / static_cast<double>(next - reached) * 100.0
                    ); // percent of the way to the next level
                }
                atMost = skill->maxLevel == level;
            }
        } else if (const auto own = learned(skill->id)) {
            level = own->level;
            const std::int64_t reached = own->level == 1 ? 0 : totalSkillLevelExp(m_services.formula, own->level);
            const std::int64_t next = totalSkillLevelExp(m_services.formula, own->level + 1);
            if (next > reached) {
                percent = std::min(
                    100.0,
                    static_cast<double>(own->exp - reached) / static_cast<double>(next - reached) * 100.0
                ); // percent of the way to the next level
            }
            const auto counted = m_services.skills.find(isRoleSkill(skill->id) ? kRoleSkillPlaceholder : skill->id);
            atMost = counted && counted->maxLevel == own->level;
        }
        std::string proficiency = "    Max";
        if (!atMost) {
            // the shown proficiency stops at 99 percent until the level is the maximum; right-aligned in 7 characters
            proficiency = padLeft(std::to_string(static_cast<int>(std::floor(percent >= 100.0 ? 99.0 : percent))), 7);
        }
        // proficiency at (60, 395), 80 x 18 px; level at (76, 374), 150 x 18 px; level text buffer of 8 chars
        canvas.drawText({at.x + 60, at.y + 395}, proficiency, plain(m_infoSpot.color()), 80, 18);
        char levelText[8];
        std::snprintf(levelText, sizeof levelText, "%2d", level);
        canvas.drawText({at.x + 76, at.y + 374}, levelText, plain(m_infoSpot.color()), 150, 18);
        std::string kind = "Aid skill";
        // skill types 1 and 2 are attacks (range kind 3 is ranged), type 13 is a daily skill; kind text at (175, 376), 150 x 18 px
        if (skill->skillType == 1 || skill->skillType == 2) {
            kind = skill->rangeKind == 3 ? "Distance" : "Near attack";
        } else if (skill->skillType == 13) {
            kind = "Daily skill";
        }
        canvas.drawText({at.x + 175, at.y + 376}, kind, plain(m_infoSpot.color()), 150, 18);
        if (const auto motion = m_services.motions.find(skillId)) {
            constexpr int kWeapons = 5; // weapon kinds 1..5 (Icon_Weapon1_1 .. Icon_Weapon5_1)
            constexpr int kGap = 2; // gap between weapon icons, px
            for (int weapon = 1; weapon <= kWeapons; ++weapon) {
                const auto icon = m_services.pictures.find("Icon_Weapon" + std::to_string(weapon) + "_1");
                if (icon && motion->get().usableWeapons[static_cast<std::size_t>(weapon)] == 1) {
                    canvas.drawPicture(
                        *icon,
                        {at.x + 174 + (m_services.pictures.picture(*icon).width() + kGap) * (weapon - 1), at.y + 395}
                    ); // weapon icons row starts at (174, 395)
                }
            }
        }
    }

    void SkillWindow::paintTree(gfx::Canvas& canvas, core::Point at) {
        if (const auto need = m_services.pictures.find("Icon_SkillNeed")) {
            canvas.drawPicture(*need, {at.x + 27, at.y + 374}); // "needs" icon position
        }
        if (shownPet()) {
            return;
        }
        const int element = m_status.element;
        if (element < 1 || element > 4) { // the four elements have a tree
            return;
        }
        if (const auto tree = m_services.pictures.find("form_skillexpress" + std::to_string(element))) {
            canvas.drawPicture(*tree, {at.x + 69, at.y + 87}); // tree picture position
        }
        for (int cell = 1; cell <= kSkillTreeCells; ++cell) {
            const SkillTreeCell& entry = skillTreeCell(element, cell);
            bool learnedHere = false;
            for (const auto& [key, own] : m_status.skills) {
                const auto skill = m_services.skills.findByKey(key);
                if (own.level > 0 && skill && skill->tree == element && skill->prerequisite == 0 && skill->id != kRoleSkillPlaceholder &&
                    skill->id == entry.skillId) {
                    learnedHere = true;
                }
            }
            const auto skill = m_services.skills.find(entry.skillId);
            if (learnedHere && skill) {
                if (const auto icon = m_services.pictures.find("Icon_sk" + std::to_string(skill->iconId) + "s")) {
                    canvas.drawPicture(*icon, at + skillTreeCellPosition(cell));
                }
            }
        }
        const core::Point mouse = m_screen.mouse();
        for (int cell = 1; cell <= kSkillTreeCells; ++cell) {
            const core::Point position = at + skillTreeCellPosition(cell);
            if (mouse.x < position.x || mouse.y < position.y || mouse.x >= position.x + m_treeCellSpot.size().width ||
                mouse.y >= position.y + m_treeCellSpot.size().height) {
                continue;
            }
            const SkillTreeCell& entry = skillTreeCell(element, cell);
            const auto skill = m_services.skills.find(entry.skillId);
            if (!skill) {
                break;
            }
            std::string needs;
            for (std::size_t stat = 0; stat < entry.required.size(); ++stat) {
                if (entry.required[stat] != 0) {
                    needs += (needs.empty() ? "" : ", ") + std::string(kStatNames[stat]) + " " + std::to_string(entry.required[stat]);
                }
            }
            // name at (27, 339) wrapped at 20 characters; description at (27, 359) wrapped at 29; needs at (160, 395); line height 15 px
            canvas.drawText({at.x + 27, at.y + 339}, ui::wrapTextLines(skill->name, 20), plain(m_infoSpot.color()), 160, 15);
            canvas.drawText({at.x + 27, at.y + 359}, ui::wrapTextLines(skill->description, 29), plain(m_infoSpot.color()), 232, 15);
            canvas.drawText({at.x + 160, at.y + 395}, needs.empty() ? "None" : needs, plain(m_infoSpot.color()), 120, 15);
            break;
        }
    }

}
