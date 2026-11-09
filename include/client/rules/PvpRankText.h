#ifndef NORTH_ISLAND_CLIENT_RULES_PVP_RANK_TEXT_H
#define NORTH_ISLAND_CLIENT_RULES_PVP_RANK_TEXT_H

#include <string>
#include <vector>

namespace client {

    /// @brief Whether a map is a PvP map.
    /// @details Original: IsPvpMap (RE_PubFunc.pas: 10096-10097 and 10099-10103; not the set the rank lines use).
    /// @param mapId The map id.
    /// @return True for a PvP map.
    bool isPvpMap(int mapId);

    /// @brief The lines drawn over a player's name: arena victories, then PvP victories and rank.
    /// @details Original: DrawHumInfo (the two lines at NameY - 42, the later one over the earlier). On the PvP maps "The  N " and "N Victory", elsewhere only "The N" for the first three ranks.
    /// @param mapId The player's map (Player.FMapId).
    /// @param trojanWarTarget IsTrojanWarTarget.
    /// @param arenaVictory FArenaVictory (one more than the victories it shows).
    /// @param pvpWins The PvP victories.
    /// @param pvpRank The PvP rank.
    /// @return The lines, possibly none.
    std::vector<std::string> pvpRankLines(
        int mapId,
        bool trojanWarTarget,
        int arenaVictory,
        int pvpWins,
        int pvpRank
    );

    /// @brief The survival game's line over the own player's name.
    /// @details Original: FSurvivalVictory, packet 77/3; drawn 20 higher than the other lines.
    /// @param mapId The player's map.
    /// @param own Whether this is the own player.
    /// @param victories The survival victories.
    /// @return "N Victory" on the survival map, empty otherwise.
    std::string survivalLine(int mapId, bool own, int victories);

}

#endif
