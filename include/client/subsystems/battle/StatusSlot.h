#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_STATUS_SLOT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_STATUS_SLOT_H

namespace client {

    /// @brief One status a fighter is under: the skill's status effect, the skill, and whether its motion has begun looping.
    /// @details Original: TFightHum's FStatusNEffect / FStatusNSkill / FStatusNStarted.
    struct StatusSlot {
        int effect = 0;
        int skill = 0;
        bool started = false;
    };

}

#endif
