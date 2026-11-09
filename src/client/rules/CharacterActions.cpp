#include "client/rules/CharacterActions.h"

#include <array>

namespace client {

    namespace {

        constexpr int kStanding = 8; // first slot of the standing group (8 slots, one per facing direction)
        // action group (first sprite slot) per emote tab action number 1..24 (0 = none, as at index 0, 1 and 16..24); 8 slots for an eight-way group, else 2
        constexpr std::array<int, 25> kEmoteActionGroups = {0, 0, 8, 16, 26, 28, 30, 32, 34, 36, 38, 40, 42, 46, 54, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        /// @brief Tells whether an action group has eight directions (the others have two).
        /// @param group First slot of the group.
        /// @return True for an eight-way group.
        // eight-way groups: 0 and 8 (walk, stand), 18 (riding), 46 (sitting), 54, 66 (swimming)
        bool isEightWay(int group) { return group == 0 || group == 8 || group == 18 || group == 46 || group == 54 || group == 66; }

        /// @brief The group playing now (standing when no action).
        /// @param character The character.
        /// @return The group's first slot.
        int currentGroup(const MapCharacter& character) { return character.action != 0 ? character.action : kStanding; }

        /// @brief The way the character faces in the group playing now.
        /// @param character The character.
        /// @return The offset within the group.
        int currentOffset(const MapCharacter& character) { return character.action != 0 ? character.actionOffset : character.direction; }

        /// @brief The facing offset the character gets in a new group, converting between eight-way and two-way groups.
        /// @param character The character.
        /// @param group The group to enter.
        /// @return The offset within the new group.
        int offsetFor(const MapCharacter& character, int group) {
            const bool fromEightWay = isEightWay(currentGroup(character));
            const bool toEightWay = isEightWay(group);
            const int offset = currentOffset(character);
            if (group == 99) { // group 99: no action
                return 0;
            }
            if (fromEightWay && !toEightWay) {
                return offset <= 3 ? 0 : 1; // directions 0..3 face up-left (0), the rest down-right (1)
            }
            if (!fromEightWay && toEightWay) {
                return offset == 0 ? 1 : 5; // up-left becomes direction 1, down-right direction 5
            }
            return offset;
        }

    }

    int actionGroup(int slot) {
        if (slot == 99) { // slot 99: no action
            return 99;
        }
        // first slots of the action groups
        for (const int first : {0, 8, 16, 18, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 54, 62, 64, 66}) {
            const int count = isEightWay(first) ? 8 : 2;
            if (slot >= first && slot < first + count) {
                return first;
            }
        }
        return 0;
    }

    int emoteActionGroup(int number) {
        return number >= 0 && number < static_cast<int>(kEmoteActionGroups.size()) ? kEmoteActionGroups[static_cast<std::size_t>(number)] : 0;
    }

    void startAction(MapCharacter& character, int group) {
        const int offset = offsetFor(character, group);
        if (group == 26) { // group 26 stops on its last frame, so it restarts
            character.animation.restart();
        }
        character.walking = false;
        character.path.clear();
        character.action = group;
        character.actionOffset = offset;
        character.holdFirstFrame = false;
    }

    void startActionSlot(MapCharacter& character, int slot) {
        character.walking = false;
        character.path.clear();
        character.action = actionGroup(slot);
        character.actionOffset = slot - character.action;
        character.holdFirstFrame = false;
    }

    void faceAction(MapCharacter& character, int direction) {
        if (character.action == 0) {
            character.direction = direction;
        } else {
            character.actionOffset =
                isEightWay(character.action) ? direction : (direction <= 3 ? 0 : 1); // directions 0..3 face up-left (0), the rest down-right (1)
        }
        character.animation.restart();
    }

    bool stopAction(MapCharacter& character) {
        if (character.action == 0) {
            return false;
        }
        character.direction = offsetFor(character, kStanding);
        character.action = 0;
        character.actionOffset = 0;
        character.holdFirstFrame = false;
        return true;
    }

    std::uint64_t actionFrameMs(int group, int frameCount) {
        return group == 38 || (group == 34 && frameCount > 2) ? 110 : 230;
    } // groups 38 and 34 (3+ frames): 110 ms per frame, others 230 ms

    bool actionHoldsLastFrame(int group) { return group == 26; } // group 26

}
