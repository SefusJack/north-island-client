#include "client/subsystems/battle/Battle.h"
#include "client/subsystems/battle/DashArc.h"

#include "client/subsystems/battle/StatusMotions.h"
#include "client/subsystems/battle/StatusRules.h"
#include "client/rules/PetWeapon.h"
#include "client/rules/SkillUse.h"
#include "client/rules/Skills.h"
#include "core/Between.h"
#include "core/Clock.h"
#include "core/Log.h"
#include "protocol/BattleRequests.h"
#include "protocol/ItemRequests.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <thread>

namespace client {

    namespace {

        constexpr int kAttack = 10001; // skill 10001: Hand Attack
        constexpr int kDefend = 60021; // skill 60021: Defense
        constexpr int kCapture = 10008; // skill 10008: Capture
        constexpr int kFlee = 60041; // skill 60041: Flee
        constexpr std::size_t kMostMembers = 4; // most player members tracked in the battle (m_members)
        constexpr int kHp = 25; // attribute 25: HP
        constexpr int kSp = 26; // attribute 26: SP
        constexpr int kMaxHp = 205; // attribute 205: max HP
        constexpr int kMaxSp = 206; // attribute 206: max SP
        constexpr int kLevel = 35; // attribute 35: level
        constexpr int kWaitingState = 5; // fighter state 5: waiting for the round
        constexpr int kWaitingStateToo = 100; // fighter state 100: also waiting for the round
        constexpr int kFightPose = 16; // NPC sprite slot 16: the fighting pose
        constexpr int kPlayerBody = 80; // body height of a player fighter, px
        constexpr std::uint64_t kSecondMs = 1000; // one countdown second, ms
        constexpr int kBeepFrom = 10; // the countdown beeps for the last 10 seconds
        constexpr int kRunningOut = 10; // round countdown second at which the clock shows time running out
        constexpr int kTwinStrike = 15172; // skill 15172: Biochemical Battery
        constexpr int kTwinStrikeWalker = 15173; // skill 15173: Biochemical Battery (walking variant)

        /// @brief Maps an NPC id to the id the battle draws instead.
        /// @details Original: AddFightHumById.
        /// @param id NPC id of the fighter.
        /// @return The id to look up in the NPC table.
        int shownNpcId(std::int32_t id) {
            switch (id) {
                case 37006: // drawn as NPC 21001 (ids 37001..37015 and 21001 are not in the NPC name table)
                    return 21001;
                case 37010: // drawn as NPC 37008
                    return 37008;
                case 37011: // drawn as NPC 37009
                    return 37009;
                case 37012: // drawn as NPC 37013
                    return 37013;
                case 37014: // drawn as NPC 37015
                    return 37015;
                default: // any other id is drawn as itself
                    return static_cast<int>(id);
            }
        }

    }

    Battle::Battle(
        GameServices& services,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_ground(services.pictures),
          m_painter(services.characters, services.pictures) {}

    std::optional<Fighter> Battle::makeFighter(const protocol::FighterRecord& record) const {
        Fighter fighter;
        fighter.kind = record.kind;
        fighter.id = record.id;
        fighter.ownerId = record.ownerId;
        fighter.row = record.row;
        fighter.col = record.col;
        fighter.hp = record.hp;
        fighter.maxHp = record.maxHp;
        fighter.sp = record.sp;
        fighter.maxSp = record.maxSp;
        fighter.level = record.level;
        fighter.element = record.element;
        fighter.reborn = record.reborn;
        fighter.job = record.job;
        fighter.waiting = record.state == kWaitingState || record.state == kWaitingStateToo;
        switch (record.kind) {
            case 2: // fighter kind 2: a player
            case 9: { // fighter kind 9: a world player who is only watching
                const auto player = m_hooks.player(record.id);
                if (!player) {
                    return std::nullopt;
                }
                fighter.look = player->first;
                fighter.name = player->second;
                fighter.bodyHeight = kPlayerBody;
                return fighter;
            }
            case 3: // fighter kind 3: a map NPC
                fighter.npc = m_hooks.mapNpc(record.mapNpc);
                break;
            case 4: // fighter kind 4: a pet (NPC kind from the NPC table)
            case 6: // fighter kind 6: NPC from the NPC table
            case 7: // fighter kind 7: NPC from the NPC table
            case 11: // fighter kind 11: NPC from the NPC table
            case 12: // fighter kind 12: NPC from the NPC table
            case 13: // fighter kind 13: NPC from the NPC table
            case 14: // fighter kind 14: NPC from the NPC table
            case 15: // fighter kind 15: NPC from the NPC table
            case 16: // fighter kind 16: NPC from the NPC table
                fighter.npc = m_services.npcs.find(shownNpcId(record.id));
                break;
            default: // unknown fighter kind: no fighter
                return std::nullopt;
        }
        if (!fighter.npc) {
            return std::nullopt;
        }
        fighter.name = fighter.npc->name;
        if (record.kind == 4) { // fighter kind 4: a pet
            if (const auto name = m_hooks.petName(record.ownerId, fighter.npc->npcId)) {
                fighter.name = *name;
            }
            if (const auto weapon = m_hooks.petWeapon(record.ownerId, fighter.npc->npcId)) {
                fighter.weaponLook = weapon->first;
                fighter.weaponColor = weapon->second;
            }
        }
        const formats::NpcInfo& npc = *fighter.npc;
        const int scale = npc.heightMode == 1 ? 2 : 1; // height mode 1 doubles the sprite height
        if (const int height = m_services.characters.npcHeight(npc.spriteId, npc.spriteSize, 0, 0); height > 0) {
            fighter.spriteHeight = height * scale;
            if (m_services.characters.npcFrameCount(npc.spriteId, kFightPose) > 0) {
                fighter.bodyHeight = height * scale;
            }
        }
        return fighter;
    }

    void Battle::start(const protocol::BattleStart& start, std::int32_t ownId, const formats::SceneTable& scenes, core::Color tint) {
        m_field.clear();
        m_itemFlights.clear();
        m_on = true;
        ++m_battleCount;
        m_round = 0;
        m_ownId = ownId;
        m_type = 0;
        m_timeLimitOn = false;
        m_timeLeft = 180; // the battle's 3-minute limit in seconds
        m_turnSeconds = 0;
        m_actor = 0;
        m_countdown = -1;
        m_roundCountdown = -1;
        m_timeRunningOut = false;
        m_commandPhase = false;
        m_skill = 0;
        m_cursor = ui::CursorKind::Attack;
        m_autoCommands = {};
        m_members.clear();
        m_ground.load(scenes, start.ground, tint);
        if (const core::Point size = m_ground.size(); size.x > 0) {
            m_camera.setGroundSize(size.x, size.y);
        }
        m_camera.reset(0);
        for (auto& action : m_actions) {
            action.reset();
        }
        m_runningOff.fill(false);
        m_ownGone = false;
        for (const protocol::FighterRecord& record : start.fighters) {
            // fighter kinds: 2 player, 9 watching player, 4 pet, 14 NPC that also comes running on
            const bool own = (record.kind == 2 || record.kind == 9) && record.id == ownId;
            addFighter(record, own || record.kind == 4 || record.kind == 14, false);
        }
        for (const protocol::FighterRecord& record : start.fighters) {
            if (record.kind == 2) { // fighter kind 2: a player
                addMember(m_field.slotAt(record.row, record.col));
            }
        }
    }

    void Battle::finishGround() {
        m_ground.finish();
        if (const core::Point size = m_ground.size(); size.x > 0) {
            m_camera.setGroundSize(size.x, size.y);
        }
    }

    void Battle::addFighter(const protocol::FighterRecord& record, bool showEnter, bool placeOthers) {
        // the grid has 4 rows and 4 columns
        if (record.row < 1 || record.row > 4 || record.col < 1 || record.col > 4) {
            return;
        }
        auto fighter = makeFighter(record);
        if (!fighter) {
            return;
        }
        const bool own = record.kind == 2 && record.id == m_ownId; // fighter kind 2: a player
        if (own) {
            m_autoRotation = {};
        }
        const int slot = m_field.freeSlot(own);
        if (slot == 0) {
            return;
        }
        if (fighter->hp == 0) {
            fighter->action = fighter->side() == 1 ? 26 : 27; // lying (knocked off) action: 26 side 1, 27 side 2
        }
        fighter->entering = showEnter ? (placeOthers && !own ? 0 : 1) : 0;
        m_field.place(slot, std::move(*fighter));
    }

    void Battle::addMember(int slot) {
        const auto fighter = m_field.fighter(slot);
        const auto own = m_field.fighter(1);
        // fighter kind 2: a player
        if (!fighter || !own || fighter->get().kind != 2 || fighter->get().side() != own->get().side() ||
            std::find(m_members.begin(), m_members.end(), slot) != m_members.end()) {
            return;
        }
        if (m_members.size() < kMostMembers) {
            m_members.push_back(slot);
        } else {
            m_members.back() = slot;
        }
    }

    void Battle::join(const std::vector<protocol::FighterRecord>& records, bool placeOthers) {
        if (!m_on) {
            return;
        }
        for (const protocol::FighterRecord& record : records) {
            if (record.kind == 9) { // fighter kind 9: a world player who is only watching
                continue;
            }
            addFighter(record, true, placeOthers);
            if (record.kind == 2 && record.id == m_ownId && record.state == kWaitingState) { // fighter kind 2: a player
                m_messages.show("Please wait for the beginning of the bout.", 2000); // message display time in ms
            }
            if (record.kind == 2) { // fighter kind 2: a player
                addMember(m_field.slotAt(record.row, record.col));
            }
        }
    }

    void Battle::setWeaponLooks(const std::vector<protocol::FighterWeaponLook>& looks) {
        constexpr int kNoCell = 255; // row/col value meaning "no cell"
        if (!m_on) {
            return;
        }
        for (const protocol::FighterWeaponLook& look : looks) {
            if (look.itemId == 0 || look.id == 0 || look.ownerId == 0 || look.row == kNoCell || look.col == kNoCell || !m_services.items.find(look.itemId)) {
                continue;
            }
            const auto fighter = m_field.fighter(m_field.slotAt(look.row, look.col));
            if (!fighter || !fighter->get().npc || fighter->get().id != look.id || fighter->get().ownerId != look.ownerId) {
                continue;
            }
            const PetWeapon weapon = weaponOfItem(look.itemId, fighter->get().npc->bodyType, m_services.items);
            fighter->get().weaponLook = weapon.lookId;
            fighter->get().weaponColor = weapon.colorCode;
        }
    }

    void Battle::playerLeft(std::int32_t playerId, std::uint64_t nowMs) {
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_field.fighter(slot);
            if (!found || found->get().kind != 2 || found->get().id != playerId) { // fighter kind 2: a player
                continue;
            }
            ActionContext context = actionContext(nowMs);
            freeAction(slot, context);
            m_actions[static_cast<std::size_t>(slot)].emplace(context, slot, slot, kFlee);
            m_runningOff[static_cast<std::size_t>(slot)] = true;
            m_members.erase(std::remove(m_members.begin(), m_members.end(), slot), m_members.end());
            return;
        }
    }

