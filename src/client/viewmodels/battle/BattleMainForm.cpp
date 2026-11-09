#include "client/viewmodels/battle/BattleMainForm.h"

#include "client/rules/Experience.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr std::uint64_t kSlideMs = 500; // time for the panels to slide in, ms
        constexpr int kWheelMs = 200; // time for the command buttons to fan out around the fighter, ms
        constexpr int kAutoButton = 5; // command button 5: Auto battle
        constexpr int kPortraitSlot = 0; // portrait slot drawn for a member (slot 0)
        // job icon picture names by job number 1..6 (index 0 unused)
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn_2", "Icon_Samurai_2", "Icon_Knight_2", "Icon_Sage_2", "Icon_Clergyman_2", "Icon_Prophet_2"};
        // command wheel, one entry per command button (index 0 is the top button), offsets {x, y} in px from the fighter's feet:
        // kHomes is where each button rests, kBends is the Bezier control point it curves through, the BattleWheelCentre spot is where the wheel starts
        constexpr std::array<core::Point, 7> kHomes{{{-16, -96}, {28, -74}, {39, -27}, {10, 11}, {-38, 11}, {-68, -27}, {-61, -74}}};
        constexpr std::array<core::Point, 7> kBends{{{-51, -64}, {-16, -71}, {12, -58}, {23, -25}, {-1, 0}, {-34, -4}, {-60, -31}}};
        // PetInfoPainter style: parts offset {19, 0} px, white digits, no exp, no preview offset, battle layout
        const PetInfoPainter::Style kMemberPetStyle{{19, 0}, false, false, {}, true};

        /// @brief Point on a quadratic Bezier curve, truncated to integers.
        /// @details Original: QuadBezierPoint.
        /// @param start Curve start.
        /// @param end Curve end.
        /// @param bend Control point the curve bends towards.
        /// @param total Total number of steps.
        /// @param step Current step.
        /// @return The point at that step.
        core::Point bezier(
            core::Point start,
            core::Point end,
            core::Point bend,
            int total,
            int step
        ) {
            if (step <= 0) {
                return start;
            }
            if (step >= total) {
                return end;
            }
            const double t = static_cast<double>(step) / total;
            const double a = (1 - t) * (1 - t);
            const double b = 2 * t * (1 - t);
            const double c = t * t;
            return {static_cast<int>(std::trunc(a * start.x + c * end.x + bend.x * b)), static_cast<int>(std::trunc(a * start.y + c * end.y + bend.y * b))};
        }

    }

    BattleMainForm::BattleMainForm(
        GameServices& services,
        ui::Screen& screen,
        Battle& battle,
        const PlayerStatus& status
    )
        : m_services(services),
          m_battle(battle),
          m_status(status),
          m_panel(screen.load(services.game.layout("BattleMainForm.xml"), services.loadContext())),
          m_ownInfo(screen.get<ui::Element>("BattleOwnInfo")),
          m_hp(screen.get<ui::DigitText>("BattleHp")),
          m_sp(screen.get<ui::DigitText>("BattleSp")),
          m_expBar(screen.get<ui::ProgressBar>("BattleExpBar")),
          m_expPercent(screen.get<ui::DigitText>("BattleExpPercent")),
          m_gold(screen.get<ui::DigitText>("BattleGold")),
          m_commands{
              std::ref(screen.get<ui::ImageButton>("BattleCommand1")),
              std::ref(screen.get<ui::ImageButton>("BattleCommand2")),
              std::ref(screen.get<ui::ImageButton>("BattleCommand3")),
              std::ref(screen.get<ui::ImageButton>("BattleCommand4")),
              std::ref(screen.get<ui::ImageButton>("BattleCommand5")),
              std::ref(screen.get<ui::ImageButton>("BattleCommand6")),
              std::ref(screen.get<ui::ImageButton>("BattleCommand7"))
          },
          m_lastCommand(screen.get<ui::ImageButton>("BattleCommand8")),
          m_autoCancel(screen.get<ui::ImageButton>("BattleAutoCancel")),
          m_memberPanelSpot(screen.get<ui::Spot>("BattleMemberPanel")),
          m_wheelCentreSpot(screen.get<ui::Spot>("BattleWheelCentre")) {
        for (int member = 1; member <= kMembers; ++member) {
            const std::string name = "BattleMember" + std::to_string(member);
            m_members.push_back(
                Member{
                    std::ref(screen.get<ui::Element>(name)),
                    std::ref(screen.get<ui::Label>(name + "Name")),
                    std::ref(screen.get<ui::ImageView>(name + "Element")),
                    std::ref(screen.get<ui::DigitText>(name + "Level")),
                    std::ref(screen.get<ui::ImageView>(name + "Job")),
                    Gauge(services.pictures, "Main_Hp"),
                    Gauge(services.pictures, "Main_Sp"),
                    PetInfoPainter(services, kMemberPetStyle)
                }
            );
            const int index = member - 1;
            screen.get<ui::DrawingArea>(name + "Hp").setPainter([this, index](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintMember(canvas, index, 0, at);
            });
            screen.get<ui::DrawingArea>(name + "Sp").setPainter([this, index](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintMember(canvas, index, 1, at);
            });
            screen.get<ui::DrawingArea>(name + "Portrait").setPainter([this, index](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintMember(canvas, index, 2, at);
            }); // part 2 = portrait
            screen.get<ui::DrawingArea>(name + "Icons").setPainter([this, index](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                const auto& members = m_battle.members();
                const auto own = m_battle.field().fighter(1); // fighter slot 1 = the own player
                if (!m_slid || index >= static_cast<int>(members.size()) || !own) {
                    return;
                }
                const auto fighter = m_battle.field().fighter(members[static_cast<std::size_t>(index)]);
                if (fighter && fighter->get().ownerId == 0 && fighter->get().side() == own->get().side()) {
                    paintStatusIcons(canvas, *fighter, at, false);
                }
            });
            screen.get<ui::DrawingArea>(name + "Pet").setPainter([this, index](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
                paintMemberPet(canvas, index, at, nowMs);
            });
        }
        for (int button = 1; button <= kButtons + 1; ++button) {
            screen.on("onBattleCommand" + std::to_string(button), [this, button](const ui::EventArgs&) { m_battle.command(button); });
        }
        screen.on("onBattleAutoCancel", [this](const ui::EventArgs&) { m_battle.cancelAuto(3); }); // 3 = FSideButtons[3], the auto-cancel button
    }

    void BattleMainForm::slideIn(std::uint64_t nowMs) {
        m_panel.setVisible(true);
        m_slideMs = nowMs;
        m_slideY = -m_memberPanelSpot.size().height;
        m_slid = false;
        m_wheelActor = 0;
    }

    void BattleMainForm::hide() {
        m_panel.setVisible(false);
        m_slideMs = 0;
        m_wheelActor = 0;
    }

    void BattleMainForm::paintMember(gfx::Canvas& canvas, int member, int part, core::Point at) {
        const auto& members = m_battle.members();
        if (member >= static_cast<int>(members.size())) {
            return;
        }
        const auto found = m_battle.field().fighter(members[static_cast<std::size_t>(member)]);
        if (!found) {
            return;
        }
        const Fighter& fighter = *found;
        Member& panel = m_members[static_cast<std::size_t>(member)];
        if (part == 0) {
            panel.hp.paint(canvas, at, fighter.hp, fighter.maxHp);
        } else if (part == 1) {
            panel.sp.paint(canvas, at, fighter.sp, fighter.maxSp);
        } else if (fighter.look) {
            m_services.characters.drawPortrait(canvas, *fighter.look, at, 0, kPortraitSlot);
        }
    }

    void BattleMainForm::paintMemberPet(gfx::Canvas& canvas, int member, core::Point at, std::uint64_t nowMs) {
        const auto& members = m_battle.members();
        if (member >= static_cast<int>(members.size())) {
            return;
        }
        const auto owner = m_battle.field().fighter(members[static_cast<std::size_t>(member)]);
        if (!owner) {
            return;
        }
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_battle.field().fighter(slot);
            // fighter kind 4 = pet
            if (!found || found->get().kind != 4 || found->get().ownerId != owner->get().id || !found->get().npc) {
                continue;
            }
            const Fighter& fighter = *found;
            Pet pet;
            pet.info = *fighter.npc;
            pet.level = fighter.level;
            pet.reborn = fighter.reborn;
            pet.hp = fighter.hp;
            pet.maxHp = fighter.maxHp;
            pet.sp = fighter.sp;
            pet.maxSp = fighter.maxSp;
            m_members[static_cast<std::size_t>(member)].pet.paint(canvas, at, pet, nowMs);
            constexpr core::Point kPetIcons{0, 27}; // status icon row offset below the pet panel, px
            const auto own = m_battle.field().fighter(1); // fighter slot 1 = the own player
            if (m_slid && own && fighter.side() == own->get().side()) {
                paintStatusIcons(canvas, fighter, at + kPetIcons, true);
            }
            return;
        }
    }

    void BattleMainForm::paintStatusIcons(gfx::Canvas& canvas, const Fighter& fighter, core::Point at, bool pet) {
        constexpr int kSource = 26; // status icon picture size, px (drawn scaled down)
        const FighterStatuses& statuses = fighter.statuses;
        std::vector<int> shown;
        for (std::size_t j = 1; j <= 2; ++j) { // special status slots 1 and 2
            if (statuses.special[j] != 0) {
                shown.push_back(statuses.special[j]);
            }
        }
        shown.insert(shown.end(), statuses.icons.begin(), statuses.icons.end());
        const int count = static_cast<int>(shown.size());
        // icon size in pixels: pet 13, character 15
        const int size = pet ? 13 : 15;
        int spacing = 0;
        if (pet) {
            // icon spacing in pixels: 6 when more than two icons, else 13
            spacing = count > 2 ? 6 : 13;
        } else {
            // icon spacing: 15 px up to two icons, else squeezed by the icon count
            spacing = count <= 2 ? 15 : 15 / count - 1 + 15 / count;
        }
        for (int n = 0; n < count; ++n) {
            constexpr int kPetMost = 4;
            if (pet && n >= kPetMost) {
                break;
            }
            const int skillId = shown[static_cast<std::size_t>(n)];
            std::optional<gfx::PictureId> picture;
            switch (skillId) {
                case 10073: // skill id whose icon is the linked item's icon
                case 10074: // skill id whose icon is the linked item's icon
                case 15139: // skill id whose icon is the linked item's icon
                case 15141: // skill id whose icon is the linked item's icon
                case 15167: // skill id whose icon is the linked item's icon
                case 15169: // skill id whose icon is the linked item's icon
                case 15172: // skill id whose icon is the linked item's icon
                    if (const auto item = m_services.items.findByLinkedSkill(skillId)) {
                        picture = m_services.pictures.find(std::to_string(item->iconId));
                    }
                    break;
                default: // any other skill: its own Icon_sk picture
                    if (const auto skill = m_services.skills.find(skillId)) {
                        picture = m_services.pictures.find("Icon_sk" + std::to_string(skill->iconId) + "s");
                    }
                    break;
            }
            if (picture) {
                canvas.drawPictureScaled(*picture, {at.x + n * spacing, at.y, size, size}, core::Rect{0, 0, kSource, kSource});
            }
        }
    }

    void BattleMainForm::showMembers() {
        const auto& members = m_battle.members();
        bool ownShown = false;
        for (std::size_t index = 0; index < m_members.size(); ++index) {
            Member& panel = m_members[index];
            const auto found = index < members.size() ? m_battle.field().fighter(members[index]) : std::nullopt;
            panel.panel.get().setVisible(found.has_value());
            if (!found) {
                continue;
            }
            const Fighter& fighter = *found;
            panel.panel.get().setPosition({static_cast<int>(index) * m_memberPanelSpot.pitch().x, m_slideY});
            panel.name.get().setText(fighter.name);
            panel.element.get().setPicture(m_services.pictures.find("Icon_Element_" + std::to_string(fighter.element) + "_1").value_or(gfx::PictureId{}));
            panel.level.get().setText(std::to_string(fighter.level));
            const std::size_t job = static_cast<std::size_t>(fighter.job);
            const bool hasJob = m_slid && fighter.ownerId == 0 && job >= 1 && job < kJobIcons.size();
            panel.job.get().setVisible(hasJob);
            if (hasJob) {
                panel.job.get().setPicture(m_services.pictures.find(kJobIcons[job]).value_or(gfx::PictureId{}));
            }
            if (members[index] == 1) {
                ownShown = true;
                m_hp.setText(std::to_string(fighter.hp) + "/" + std::to_string(fighter.maxHp));
                m_sp.setText(std::to_string(fighter.sp) + "/" + std::to_string(fighter.maxSp));
            }
        }
        m_ownInfo.setVisible(ownShown);
        m_ownInfo.setPosition({0, m_slideY});
        const double progress = std::min(1.0, levelProgress(m_services.formula, m_status.level, m_status.exp, m_status.reborn));
        m_expBar.setMaximum(m_expBar.bounds().width);
        m_expBar.setValue(static_cast<int>(std::trunc(m_expBar.bounds().width * progress)));
        m_expPercent.setText(std::to_string(static_cast<int>(std::trunc(progress * 100))) + "%"); // fraction to percent
        m_gold.setText(std::to_string(m_status.gold));
    }

    void BattleMainForm::placeCommands(std::uint64_t nowMs) {
        const bool shown = m_battle.choosing();
        const int actor = shown ? m_battle.actor() : 0;
        if (actor != m_wheelActor) {
            m_wheelActor = actor;
            m_wheelMs = nowMs;
        }
        const auto fighter = actor != 0 ? m_battle.field().fighter(actor) : std::nullopt;
        const core::Point feet = fighter ? (m_toWindow ? m_toWindow(fighter->get().position) : fighter->get().position) : core::Point{};
        const int elapsed = static_cast<int>(std::min<std::uint64_t>(nowMs - m_wheelMs, kWheelMs));
        for (std::size_t i = 0; i < m_commands.size(); ++i) {
            ui::ImageButton& button = m_commands[i];
            button.setVisible(fighter.has_value());
            if (!fighter) {
                continue;
            }
            const core::Point offset = bezier(m_wheelCentreSpot.at(), kHomes[i], kBends[i], kWheelMs, elapsed);
            button.setPosition({feet.x + offset.x, feet.y + offset.y});
        }
        const int last = fighter ? m_battle.lastCommandKind() : 0;
        const auto picture = last != 0 ? m_services.pictures.find("Btn_Battle_" + std::to_string(last) + "_1") : std::nullopt;
        m_lastCommand.setVisible(picture.has_value());
        if (picture) {
            m_lastCommand.setPicture(*picture);
            m_lastCommand.setPosition({feet.x + m_wheelCentreSpot.at().x, feet.y + m_wheelCentreSpot.at().y});
        }
        m_autoCancel.setVisible(m_battle.autoOn());
    }

    void BattleMainForm::update(std::uint64_t nowMs) {
        if (!m_panel.isVisible()) {
            return;
        }
        if (m_slideMs != 0) {
            const int offset = static_cast<int>(std::ceil(static_cast<double>(nowMs - m_slideMs) * m_memberPanelSpot.size().height / kSlideMs));
            if (offset >= m_memberPanelSpot.size().height) {
                m_slideY = 0;
                m_slideMs = 0;
                m_slid = true;
            } else {
                m_slideY = -m_memberPanelSpot.size().height + offset;
            }
        }
        showMembers();
        placeCommands(nowMs);
        if (m_autoFight && m_autoFight() && m_battle.choosing()) {
            m_battle.command(kAutoButton);
        }
    }

}
