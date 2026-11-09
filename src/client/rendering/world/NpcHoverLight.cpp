#include "client/rendering/world/NpcHoverLight.h"

namespace client {

    bool NpcHoverLight::applies(int npcId) {
        switch (npcId) {
            case 18061: // furniture NPC, no hover light
            case 18063: // furniture NPC, no hover light
            case 18068: // furniture NPC, no hover light
            case 18069: // furniture NPC, no hover light
            case 18070: // furniture NPC, no hover light
            case 18071: // furniture NPC, no hover light
            case 18072: // furniture NPC, no hover light
                return false;
            default: // any other NPC gets the hover light
                return true;
        }
    }

}
