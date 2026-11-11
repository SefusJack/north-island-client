#include "client/viewmodels/system/RemotePlusWindow.h"

#include "client/rules/Skills.h"
#include "formats/SkillRecord.h"
#include "gfx/Canvas.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kMessageMs = 2000; // message time, ms
        constexpr int kBaseRemote = 34058; // item 34058: Remote Control (the plain one, without the Plus form)
        constexpr int kElectronControl = 34128; // item 34128: Electron Control, which runs a battle script
        constexpr int kKeyCell = 30; // width and height of a slot key, px
        constexpr char kNotSuitable[] = "This skill is not suitable for the target";
        constexpr char kCannotSetItem[] = "This function is unable to set this item";
        constexpr int kNewbieStunt = 15003; // skill 15003: Newbie's Stunt (replaced by the role's own stunt)
        constexpr int kDefence = 60021; // skill 60021: Defense
        constexpr int kFlee = 60041; // skill 60041: Flee
        constexpr int kAttackRight = 60043; // skill id 60043: not in Skill.dat; the Plus form lets it through like the normal attack and defence

        /// @brief Loads RemotePlusWindow.xml into the screen.
        /// @param services Shared game services (layout path and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The RemotePlusWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("RemotePlusWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("RemotePlusWindow");
        }

        /// @brief Reads the number a control name ends in (PlusKey3 gives 3).
        /// @param name Control name.
        /// @param prefix Leading part of the name to skip.
        /// @return The trailing number.
        int numberAfter(const std::string& name, const std::string& prefix) { return std::stoi(name.substr(prefix.size())); }

        /// @brief Tells whether a skill is one of the stunts the first tab turns into the role's own.
        /// @details Original: DropSkillOnAutoPlaySlot.
        /// @param skillId Skill id.
        /// @return True for a stunt.
        bool isStunt(int skillId) {
            switch (skillId) {
                case 11075: // Summon Dogs Groups
                case 11076: // Combo x3 Attack
                case 11077: // Cure 2 Players
                case 11078: // Bomber Attack
                case 11182: // Ghost Hammer
                case 11183: // Deacon Attack
                case 12036: // Leap
                case 12049: // Fire Dance
                case 12051: // Note
                case 12053: // Gallop
                case 15038: // Overarm Stumble
                case 15039: // Wine Flame
                case 15040: // Palm
                case 15041: // Love Wish
                case 15060: // Throw Dish
                    return true;
                default: // any other skill is not a stunt
                    return false;
            }
        }

    }

    RemotePlusWindow::RemotePlusWindow(
        GameServices& services,
        ui::Screen& screen,
        RemoteControl& remote,
        const PetTeam& pets,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_remote(remote),
          m_pets(pets),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_keys{
              screen.get<ui::DrawingArea>("PlusKey1"),
              screen.get<ui::DrawingArea>("PlusKey1"),
              screen.get<ui::DrawingArea>("PlusKey2"),
              screen.get<ui::DrawingArea>("PlusKey3"),
              screen.get<ui::DrawingArea>("PlusKey4"),
              screen.get<ui::DrawingArea>("PlusKey5"),
              screen.get<ui::DrawingArea>("PlusKey6"),
              screen.get<ui::DrawingArea>("PlusKey7"),
              screen.get<ui::DrawingArea>("PlusKey8"),
              screen.get<ui::DrawingArea>("PlusKey9")
          },
          m_skillRoles{
              screen.get<ui::ImageButton>("PlusSkillRole1"),
              screen.get<ui::ImageButton>("PlusSkillRole1"),
              screen.get<ui::ImageButton>("PlusSkillRole2"),
              screen.get<ui::ImageButton>("PlusSkillRole3"),
              screen.get<ui::ImageButton>("PlusSkillRole4")
          },
          m_healRoles{screen.get<ui::ImageButton>("PlusHealRole1"), screen.get<ui::ImageButton>("PlusHealRole1"), screen.get<ui::ImageButton>("PlusHealRole2")},
          m_skillMenu{
              screen.get<ui::ImageButton>("PlusSkillMenu1"),
              screen.get<ui::ImageButton>("PlusSkillMenu1"),
              screen.get<ui::ImageButton>("PlusSkillMenu2"),
              screen.get<ui::ImageButton>("PlusSkillMenu3")
          },
          m_healMenu{
              screen.get<ui::ImageButton>("PlusHealMenu1"),
              screen.get<ui::ImageButton>("PlusHealMenu1"),
              screen.get<ui::ImageButton>("PlusHealMenu2"),
              screen.get<ui::ImageButton>("PlusHealMenu3")
          },
          m_tabs{
              screen.get<ui::ImageButton>("PlusTab1"),
              screen.get<ui::ImageButton>("PlusTab1"),
              screen.get<ui::ImageButton>("PlusTab2"),
              screen.get<ui::ImageButton>("PlusTab3")
          },
          m_checks{
              screen.get<ui::CheckBox>("PlusCheck1"),
              screen.get<ui::CheckBox>("PlusCheck1"),
              screen.get<ui::CheckBox>("PlusCheck2"),
              screen.get<ui::CheckBox>("PlusCheck3"),
              screen.get<ui::CheckBox>("PlusCheck3"),
              screen.get<ui::CheckBox>("PlusCheck5"),
              screen.get<ui::CheckBox>("PlusCheck6")
          },
          m_values{
              screen.get<ui::TextBox>("PlusValue1"),
              screen.get<ui::TextBox>("PlusValue1"),
              screen.get<ui::TextBox>("PlusValue2"),
              screen.get<ui::TextBox>("PlusValue3")
          },
          m_compare(screen.get<ui::ImageButton>("PlusCompare")),
          m_cancel(screen.get<ui::ImageButton>("PlusCancel")),
          m_scriptBack(screen.get<ui::DrawingArea>("PlusScriptBack")),
          m_scriptText(screen.get<ui::Label>("PlusScriptText")),
          m_scriptPath(screen.get<ui::TextBox>("PlusScriptPath")),
          m_scriptDebug(screen.get<ui::CheckBox>("PlusScriptDebug")),
          m_scriptDebugText(screen.get<ui::Label>("PlusScriptDebugText")) {
        const auto find = [&](const char* name) { return services.pictures.find(name).value_or(gfx::PictureId{}); };
        // four role pictures Btn_Role_1..4
        for (int role = 1; role <= 4; ++role) {
            m_rolePictures[static_cast<std::size_t>(role)] = find(("Btn_Role_" + std::to_string(role)).c_str());
        }
        m_smaller = find("Btn_Smaller");
        m_surpass = find("Btn_Surpass");
        m_to = find("Icon_To");
        m_frame = find("Icon_RoleSelectFrame");
        m_sets[1] = AutoBattleSettings::defaults();
        m_sets[2] = AutoBattleSettings::defaults();
        m_tabs[1].get().setSelected(true);
        for (std::size_t value = 1; value <= 3; ++value) { // the three value boxes
            m_values[value].get().setText("0");
        }
        screen.get<ui::DrawingArea>("PlusOverlay").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintOverlay(canvas, at); });
        m_scriptBack.setPainter([](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            canvas.fillRect({at.x, at.y, 194, 24}, {222, 239, 255}); // the form's light blue over the help line, 194 x 24 px
            canvas.fillRect({at.x + 43, at.y + 2, 94, 20}, {148, 182, 206}); // the field's frame, in the form's border blue
            canvas.fillRect({at.x + 44, at.y + 3, 92, 18}, {255, 255, 255}); // the field, under the path box
        });
        for (int slot = 1; slot <= 9; ++slot) { // the nine key slots
            m_keys[static_cast<std::size_t>(slot)].get().setPainter([this, slot](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintSlot(canvas, at, slot);
            });
        }
        screen.on("onPlusClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onPlusOk", [this](const ui::EventArgs&) { save(); });
        screen.on("onPlusCancel", [this](const ui::EventArgs&) {
            loadFromPlayer();
            showScriptChoice();
            showTab(m_tab);
            m_modified = false;
        });
        screen.on("onPlusTab", [this](const ui::EventArgs& args) { selectTab(numberAfter(args.source, "PlusTab")); });
        screen.on("onPlusCheck", [this](const ui::EventArgs& args) { toggleOption(numberAfter(args.source, "PlusCheck")); });
        screen.on("onPlusSkillRole", [this](const ui::EventArgs& args) { pickTargetRole(numberAfter(args.source, "PlusSkillRole")); });
        screen.on("onPlusHealRole", [this](const ui::EventArgs& args) {
            pickTargetRole(numberAfter(args.source, "PlusHealRole") + 10);
        }); // heal roles are selections 11 and 12
        screen.on("onPlusTarget", [this](const ui::EventArgs& args) {
            const int number = numberAfter(args.source, args.source.rfind("PlusHealMenu", 0) == 0 ? "PlusHealMenu" : "PlusSkillMenu");
            setTarget(args.source.rfind("PlusHealMenu", 0) == 0 && number == 1 ? 4 : number); // target 4 is the party
        });
        screen.on("onPlusCompare", [this](const ui::EventArgs&) { setCompare(current().faintCompareMode == 1 ? 2 : 1); }); // compare mode 1 smaller, 2 surpass
        screen.on("onPlusKeyDouble", [this](const ui::EventArgs& args) { cycleDefaultAction(numberAfter(args.source, "PlusKey")); });
        screen.on("onPlusKeyClear", [this](const ui::EventArgs& args) { clearSlot(numberAfter(args.source, "PlusKey")); });
    }

    int RemotePlusWindow::numberIn(const ui::TextBox& box) {
        try {
            return std::stoi(box.text());
        } catch (const std::exception&) {
            return 0;
        }
    }

    void RemotePlusWindow::toggle() {
        if (m_window.isVisible()) {
            close();
        } else {
            open();
        }
    }

    void RemotePlusWindow::open() {
        if (m_hooks.remoteItem() == kBaseRemote || !m_hooks.remoteFormOpen()) {
            return;
        }
        loadFromPlayer();
        m_savedScript = m_remote.scriptChoice();
        showScriptChoice();
        showTab(m_tab);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void RemotePlusWindow::close() {
        m_window.setVisible(false);
        loadFromPlayer();
        showScriptChoice();
        showTab(m_tab);
        m_modified = false;
    }

    void RemotePlusWindow::showScriptChoice() {
        m_scriptPath.setText(m_savedScript.path);
        m_scriptDebug.setChecked(m_savedScript.debug);
    }

    void RemotePlusWindow::loadFromPlayer() {
        m_sets[1] = m_remote.playSets()[1];
        m_sets[2] = m_remote.playSets()[2];
    }

    void RemotePlusWindow::save() {
        for (std::size_t set = 1; set <= 2; ++set) { // the two sets (player, pet)
            bool any = false;
            for (std::size_t flag = 1; flag <= 6; ++flag) { // the six option flags
                any = any || m_sets[set].flags[flag] != 0;
            }
            m_sets[set].flags[0] = any ? 1 : 0;
        }
        if (m_hooks.remoteItem() == kBaseRemote) {
            m_remote.playSets() = {};
        } else {
            m_remote.playSets()[1] = m_sets[1];
            m_remote.playSets()[2] = m_sets[2];
        }
        // the typed script path, without the spaces and quotes a pasted path may carry
        std::string path = m_scriptPath.text();
        const auto first = path.find_first_not_of(" \t\"'");
        const auto last = path.find_last_not_of(" \t\"'");
        path = first == std::string::npos ? std::string() : path.substr(first, last - first + 1);
        if (path != m_savedScript.path || m_scriptDebug.isChecked() != m_savedScript.debug) {
            m_savedScript.path = path;
            m_savedScript.debug = m_scriptDebug.isChecked();
            m_remote.setScriptChoice(m_savedScript);
        }
        close();
        checkModified();
    }

    void RemotePlusWindow::selectTab(int tab) {
        for (std::size_t other = 1; other <= 3; ++other) { // the three tabs
            m_tabs[other].get().setSelected(false);
        }
        m_tabs[static_cast<std::size_t>(tab)].get().setSelected(true);
        m_tab = tab;
        showTab(tab);
        checkModified();
    }

    void RemotePlusWindow::checkModified() { m_modified = !(m_sets[1].sameForForm(m_remote.playSets()[1]) && m_sets[2].sameForForm(m_remote.playSets()[2])); }

    void RemotePlusWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.remoteFormOpen()) {
            close();
            return;
        }
        if (m_tab == 1 || m_tab == 2) { // tabs 1 and 2 are the two battle sets (player and pet); other tabs have no set
            if (!m_values[1].get().text().empty()) {
                current().healThresholds[1] = numberIn(m_values[1]);
            }
            if (!m_values[2].get().text().empty()) {
                current().healThresholds[2] = numberIn(m_values[2]);
            }
            if (!m_values[3].get().text().empty()) {
                current().faintHpValue = numberIn(m_values[3]);
            }
        }
        if (!m_modified && m_tab == 3) { // tab 3 holds the script path
            m_modified = m_scriptPath.text() != m_savedScript.path || m_scriptDebug.isChecked() != m_savedScript.debug;
        }
        if (!m_modified && (m_tab == 1 || m_tab == 2)) {
            const AutoBattleSettings& inForce = m_remote.playSets()[static_cast<std::size_t>(m_tab)];
            m_modified = m_values[1].get().text() != std::to_string(inForce.healThresholds[1]) ||
                         m_values[2].get().text() != std::to_string(inForce.healThresholds[2]) ||
                         m_values[3].get().text() != std::to_string(inForce.faintHpValue);
        }
        m_cancel.setVisible(m_modified);
    }

    void RemotePlusWindow::setRolePicture(ui::ImageButton& button, int role) {
        if (role >= 1 && role <= 4) {
            button.setPicture(m_rolePictures[static_cast<std::size_t>(role)]);
        }
    }

    gfx::PictureId RemotePlusWindow::skillIcon(int skillId, bool& found) {
        std::string name;
        // 10001 is the normal attack; its icon is sk60022
        if (skillId == 10001) {
            name = "Icon_sk60022s";
        } else if (skillId == kDefence) {
            name = "Icon_sk60021s"; // icon id of the defence skill
        } else if (skillId == kFlee) {
            name = "Icon_sk60041s"; // icon id of the flee skill
        } else {
            const auto skill = m_services.skills.find(skillId);
            name = "icon_sk" + std::to_string(skill ? skill->iconId : 0) + "s";
        }
        const auto picture = m_services.pictures.find(name);
        found = picture.has_value();
        return picture.value_or(gfx::PictureId{});
    }

    gfx::PictureId RemotePlusWindow::itemIcon(int itemId, bool& found) {
        const auto item = m_services.items.find(itemId);
        const auto picture = item ? m_services.pictures.find(std::to_string(item->iconId)) : std::nullopt;
        found = picture.has_value();
        return picture.value_or(gfx::PictureId{});
    }

    void RemotePlusWindow::showTab(int tab) {
        const bool first = tab == 1 || tab == 2; // tabs 1 and 2 use the first form picture (the battle set pages)
        m_window.setPicture(first ? "Form_AutoPlayerPlus_1" : "Form_AutoPlayerPlus_2", m_services.pictures);
        for (std::size_t role = 1; role <= 4; ++role) { // four skill role buttons
            m_skillRoles[role].get().setVisible(false);
        }
        for (std::size_t option = 1; option <= 6; ++option) { // six options; option 4 has no box of its own
            if (option != 4) {
                m_checks[option].get().setVisible(first);
            }
        }
        for (std::size_t value = 1; value <= 3; ++value) { // three value boxes
            m_values[value].get().setVisible(first);
        }
        for (std::size_t heal = 1; heal <= 2; ++heal) { // two heal role buttons
            m_healRoles[heal].get().setVisible(first);
        }
        m_compare.setVisible(first);
        const bool script = tab == 3 && m_hooks.remoteItem() == kElectronControl; // tab 3 of the Electron Control has the script path
        m_scriptBack.setVisible(script);
        m_scriptText.setVisible(script);
        m_scriptPath.setVisible(script);
        m_scriptDebug.setVisible(script);
        m_scriptDebugText.setVisible(script);
        for (std::size_t slot = 1; slot <= 9; ++slot) { // nine key slots
            m_keys[slot].get().setVisible(first);
        }
        if (!first) {
            checkModified();
            return;
        }
        const AutoBattleSettings& sets = m_sets[static_cast<std::size_t>(tab)];
        for (std::size_t proc = 1; proc <= 3; ++proc) { // three proc skills
            m_skillRoles[proc].get().setVisible(sets.procSkillIds[proc] != 0);
            setRolePicture(m_skillRoles[proc], sets.procSkillTargets[proc]);
        }
        m_skillRoles[4].get().setVisible(sets.faintActionId != 0); // role button 4 belongs to the faint action
        setRolePicture(m_skillRoles[4], sets.faintTarget);
        for (std::size_t option = 1; option <= 6; ++option) { // six options; option 4 has no box of its own
            if (option != 4) {
                m_checks[option].get().setChecked(sets.flags[option] != 0);
            }
        }
        for (std::size_t heal = 1; heal <= 2; ++heal) { // two heal thresholds
            m_values[heal].get().setText(std::to_string(sets.healThresholds[heal]));
            setRolePicture(m_healRoles[heal], sets.healTargets[heal]);
        }
        m_values[3].get().setText(std::to_string(sets.faintHpValue)); // value box 3 holds the faint HP value
        setCompare(sets.faintCompareMode);
        const auto shown = [&](int slot, int kind, int id, bool itemIcons) {
            if (id == 0) {
                setSlotIcon(slot, 0, 0, std::nullopt, true);
                return;
            }
            bool found = false;
            const gfx::PictureId icon = itemIcons ? itemIcon(id, found) : skillIcon(id, found);
            setSlotIcon(slot, kind, id, icon, false);
        };
        for (int key = 1; key <= 2; ++key) { // slots 1 and 2 are the default skills; kind 2 is a skill
            shown(key, 2, sets.defaultSkillIds[static_cast<std::size_t>(key)], false);
        }
        for (int proc = 1; proc <= 3; ++proc) { // proc skills fill slots 3..5, heals slots 6..8
            shown(proc + 2, 2, sets.procSkillIds[static_cast<std::size_t>(proc)], sets.procSkillIsItem[static_cast<std::size_t>(proc)] == 1);
            shown(
                proc + 5,
                sets.healIsItem[static_cast<std::size_t>(proc)],
                sets.healActionIds[static_cast<std::size_t>(proc)],
                sets.healIsItem[static_cast<std::size_t>(proc)] == 1
            );
        }
        shown(9, 2, sets.faintActionId, false); // slot 9 is the faint action; kind 2 is a skill
        checkModified();
    }

    void RemotePlusWindow::toggleOption(int option) {
        if (m_tab == 1 || m_tab == 2) {
            current().flags[static_cast<std::size_t>(option)] ^= 1;
        }
        m_checks[static_cast<std::size_t>(option)].get().setChecked(current().flags[static_cast<std::size_t>(option)] != 0);
        // option 3 is the HP tick, option 4 the SP tick it carries
        if (option == 3) {
            current().flags[4] = current().flags[3];
        }
        checkModified();
    }

    core::Point RemotePlusWindow::roleButtonPosition(int selection) const {
        // selections above 10 are the heal role buttons (11, 12)
        const core::Rect bounds = selection > 10 ? m_healRoles[static_cast<std::size_t>(selection - 10)].get().bounds()
                                                 : m_skillRoles[static_cast<std::size_t>(selection)].get().bounds();
        return {bounds.x, bounds.y};
    }

    void RemotePlusWindow::pickTargetRole(int selection) {
        m_menuAt = roleButtonPosition(selection);
        // selections above 10 (11, 12) open the heal menu (kind 2), 1..4 the skills' menu (kind 1)
        if (selection > 10) {
            if (m_menuShown) {
                hideTargetMenu(2);
            } else {
                if (selection == 12) { // selection 12: the second heal role button hides the first heal role
                    m_healRoles[1].get().setVisible(false);
                }
                showTargetMenu(2);
            }
        } else if (m_menuShown) {
            hideTargetMenu(1);
        } else {
            showTargetMenu(1);
        }
        m_selection = selection;
    }

    void RemotePlusWindow::showTargetMenu(int kind) {
        m_menuShown = true;
        m_menuKind = kind;
        for (std::size_t role = 1; role <= 3; ++role) { // three menu buttons
            const int y = static_cast<int>(role - 1) * 20; // menu buttons are 20 pixels apart
            ui::ImageButton& button = kind == 1 ? m_skillMenu[role].get() : m_healMenu[role].get();
            // menu offset from the role button: skills' menu 46 right and 40 up, heal menu 63 up
            button.setPosition(kind == 1 ? core::Point{m_menuAt.x + 46, y + m_menuAt.y - 40} : core::Point{m_menuAt.x, y + m_menuAt.y - 63});
            button.setVisible(true);
        }
    }

    void RemotePlusWindow::hideTargetMenu(int mode) {
        m_menuShown = false;
        for (std::size_t role = 1; role <= 3; ++role) { // three menu buttons
            if (mode == 0 || mode == 1) {
                m_skillMenu[role].get().setVisible(false);
            }
            if (mode == 0 || mode == 2) {
                m_healMenu[role].get().setVisible(false);
            }
        }
        if (mode != 1) {
            m_healRoles[1].get().setVisible(true);
        }
    }

    void RemotePlusWindow::notSuitable() { m_messages.show(kNotSuitable, kMessageMs); }

    void RemotePlusWindow::setTarget(int target) {
        hideTargetMenu(0);
        if (m_tab != 1 && m_tab != 2) {
            checkModified();
            return;
        }
        AutoBattleSettings& sets = current();
        const auto targetTypeOf = [&](int skillId) {
            const auto skill = m_services.skills.find(skillId);
            return skill ? skill->targetType : 0;
        };
        const int slot = m_selection;
        // selections 1..3 are the proc skill roles, 4 the faint action, 11..13 the heal roles; targets 1 enemy, 2 player, 3 pet, 4 party
        if (slot >= 1 && slot <= 3) {
            const auto index = static_cast<std::size_t>(slot);
            if (sets.procSkillIsItem[index] != 1) {
                const int type = targetTypeOf(sets.procSkillIds[index]);
                // skill target types: 0 enemy only, 1 not an enemy, 3 and 5 the player or the pet by tab
                const bool refused = type == 0   ? target != 1
                                     : type == 1 ? target == 1
                                     : type == 3 ? (m_tab == 1 ? target != 2 : target != 3)
                                     : type == 5 ? (m_tab == 1 ? target != 3 : target != 2)
                                                 : false;
                if (refused) {
                    notSuitable();
                    return;
                }
            } else if (target == 1) {
                notSuitable();
                return;
            }
            sets.procSkillTargets[index] = target;
        } else if (slot == 4) {
            const int type = targetTypeOf(sets.faintActionId);
            // skill target types: 0 enemy only, 1 not an enemy, 3 the player, 5 the player or the pet by tab
            const bool refused = type == 0   ? target != 1
                                 : type == 1 ? target == 1
                                 : type == 3 ? target != 2
                                 : type == 5 ? (m_tab == 1 ? target != 3 : target != 2)
                                             : false;
            if (refused) {
                notSuitable();
                return;
            }
            sets.faintTarget = target;
        } else if (slot >= 11 && slot <= 13) { // heal role selections
            const auto index = static_cast<std::size_t>(slot - 10);
            if (sets.healActionIds[index] != 0) {
                // healIsItem 2 is a skill, 1 an item
                if (sets.healIsItem[index] == 2) {
                    const int type = targetTypeOf(sets.healActionIds[index]);
                    if (type == 5) {
                        if ((m_tab == 1 && target == 2) || (m_tab == 2 && target == 3)) {
                            notSuitable();
                            return;
                        }
                    } else if (type != 0 && type != 1 && type != 2 && type != 3) {
                        return;
                    }
                } else if (sets.healIsItem[index] != 1) {
                    return;
                }
            }
            sets.healTargets[index] = target;
        }
        // slots up to 10 are skill roles, above are heal roles; targets up to 3 have a role picture
        if (slot <= 10) {
            if (target <= 3) {
                setRolePicture(m_skillRoles[static_cast<std::size_t>(slot)], target);
            }
        } else if (target != 1) {
            setRolePicture(m_healRoles[static_cast<std::size_t>(slot - 10)], target);
        }
        checkModified();
    }

    void RemotePlusWindow::setCompare(int mode) {
        m_compare.setPicture(mode == 1 ? m_smaller : m_surpass); // mode 1 smaller, otherwise surpass
        current().faintCompareMode = mode;
    }

    void RemotePlusWindow::clearSlot(int slot) {
        if (slot < 1 || slot > 9) { // slots are 1..9
            return;
        }
        AutoBattleSettings& sets = current();
        Slot& shown = m_slots[static_cast<std::size_t>(slot)];
        shown = {};
        m_keys[static_cast<std::size_t>(slot)].get().setHint({});
        // slots 1..2 default skills, 3..5 proc skills, 6..8 heals, 9 the faint action
        if (slot <= 2) {
            sets.defaultSkillIds[static_cast<std::size_t>(slot)] = 0;
        } else if (slot <= 5) {
            m_skillRoles[static_cast<std::size_t>(slot - 2)].get().setVisible(false);
            sets.procSkillIds[static_cast<std::size_t>(slot - 2)] = 0;
            sets.procSkillTargets[static_cast<std::size_t>(slot - 2)] = 0;
        } else if (slot <= 8) {
            sets.healIsItem[static_cast<std::size_t>(slot - 5)] = 0;
            sets.healActionIds[static_cast<std::size_t>(slot - 5)] = 0;
        } else {
            m_skillRoles[4].get().setVisible(false); // role button 4 belongs to the faint action
            sets.faintActionId = 0;
            sets.faintTarget = 0;
        }
        checkModified();
    }

    void RemotePlusWindow::cycleDefaultAction(int slot) {
        if (slot < 1 || slot > 9) { // slots are 1..9
            return;
        }
        int id = 10001; // skill id of the normal attack
        const char* hint = "Normal attack";
        switch (m_slots[static_cast<std::size_t>(slot)].defaultAction) {
            case 1: // the slot shows the normal attack: next is defence
                id = kDefence;
                hint = "Defence";
                break;
            case 2: // the slot shows defence: next is flee
                id = kFlee;
                hint = "Flee";
                break;
            default: // anything else: next is the normal attack
                break;
        }
        bool found = false;
        const gfx::PictureId icon = skillIcon(id, found);
        setSlotIcon(slot, 2, id, icon, false); // kind 2 is a skill
        storeSlot(slot, 2, id, false); // kind 2 is a skill
        m_slots[static_cast<std::size_t>(slot)].hint = hint;
        m_keys[static_cast<std::size_t>(slot)].get().setHint(hint);
        m_modified = true;
    }

    bool RemotePlusWindow::canSet(int slot, int kind, int id) {
        // kind 1 is an item, kind 2 a skill
        if (slot == 0 || id == 0 || (kind != 1 && kind != 2)) {
            return false;
        }
        const auto skill = kind == 2 ? m_services.skills.find(id) : std::nullopt;
        const auto item = kind == 1 ? m_services.items.find(id) : std::nullopt;
        if ((kind == 2 && !skill) || (kind == 1 && !item)) {
            return false;
        }
        const auto refuse = [&] {
            m_messages.show(kCannotSetItem, kMessageMs);
            return false;
        };
        const auto healing = [&] {
            for (std::size_t i = 0; i < 2; ++i) { // the item's two attributes
                // item sub types 118 and 151 are not supplies; 25 and 123 heal fully
                if (item->subType == 118 || item->subType == 151) {
                    return false;
                }
                if (item->subType == 25 || item->subType == 123) {
                    return true;
                }
                // attribute kinds 25 and 26 are HP and SP; a value over 100 heals
                if ((item->attributeKinds[i] == 25 || item->attributeKinds[i] == 26) && item->attributeValues[i] > 100) {
                    return true;
                }
            }
            return false;
        };
        bool allowed = false;
        switch (slot) {
            case 1: // slot 1: default attack skill
            case 2: // slot 2: default attack skill
                if (kind != 2) {
                    return refuse();
                }
                // skill target types 0 and 2 are attacks; skill type 8 is a seal
                if (skill->targetType != 0 && skill->targetType != 2 && id != kDefence) {
                    m_messages.show("Unable to set non-attack skill in this stall", kMessageMs);
                    return false;
                }
                allowed = true;
                break;
            case 3: // slot 3: proc skill, anything that is not a seal (skill type 8)
            case 4: // slot 4: proc skill
            case 5: // slot 5: proc skill
                allowed = kind != 2 ? healing() : skill->skillType != 8;
                break;
            case 6: // slot 6: heal supply
            case 7: // slot 7: heal supply
                allowed = kind != 2 ? healing() : skill->skillType != 8;
                break;
            case 8: // slot 8: the party's death (item sub type 118, or a seal, defence or attack skill)
                if (kind != 2) {
                    if (item->subType != 118) {
                        return refuse();
                    }
                    allowed = true;
                } else {
                    allowed = skill->skillType == 8 || id == kDefence || skill->targetType == 0;
                }
                break;
            case 9: // slot 9: the faint action, a skill that is not a seal
                if (kind != 2) {
                    return refuse();
                }
                allowed = skill->skillType != 8;
                break;
            default: // any other slot takes nothing
                return false;
        }
        if (!allowed) {
            return refuse();
        }
        return true;
    }

    bool RemotePlusWindow::setSlotIcon(
        int slot,
        int kind,
        int id,
        std::optional<gfx::PictureId> icon,
        bool noMove
    ) {
        if (slot < 1 || slot > 9) { // slots are 1..9
            return false;
        }
        std::string hint;
        int offset = 0;
        bool changed = false;
        // kind 1 is an item, kind 2 a skill
        if (id != 0 && (kind == 1 || kind == 2)) {
            if (kind == 1) {
                const auto item = m_services.items.find(id);
                if (item) {
                    for (std::size_t i = 0; i < 2; ++i) { // the item's two attributes
                        // item sub type 123 restores HP and SP fully, 25 HP fully
                        if (item->subType == 123) {
                            hint = " HP/SP: Max";
                            break;
                        }
                        if (item->subType == 25) {
                            hint = " HP: Max";
                            break;
                        }
                        std::string name;
                        // attribute kind 250 ends the hint; 25 is HP, 26 is SP
                        if (item->attributeKinds[i] == 250) {
                            break;
                        }
                        if (item->attributeKinds[i] == 25) {
                            name = "HP";
                        } else if (item->attributeKinds[i] == 26) {
                            name = "SP";
                        }
                        if (i == 1 && !name.empty()) {
                            hint += ",";
                        }
                        const int value = item->attributeValues[i];
                        // 100 is the neutral attribute value; above it is a bonus, below a penalty
                        if (value != 0 && value != 100) {
                            hint += value - 100 > 0 ? " " + name + "+" + std::to_string(value - 100) : " " + name + "-" + std::to_string(100 - value);
                        }
                    }
                    hint = item->name + hint;
                }
                offset = -1; // an item's key is one pixel out on each side
            } else {
                const auto skill = m_services.skills.find(id);
                // 10001 is the normal attack
                if (id == 10001) {
                    hint = "Normal attack";
                } else if (id == kDefence) {
                    hint = "Defence";
                } else if (id == kFlee) {
                    hint = "Flee";
                } else {
                    hint = (skill ? skill->name : std::string()) + " SP:" + std::to_string(skill ? skill->spCost : 0);
                }
            }
            changed = true;
        }
        core::Point at;
        switch (slot) {
            case 1: // slot 1: x 162, y 91, rows 32 pixels apart
            case 2: // slot 2: x 162, y 123
                // position on the form in pixels
                at = {162, (slot - 1) * 32 + 91};
                break;
            case 3: // slot 3: x 114 set or 106 empty, y 177, rows 33 pixels apart
            case 4: // slot 4: y 210
            case 5: // slot 5: y 243
                // position on the form in pixels
                at = {changed && !noMove ? 114 : 106, (slot - 3) * 33 + 177};
                break;
            case 6: // slot 6: x 187, y 269, rows 36 pixels apart
            case 7: // slot 7: y 305
                // position on the form in pixels
                at = {187, (slot - 6) * 36 + 269};
                break;
            case 8: // slot 8: the party's death slot
                // position on the form in pixels
                at = {162, 410};
                break;
            default: // slot 9: the faint action slot
                // position on the form in pixels
                at = {92, 375};
                break;
        }
        Slot& shown = m_slots[static_cast<std::size_t>(slot)];
        ui::DrawingArea& key = m_keys[static_cast<std::size_t>(slot)].get();
        const bool set = changed && !noMove;
        const int shift = set ? offset : 0;
        key.setBounds({at.x + shift, at.y + shift, kKeyCell - shift * 2, kKeyCell - shift * 2}); // the shift applies to both sides
        shown.kind = kind;
        shown.id = set ? id : 0;
        shown.hasIcon = set && icon.has_value();
        shown.icon = shown.hasIcon ? *icon : gfx::PictureId{};
        shown.hint = set ? hint : std::string();
        key.setHint(shown.hint);
        shown.defaultAction = 0;
        // defaultAction 1 normal attack (10001), 2 defence, 3 flee
        if (set) {
            if (id == 10001) {
                shown.defaultAction = 1;
            } else if (id == kDefence) {
                shown.defaultAction = 2;
            } else if (id == kFlee) {
                shown.defaultAction = 3;
            }
        }
        // slots 3..5 show role buttons 1..3, slot 9 shows role button 4
        if (slot >= 3 && slot <= 5) {
            m_skillRoles[static_cast<std::size_t>(slot - 2)].get().setVisible(set);
        } else if (slot == 9) {
            m_skillRoles[4].get().setVisible(set);
        }
        return set;
    }

    bool RemotePlusWindow::storeSlot(int slot, int kind, int id, bool noStore) {
        if (id == 0) {
            return false;
        }
        AutoBattleSettings& sets = current();
        // slots 1..2 default skills, 3..5 proc skills, 6..8 heals, 9 the faint action
        if (noStore) {
            if (slot <= 2) {
                sets.defaultSkillIds[static_cast<std::size_t>(slot)] = 0;
            } else if (slot <= 5) {
                sets.procSkillIds[static_cast<std::size_t>(slot - 2)] = 0;
            } else if (slot <= 8) {
                sets.healIsItem[static_cast<std::size_t>(slot - 5)] = 0;
                sets.healActionIds[static_cast<std::size_t>(slot - 5)] = 0;
            } else {
                sets.faintActionId = 0;
            }
            return true;
        }
        const auto skill = m_services.skills.find(id);
        const int type = skill ? skill->targetType : 0;
        const auto role = [&](ui::ImageButton& button, int value) { setRolePicture(button, value); };
        if (slot <= 2) {
            sets.defaultSkillIds[static_cast<std::size_t>(slot)] = id;
        } else if (slot <= 5) {
            const auto index = static_cast<std::size_t>(slot - 2);
            const auto put = [&](int target) {
                sets.procSkillTargets[index] = target;
                role(m_skillRoles[index], target);
            };
            // kind 2 is a skill; skill target types 0 enemy, 1 and 2 own side, 3 and 5 the player or the pet by tab; targets 1 enemy, 2 player, 3 pet
            if (kind == 2) {
                if (type == 0) {
                    put(1);
                } else if ((type == 1 || type == 2) && sets.procSkillTargets[index] <= 1) {
                    put(2);
                } else if (type == 3) {
                    put(m_tab == 1 ? 2 : 3);
                } else if (type == 5) {
                    put(m_tab == 1 ? 3 : 2);
                }
            } else if (sets.procSkillTargets[index] <= 1) {
                put(2);
            }
            sets.procSkillIsItem[index] = kind;
            sets.procSkillIds[index] = id;
        } else if (slot <= 8) {
            const auto index = static_cast<std::size_t>(slot - 5);
            // kind 2 is a skill, kind 1 an item; skill target type 5 is the player or the pet by tab
            if (kind == 2) {
                if (type == 5) {
                    sets.healTargets[index] = m_tab == 1 ? 3 : 2;
                    role(m_healRoles[index], sets.healTargets[index]);
                }
            } else if (kind != 1) {
                return false;
            }
            sets.healIsItem[index] = kind;
            sets.healActionIds[index] = id;
        } else {
            sets.faintActionId = id;
            // targets 1 enemy, 2 player, 3 pet; skill target types 0 enemy, 1 and 2 and 3 the player, 5 the pet
            const int target = type == 0 ? 1 : (type == 1 || type == 2) ? 2 : type == 3 ? 2 : type == 5 ? 3 : sets.faintTarget;
            sets.faintTarget = target;
            role(m_skillRoles[4], target);
        }
        return true;
    }

    void RemotePlusWindow::dropItem(int slot, int itemId) {
        if (itemId == 0 || !m_services.items.find(itemId)) {
            return;
        }
        if (canSet(slot, 1, itemId)) { // kind 1 is an item
            bool found = false;
            const gfx::PictureId icon = itemIcon(itemId, found);
            // kind 1 is an item
            storeSlot(slot, 1, itemId, !setSlotIcon(slot, 1, itemId, found ? std::optional<gfx::PictureId>(icon) : std::nullopt, false));
        }
        checkModified();
    }

    void RemotePlusWindow::dropSkill(int slot, int skillId) {
        if ((m_tab != 1 && m_tab != 2) || skillId == 0) {
            return;
        }
        // 10001 is the normal attack; tab 1 is the player, tab 2 the pet
        if (skillId != 10001 && skillId != kDefence && skillId != kAttackRight && m_tab == 1) {
            if (isStunt(skillId)) {
                skillId = kNewbieStunt;
            }
            const auto skill = m_services.skills.find(skillId);
            if (!skill || m_hooks.skillLevel(skill->key) <= 0) {
                m_messages.show("Please select the player's spell", kMessageMs);
                return;
            }
            if (skillId == kNewbieStunt) {
                skillId = m_hooks.roleSkill();
            }
            // role skill ids that cannot be set: 17018 Gaea's Guard, 17011 Najas's Memory, 17019 Zavrl's Wing, 17013 Gephil's Mutter
            if (skillId == 17018 || skillId == 17011 || skillId == 17019 || skillId == 17013) {
                notSuitable();
                return;
            }
        }
        if (skillId != 10001 && skillId != kDefence && skillId != kAttackRight && m_tab == 2) {
            bool knows = false;
            for (int place = 1; place <= 4; ++place) { // the four pet places
                const auto& pet = m_pets.at(place);
                if (pet) {
                    for (const int known : pet->info.skills) {
                        knows = knows || known == skillId;
                    }
                }
            }
            if (!knows) {
                m_messages.show("Please select the pet's spell", kMessageMs);
                return;
            }
        }
        // kind 2 is a skill
        if (canSet(slot, 2, skillId)) {
            bool found = false;
            const gfx::PictureId icon = skillIcon(skillId, found);
            storeSlot(slot, 2, skillId, !setSlotIcon(slot, 2, skillId, found ? std::optional<gfx::PictureId>(icon) : std::nullopt, false));
        }
        checkModified();
    }

    void RemotePlusWindow::paintSlot(gfx::Canvas& canvas, core::Point at, int slot) {
        const Slot& shown = m_slots[static_cast<std::size_t>(slot)];
        if (shown.hasIcon && m_keys[static_cast<std::size_t>(slot)].get().isVisible()) {
            canvas.drawPicture(shown.icon, at);
        }
    }

    void RemotePlusWindow::paintOverlay(gfx::Canvas& canvas, core::Point at) {
        if (m_tab != 1 && m_tab != 2) {
            return;
        }
        const AutoBattleSettings& sets = current();
        if (m_menuShown) {
            const core::Point mouse = m_screen.mouse();
            // menu panel offsets from the role button, panel size 49 x 66, button size 43 x 20, frame offsets 43/-43 and -3/-66
            const core::Point menu =
                m_menuKind == 1 ? core::Point{at.x + m_menuAt.x + 43, at.y + m_menuAt.y - 43} : core::Point{at.x + m_menuAt.x, at.y + m_menuAt.y - 63};
            const bool inMenu = core::Rect{menu.x, menu.y, 49, 66}.contains(mouse);
            const bool onButton = core::Rect{at.x + m_menuAt.x, at.y + m_menuAt.y, 43, 20}.contains(mouse);
            if (!inMenu && !onButton) {
                hideTargetMenu(0);
            } else {
                canvas.drawPicture(
                    m_frame,
                    m_menuKind == 1 ? core::Point{at.x + m_menuAt.x + 43, at.y + m_menuAt.y - 43} : core::Point{at.x + m_menuAt.x - 3, at.y + m_menuAt.y - 66}
                );
            }
        }
        for (int proc = 1; proc <= 3; ++proc) { // three proc skills
            if (m_skillRoles[static_cast<std::size_t>(proc)].get().isVisible() && sets.procSkillIds[static_cast<std::size_t>(proc)] != 0) {
                // arrow at x 145, y 185, rows 30 pixels apart
                canvas.drawPicture(m_to, {at.x + 145, at.y + 185 + (proc - 1) * 30});
            }
        }
        if (m_skillRoles[4].get().isVisible() && sets.faintActionId != 0) { // role button 4 belongs to the faint action
            // arrow position of the faint action
            canvas.drawPicture(m_to, {at.x + 133, at.y + 382});
        }
    }

}
