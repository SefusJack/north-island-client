#ifndef NORTH_ISLAND_CLIENT_RULES_WIN_PLATE_RULE_H
#define NORTH_ISLAND_CLIENT_RULES_WIN_PLATE_RULE_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The refusal rules for using the Win Plate item.
    /// @details Original: UseGridItem for the Win Plate (item 34165, Se_EquipUI.pas).
    class WinPlateRule {
        public:
            /// @brief Why the item is refused: the message and how long it shows in ms.
            struct Refusal {
                std::string message;
                int ms = 0;
            };

            /// @brief A fighter of the field: id, FHumKind, hit points and (for a player on the map) FFightState, -1 when not one.
            struct Fighter {
                std::int32_t id = 0;
                int kind = 0;
                int hp = 0;
                int fightState = -1;
            };

            /// @brief Tells whether a map is a castle map (Original: CastleManage.IsCastleMap).
            /// @param mapId The map id.
            /// @return True for a castle map.
            static bool isCastleMap(int mapId);

            /// @brief Checks whether the Win Plate may be used now.
            /// @param fightState Player.FFightState.
            /// @param battleType FightField.FBattleType.
            /// @param mapId The current map id.
            /// @param mapRefused IsNoTeleportMapId or IsQuizZoneMap.
            /// @param leading Player.FTeamStatus = 1.
            /// @param ownId The player's id.
            /// @param leaderId Player.FTeamLeaderId.
            /// @param fighters The fighters of the field.
            /// @return The refusal, or nothing when the question is to be asked.
            static std::optional<Refusal> refusal(
                int fightState,
                int battleType,
                int mapId,
                bool mapRefused,
                bool leading,
                std::int32_t ownId,
                std::int32_t leaderId,
                const std::vector<Fighter>& fighters
            );
    };

}

#endif
