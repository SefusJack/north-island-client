#ifndef NORTH_ISLAND_CLIENT_RULES_CHARACTER_ACTIONS_H
#define NORTH_ISLAND_CLIENT_RULES_CHARACTER_ACTIONS_H

#include "client/models/world/MapCharacter.h"

#include <cstdint>

namespace client {

    /// @brief Finds the action group a sprite slot belongs to.
    /// @details Original: GetDirGroupBase.
    /// @param slot Sprite slot.
    /// @return The first slot of the group.
    int actionGroup(int slot);

    /// @brief Finds the action group of the emote tab's action 1..24.
    /// @details Original: ActionMotionIds.
    /// @param number Emote action number.
    /// @return The group, or 0 for none.
    int emoteActionGroup(int number);

    /// @brief Starts an action group: an eight-way group keeps the direction, a two-way one faces up-left (0) or down-right (1) by it.
    /// @details Original: SetHumanAction. From a two-way group into an eight-way one the character faces 1 or 5.
    /// @param character The character.
    /// @param group First slot of the action group.
    void startAction(MapCharacter& character, int group);

    /// @brief Starts the action of a sprite slot as other players' 32/2 tells it.
    /// @param character The character.
    /// @param slot Sprite slot (group first slot plus facing).
    void startActionSlot(MapCharacter& character, int slot);

    /// @brief Turns the character to a direction in the pose it holds (a two-way action faces up-left or down-right by it) and restarts the animation.
    /// @details Original: FaceScreenPoint, PlayHumanAction.
    /// @param character The character.
    /// @param direction Direction 0..7.
    void faceAction(MapCharacter& character, int direction);

    /// @brief Stops the action and goes back to standing.
    /// @details Original: StopHumanAction.
    /// @param character The character.
    /// @return Whether an action was playing.
    bool stopAction(MapCharacter& character);

    /// @brief Time per frame of an action (38, and 34 with more than two frames, are quicker).
    /// @details Original: NextAnimFrame.
    /// @param group Action group.
    /// @param frameCount Frames in the action.
    /// @return Milliseconds per frame.
    std::uint64_t actionFrameMs(int group, int frameCount);

    /// @brief Tells whether an action stops on its last frame (26, out of battle).
    /// @details Original: NextAnimFrame.
    /// @param group Action group.
    /// @return True when it holds its last frame.
    bool actionHoldsLastFrame(int group);

}

#endif
