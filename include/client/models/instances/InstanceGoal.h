#ifndef NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_GOAL_H
#define NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_GOAL_H

namespace client {

    /// @brief A thing to beat in an instance: NPC, needed and beaten counts.
    /// @details Original: TDupGoalRec.
    struct InstanceGoal {
        int npcId = 0;
        int need = 0;
        int have = 0;
    };

}

#endif
