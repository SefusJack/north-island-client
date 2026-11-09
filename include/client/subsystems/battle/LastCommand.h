#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_LAST_COMMAND_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_LAST_COMMAND_H

namespace client {

    /// @brief What a fighter was last commanded to do, or had done to it.
    /// @details Original: TFightHumLastCmd. The kind is as the ring's buttons number it (1 defend, 2 capture, 3 flee,
    /// 4 item, 6 attack, 7 skill); then the target's slot and the skill or item.
    struct LastCommand {
        int kind = 0;
        int target = 0;
        int value = 0;
    };

}

#endif
