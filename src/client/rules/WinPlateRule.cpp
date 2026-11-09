#include "client/rules/WinPlateRule.h"

namespace client {

    namespace {

        constexpr int kFighting = 2; // FFightState 2: in a battle
        constexpr int kWatching = 4; // FFightState 4: watching a battle
        constexpr int kPlayerKind = 2; // FHumKind 2: a player
        constexpr int kItemRefusedMs = 2000; // how long an item refusal message shows, ms
        constexpr int kCaptainMessageMs = 3000; // how long the captain-died message shows, ms

    }

    // castle map ids 10065 Trojan Round 1, 10066 Trojan Round 2, 10068 Trojan Round 3 and 11128 Abyss Cave, 11129 and 11130 Castle Subway
    bool WinPlateRule::isCastleMap(int mapId) { return mapId == 10065 || mapId == 10066 || mapId == 10068 || (mapId >= 11128 && mapId <= 11130); }

    std::optional<WinPlateRule::Refusal> WinPlateRule::refusal(
        int fightState,
        int battleType,
        int mapId,
        bool mapRefused,
        bool leading,
        std::int32_t ownId,
        std::int32_t leaderId,
        const std::vector<Fighter>& fighters
    ) {
        if (fightState == 0) {
            return Refusal{"Can be used during the battle", kItemRefusedMs};
        }
        // battle types 3, 4, 5, 8, 9, 11 refuse the item
        if (fightState == kFighting || fightState == kWatching || battleType == 3 || battleType == 4 || battleType == 5 || battleType == 8 || battleType == 9 ||
            battleType == 11) {
            return Refusal{"The item can't be used", kItemRefusedMs};
        }
        if (mapRefused || isCastleMap(mapId)) {
            return Refusal{"Can't be used in the scene", kItemRefusedMs};
        }
        for (const Fighter& fighter : fighters) {
            if (fighter.id != 0 && fighter.fightState == kFighting) {
                return Refusal{"The item can't be used", kItemRefusedMs};
            }
            const bool own = fighter.id == ownId && ownId != 0;
            const bool captainDead = leading ? own && fighter.hp == 0 : !own && fighter.kind == kPlayerKind && fighter.id == leaderId && fighter.hp == 0;
            if (captainDead) {
                return Refusal{"Can't be used, because captain died", kCaptainMessageMs};
            }
        }
        return std::nullopt;
    }

}