    void Battle::updateEntering(Fighter& fighter, std::uint64_t nowMs) {
        constexpr int kBehind = 80; // px behind the cell where an entering fighter starts
        constexpr double kSpeed = 0.12; // entering walk speed, px per ms
        constexpr int kWalks = 12; // NPC type 12: walks onto its cell
        constexpr int kStands = 21; // NPC type 21: stands on its cell at once
        if (fighter.entering == 1) {
            const core::Point cell = BattleField::cellPosition(fighter.row, fighter.col);
            fighter.position = {cell.x + (fighter.side() == 1 ? kBehind : -kBehind), cell.y};
            fighter.heading = cell;
            fighter.movedMs = nowMs;
            fighter.entering = 2; // entering phase 2: moving onto the cell
            const int type = fighter.npc ? fighter.npc->type : 0;
            // NPC ids 37002..37006 walk in; 37001 and 37007..37009 stand on their cell at once
            const bool walks = type == kWalks || (!fighter.isPlayer() && fighter.id >= 37002 && fighter.id <= 37006);
            const bool stands = type == kStands || (!fighter.isPlayer() && (fighter.id == 37001 || (fighter.id >= 37007 && fighter.id <= 37009)));
            if (stands) {
                fighter.position = cell;
            } else if (!walks) {
                // dash action: 36 side 1, 37 side 2
                fighter.action = fighter.side() == 1 ? 36 : 37;
                fighter.holdFrame = 255; // 255: hold no particular frame (36/37 above are the dash action numbers)
                fighter.dashLength = DashArc::length(fighter.position, fighter.heading);
                fighter.dashing = fighter.dashLength != 0;
            }
            return;
        }
        const int step = static_cast<int>(std::lrint(static_cast<double>(nowMs - fighter.movedMs) * kSpeed));
        fighter.movedMs = nowMs;
        const int distX = std::abs(fighter.heading.x - fighter.position.x);
        const int distY = std::abs(fighter.heading.y - fighter.position.y);
        if (fighter.position == fighter.heading || (step > distX && step > distY)) {
            fighter.position = fighter.heading;
            fighter.dashing = false;
            fighter.entering = 3; // entering phase 3: on the cell (FMoveDone)
            return;
        }
        const auto toward = [step](int from, int to, int dist) {
            const int move = std::min(step, dist);
            return from < to ? from + move : from - move;
        };
        fighter.position.x = toward(fighter.position.x, fighter.heading.x, distX);
        fighter.position.y = toward(fighter.position.y, fighter.heading.y, distY);
    }

    void Battle::tickEntering(Fighter& fighter) {
        // entering phases: 3 on the cell, 4 one tick later, then freed
        if (fighter.entering == 3) {
            fighter.entering = 4;
        } else if (fighter.entering == 4) {
            endEntering(fighter);
        }
    }

    void Battle::endEntering(Fighter& fighter) {
        fighter.entering = 0;
        fighter.dashing = false;
        fighter.position = BattleField::cellPosition(fighter.row, fighter.col);
        // actions: lying 26 / 27, standing 16 / 17 (side 1 / side 2)
        fighter.action = fighter.hp <= 0 ? (fighter.side() == 1 ? 26 : 27) : (fighter.side() == 1 ? 16 : 17);
        if (fighter.hp <= 0) {
            fighter.fall();
        } else {
            fighter.stand();
        }
        fighter.holdFrame = 255; // 255: hold no particular frame
    }

    void Battle::setType(int type) {
        m_type = type;
        switch (type) {
            case 2: // battle type 2: 30 seconds to command
            case 4: // battle type 4: 30 seconds to command
            case 7: // battle type 7: 30 seconds to command
            case 8: // battle type 8: 30 seconds to command
            case 9: // battle type 9: 30 seconds to command
                m_turnSeconds = 30; // seconds to command
                break;
            default: // other battle types: 20 seconds to command
                m_turnSeconds = 20; // seconds to command
                break;
        }
    }

