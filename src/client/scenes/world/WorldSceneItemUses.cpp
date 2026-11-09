#include "client/scenes/world/WorldScene.h"

#include "client/data/RoleOrder.h"
#include "client/rules/BoxItems.h"
#include "client/rules/ThrowItems.h"
#include "client/rules/PotentialRules.h"
#include "client/rules/Vitality.h"
#include "client/rules/WinPlateRule.h"
#include "client/rules/CharacterActions.h"
#include "protocol/EmotionRequests.h"
#include "protocol/ItemRequests.h"
#include "protocol/StatusEffectRequests.h"
#include "protocol/StoreroomRequests.h"

#include <algorithm>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kPurpleKeys[] = {30334, 30371}; // both items are named Purple Chest Key
        constexpr int kForgetScrolls[] = {34089, 34122}; // Forgotten Scroll, Lethe Scroll
        constexpr int kPetSwapScrolls[] = {34137, 34139}; // Pet Lethe Scroll, Pet Return Scroll
        constexpr int kRiceballType = 66; // item sub type 66: riceballs
        constexpr int kWinPlate = 34165; // item 34165: Win Plate
        constexpr int kTimeMantle = 34168; // item 34168: Time Mantle
        constexpr int kDelayClock = 34166; // item 34166: Delay Clock
        constexpr int kEscapeButton = 34164; // item 34164: Escape Button
        constexpr int kYesNoBoard = 34378; // item 34378: Yes-No Board
        constexpr int kVacuumCleanerSubType = 117; // item sub type 117: vacuum cleaners
        constexpr int kStrongVacuumCleaner = 34129; // item 34129: Big Vacuum Cleaner
        constexpr int kAutoCollectAction = 16; // character action group 16: auto-collecting pose
        constexpr std::uint64_t kAutoCollectMs = 190000; // Big Vacuum Cleaner collect interval, ms (190 s)
        constexpr std::uint64_t kSlowAutoCollectMs = 310000; // other vacuum cleaners collect interval, ms (310 s)
        constexpr int kBodyShiftMapA = 10085; // map 10085: Wedding Hall
        constexpr int kBodyShiftMapB = 10079; // map 10079: Quiz Zone
        constexpr int kHouseCellar = 4; // house room 4: Love cabin (map file 10084)
        constexpr int kMessageMs = 2000; // message display time, ms
        const std::vector<std::string> kAbilities{"Strength ( Str )", "Constitution ( Con )", "Intelligence ( Int )", "Wisdom ( Wis )", "Agility ( Agi )"};
        const char* const kAbilityNames[] = {"Str", "Con", "Int", "Wis", "Agi"};

        /// @brief Tells whether an item id is one of the two ids of a list.
        /// @param itemId The item id to look for.
        /// @param list The two item ids.
        /// @return True when itemId equals either entry.
        bool oneOf(int itemId, const int (&list)[2]) { return itemId == list[0] || itemId == list[1]; }

        /// @brief Builds the talk dialog timing of the Se_TalkMsgFormPlus_2 box (lines of 32 characters, game windows stay usable).
        /// @return The timing to pass to the ask/choose dialogs.
        TalkDialog::Timing requestBox() {
            TalkDialog::Timing timing;
            timing.wrapChars = 32; // characters per line of the Se_TalkMsgFormPlus_2 box
            timing.exclusive = false;
            return timing;
        }

    }

    bool WorldScene::bagHasFreeCell() const {
        for (int cell = 1; cell <= Inventory::kBagSlots; ++cell) {
            if (m_inventory.occupantOf(cell) == 0) {
                return true;
            }
        }
        return false;
    }

    int WorldScene::purpleKeySlot() const {
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.count > 0 && (item.itemId == kPurpleKeys[0] || item.itemId == kPurpleKeys[1])) {
                return slot;
            }
        }
        return 0;
    }

    bool WorldScene::useRequestItem(int itemId, int bagSlot, int petOrder) {
        const auto record = m_services.items.find(itemId);
        constexpr int kLoveFireworks = 143; // item sub type 143: love fireworks
        constexpr int kYearFireworks = 144; // item sub type 144: new-year fireworks
        if (record && (record->subType == kLoveFireworks || record->subType == kYearFireworks) && m_fireworkOwn) {
            m_messages.show("Fireworks is being used", 1200); // message display time in ms
            return true;
        }
        if (record && record->subType == kRiceballType) {
            useRiceball(bagSlot);
            return true;
        }
        if (record && record->subType == BoxItems::kCompanionVoucherSubType) {
            m_services.connection.send(protocol::companionVoucherRequest(bagSlot));
            return true;
        }
        if (itemId == BoxItems::kSuperHandbag) {
            if (m_fightState == 0) {
                m_services.connection.send(protocol::superHandbagRequest());
            }
            return true;
        }
        if (record && record->type == BoxItems::kMallGiftBoxType) {
            m_services.connection.send(protocol::mallGiftBoxRequest(bagSlot));
            return true;
        }
        if (itemId == BoxItems::kSweetsPack || BoxItems::opensWithBox(itemId) || BoxItems::opensWithBox2(itemId) ||
            (record && record->type == BoxItems::kLuckyBagType)) {
            int key = 0;
            if (itemId == BoxItems::kPurpleChest) {
                key = purpleKeySlot();
                if (key == 0) {
                    m_messages.show("Only the Purple Chest Key can open it", kMessageMs);
                    return true;
                }
            }
            if (!bagHasFreeCell()) {
                m_messages.show("You should unfill at least 2 items space", kMessageMs);
                return true;
            }
            if (itemId == BoxItems::kSweetsPack) {
                m_services.connection.send(protocol::sweetsPackRequest(bagSlot));
            } else if (BoxItems::opensWithBox(itemId)) {
                m_services.connection.send(protocol::openBoxRequest(bagSlot, key));
            } else {
                m_services.connection.send(protocol::openBox2Request(bagSlot, 0));
            }
            return true;
        }
        if (oneOf(itemId, kForgetScrolls)) {
            useForgetScroll(bagSlot, petOrder);
            return true;
        }
        if (oneOf(itemId, kPetSwapScrolls)) {
            usePetSwapScroll(bagSlot, petOrder);
            return true;
        }
        if (ThrowItems::kindOf(itemId) != 0) {
            if (m_fightState != 0) {
                m_messages.show("Unable to use in combat", 1000); // message display time in ms
            } else if (m_player.usedUtensil != 0) {
                m_messages.show("Unable to use", 1000); // message display time in ms
            } else {
                m_throwSlot = bagSlot;
                m_throwKind = ThrowItems::kindOf(itemId);
                setMouseMode(kThrowMouseMode);
            }
            return true;
        }
        if (itemId == kWinPlate) {
            useWinPlate(bagSlot);
            return true;
        }
        if (itemId == kTimeMantle) {
            useTimeMantle(bagSlot);
            return true;
        }
        if (itemId == kDelayClock) {
            useDelayClock(bagSlot);
            return true;
        }
        if (itemId == kEscapeButton) {
            useEscapeButton(bagSlot, itemId);
            return true;
        }
        if (itemId == VoyageCompassWindow::kCompassItem) {
            m_voyageCompass.use(bagSlot);
            return true;
        }
        if (itemId == kYesNoBoard) {
            useYesNoBoard(bagSlot);
            return true;
        }
        return false;
    }

    void WorldScene::useForgetScroll(int bagSlot, int petOrder) {
        if (petOrder != 0) {
            m_messages.show("This item can be used only on the player", 1500); // message display time in ms
            return;
        }
        m_talk.choose(
            "Using this item, the #B#R a skill will be forgotten/#B/#R, please select which skill ",
            std::nullopt,
            kAbilities,
            [this, bagSlot](int row) { confirmForgetAbility(bagSlot, row); },
            [] {},
            requestBox()
        );
    }

    void WorldScene::confirmForgetAbility(int bagSlot, int row) {
        if (row < 0 || row > 4 || m_inventory.bagItem(bagSlot).empty()) { // 4 = last ability row (Agi)
            return;
        }
        const std::size_t stat = static_cast<std::size_t>(row);
        if (kPetStatRequirements[static_cast<std::size_t>(m_status.potential)] >= m_status.stats[stat]) {
            m_messages.show("This ability without any points can be forgotten", 1500); // message display time in ms
            return;
        }
        std::array<int, 5> stats = m_status.stats;
        --stats[stat];
        std::string names;
        for (const int skillId : lostSkills(m_services.skills, m_status, m_status.element, stats)) {
            if (const auto skill = m_services.skills.find(skillId)) {
                names += (names.empty() ? "" : "\r") + skill->name;
            }
        }
        const std::string text =
            names.empty() ? "The ability points will return to 1, are you sure? " : "Will forget the following skill, are you sure? \r" + names;
        m_talk.ask(text, std::nullopt, [this, bagSlot, row] { m_services.connection.send(protocol::forgetAbilityRequest(bagSlot, 1, 0, row + 1)); }, [] {});
    }

    void WorldScene::usePetSwapScroll(int bagSlot, int petOrder) {
        if (petOrder < 1 || petOrder > 4) { // pet orders 1 to 4 are valid
            m_messages.show("This skill can only be used on pet", 1500); // message display time in ms
            return;
        }
        const int place = m_pets.placeByOrder(petOrder);
        const auto pet = place != 0 ? m_pets.at(place) : std::nullopt;
        if (!pet) {
            return;
        }
        if (totalLevel(pet->level, pet->reborn) < 20) { // minimum pet level
            m_messages.show("Level requirements not met, unable to use.", 1500); // message display time in ms
            return;
        }
        if (pet->info.mercenary != 0) {
            m_messages.show("The target is unavailable!", 1500); // message display time in ms
            return;
        }
        m_talk.choose(
            "Please select the ability to forget",
            std::nullopt,
            kAbilities,
            [this, bagSlot, place](int row) { selectPetAbilityToForget(bagSlot, place, row); },
            [] {},
            requestBox()
        );
    }

    namespace {

        /// @brief Gives the current value of one of a pet's five abilities.
        /// @param pet The pet.
        /// @param row Ability row: 0 Str, 1 Con, 2 Int, 3 Wis, 4 Agi.
        /// @return The ability value.
        int petStat(const Pet& pet, int row) {
            const int values[] = {pet.strength, pet.constitution, pet.intelligence, pet.wisdom, pet.agility};
            return values[row];
        }

        /// @brief Gives the base (species) value of one of a pet's five abilities.
        /// @param pet The pet.
        /// @param row Ability row: 0 Str, 1 Con, 2 Int, 3 Wis, 4 Agi.
        /// @return The base ability value.
        int petBaseStat(const Pet& pet, int row) {
            const int values[] = {pet.info.strength, pet.info.constitution, pet.info.intelligence, pet.info.wisdom, pet.info.agility};
            return values[row];
        }

    }

    void WorldScene::selectPetAbilityToForget(int bagSlot, int place, int row) {
        const auto pet = m_pets.at(place);
        if (m_fightState != 0 || !pet || row < 0 || row > 4) { // 4 = last ability row (Agi)
            return;
        }
        if (petStat(*pet, row) <= 0) {
            m_messages.show("This ability without any points can be forgotten", 1500); // message display time in ms
            return;
        }
        if ((petBaseStat(*pet, row) >> 1) + kPetStatRequirements[static_cast<std::size_t>(pet->potential)] >= petStat(*pet, row)) {
            m_messages.show("Unable to forget basic ability", 1500); // message display time in ms
            return;
        }
        m_talk.choose(
            "Please select the ability to add",
            std::nullopt,
            kAbilities,
            [this, bagSlot, place, row](int added) { selectPetAbilityToAdd(bagSlot, place, row, added); },
            [] {},
            requestBox()
        );
    }

    void WorldScene::selectPetAbilityToAdd(int bagSlot, int place, int forgotten, int added) {
        const auto pet = m_pets.at(place);
        if (m_fightState != 0 || !pet || added < 0 || added > 4) { // 4 = last ability row (Agi)
            return;
        }
        if (added == forgotten) {
            m_messages.show("Forgotten ability and added ability are the same", 1500); // message display time in ms
            return;
        }
        constexpr int kAddAmount = 1;
        constexpr int kAbilityLimit = 602; // points a pet ability may rise above the requirement for its potential
        if (kAddAmount + petStat(*pet, added) > kPetStatRequirements[static_cast<std::size_t>(pet->potential)] + kAbilityLimit) {
            m_messages.show("The ability you want to increase has reached its limit", 2000); // message display time in ms
            return;
        }
        const auto line = [&](const char* label, int row, int delta) {
            return std::string(label) + kAbilityNames[row] + "(" + std::to_string(petStat(*pet, row)) + "->" + std::to_string(petStat(*pet, row) + delta) + ")";
        };
        const std::string text = "Character: " + (pet->name.empty() ? pet->info.name : pet->name) + "\r" + line("Ability to forget: ", forgotten, -kAddAmount) +
                                 "\r" + line("Ability to increase: ", added, kAddAmount) + "\rAre you sure you want to use?";
        m_talk.ask(
            text,
            std::nullopt,
            [this, bagSlot, place, forgotten, added] {
                m_services.connection.send(protocol::petAbilitySwapRequest(bagSlot, 1, place, forgotten + 1, added + 1));
            },
            [] {}
        );
    }

    void WorldScene::onFirework(std::int32_t playerId, int kind, int variant) {
        const auto character = characterOf(playerId);
        if (!character) {
            return;
        }
        const auto found = m_fireworks.try_emplace(playerId, m_services.pictures, [this](const std::string& sound) { m_services.playSound(sound); }).first;
        found->second.start(kind, variant, character->get().position, m_nowMs);
        if (isOwn(playerId)) {
            m_fireworkOwn = true;
        }
    }

    void WorldScene::onThrowEffect(int kind, std::int32_t playerId, int x, int y) {
        if (const auto character = characterOf(playerId)) {
            throwPose(character->get(), {x, y}, isOwn(playerId));
            m_throwShow.start(kind, character->get().position, {x, y}, m_nowMs);
        }
    }

    void WorldScene::throwPose(MapCharacter& thrower, core::Point to, bool own) {
        constexpr int kThrowAction = 34; // character action group 34: throw pose (35 when throwing to the right)
        constexpr int kSafetyMs = 2000; // time after which the throw pose ends anyway, ms
        constexpr int kRidingAction = 18; // character action group 18: riding
        constexpr int kSwimmingAction = 66; // character action group 66: swimming
        constexpr int kAcross = 150; // half width of the "level" band around the thrower, px
        constexpr int kDown = 100; // half height of the "level" band around the thrower, px
        thrower.hit.reset();
        if (thrower.walking || thrower.riding || thrower.action == kRidingAction || thrower.action == kSwimmingAction) {
            return;
        }
        const core::Point from = thrower.position;
        const int dirX = from.x + kAcross < to.x ? 2 : (from.x - kAcross > to.x ? 0 : 1);
        const int dirY = from.y + kDown < to.y ? 2 : (from.y - kDown > to.y ? 0 : 1);
        const bool right = to.x > from.x;
        // standing slots 8..15: 14 right, 10 own left, 14 other left
        int standing = right ? 14 : (own ? 10 : 14);
        // dirX*10+dirY with 0 = point left/above, 1 = within the box, 2 = right/below
        switch (dirX * 10 + dirY) {
            case 0: standing = 9; break; // left, above: standing slot 9
            case 1: standing = 10; break; // left, level: standing slot 10
            case 2: standing = 11; break; // left, below: standing slot 11
            case 10: standing = 8; break; // level, above: standing slot 8
            case 12: standing = 12; break; // level, below: standing slot 12
            case 20: standing = 15; break; // right, above: standing slot 15
            case 21: standing = 14; break; // right, level: standing slot 14
            case 22: standing = 13; break; // right, below: standing slot 13
            default: break; // point within the box on both axes: keep the default slot
        }
        startActionSlot(thrower, kThrowAction + (right ? 1 : 0));
        thrower.animation.restart();
        thrower.throwPose = ThrowPose{m_nowMs + kSafetyMs, standing - 8}; // standing slots start at 8: facing = slot - 8
    }

    void WorldScene::updateThrowActions() {
        const auto update = [this](MapCharacter& character) {
            if (character.throwPose && character.throwPose->finished(character.animation.current(), m_nowMs, character.action)) {
                if (character.action != 0) {
                    stopAction(character);
                    character.direction = character.throwPose->facing;
                }
                character.throwPose.reset();
            }
            if (character.hit && character.hit->at != character.position) {
                character.hit.reset();
            }
        };
        update(m_player);
        for (auto& [id, other] : m_others) {
            update(other);
        }
    }

    void WorldScene::hitByThrow(int kind, core::Point target) {
        constexpr int kHitAction = 30; // character action group 30: hit reaction
        constexpr int kClayFrames = 5; // frames of the clay hit sprite (a random one is picked)
        const int halfWidth = kind == 1 ? 50 : 60; // half width of the hit box in pixels (first kind, others)
        const int halfHeight = kind == 1 ? 30 : 40; // half height of the hit box in pixels (first kind, others)
        const int lift = kind == 1 ? 30 : 65; // pixels the box centre lies over the target point (first kind, others)
        const int centerY = target.y - lift;
        const auto react = [&](MapCharacter& character) {
            const core::Point feet = character.position;
            if (feet.x < target.x - halfWidth || feet.x >= target.x + halfWidth || feet.y < centerY - halfHeight || feet.y >= centerY + halfHeight) {
                return;
            }
            if (character.fishing || character.bathing || character.usedUtensil != 0 || character.fightState != 0 || character.vehicle || character.riding) {
                return;
            }
            if (!character.walking) {
                startAction(character, kHitAction);
            }
            character.hit.reset();
            if (kind == 2) { // throw kind 2: clay
                character.hit = HitReaction{1, std::rand() % kClayFrames, feet}; // reaction type 1 (clay sprite 9081)
            } else if (kind == 3) { // throw kind 3
                character.hit = HitReaction{2, 0, feet}; // reaction type 2
            }
            std::optional<std::string> voice;
            switch (roleOrder(character.look.bodyType, character.look.head)) {
                case 1: voice = "sound/SEB0173.wav"; break; // role order 1: voice SEB0173
                case 2: // role order 2: voice SEB0221
                case 3: voice = "sound/SEB0221.wav"; break; // role order 3: voice SEB0221
                case 4: // role order 4: voice SEB0156
                case 5: // role order 5: voice SEB0156
                case 6: // role order 6: voice SEB0156
                case 14: voice = "sound/SEB0156.wav"; break; // role order 14: voice SEB0156
                case 7: // role order 7: voice SEB0199
                case 8: // role order 8: voice SEB0199
                case 9: // role order 9: voice SEB0199
                case 10: // role order 10: voice SEB0199
                case 11: // role order 11: voice SEB0199
                case 12: // role order 12: voice SEB0199
                case 13: // role order 13: voice SEB0199
                case 15: voice = "sound/SEB0199.wav"; break; // role order 15: voice SEB0199
                default: break; // other roles: no voice
            }
            if (voice) {
                m_services.playSound(*voice);
            }
        };
        react(m_player);
        for (auto& [id, other] : m_others) {
            react(other);
        }
    }

    void WorldScene::drawFireworks(gfx::Canvas& canvas) {
        updateThrowActions();
        m_throwShow.draw(canvas, m_camera.origin(), m_nowMs);
        for (auto it = m_fireworks.begin(); it != m_fireworks.end();) {
            if (it->second.draw(canvas, m_camera.origin(), m_nowMs)) {
                ++it;
            } else {
                m_fireworkOwn = false;
                it = m_fireworks.erase(it);
            }
        }
    }

    void WorldScene::onDiscardAsked(int reason, int bagSlot, int itemId, int count) {
        const auto record = m_services.items.find(itemId);
        if (!record) {
            return;
        }
        const std::string name = "#R" + record->name + "/#R";
        std::string text;
        switch (reason) {
            case 1: // reason 1: below level 10
                text = "Less than level 10\r" + name + " Abandon means destroy,Sure? ";
                break;
            case 2: // reason 2: not enough online time
                text = "The online time is not enough\241I\r" + name + " Abandon means destroy,Sure? ";
                break;
            case 3: // reason 3: style and equipment
                text = "Style and equipment" + name + " Abandon means destroy,\rSure? ";
                break;
            default: // any other reason: plain question
                text = name + " Abandon means destroy,Sure? ";
                break;
        }
        m_talk
            .ask(text, std::nullopt, [this, bagSlot, count] { m_services.connection.send(protocol::discardItemRequest(bagSlot, count)); }, [] {}, requestBox());
    }

    std::optional<std::string> WorldScene::bodyShiftRefusal(bool riceball) const {
        const int mapId = m_map ? m_map->id() : 0;
        std::optional<std::string> refusal;
        if (m_fightState != 0) {
            refusal = "Unable to use in combat";
        } else if (m_player.bathing) {
            refusal = "Can't use during washing";
        } else if (m_player.inShallowWater()) {
            refusal = "Can't use when you are in water";
        } else if (m_player.vehicle) {
            refusal = "Can't use when you are using vehicle.";
        } else if (m_player.riding) {
            refusal = "Can't use when you are riding pets";
        } else if (m_player.usedUtensil != 0) {
            refusal = riceball ? "You are unable to use the Rice Balls." : "You are unable to shapeshift";
        } else if (mapId == kBodyShiftMapA || mapId == kBodyShiftMapB || m_houseRoom == kHouseCellar) {
            refusal = "The scene can't be used";
        }
        return refusal;
    }

    void WorldScene::useRiceball(int bagSlot) {
        if (const auto refusal = bodyShiftRefusal(true)) {
            m_messages.show(*refusal, kMessageMs);
            return;
        }
        const auto send = [this, bagSlot] { m_services.connection.send(protocol::bodyShiftUseRequest(bagSlot)); };
        if (m_statusEffects.bodyShiftState() != 0) {
            m_talk.ask(
                "Count the time of body shifting again, are you sure?",
                std::nullopt,
                send,
                [] {},
                TalkDialog::Timing{32, 5000, [] {}, false}
            ); // 32 chars per line, 5000 ms timeout
        } else {
            send();
        }
    }

    void WorldScene::useBodyShiftIcon() {
        if (const auto refusal = bodyShiftRefusal(false)) {
            m_messages.show(*refusal, kMessageMs);
            return;
        }
        m_services.connection.send(protocol::bodyShiftAskRequest());
    }

    void WorldScene::throwAtMouse() {
        if (m_fightState != 0 || m_player.usedUtensil != 0) {
            return;
        }
        if (m_throwSlot != 0 && ThrowItems::kindOf(m_inventory.bagItem(m_throwSlot).itemId) != m_throwKind) {
            m_throwSlot = 0;
        }
        if (m_throwKind != 0 && m_throwSlot == 0) {
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                if (ThrowItems::kindOf(m_inventory.bagItem(slot).itemId) == m_throwKind) {
                    m_throwSlot = slot;
                    break;
                }
            }
        }
        if (m_throwSlot != 0) {
            const core::Point at = m_camera.toMap(m_mouse);
            m_services.connection.send(protocol::throwItemRequest(m_throwSlot, at.x, at.y));
            m_throwSlot = 0;
        }
    }

    void WorldScene::endThrowAim() {
        if (m_mouseMode == kThrowMouseMode) {
            setMouseMode(0);
        }
        m_throwKind = 0;
    }

    void WorldScene::useWinPlate(int bagSlot) {
        const int mapId = m_map ? m_map->id() : 0;
        std::vector<WinPlateRule::Fighter> fighters;
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto fighter = m_battle.field().fighter(slot);
            if (!fighter) {
                continue;
            }
            const Fighter& hum = fighter->get();
            const auto other = characterOf(hum.id);
            fighters.push_back({hum.id, hum.kind, hum.hp, hum.id != 0 && other ? other->get().fightState : -1});
        }
        const auto refused = WinPlateRule::refusal(
            m_fightState,
            m_battle.type(),
            mapId,
            isNoTeleportMap(mapId) || isQuizZoneMap(mapId),
            m_teams.isLeader(m_player.id),
            m_player.id,
            m_teams.leaderOf(m_player.id).value_or(0),
            fighters
        );
        if (refused) {
            m_messages.show(refused->message, refused->ms);
            return;
        }
        m_talk.ask(
            "Do you want to use #RWin Plate/#R to win the game?",
            std::nullopt,
            [this, bagSlot] {
                m_battle.stopRunningActions(m_nowMs);
                m_services.connection.send(protocol::winPlateRequest(bagSlot));
            },
            [] {},
            requestBox()
        );
    }

    void WorldScene::useYesNoBoard(int bagSlot) {
        m_talk.choose(
            "Please choose",
            std::nullopt,
            {"O", "X", "Random"},
            [this, bagSlot](int row) { m_services.connection.send(protocol::yesNoBoardRequest(bagSlot, row + 1)); },
            [] {},
            requestBox()
        );
    }

    void WorldScene::useEscapeButton(int bagSlot, int itemId) {
        constexpr int kWatching = 4; // fight state 4: watching a battle
        if (m_fightState == 0 || m_fightState == kWatching) {
            m_messages.show("The item only can be used in battle.", 1000); // message display time in ms
            return;
        }
        const auto record = m_services.items.find(itemId);
        m_talk.ask(
            "Want to use? #R" + (record ? record->name : std::string()) + "/#R Quit the battle?",
            std::nullopt,
            [this, bagSlot] { m_services.connection.send(protocol::quitBattleItemRequest(bagSlot)); },
            [] {}
        );
    }

    void WorldScene::useDelayClock(int bagSlot) {
        constexpr int kWatching = 4; // fight state 4: watching a battle
        if (m_fightState == 0) {
            m_messages.show("Can be used during the battle", kMessageMs);
        } else if (isNoTeleportMap(m_map ? m_map->id() : 0)) {
            m_messages.show("Can't be used in the scene", kMessageMs);
        } else if (m_fightState == kWatching) {
            m_messages.show("The item can't be used", kMessageMs);
        } else if (!m_battle.useDelayClock(bagSlot)) {
            m_messages.show("The item can't be used", kMessageMs);
        }
    }

    void WorldScene::useTimeMantle(int bagSlot) {
        if (m_fightState != 0) {
            m_messages.show("Unable to use in combat", 1200); // message display time in ms
        } else if (!isHouseMap(m_map ? m_map->id() : 0)) {
            m_messages.show("Can only be used in home", kMessageMs);
        } else if (m_makeJobs.playerSlot() == 0) {
            m_messages.show("Can't be used", kMessageMs);
        } else {
            m_services.connection.send(protocol::timeMantleRequest(bagSlot));
        }
    }

    void WorldScene::startAutoCollect(const MapNpc& npc) {
        // NPC ids of the gather NPCs the Vacuum Cleaner collects from: Wood, Crystal Ore, Crude Oil Ore, Sulfur Ore, Pure Ore, Water Source, Leaf, Alloy Ore
        constexpr int kGatherNpcs[] = {19052, 19058, 19059, 19060, 19061, 19069, 19070, 19071};
        if (std::find(std::begin(kGatherNpcs), std::end(kGatherNpcs), npc.info.npcId) == std::end(kGatherNpcs) || m_autoCollecting || m_remote.isOn() ||
            m_player.fishing || m_player.stall || m_itemStall.isOpen() || m_petStall.isOpen() || m_player.riding) {
            return;
        }
        const auto cleaner = m_services.items.find(m_inventory.equipmentIds()[5]); // equipment slot 5 holds the cleaner
        if (!cleaner || cleaner->subType != kVacuumCleanerSubType) {
            return;
        }
        m_autoCollecting = true;
        m_autoCollectNpc = npc.index;
        m_autoCollectMs = m_nowMs;
        startAction(m_player, kAutoCollectAction);
        m_services.connection.send(protocol::actionRequest(m_player.actionSlot()));
        m_services.connection.send(protocol::autoCollectRequest(npc.index));
    }

    void WorldScene::updateAutoCollect(std::uint64_t nowMs) {
        if (!m_autoCollecting || m_autoCollectNpc == 0) {
            return;
        }
        // equipment slot 5 holds the cleaner
        const std::uint64_t interval = m_inventory.equipmentIds()[5] == kStrongVacuumCleaner ? kAutoCollectMs : kSlowAutoCollectMs;
        if (nowMs - m_autoCollectMs >= interval) {
            m_autoCollectMs = nowMs;
            clickNpc(m_autoCollectNpc);
        }
    }

    void WorldScene::stopAutoCollect() {
        if (m_autoCollecting) {
            m_autoCollecting = false;
            m_autoCollectNpc = 0;
            m_services.connection.send(protocol::autoCollectRequest(0));
        }
    }

}
