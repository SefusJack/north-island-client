#include "client/viewmodels/battle/BattleSkillWindow.h"

#include "client/rules/SkillUse.h"
#include "client/rules/Skills.h"
#include "ui/DrawingArea.h"
#include "ui/WrappedTooltip.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kNameColumns = 26; // character columns a skill name is padded to in the row text
        constexpr int kTipChars = 16; // tooltip width, characters per line
        constexpr int kTipLineHeight = 16; // tooltip line height, px
        constexpr int kCharWidth = 8; // width of one character, px
        constexpr int kScreenWidth = 800; // game screen width, px

        /// @brief Loads BattleSkillWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "BattleSkill" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BattleSkillWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BattleSkill");
        }

        /// @brief Joins a skill name and its SP cost, the SP right-aligned so both fill the name columns.
        /// @details Original: Format('%*s').
        /// @param name Skill name.
        /// @param spCost SP cost.
        /// @return The padded text.
        std::string withSp(const std::string& name, int spCost) {
            std::string sp = std::to_string(spCost) + " sp";
            const int width = kNameColumns - static_cast<int>(name.size());
            if (width > static_cast<int>(sp.size())) {
                sp.insert(0, static_cast<std::size_t>(width) - sp.size(), ' ');
            }
            return name + sp;
        }

        /// @brief Puts a skill first in a recent list; one already there moves up.
        /// @details Original: PushRecentBattleSkill.
        /// @param recent The recent list.
        /// @param skillId Skill id.
        void pushFront(std::array<int, 3>& recent, int skillId) {
            const auto found = std::find(recent.begin(), recent.end(), skillId);
            if (found == recent.begin()) {
                return;
            }
            const auto end = found != recent.end() ? found : recent.end() - 1;
            std::move_backward(recent.begin(), end, end + 1);
            recent[0] = skillId;
        }

    }

    BattleSkillWindow::BattleSkillWindow(
        GameServices& services,
        ui::Screen& screen,
        Battle& battle,
        const PlayerStatus& status,
        const PetTeam& pets,
        const CharacterLook& look
    )
        : m_services(services),
          m_screen(screen),
          m_battle(battle),
          m_status(status),
          m_pets(pets),
          m_look(look),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("BattleSkillList")),
          m_recent(screen.get<ui::BlockList>("BattleSkillRecent")),
          m_textColorSpot(screen.get<ui::Spot>("BattleSkillTextColor")),
          m_levelColorSpot(screen.get<ui::Spot>("BattleSkillLevelColor")) {
        screen.get<ui::DrawingArea>("BattleSkillPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onBattleSkillPick", [this](const ui::EventArgs& args) { pick(m_listed, args.index); });
        screen.on("onBattleSkillRecent", [this](const ui::EventArgs& args) { pick(m_recentListed, args.index); });
        screen.on("onBattleSkillClose", [this](const ui::EventArgs&) { hide(); });
        screen.on("onBattleSkillUp", [this](const ui::EventArgs&) {
            m_list.pageUp();
            (petCommands() ? m_petTop : m_playerTop) = m_list.topIndex();
        });
        screen.on("onBattleSkillDown", [this](const ui::EventArgs&) {
            m_list.pageDown();
            (petCommands() ? m_petTop : m_playerTop) = m_list.topIndex();
        });
    }

    bool BattleSkillWindow::petCommands() const {
        const auto actor = m_battle.field().fighter(m_battle.actor());
        return actor && actor->get().kind == 4; // fighter kind 4 = pet
    }

    void BattleSkillWindow::toggle() {
        if (m_window.isVisible()) {
            hide();
            return;
        }
        if (m_battle.actor() < 1) {
            return;
        }
        fill();
        fillRecent();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void BattleSkillWindow::update() {
        if (m_window.isVisible() && (!m_battle.isOn() || m_battle.actor() < 1 || m_battle.chosenSkill() != 0)) {
            hide();
        }
    }

    void BattleSkillWindow::setRecent(const std::array<int, 3>& skills) {
        m_playerRecent = skills;
        m_petRecent = {};
        m_list.clear();
        m_recent.clear();
        m_listed.clear();
        m_recentListed.clear();
    }

    void BattleSkillWindow::pushRecent(int skillId, bool pet) {
        if (isRoleSkill(skillId)) {
            skillId = kRoleSkillPlaceholder;
        }
        if (!m_services.skills.find(skillId)) {
            return;
        }
        if (!pet) {
            if (isBattleListSkill(m_services.skills, skillId)) {
                pushFront(m_playerRecent, skillId);
            }
            return;
        }
        const int place = m_pets.activePlace();
        const auto& active = place != 0 ? m_pets.at(place) : std::nullopt;
        if (active && std::find(active->info.skills.begin(), active->info.skills.end(), skillId) != active->info.skills.end()) {
            pushFront(m_petRecent, skillId);
        }
    }

    int BattleSkillWindow::levelOf(int skillId, bool pet) const {
        if (pet) {
            const int place = m_pets.activePlace();
            const auto& active = place != 0 ? m_pets.at(place) : std::nullopt;
            if (!active) {
                return 0;
            }
            for (std::size_t j = 0; j < active->info.skills.size(); ++j) {
                if (active->info.skills[j] == skillId) {
                    return active->skillLevels[j];
                }
            }
            return 0;
        }
        const auto skill = m_services.skills.find(skillId);
        if (!skill) {
            return 0;
        }
        const auto learnt = m_status.skills.find(skill->key);
        return learnt != m_status.skills.end() ? learnt->second.level : 0;
    }

    ui::BlockList::Row BattleSkillWindow::makeRow(int skillId, int level) const {
        const int shownId = skillId == kRoleSkillPlaceholder ? roleSkillId(m_look.bodyType, m_look.head) : skillId;
        const auto skill = m_services.skills.find(shownId);
        ui::BlockList::Row row;
        if (!skill) {
            return row;
        }
        using Align = ui::BlockList::Align;
        ui::BlockList::PictureBlock icon;
        icon.picture = m_services.pictures.find("Icon_sk" + std::to_string(skill->iconId) + "s").value_or(gfx::PictureId{});
        if (icon.picture.isValid()) {
            const gfx::Picture& picture = m_services.pictures.picture(icon.picture);
            icon.size = {picture.width(), picture.height()};
            icon.area = icon.size;
        }
        row.pictures.push_back(icon);
        // name text: offset (28,0), area 214x30; level text: offset (0,13), area 20x13 (under the icon)
        row.texts.push_back({withSp(skill->name, skill->spCost), {28, 0}, {214, 30}, Align::Centre, Align::Centre, m_textColorSpot.color()});
        row.texts.push_back({std::to_string(level), {0, 13}, {20, 13}, Align::Start, Align::Centre, m_levelColorSpot.color()});
        return row;
    }

    void BattleSkillWindow::fill() {
        m_list.clear();
        m_listed.clear();
        const bool pet = petCommands();
        if (!pet) {
            for (const auto& [key, learnt] : m_status.skills) {
                if (learnt.level == 0) {
                    continue;
                }
                const auto skill = m_services.skills.findByKey(key);
                if (!skill || !isBattleListSkill(m_services.skills, skill->id)) {
                    continue;
                }
                m_list.addRow(makeRow(skill->id, learnt.level));
                m_listed.push_back(skill->id == kRoleSkillPlaceholder ? roleSkillId(m_look.bodyType, m_look.head) : skill->id);
            }
            m_list.setTopIndex(m_playerTop);
            return;
        }
        const int place = m_pets.activePlace();
        const auto& active = place != 0 ? m_pets.at(place) : std::nullopt;
        if (active) {
            for (std::size_t j = 0; j < active->info.skills.size(); ++j) {
                const int skillId = active->info.skills[j];
                if (active->skillLevels[j] > 0 && skillId > 0 && m_services.skills.find(skillId)) {
                    m_list.addRow(makeRow(skillId, active->skillLevels[j]));
                    m_listed.push_back(skillId);
                }
            }
        }
        m_list.setTopIndex(m_petTop);
    }

    void BattleSkillWindow::fillRecent() {
        m_recent.clear();
        m_recentListed.clear();
        const bool pet = petCommands();
        for (const int skillId : pet ? m_petRecent : m_playerRecent) {
            if (skillId == 0) {
                break;
            }
            m_recent.addRow(makeRow(skillId, levelOf(skillId, pet)));
            m_recentListed.push_back(skillId);
        }
    }

    void BattleSkillWindow::markUsable(ui::BlockList& list, const std::vector<int>& skillIds) {
        const auto user = m_battle.actorAsUser();
        const int last = std::min(list.topIndex() + list.visibleRows(), list.rowCount());
        for (int index = list.topIndex(); index < last; ++index) {
            int skillId = skillIds[static_cast<std::size_t>(index)];
            const int shownId = skillId == kRoleSkillPlaceholder ? roleSkillId(m_look.bodyType, m_look.head) : skillId;
            if (isRoleSkill(skillId)) {
                skillId = kRoleSkillPlaceholder;
            }
            const bool usable = user && checkCanUseSkill(m_services.skills, m_services.motions, *user, skillId) == 0;
            const auto skill = m_services.skills.find(shownId);
            if (!skill) {
                continue;
            }
            const std::string name = "Icon_sk" + std::to_string(skill->iconId) + (usable ? "s" : "s_g");
            auto& pictures = list.row(index).pictures;
            if (!pictures.empty()) {
                pictures.front().picture = m_services.pictures.find(name).value_or(gfx::PictureId{});
            }
        }
    }

    void BattleSkillWindow::paint(gfx::Canvas& canvas, core::Point at) {
        markUsable(m_list, m_listed);
        markUsable(m_recent, m_recentListed);
        for (const auto& [list, skillIds] :
             {std::pair<ui::BlockList&, const std::vector<int>&>{m_list, m_listed},
              std::pair<ui::BlockList&, const std::vector<int>&>{m_recent, m_recentListed}}) {
            if (m_screen.hoveredId() != list.id() || list.rowCount() == 0) {
                continue;
            }
            const core::Rect area = list.bounds();
            const int row = list.rowAt(m_screen.mouse() - core::Point{at.x + area.x, at.y + area.y});
            const int index = list.topIndex() + row;
            if (row < 0 || index >= static_cast<int>(skillIds.size())) {
                continue;
            }
            int skillId = skillIds[static_cast<std::size_t>(index)];
            if (skillId == kRoleSkillPlaceholder) {
                skillId = roleSkillId(m_look.bodyType, m_look.head);
            }
            const auto skill = m_services.skills.find(skillId);
            if (!skill) {
                return;
            }
            int tipX = at.x + area.x + area.width + 5; // 5 px gap right of the list
            if (kTipChars * kCharWidth + tipX > kScreenWidth) {
                tipX = at.x + area.x - (kTipChars + 1) * kCharWidth;
            }
            const int tipY = at.y + area.y + list.pitch() * row;
            // tooltip colours (RGBA): text white, background (61,138,249,200), border blue
            ui::drawWrappedTooltip(
                canvas,
                {tipX, tipY},
                skill->description,
                kTipChars,
                kTipLineHeight,
                {255, 255, 255, 255},
                {61, 138, 249, 200},
                {0, 0, 255, 255}
            );
            return;
        }
    }

    void BattleSkillWindow::pick(const std::vector<int>& skillIds, int index) {
        if (index < 0 || index >= static_cast<int>(skillIds.size())) {
            return;
        }
        if (m_battle.useListedSkill(skillIds[static_cast<std::size_t>(index)])) {
            hide();
        }
    }

}
