#include "client/rules/PvpRankText.h"

namespace client {

    namespace {

        constexpr int kArenaMap = 0x2F20; // map 12064: 2F, the Capitol (arena)
        constexpr int kPvpSquareMap = 0x2745; // map 10053: TGS (PvP square)
        constexpr int kSurvivalMap = 0x2758; // map 10072: Survival Isle

        /// @brief Whether a map is a PvP field map (the extra PvP maps of DZOutputObj are none).
        /// @details Original: IsPvpFieldMap.
        /// @param mapId The map id.
        /// @return True for a PvP field map.
        bool pvpFieldMap(int mapId) {
            return mapId == 10097 || mapId == 10099 || mapId == 10100;
        } // PvP field map ids: 10097 PvP Field, 10099 PvP Field, 10100 PvP Field

        /// @brief Whether a map shows the PvP rank lines (arena, PvP fields, PvP square).
        /// @param mapId The map id.
        /// @return True when it does.
        bool pvpMap(int mapId) { return mapId == kArenaMap || pvpFieldMap(mapId) || mapId == kPvpSquareMap; }

    }

    bool isPvpMap(int mapId) {
        return (mapId >= 10096 && mapId <= 10097) || (mapId >= 10099 && mapId <= 10103);
    } // PvP map id ranges 10096-10097 (PvP Hall 1, PvP Field) and 10099-10103 (PvP Field x2, PvP Hall 2-4)

    std::vector<std::string> pvpRankLines(
        int mapId,
        bool trojanWarTarget,
        int arenaVictory,
        int pvpWins,
        int pvpRank
    ) {
        std::vector<std::string> lines;
        if (pvpMap(mapId) && arenaVictory - 1 > 0) {
            lines.push_back(std::to_string(arenaVictory - 1) + "  Victory");
        }
        if (!trojanWarTarget && mapId != kSurvivalMap && pvpWins > 0) {
            const bool ranked = pvpRank >= 1 && pvpRank <= 3; // only ranks 1..3 are shown
            if (pvpMap(mapId)) {
                lines.push_back((ranked ? "The  " + std::to_string(pvpRank) + " " : std::string()) + std::to_string(pvpWins) + " Victory");
            } else if (ranked) {
                lines.push_back("The " + std::to_string(pvpRank));
            }
        }
        return lines;
    }

    std::string survivalLine(int mapId, bool own, int victories) {
        return own && mapId == kSurvivalMap && victories > 0 ? std::to_string(victories) + " Victory" : std::string();
    }

}