    void Battle::startRound(std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        m_commandPhase = true;
        ++m_round;
        m_services.zoom.reset();
        ActionContext context = actionContext(nowMs);
        resetForRound(context);
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto fighter = m_field.fighter(slot)) {
                fighter->get().waiting = false;
                fighter->get().acted = false;
            }
        }
        m_roundCountdown = m_turnSeconds;
        m_roundCountdownMs = nowMs;
        m_actor = 1;
        const auto own = m_field.fighter(1);
        if (!own || own->get().hp <= 0) {
            nextCommander();
        } else {
            resetCommand();
        }
        if (m_actor != 0 && cannotCommand(m_actor)) {
            nextCommander();
        }
        if (m_actor != 0) {
            m_countdown = m_turnSeconds;
            m_countdownMs = nowMs;
        }
    }

    int Battle::nextOwnFighter(int afterSlot) const {
        for (int slot = afterSlot + 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_field.fighter(slot);
            if (!found) {
                continue;
            }
            const Fighter& fighter = *found;
            // fighter kinds that can be commanded: 2 player, 4 pet, 13, 15
            const bool commandable = fighter.kind == 2 || fighter.kind == 4 || fighter.kind == 13 || fighter.kind == 15;
            if (fighter.ownerId != m_ownId || !commandable || fighter.waiting || fighter.hp <= 0) {
                continue;
            }
            return slot;
        }
        return 0;
    }

    void Battle::nextCommander() {
        if (m_actor == 0) {
            return;
        }
        if (m_hooks.hideItems) {
            m_hooks.hideItems();
        }
        m_actor = nextOwnFighter(m_actor);
        while (m_actor != 0 && cannotCommand(m_actor)) {
            m_actor = nextOwnFighter(m_actor);
        }
        if (m_actor != 0) {
            resetCommand();
        } else {
            m_countdown = -1;
            m_cursor = ui::CursorKind::Normal;
        }
    }

    bool Battle::cannotCommand(int slot) const {
        const auto found = m_field.fighter(slot);
        if (!found) {
            return false;
        }
        const FighterStatuses& statuses = found->get().statuses;
        // status effects 1..50 (second place) and 173 (fourth place) give no command
        return (statuses.main[1].effect >= 1 && statuses.main[1].effect <= 50) || statuses.main[3].effect == 173;
    }

    void Battle::resetCommand() {
        m_skill = 0;
        m_item = 0;
        m_itemSlot = 0;
        m_cursor = ui::CursorKind::Normal;
        selectActor();
    }

    void Battle::cancelCommand() {
        if (!m_on) {
            return;
        }
        m_cursor = ui::CursorKind::Normal;
        m_skill = 0;
        m_item = 0;
        m_itemSlot = 0;
    }

    int Battle::commandSlotKind(int slot) const {
        const auto found = m_field.fighter(slot);
        if (!found) {
            return 0;
        }
        const Fighter& fighter = *found;
        if (fighter.kind == 2 && fighter.id == m_ownId) { // fighter kind 2: a player
            return 1;
        }
        return fighter.kind == 4 ? 2 : 0; // fighter kind 4: a pet
    }

    int Battle::lastCommandKind() const {
        const int kind = commandSlotKind(m_actor);
        const auto actor = m_field.fighter(m_actor);
        return kind != 0 && actor ? actor->get().lastCommands[static_cast<std::size_t>(kind)].kind : 0;
    }

    void Battle::rememberLastCommand(int casterSlot, int targetSlot, int skillId) {
        const auto casterFound = m_field.fighter(casterSlot);
        const auto targetFound = m_field.fighter(targetSlot);
        if (!casterFound || !targetFound) {
            return;
        }
        Fighter& caster = *casterFound;
        // fighter kinds: 4 a pet, 2 a player
        if (caster.kind == 4 ? caster.ownerId != m_ownId : !(caster.kind == 2 && caster.id == m_ownId)) {
            return;
        }
        const int kind = commandSlotKind(casterSlot);
        if (kind == 0) {
            return;
        }
        LastCommand& own = caster.lastCommands[static_cast<std::size_t>(kind)];
        LastCommand& on = targetFound->get().lastCommands[static_cast<std::size_t>(kind)];
        own.target = 0;
        own.value = 0;
        on.target = 0;
        on.value = 0;
        const auto skill = m_services.skills.find(skillId);
        if (skill) {
            switch (skill->skillType) {
                case 10: // skill type 10 (defend): last command kind 1
                    own.kind = 1;
                    own.value = skillId;
                    break;
                case 11: // skill type 11 (capture): last command kind 2, remembered on the target too
                    own.kind = 2;
                    own.target = targetSlot;
                    own.value = skillId;
                    on.kind = 2;
                    break;
                case 12: // skill type 12 (flee): last command kind 3
                    own.kind = 3;
                    own.value = skillId;
                    break;
                default: // other skills: last command kind 6 (attack, category 0) or 7 (skill)
                    own.kind = skill->category == 0 ? 6 : 7;
                    own.target = targetSlot;
                    own.value = skillId;
                    on.kind = own.kind;
                    if (skill->category != 0) {
                        on.value = skillId;
                        if (m_hooks.skillCommanded) {
                            // command slot kind 2: the pet
                            m_hooks.skillCommanded(skillId, kind == 2);
                        }
                    }
                    break;
            }
        }
        m_lastCommandKinds[static_cast<std::size_t>(kind)] = own.kind;
    }

    void Battle::selectActor() {
        const int kind = commandSlotKind(m_actor);
        if (kind != 0 && m_autoCommands[static_cast<std::size_t>(kind)] != 0) {
            issueAutoCommand(m_autoCommands[static_cast<std::size_t>(kind)]);
        }
    }

    int Battle::enemySide() const {
        const auto own = m_field.fighter(1);
        return own && own->get().side() == 1 ? 2 : 1;
    }

    int Battle::livingFighterOnSide(int side) const {
        // side 2 lies in rows 1 and 2, side 1 in rows 4 and 3; 4 columns per row
        const std::array<int, 2> rows = side == 2 ? std::array<int, 2>{1, 2} : std::array<int, 2>{4, 3};
        for (const int row : rows) {
            for (int col = 1; col <= 4; ++col) {
                const int slot = m_field.slotAt(row, col);
                if (const auto fighter = m_field.fighter(slot); fighter && fighter->get().hp != 0) {
                    return slot;
                }
            }
        }
        return 0;
    }

    void Battle::issueAutoCommand(int mode) {
        constexpr int kAuto = 3;
        constexpr int kRemote = 5;
        if ((mode != kAuto && mode != kRemote) || m_actor == 0) {
            return;
        }
        const auto actor = m_field.fighter(m_actor);
        if (!actor) {
            return;
        }
        if (mode == kRemote) {
            const int slotKind = commandSlotKind(m_actor);
            if (slotKind != 0 && m_hooks.scriptDecision) {
                if (const auto decision = m_hooks.scriptDecision(remoteView()); decision && applyDecision(*decision, slotKind)) {
                    return;
                }
            }
            if (slotKind != 0 && m_hooks.playSets && runAutoSettings(m_hooks.playSets(slotKind))) {
                return;
            }
        }
        const int target = livingFighterOnSide(actor->get().side() == 1 ? 2 : 1);
        if (target == 0) {
            return;
        }
        const int kind = commandSlotKind(m_actor);
        constexpr int kAttackKind = 6;
        if (kind != 0 && m_lastCommandKinds[static_cast<std::size_t>(kind)] == kAttackKind) {
            chooseSkill(kAttack, false);
        }
        if (m_skill == 0 && m_hooks.recentSkills) {
            const auto user = actorAsUser();
            for (int skillId : m_hooks.recentSkills(kind == 2)) {
                if (skillId == kRoleSkillPlaceholder && user) {
                    skillId = roleSkillId(user->bodyType, user->head);
                }
                const auto skill = m_services.skills.find(skillId);
                constexpr int kSkipped = 15026; // skill 15026: Soul Dodge
                // target type 0: an enemy; skill types 1 and 2: offensive skills
                if (!skill || skill->targetType != 0 || (skill->skillType != 1 && skill->skillType != 2) || skillId == kSkipped) {
                    continue;
                }
                chooseSkill(isRoleSkill(skillId) ? kRoleSkillPlaceholder : skillId, false);
                if (m_skill != 0) {
                    break;
                }
            }
        }
        if (m_skill == 0) {
            chooseSkill(kAttack, false);
        }
        if (m_skill == kRoleSkillPlaceholder) {
            if (const auto user = actorAsUser()) {
                m_skill = roleSkillId(user->bodyType, user->head);
            }
        }
        if (m_skill != 0) {
            submit(target);
        } else if (kind != 0) {
            cancelAuto(kind);
        }
    }

    void Battle::cancelAuto(int which) {
        switch (which) {
            case 1: // the player's auto command
            case 2: // the pet's auto command
                m_autoCommands[static_cast<std::size_t>(which)] = 0;
                break;
            case 3: // both auto commands
                m_autoCommands[1] = 0; // commander kind 1: the player
                m_autoCommands[2] = 0; // commander kind 2: the pet
                break;
            default: // unknown selector: nothing cancelled
                break;
        }
    }

    void Battle::repeatLastCommand() {
        const int kind = commandSlotKind(m_actor);
        const auto actorFound = m_field.fighter(m_actor);
        if (kind == 0 || !actorFound) {
            return;
        }
        const LastCommand last = actorFound->get().lastCommands[static_cast<std::size_t>(kind)];
        const auto standing = [&](int slot) {
            const auto target = m_field.fighter(slot);
            return target && target->get().id != 0 && target->get().hp != 0 ? slot : livingFighterOnSide(enemySide());
        };
        switch (last.kind) {
            case 1: // last command kind 1: defend, repeated on oneself
            case 3: // last command kind 3: flee, repeated on oneself
                chooseSkill(last.value);
                if (m_skill != 0) {
                    submit(m_actor);
                }
                break;
            case 2: // last command kind 2: capture, repeated on the remembered target
            case 6: // last command kind 6: attack, repeated on the remembered target
            case 7: { // last command kind 7: skill, repeated on the remembered target
                if (last.target < 1) {
                    return;
                }
                int skillId = last.kind == 6 ? kAttack : last.value; // last command kind 6 is a plain attack
                if (isRoleSkill(skillId)) {
                    skillId = kRoleSkillPlaceholder;
                }
                chooseSkill(skillId);
                if (m_skill == 0) {
                    return;
                }
                if (m_skill == kRoleSkillPlaceholder) {
                    if (const auto user = actorAsUser()) {
                        m_skill = roleSkillId(user->bodyType, user->head);
                    }
                }
                if (const int target = standing(last.target)) {
                    submit(target);
                }
                break;
            }
            default: // no last command kind: nothing to repeat
                break;
        }
    }

    std::optional<SkillUser> Battle::actorAsUser() const {
        const auto actor = m_field.fighter(m_actor);
        if (!actor || !m_hooks.skillUser) {
            return std::nullopt;
        }
        return m_hooks.skillUser(*actor);
    }

    bool Battle::hotKeySkillUsable(int skillId) const {
        std::optional<std::reference_wrapper<const Fighter>> caster;
        if (m_actor > 0) {
            if (const auto actor = m_field.fighter(m_actor); actor && actor->get().hp != 0) {
                caster = *actor;
            }
        } else {
            for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
                const auto found = m_field.fighter(slot);
                if (found && found->get().kind == 2 && found->get().id == m_ownId) { // fighter kind 2: a player
                    caster = *found;
                }
            }
        }
        const auto skill = m_services.skills.find(skillId);
        if (!caster || !skill || !m_hooks.skillUser) {
            return false;
        }
        const int sp = caster->get().sp;
        if (skill->spCost == 1 ? sp <= 0 : skill->spCost > sp) {
            return false;
        }
        int asked = isRoleSkill(skillId) ? kRoleSkillPlaceholder : skillId;
        return checkCanUseSkill(m_services.skills, m_services.motions, m_hooks.skillUser(*caster), asked) == 0;
    }

    void Battle::chooseSkill(int skillId, bool showMessage) {
        const auto user = actorAsUser();
        if (!user) {
            return;
        }
        const int asked = skillId;
        const int result = checkCanUseSkill(m_services.skills, m_services.motions, *user, skillId);
        if (const auto message = showMessage ? skillUseMessage(m_services.skills, result, asked) : std::nullopt) {
            m_messages.show(message->first, message->second);
        }
        m_skill = skillId;
        const auto skill = m_services.skills.find(skillId);
        if (!skill) {
            return;
        }
        switch (skill->skillType) {
            case 10: // skill type 10: defend, plain cursor
            case 12: // skill type 12: flee, plain cursor
                m_cursor = ui::CursorKind::Normal;
                break;
            case 11: // skill type 11: capture
                m_cursor = ui::CursorKind::Capture;
                break;
            default: // other skills: attack cursor, or magic cursor for a learnt skill
                if (skill->category == 0) {
                    m_cursor = ui::CursorKind::Attack;
                    break;
                }
                if (user->kind == 4) { // fighter kind 4: a pet
                    const bool own = user->npc && std::find(user->npc->skills.begin(), user->npc->skills.end(), skillId) != user->npc->skills.end();
                    if (!own) {
                        m_messages.show("Haven't learnt the spell", 2000); // message display time in ms
                        return;
                    }
                } else if (skill->key == 0) {
                    return;
                } else if (!user->skillLevel || user->skillLevel(skill->key) <= 0) {
                    m_messages.show("Haven't learnt the spell", 2000); // message display time in ms
                    return;
                }
                m_cursor = ui::CursorKind::Magic;
                break;
        }
    }

    bool Battle::useListedSkill(int skillId, bool skillForm) {
        if (!m_on || m_actor == 0) {
            return false;
        }
        const int listed = skillId;
        if (const auto actor = m_field.fighter(m_actor); actor && skillSealed(*actor, isRoleSkill(skillId) ? kRoleSkillPlaceholder : skillId)) {
            m_messages.show("This skill is sealed, unable to use", 1200); // message display time in ms
            return false;
        }
        if (glazeMissing(skillId)) {
            return false;
        }
        chooseSkill(isRoleSkill(skillId) ? kRoleSkillPlaceholder : skillId);
        if (m_skill == 0) {
            return false;
        }
        if (m_skill == kRoleSkillPlaceholder) {
            if (const auto user = actorAsUser()) {
                m_skill = roleSkillId(user->bodyType, user->head);
            }
        }
        if (skillForm) {
            m_cursor = ui::CursorKind::Magic;
        }
        // target type 3: the skill goes on oneself at once
        constexpr int kOnSelf = 3;
        if (const auto skill = m_services.skills.find(skillForm ? m_skill : listed); skill && skill->targetType == kOnSelf) {
            submit(m_actor);
        }
        return true;
    }

    void Battle::submit(int targetSlot) {
        const auto actorFound = m_field.fighter(m_actor);
        const auto targetFound = m_field.fighter(targetSlot);
        if (!actorFound || !targetFound) {
            return;
        }
        const Fighter& actor = *actorFound;
        const Fighter& target = *targetFound;
        if (m_skill == 0 && m_item == 0 && target.id != m_ownId && actor.side() != target.side()) {
            chooseSkill(kAttack);
        }
        if (target.waiting) {
            m_messages.show("Unable to choose. The target is not ready.", 2000); // message display time in ms
            return;
        }
        if (m_skill == 0) {
            if (m_item != 0) {
                submitItem(actor, target, targetSlot);
            } else if (targetSlot == m_actor) {
                repeatLastCommand();
            }
            return;
        }
        const auto skill = m_services.skills.find(m_skill);
        if (!skill) {
            return;
        }
        if (skill->spCost > actor.sp) {
            m_messages.show("not enough SP", 1200); // message display time in ms
            return;
        }
        // the messages below show for 1200 ms
        switch (skill->targetType) {
            case 0: // target type 0: enemies only
                if (actor.side() == target.side()) {
                    m_messages.show("Unable to attack players on my side.", 1200);
                    return;
                }
                break;
            case 1: // target type 1: own side only
                if (actor.side() != target.side()) {
                    m_messages.show("Unable to perform on enemy", 1200);
                    return;
                }
                break;
            case 2: // target type 2: anyone
                break;
            case 3: // target type 3: oneself only
                if (targetSlot != m_actor) {
                    m_messages.show("This skill can only be used on yourself", 1200);
                    return;
                }
                break;
            case 5: // target type 5: own side, not oneself
                if (actor.side() != target.side() || targetSlot == m_actor) {
                    m_messages.show("Unable to use on the target", 1200);
                    return;
                }
                break;
            default: // unknown target type: not submitted
                return;
        }
        // skill type 8 revives; types 6, 7 and 14 heal
        const bool revives = skill->skillType == 8;
        const bool heals = skill->skillType == 6 || skill->skillType == 7 || skill->skillType == 14;
        if ((revives && target.hp > 0) || (heals && target.hp <= 0)) {
            m_messages.show("Ability is not available", 1200); // message display time in ms
            return;
        }
        if (skillSealed(actor, m_skill)) {
            m_messages.show("This skill is sealed, unable to use", 1200); // message display time in ms
            return;
        }
        if (glazeMissing(m_skill)) {
            return;
        }
        m_hooks.send(protocol::battleSkillRequest(actor.row, actor.col, target.row, target.col, m_skill, m_glazeSlot));
        rememberLastCommand(m_actor, targetSlot, m_skill);
        commandSent();
    }

    bool Battle::skillSealed(const Fighter& actor, int skillId) const {
        constexpr int kSealing = 178; // status effect 178: skills sealed
        // status places 3 and 8 can hold the sealing effect
        if (actor.statuses.main[3].effect != kSealing && actor.statuses.main[8].effect != kSealing) {
            return false;
        }
        constexpr std::array<int, 5> kAlwaysSealed{
            15222, // Collapse
            15225, // Curdle
            25067, // Fervidity
            15166, // Fast
            25147 // Deadly Hit
        };
        if (std::find(kAlwaysSealed.begin(), kAlwaysSealed.end(), skillId) != kAlwaysSealed.end()) {
            return true;
        }
        const auto skill = m_services.skills.find(skillId);
        const int type = skill ? skill->skillType : 0;
        // skill types left usable while sealed: 1, 2, 10..13 and 20
        return !(type == 1 || type == 2 || (type >= 10 && type <= 13) || type == 20);
    }

    bool Battle::glazeMissing(int skillId) {
        /// @brief A glaze skill with the bag items that serve as its glaze.
        struct Glaze {
            int skillId;
            std::array<int, 3> items;
            const char* message;
        };

        // glaze skill id (ground, water, fire, wind) and the three bag item ids that serve as its glaze
        static constexpr std::array<Glaze, 4> kGlazes{{
            // skill 17018: Gaea's Guard; items: Earth Glaze, Ground Glaze, Ground Glaze A
            {17018, {33047, 33040, 33057}, "Cannot use this skill without Ground Glaze"},
            // skill 17011: Najas's Memory; items: Ocean Glaze, Water Glaze, Water Glaze A
            {17011, {33048, 33041, 33058}, "Cannot use this skill without Water Glaze"},
            // skill 17019: Zavrl's Wing; items: Heaven Fire Glaze, Fire Glaze, Fire Glaze A
            {17019, {33049, 33042, 33059}, "Cannot use this skill without Fire Glaze"},
            // skill 17013: Gephil's Mutter; items: Gale Glaze, Wind Glaze, Wind Glaze A
            {17013, {33050, 33043, 33060}, "Cannot use this skill without Wind Glaze"},
        }};
        m_glazeSlot = 0;
        const auto glaze = std::find_if(kGlazes.begin(), kGlazes.end(), [skillId](const Glaze& g) { return g.skillId == skillId; });
        if (glaze == kGlazes.end()) {
            return false;
        }
        for (const int item : glaze->items) {
            if ((m_glazeSlot = m_hooks.bagSlotOf(item)) != 0) {
                return false;
            }
        }
        m_messages.show(glaze->message, 2000); // message display time in ms
        resetCommand();
        return true;
    }

    void Battle::commandSent() {
        m_services.playSound("sound/wav0151.wav"); // command-sent sound
        m_skill = 0;
        m_item = 0;
        m_itemSlot = 0;
        m_cursor = ui::CursorKind::Normal;
        nextCommander();
    }

    void Battle::chooseItem(int itemId, int bagSlot, int count) {
        if (!m_on || m_actor == 0 || itemId <= 0 || bagSlot <= 0) {
            return;
        }
        const auto item = m_services.items.find(itemId);
        if (!item) {
            return;
        }
        constexpr int kUnableMs = 1200; // message duration, ms
        if (count == 0) {
            m_messages.show("Not enough items.", kUnableMs);
            return;
        }
        if (item->subType == 127 || item->subType == 128) { // item sub types 127 and 128 cannot be used in combat
            m_messages.show("Unable to use in combat", kUnableMs);
            return;
        }
        constexpr int kSkillScroll = 24; // item type 24: consumable battle items (skill scrolls, Bomb, Glazes, ...)
        constexpr int kConsumable = 8;
        if (item->type == kSkillScroll) {
            if (!m_services.skills.find(item->linkedSkillId) || !m_services.motions.find(item->linkedSkillId)) {
                m_messages.show("Unable to use", kUnableMs);
                return;
            }
        } else {
            if (item->equipSlot != kConsumable) {
                m_messages.show("Unable to use", kUnableMs);
                return;
            }
            // item ids 34026, 34029, 34168: Protective EXP Pill, Protective EXP Bolus, Time Mantle
            if (itemId == 34026 || itemId == 34029 || itemId == 34168) {
                m_messages.show("Unable to use in combat", kUnableMs);
                return;
            }
            if (item->attributeKinds[0] <= 0) {
                m_messages.show("Unable to use", kUnableMs);
                return;
            }
        }
        m_item = itemId;
        m_itemSlot = bagSlot;
        m_skill = 0;
        m_cursor = ui::CursorKind::Magic;
    }

    void Battle::submitItem(const Fighter& actor, const Fighter& target, int targetSlot) {
        const auto item = m_services.items.find(m_item);
        if (!item) {
            return;
        }
        constexpr int kSkillScroll = 24; // item type 24: consumable battle items (skill scrolls, Bomb, Glazes, ...)
        int skillId = 0;
        if (item->type == kSkillScroll) {
            skillId = item->linkedSkillId;
            if (!m_services.skills.find(skillId)) {
                m_messages.show("Skill data not found.",
                                1200); // message display time in ms
                return;
            }
            if (!m_services.motions.find(skillId)) {
                m_messages.show("Skill file not found.",
                                1200); // message display time in ms
                return;
            }
        } else {
            skillId = target.isPlayer() ? 10024 : 10025; // heal skill ids: 10024 (Item Usage) on a player, 10025 (Monster Item Usage) on others
        }
        const auto skill = m_services.skills.find(skillId);
        if (!skill) {
            return;
        }
        // target type 0: enemies only, 1: own side only, 3: oneself only; messages show for 1200 ms
        if (skill->targetType == 0 && actor.side() == target.side()) {
            m_messages.show("Unable to attack players on my side.", 1200);
            return;
        }
        if ((skill->targetType == 1 || skill->targetType == 3) && actor.side() != target.side()) {
            m_messages.show("Unable to perform on enemy", 1200);
            return;
        }
        if (skill->targetType == 3 && targetSlot != m_actor) {
            m_messages.show("It can be only cast on yourself", 1200);
            return;
        }
        // item ids 33052..33055: the castle bombs (System Grenade, Korean Cake, Lollipop, Big Bowling)
        const bool bomb = m_item == 33052 || m_item == 33053 || m_item == 33054 || m_item == 33055;
        // fighter kinds 2 (player) and 4 (pet) are no targets for bombs
        if (bomb && (target.kind == 2 || target.kind == 4)) {
            m_messages.show("Unable to use on the target", 1200);
            return;
        }
        // map ids 10065, 10066, 10068: Trojan Round 1, 2 and 3, where bombs are refused
        if (bomb && m_hooks.mapId && (m_hooks.mapId() == 10065 || m_hooks.mapId() == 10066 || m_hooks.mapId() == 10068)) {
            m_messages.show("Unable to use the item here.", 1200); // message display time in ms
            return;
        }
        m_hooks.send(protocol::battleItemRequest(actor.row, actor.col, target.row, target.col, m_item, m_itemSlot));
        commandSent();
    }

    void Battle::click(core::Point point) {
        if (!m_on || m_actor == 0) {
            return;
        }
        if (const int slot = m_painter.slotAt(m_field, point)) {
            submit(slot);
        }
    }

    void Battle::command(int button) {
        if (!m_on || m_actor == 0) {
            return;
        }
        switch (button) {
            case 1: // ring button 1: defend
                chooseSkill(kDefend);
                submit(m_actor);
                break;
            case 2: // ring button 2: capture
                chooseSkill(kCapture);
                break;
            case 3: // ring button 3: flee
                chooseSkill(kFlee);
                submit(m_actor);
                break;
            case 6: // ring button 6: attack
                chooseSkill(kAttack);
                break;
            case 4: // ring button 4: items
                if (m_hooks.toggleItems) {
                    m_hooks.toggleItems();
                }
                break;
            case 7: // ring button 7: skills
                if (m_hooks.toggleSkills) {
                    m_hooks.toggleSkills();
                }
                break;
            case 5: // ring button 5: auto
                if (const int kind = commandSlotKind(m_actor); kind != 0) {
                    // sides 1 and 2 are the player and the pet; auto mode 5 runs the Electron Control's script and the Plus sets, 3 is the
                    // plain auto; flags[0] marks Plus sets
                    const bool script = m_hooks.scriptOn && m_hooks.scriptOn();
                    for (int side = 1; side <= 2; ++side) {
                        const bool plus = m_hooks.remoteOn && m_hooks.remoteOn() &&
                                          (script || (m_hooks.playSets && m_hooks.playSets(side).flags[0] != 0));
                        m_autoCommands[static_cast<std::size_t>(side)] = plus ? 5 : 3;
                    }
                    issueAutoCommand(m_autoCommands[static_cast<std::size_t>(kind)]);
                }
                break;
            case 8: // ring button 8: the last command again
                repeatLastCommand();
                break;
            default: // other buttons are not built
                core::logInfo("the battle command ", button, " is not built yet");
                break;
        }
    }

    ActionContext Battle::actionContext(std::uint64_t nowMs) {
        ActionContext context{
            m_field,
            m_camera,
            m_services.motions,
            m_services.skills,
            m_services.pictures,
            [this](int number) {
                if (const auto known = m_soundFiles.find(number); known != m_soundFiles.end()) {
                    return known->second;
                }
                std::ostringstream name;
                name << "Sound/SEB" << std::setw(4) << std::setfill('0') << number << ".WAV";
                const std::filesystem::path path = m_services.game.data(name.str());
                std::string file = std::filesystem::exists(path) ? name.str() : std::string();
                if (!file.empty()) {
                    m_services.sounds.preload(path); // loaded on a worker before the sound's delay runs out
                }
                m_soundFiles.emplace(number, file);
                return file;
            },
            // message display time in ms
            [this](const std::string& text) { m_messages.show(text, 1200); }, // message display time in ms
            m_hooks.chat,
            m_ownId,
            nowMs,
            [this](int ms) {
                if (m_services.freeFrames) {
                    m_holdMs += static_cast<std::uint64_t>(ms);
                    m_heldUntilMs = std::max(m_heldUntilMs, m_holdMs);
                    return m_holdMs;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                return core::nowMs();
            }
        };
        m_holdMs = std::max(m_holdMs, nowMs);
        context.actionStage = [this](int slot) -> std::optional<int> {
            const auto& action = m_actions[static_cast<std::size_t>(slot)];
            return action ? std::optional<int>(action->stage()) : std::nullopt;
        };
        context.skillZoomOn = m_hooks.skillZoomOn && m_hooks.skillZoomOn();
        context.skillZoom = [this](int span, int position, int targetPosition, int casterY) {
            m_services.zoom.reset(); // the skill zoom takes FZoomRect and FZoomLevel over, so a zoom of the view's own is gone
            if (m_skillZoom.decide(span, position, targetPosition)) {
                m_skillZoom.apply(casterY, m_ground.nearBottom());
            }
        };
        return context;
    }

    void Battle::freeAction(int slot, ActionContext& context) {
        auto& action = m_actions[static_cast<std::size_t>(slot)];
        if (!action) {
            return;
        }
        action->finish(context);
        action.reset();
        if (m_runningOff[static_cast<std::size_t>(slot)]) {
            m_runningOff[static_cast<std::size_t>(slot)] = false;
            leaveField(slot);
        }
    }

    void Battle::leaveField(int slot) {
        const auto found = m_field.fighter(slot);
        if (!found) {
            return;
        }
        if (slot == 1 && found->get().kind == 2 && found->get().id == m_ownId) { // slot 1 is the own player, fighter kind 2 a player
            m_ownGone = true;
            return;
        }
        m_actions[static_cast<std::size_t>(slot)].reset();
        m_runningOff[static_cast<std::size_t>(slot)] = false;
        m_field.remove(slot);
    }

    void Battle::resetForRound(ActionContext& context) {
        m_camera.reset(context.nowMs);
        m_skillZoom.reset();
        m_zoomBefore.reset();
        m_zoomAfter.reset();
        m_field.effects = ScreenEffects{};
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto& action = m_actions[static_cast<std::size_t>(slot)];
            if (action && !action->done() && action->skillId() == kFlee) {
                continue;
            }
            const auto found = m_field.fighter(slot);
            if (!found) {
                continue;
            }
            Fighter& fighter = *found;
            if (fighter.killed) {
                fighter.hp = 0;
                fighter.killed = false;
            }
            fighter.pendingHit65090 = false;
            fighter.pendingHit60011 = false;
            fighter.specialHitStarted = false;
            fighter.cmdSkillId = 0;
            if (fighter.hitKind != 0) {
                (fighter.hitKind == 1 ? fighter.pendingHit60011 : fighter.pendingHit65090) = true;
                fighter.hitKind = 0;
            }
            const core::Point cell = BattleField::cellPosition(fighter.row, fighter.col);
            const core::Point ground{cell.x + m_camera.position().x, cell.y + m_camera.position().y};
            fighter.skillMotion.setStart(ground);
            fighter.hitMotion.setStart(ground);
            fighter.jumping = false;
            fighter.jumpHeight = 0;
            fighter.skillSounds.clear();
            fighter.hitSounds.clear();
            fighter.skillMotion.reset(context.nowMs);
            fighter.hitMotion.reset(context.nowMs);
            for (EffectLayer& layer : fighter.skillLayers) {
                layer = EffectLayer{};
            }
            for (EffectLayer& layer : fighter.hitLayers) {
                layer = EffectLayer{};
            }
            fighter.cameraFollow = false;
            fighter.casting = false;
            fighter.isHit = false;
            fighter.hitEffectOn = false;
            fighter.hitAvoided = false;
            fighter.missedBy.fill(false);
            fighter.numbers = HitNumbers{};
            fighter.pending.clear();
            fighter.position = cell;
            fighter.entering = 0;
            fighter.dashing = false;
            fighter.action = fighter.side() == 1 ? 16 : 17; // standing action: 16 side 1, 17 side 2
            fighter.stand();
            fighter.holdFrame = 255; // 255: hold no particular frame
            freeAction(slot, context);
        }
    }

    void Battle::stopRunningActions(std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        m_services.zoom.reset();
        ActionContext context = actionContext(nowMs);
        resetForRound(context);
    }

    void Battle::playItems(std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        ActionContext context = actionContext(nowMs);
        resetForRound(context);
    }

    void Battle::castAttribute(const protocol::FighterCast& cast) {
        constexpr int kAttributeKinds[] = {241, 242}; // cast kinds 241 and 242: skill status effects that change an attribute
        constexpr int kWordMask = 0xFFFF; // low 16 bits of the amount
        const auto caster = m_field.fighter(m_field.slotAt(cast.casterRow, cast.casterCol));
        const auto target = m_field.fighter(m_field.slotAt(cast.targetRow, cast.targetCol));
        const auto skill = m_services.skills.find(cast.skillId);
        if (!m_on || !caster || !target || !skill || skill->statusEffect != cast.kind ||
            std::find(std::begin(kAttributeKinds), std::end(kAttributeKinds), cast.kind) == std::end(kAttributeKinds)) {
            return;
        }
        TargetEffect::Value value;
        value.attribute = cast.attribute;
        value.negative = cast.negative;
        value.hit = cast.amount & kWordMask;
        target->get().cast = value;
    }

    void Battle::marks(int count) {
        if (!m_on) {
            return;
        }
        for (int i = 0; i < count; ++i) {
            m_services.playSound("sound/wav0153.wav"); // mark sound
        }
    }

    void Battle::setFighterDamage(int kind, int row, int col, std::int32_t damage) {
        if (const auto found = m_field.fighter(m_field.slotAt(row, col))) {
            found->get().hitKind = kind == 1 ? 2 : 1; // damage kind 1 gives hit kind 2, others hit kind 1
            found->get().pendingHpDamage = damage;
        }
    }

    void Battle::setAttackArea(int row, int col, int area) {
        if (const auto found = m_field.fighter(m_field.slotAt(row, col))) {
            found->get().attackArea = area;
        }
    }

    void Battle::play(const std::vector<protocol::BattleAction>& actions, std::uint64_t nowMs) {
        m_countdown = -1;
        m_roundCountdown = -1;
        m_timeRunningOut = false;
        m_actor = 0;
        m_cursor = ui::CursorKind::Normal;
        m_commandPhase = false;
        ActionContext context = actionContext(nowMs);
        resetForRound(context);
        for (const protocol::BattleAction& action : actions) {
            const int slot = m_field.slotAt(action.row, action.col);
            const auto found = m_field.fighter(slot);
            if (!found) {
                continue;
            }
            Fighter& attacker = *found;
            bool followed = false;
            for (int other = 1; other <= BattleField::kSlots; ++other) {
                if (const auto fighter = m_field.fighter(other); fighter && fighter->get().cameraFollow) {
                    followed = true;
                    break;
                }
            }
            if (!followed) {
                attacker.cameraFollow = true;
            }
            int skillId = action.skillId;
            if (skillId == kTwinStrike && attacker.npc && attacker.npc->directionKind == 1) { // direction kind 1: the walking variant
                skillId = kTwinStrikeWalker;
            }
            for (const protocol::BattleTarget& target : action.targets) {
                TargetEffect effect;
                effect.skillId = skillId;
                effect.row = target.row;
                effect.col = target.col;
                effect.hit = target.hit;
                effect.guard = target.guard;
                for (const protocol::BattleValue& value : target.values) {
                    effect.values.push_back({value.attribute, value.amount, value.amount, 0, value.negative});
                }
                attacker.effects.push_back(std::move(effect));
            }
            attacker.cmdSkillId = skillId;
            const int targetSlot = action.targets.empty() ? 0 : m_field.slotAt(action.targets.front().row, action.targets.front().col);
            if (attacker.hp != 0) {
                freeAction(slot, context);
                m_actions[static_cast<std::size_t>(slot)].emplace(context, slot, targetSlot, skillId);
            }
        }
    }

    void Battle::setAttribute(int row, int col, int attribute, std::int32_t value) {
        const auto found = m_field.fighter(m_field.slotAt(row, col));
        if (!found) {
            return;
        }
        Fighter& fighter = *found;
        switch (attribute) {
            case kHp: // attribute 25: HP
                fighter.hp = value;
                break;
            case kSp: // attribute 26: SP
                fighter.sp = value;
                break;
            case kMaxHp: // attribute 205: max HP
                fighter.maxHp = value;
                break;
            case kMaxSp: // attribute 206: max SP
                fighter.maxSp = value;
                break;
            case kLevel: // attribute 35: level
                fighter.level = value;
                break;
            default: // other attributes are not kept
                break;
        }
    }

    void Battle::leave(int row, int col, std::uint64_t nowMs) {
        const int slot = m_field.slotAt(row, col);
        const auto found = m_field.fighter(slot);
        if (!found) {
            return;
        }
        const Fighter& fighter = *found;
        int count = 0;
        for (int other = 1; other <= BattleField::kSlots; ++other) {
            count += m_field.fighter(other) ? 1 : 0;
        }
        clearStatusIcons(*found);
        // fighter kind 2: a player (with an owner: leaves at once)
        if (fighter.flyAwayMs || fighter.dead || count >= BattleField::kSlots || (fighter.kind == 2 && fighter.ownerId != 0)) {
            leaveField(slot);
            return;
        }
        ActionContext context = actionContext(nowMs);
        freeAction(slot, context);
        m_actions[static_cast<std::size_t>(slot)].emplace(context, slot, slot, kFlee);
        m_runningOff[static_cast<std::size_t>(slot)] = true;
    }

    void Battle::setStatus(
        int row,
        int col,
        int kind,
        int skillId,
        int place
    ) {
        const auto found = m_field.fighter(m_field.slotAt(row, col));
        if (!found) {
            return;
        }
        client::setStatus(found->get().statuses, m_services.skills, kind, skillId, place);
        if (skillId == 0) {
            found->get().holdFrame = 255; // 255: hold no particular frame
        }
    }

    void Battle::clearStatusIcons(Fighter& fighter) {
        const auto own = m_field.fighter(1);
        if (own && own->get().side() == fighter.side() && fighter.ownerId == 0) {
            fighter.statuses.icons.clear();
            fighter.statuses.special = {};
        }
    }

    void Battle::knockOut(int row, int col, std::uint64_t nowMs) {
        const auto found = m_field.fighter(m_field.slotAt(row, col));
        if (!found) {
            return;
        }
        ActionContext context = actionContext(nowMs);
        resetForRound(context);
        clearStatusIcons(*found);
        Fighter& fighter = *found;
        fighter.flyAwayMs = nowMs;
        fighter.flyRise = 0;
        fighter.fall();
        fighter.action = fighter.side() == 1 ? 26 : 27; // lying action: 26 side 1, 27 side 2
    }

    void Battle::acted(int row, int col) {
        if (const auto found = m_field.fighter(m_field.slotAt(row, col))) {
            found->get().acted = true;
        }
    }

    void Battle::updateTimeLimit(std::uint64_t nowMs) {
        // 1000 ms: one clock second; 180 s: the 3-minute limit; messages show for 4000 ms
        if (!m_timeLimitOn || m_timeLeft <= 0 || nowMs - m_timeTickMs < 1000) {
            return;
        }
        if (m_timeLeft == 180) {
            m_messages.show("Notice, 3 minutes left", 4000);
        }
        --m_timeLeft;
        m_timeTickMs = nowMs;
        if (m_timeLeft == 0) {
            m_messages.show("Battle ended", 4000);
        }
    }

    void Battle::startTimeLimit(std::uint64_t nowMs) {
        m_timeLimitOn = true;
        m_timeLeft = 180; // the battle's 3-minute limit in seconds
        m_timeTickMs = nowMs;
    }

    void Battle::resetTimeLimit(std::uint64_t nowMs) {
        m_timeTickMs = nowMs;
        m_messages.show("Battle time is reset.", 3000); // message display time in ms
        m_timeLimitOn = false;
    }

    bool Battle::useDelayClock(int bagSlot) {
        constexpr int kNoDelayTypes[] = {3, 4, 5, 8, 9, 11}; // battle types that have no delay
        if (!m_on || m_actor == 0 || std::find(std::begin(kNoDelayTypes), std::end(kNoDelayTypes), m_type) != std::end(kNoDelayTypes)) {
            return false;
        }
        const auto actor = m_field.fighter(m_actor);
        if (!actor) {
            return false;
        }
        m_hooks.send(protocol::delayClockRequest(bagSlot));
        m_hooks.send(protocol::battleItemRequest(actor->get().row, actor->get().col, actor->get().row, actor->get().col, 0, 0));
        commandSent();
        return true;
    }

    void Battle::end() {
        m_services.zoom.reset();
        m_skillZoom.reset();
        m_zoomBefore.reset();
        m_zoomAfter.reset();
        m_on = false;
        for (auto& action : m_actions) {
            action.reset();
        }
        m_runningOff.fill(false);
        m_field.clear();
        m_members.clear();
        m_cursor = ui::CursorKind::Normal;
        m_actor = 0;
        m_countdown = -1;
        m_commandPhase = false;
        m_timeLimitOn = false;
        m_timeLeft = 180; // the battle's 3-minute limit in seconds
    }

    void Battle::throwItem(int itemId, int row, int col, const std::vector<std::pair<int, int>>& targets) {
        const auto source = m_field.fighter(m_field.slotAt(row, col));
        // the grid has 4 rows and 4 columns
        if (!m_on || itemId == 0 || !source || row < 1 || row > 4 || col < 1 || col > 4) {
            return;
        }
        for (const auto& [targetRow, targetCol] : targets) {
            source->get().thrown.push_back({targetRow, targetCol, itemId});
        }
    }

    void Battle::moveFighter(int fromRow, int fromCol, int toRow, int toCol) {
        const int slot = m_field.slotAt(fromRow, fromCol);
        const auto fighter = m_field.fighter(slot);
        // the grid has 4 rows and 4 columns
        if (!m_on || !fighter || toRow < 1 || toRow > 4 || toCol < 1 || toCol > 4) {
            return;
        }
        const int there = m_field.slotAt(toRow, toCol);
        if (there != 0 && there != slot && m_runningOff[static_cast<std::size_t>(there)]) {
            ActionContext context = actionContext(0);
            freeAction(there, context);
            m_field.remove(there);
            m_members.erase(std::remove(m_members.begin(), m_members.end(), there), m_members.end());
        }
        Fighter& moved = fighter->get();
        moved.row = toRow;
        moved.col = toCol;
        moved.position = BattleField::cellPosition(toRow, toCol);
        moved.heading = moved.position;
    }

    void Battle::update(std::uint64_t nowMs) {
        m_services.pictures.pumpPrefetch(nowMs); // one finished effect picture a frame, before its layer starts
        updateTimeLimit(nowMs);
        if (!m_on) {
            m_tween.clear();
            return;
        }
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_field.fighter(slot);
            if (!found || found->get().thrown.empty() || !found->get().killed || found->get().hp > 0 || found->get().hitEffectOn || found->get().casting) {
                continue;
            }
            Fighter& fighter = *found;
            for (const ThrownItem& item : fighter.thrown) {
                const auto record = m_services.items.find(item.itemId);
                auto picture = record ? m_services.pictures.find(std::to_string(record->iconId)) : std::nullopt;
                if (!picture) {
                    picture = m_services.pictures.find("1000"); // picture 1000: the icon used when the item has none
                }
                if (!picture) {
                    continue;
                }
                LightEffect::Setup setup;
                setup.picture = *picture;
                setup.frameMs = 10; // frame time in ms
                setup.motion = LightEffect::Motion::Arc;
                setup.loop = false;
                setup.target = BattleField::cellPosition(item.row, item.col);
                setup.speed = 0.36; // flight speed in px per ms
                m_itemFlights.emplace_back().start(setup, BattleField::cellPosition(fighter.row, fighter.col), nowMs);
            }
            fighter.thrown.clear();
        }
        for (LightEffect& flight : m_itemFlights) {
            flight.update(nowMs);
        }
        m_itemFlights.erase(
            std::remove_if(m_itemFlights.begin(), m_itemFlights.end(), [](const LightEffect& flight) { return !flight.isActive(); }),
            m_itemFlights.end()
        );
        constexpr double kFlySpeed = 0.24; // fly-away speed, px per ms
        constexpr int kFlyHeight = 300; // px a fighter flies up before leaving the field
        ActionContext context = actionContext(nowMs);
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_field.fighter(slot);
            if (!found) {
                continue;
            }
            Fighter& fighter = *found;
            // entering phases 1 (set off) and 2 (moving onto the cell)
            if (fighter.entering == 1 || fighter.entering == 2) {
                updateEntering(fighter, nowMs);
            }
            fighter.texts.update(nowMs);
            if (fighter.flyAwayMs) {
                fighter.flyRise = static_cast<int>(std::lrint(kFlySpeed * static_cast<double>(nowMs - *fighter.flyAwayMs)));
                if (fighter.flyRise >= kFlyHeight) {
                    fighter.flyAwayMs.reset();
                    leaveField(slot);
                }
            }
        }
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto entering = m_field.fighter(slot); entering && entering->get().entering >= 3) { // entering phases 3 and 4: on the cell
                tickEntering(*entering);
            }
            auto& action = m_actions[static_cast<std::size_t>(slot)];
            if (!action || !m_field.fighter(slot)) {
                continue;
            }
            if (action->done()) {
                freeAction(slot, context);
                continue;
            }
            action->update(context);
        }
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto fighter = m_field.fighter(slot)) {
                updateStatusMotions(*fighter, context);
            }
        }
        const auto play = [this](const std::string& file) { m_services.playSound(file); };
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_field.fighter(slot);
            if (!found) {
                continue;
            }
            Fighter& fighter = *found;
            if (const auto& action = m_actions[static_cast<std::size_t>(slot)]; action && action->effectsActive()) {
                fighter.skillSounds.play(play);
            }
            if (fighter.isHit && fighter.hitEffectOn) {
                fighter.hitSounds.play(play);
            }
        }
        if (m_countdown >= 0 && nowMs - m_countdownMs >= kSecondMs) {
            --m_countdown;
            m_countdownMs = nowMs;
            if (m_countdown >= 0 && m_countdown < kBeepFrom) {
                m_services.playSound("sound/wav0145.wav"); // countdown beep
            }
            if (m_countdown < 0) {
                m_actor = 0;
                m_cursor = ui::CursorKind::Normal;
                if (m_hooks.hideItems) {
                    m_hooks.hideItems();
                }
            }
        }
        if (m_roundCountdown >= 0 && nowMs - m_roundCountdownMs >= kSecondMs) {
            --m_roundCountdown;
            m_roundCountdownMs = nowMs;
            if (m_roundCountdown == kRunningOut) {
                m_timeRunningOut = true;
            } else if (m_roundCountdown < 0) {
                m_timeRunningOut = false;
            }
        }
        if (m_services.freeFrames) {
            m_tween.record(m_camera, m_field, context.nowMs);
            m_zoomBefore = m_zoomAfter;
            m_zoomAfter = m_skillZoom.part();
            m_zoomTickMs = context.nowMs;
        }
    }

    void Battle::beginPresent(std::uint64_t nowMs) {
        if (m_services.freeFrames && m_on) {
            m_tween.apply(m_camera, m_field, nowMs, m_services.nextBeatMs);
            const core::Rect screen{0, 0, SkillZoom::kScreenWidth, SkillZoom::kScreenHeight};
            const double amount = BattleTween::progress(m_zoomTickMs, nowMs, m_services.nextBeatMs);
            const core::Rect shown = core::between(m_zoomBefore.value_or(screen), m_zoomAfter.value_or(screen), amount);
            m_zoomPresented = shown == screen ? std::nullopt : std::optional<core::Rect>(shown);
        }
    }

    void Battle::endPresent() {
        m_tween.restore(m_camera, m_field);
        m_zoomPresented.reset();
    }

    std::optional<core::Rect> Battle::skillZoomPart() const { return m_zoomPresented ? *m_zoomPresented : m_skillZoom.part(); }

    void Battle::paintGround(gfx::Canvas& canvas) const {
        const ScreenEffects& effects = m_field.effects;
        m_ground.draw(canvas, m_camera, effects.shaking ? effects.shake : core::Point{});
        if (effects.fading) {
            canvas.fillRect(canvas.window(), {0, 0, 0, static_cast<std::uint8_t>(255 - effects.fadeLevel)}); // black overlay; alpha 255 is opaque
        }
    }

    void Battle::paintLayers(gfx::Canvas& canvas, bool front, std::uint64_t nowMs) {
        const core::Point shake = m_field.effects.shake;
        const auto hitLayers = [&] {
            for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
                const auto found = m_field.fighter(slot);
                if (!found || !found->get().isHit || !found->get().hitEffectOn) {
                    continue;
                }
                const int attacker = found->get().hitAttacker;
                if (attacker < 1 || attacker > BattleField::kSlots) {
                    continue;
                }
                if (const auto& action = m_actions[static_cast<std::size_t>(attacker)]) {
                    action->drawHitLayers(canvas, *found, front, shake);
                }
            }
        };
        const auto skillLayers = [&] {
            for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
                const auto found = m_field.fighter(slot);
                if (const auto& action = m_actions[static_cast<std::size_t>(slot)]; found && action) {
                    action->drawCasterLayers(canvas, *found, front, shake, [&](Fighter& caster, const EffectLayer& layer) {
                        m_painter.drawCutIn(canvas, caster, layer, nowMs);
                    });
                }
            }
        };
        if (front) {
            skillLayers();
            hitLayers();
        } else {
            hitLayers();
            skillLayers();
        }
    }

    void Battle::paintNumbers(gfx::Canvas& canvas) {
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = m_field.fighter(slot);
            if (!found) {
                continue;
            }
            Fighter& fighter = *found;
            if (const auto motion = m_services.motions.find(fighter.effectMotionId)) {
                fighter.numbers.draw(canvas, *motion, fighter.side(), m_camera.position().x);
            } else if (fighter.effectMotionId == 0) {
                fighter.numbers.drawSingle(canvas, fighter.side(), m_camera.position().x);
            }
            fighter.texts.draw(canvas);
        }
    }

    void Battle::paintField(gfx::Canvas& canvas, core::Point mouse, std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        const int hovered = m_painter.slotAt(m_field, mouse);
        paintLayers(canvas, false, nowMs);
        m_painter.paintFighters(canvas, m_field, hovered, nowMs);
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto fighter = m_field.fighter(slot)) {
                drawStatusMotions(canvas, *fighter, m_services.skills, m_services.motions, m_field.effects.shake);
            }
        }
        paintLayers(canvas, true, nowMs);
        paintNumbers(canvas);
        for (LightEffect& flight : m_itemFlights) {
            flight.draw(canvas, nowMs);
        }
        m_painter
            .paintClock(canvas, m_field, {m_countdown, m_type, m_timeRunningOut, m_timeLimitOn ? m_timeLeft : -1}, nowMs); // -1: no battle time limit clock
    }

    void Battle::paintLabels(gfx::Canvas& canvas, core::Point mouse) {
        if (!m_on) {
            return;
        }
        const std::optional<core::Rect> zoomed = skillZoomPart();
        m_painter.paintNames(canvas, m_field, zoomed && zoomed->y > 0 ? std::optional<float>(m_skillZoom.scale()) : std::nullopt);
        if (m_commandPhase || m_countdown >= 0) {
            if (const auto hovered = m_field.fighter(m_painter.slotAt(m_field, mouse))) {
                m_painter.paintInfo(canvas, *hovered);
            }
        }
    }

}
