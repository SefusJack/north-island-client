#ifndef NORTH_ISLAND_PROTOCOL_SCENE_PET_H
#define NORTH_ISLAND_PROTOCOL_SCENE_PET_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Another player's fighting pet shown in the scene.
    /// @details Original: server 15/4, RecvScenePlayerPets; wornItem is put on by HandlePetEquipChangePacket; mounted means ridden the old way (MountPet).
    struct ScenePet {
        std::int32_t playerId = 0;
        int npcId = 0;
        bool mounted = false;
        int tradable = 0;
        std::string name;
        int wornItem = 0;
    };

}

#endif
