#include "client/rules/NpcVisibility.h"

namespace client {

    namespace {

        constexpr int kFleeKillMap = 10072; // map 10072: Survival Isle
        constexpr int kFleeKillNpc = 14229; // NPC 14229: Award Carrier

    }

    bool shouldHideMapNpc(int mapId, int npcId, bool fleeKillActive) {
        switch (npcId) {
            case kFleeKillNpc: // flee-kill game NPC: hidden on its map while the game runs
                return mapId == kFleeKillMap && fleeKillActive;
            case 12105: // NPC 12105 (Supermarket Clerk): hidden on map 11094
                return mapId == 11094; // map 11094: Carnie
            case 14157: // NPC 14157 (Exchanger): hidden on map 12000
            case 14286: // NPC 14286 (Rewards Keeper): hidden on map 12000
                return mapId == 12000; // map 12000: Kelan Village
            case 14273: // NPC 14273 (Update Servant): hidden on map 12062
                return mapId == 12062; // map 12062: 1F, the Capitol
            case 14417: // NPC 14417 (Villager): hidden on map 12100
            case 15551: // NPC 15551 (Fairy): hidden on map 12100
            case 20153: // NPC 20153 (Small Year Beast): hidden on map 12100
            case 20154: // NPC 20154 (Small Year Beast): hidden on map 12100
            case 20155: // NPC 20155 (Small Year Beast): hidden on map 12100
            case 20156: // NPC 20156 (Small Year Beast): hidden on map 12100
            case 20157: // NPC 20157 (Huge Year Beast): hidden on map 12100
                return mapId == 12100; // map 12100: Fishing Village
            case 15557: // NPC 15557 (Miss White): hidden on maps 12110, 13015, 13017
            case 15558: // NPC 15558 (Mr Xu): hidden on maps 12110, 13015, 13017
            case 15559: // NPC 15559 (Bloody Monk): hidden on maps 12110, 13015, 13017
            case 31053: // NPC 31053 (Snake Demon): hidden on maps 12110, 13015, 13017
                return mapId == 12110 || mapId == 13015 || mapId == 13017; // maps 12110: Chang'an, 13015: Deep Forest, 13017: House
            case 14464: // NPC 14464 (Aunt): hidden on map 60010
            case 15556: // NPC 15556 (Qu Yuan): hidden on map 60010
                return mapId == 60010; // map 60010: China
            default: // any other NPC: shown
                return false;
        }
    }

    bool shouldShowMapNpc(int mapId, int npcId, bool fleeKillActive) { return mapId == kFleeKillMap && npcId == kFleeKillNpc && !fleeKillActive; }

}
