#ifndef NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_BATTLE_VIEW_H
#define NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_BATTLE_VIEW_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief One fighter as the Electron Control's script sees it.
    struct RemoteFighter {
        int slot = 0;
        int side = 0; // 1 rows 3..4, 2 rows 1..2
        int row = 0;
        int col = 0;
        int kind = 0; // fighter kind: 2 a player, 4 a pet, others monsters and NPCs
        std::int32_t id = 0;
        std::int32_t ownerId = 0;
        std::string name;
        std::int32_t hp = 0;
        std::int32_t maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        int level = 0;
        int element = 0;
        std::vector<int> statusSkills; // skills shown as status icons in the battle panel
        std::vector<int> statusKinds; // main status kinds 1..8 the fighter is under
    };

    /// @brief The battle as the Electron Control's script sees it when one of the player's fighters is to command.
    struct RemoteBattleView {
        std::uint32_t battle = 0; // counts the battles, so the script's memory starts over with each
        int round = 0; // command round of the battle, from 1
        int actorKind = 0; // 1 the player commands, 2 the pet
        int actorSlot = 0;
        int ownSlot = 0; // the player's slot, 0 when none
        int petSlot = 0; // the fighting pet's slot, 0 when none
        int allySide = 0;
        int enemySide = 0;
        std::vector<RemoteFighter> fighters;
        std::function<bool(int skillId, int targetSlot)> canUse; // the actor may use the skill on the slot now
        std::function<bool(int itemId)> hasItem; // the bag holds the item
    };

    /// @brief What the script chose for the commanding fighter.
    struct RemoteDecision {
        bool item = false; // true: id is a bag item, false: a skill
        int id = 0;
        int targetSlot = 0;
    };

}

#endif
