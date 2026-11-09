#ifndef NORTH_ISLAND_FORMATS_SCENE_INFO_H
#define NORTH_ISLAND_FORMATS_SCENE_INFO_H

#include <array>
#include <string>

namespace formats {

    /// @brief What SceneData.Dat says about a map.
    /// @details Its name, scene type (1 fields, 2 towns, 3 houses, 4 caves, 5 ships, ...) and its music (Sound/<music>.wav); for a battle ground
    /// its layers (59000 + n: the near layer, the far layer and the backdrop JPEG pictures, and where the two layers lie; LoadBattleGround).
    /// Flags: 2 no stalls, 4 no going back to a revival spot. Effect (SceneEffect): 1 leaves, 2 snow, 3 steam, 4 rain, 5 falling leaves, 6 bubbles, 7 ribbons.
    /// TitleLight: the picture T<number> that lights up the screen as the map is entered (0: none). The dup fields describe instances
    /// ("duplicate missions"): how many players, 1 for guilds only, the Mark.dat mark plus one that names it (0: the scene's name), the level needed and the minutes it lasts.
    struct SceneInfo {
        std::string name;
        int type = 0;
        std::string music;
        int flags = 0;
        int effect = 0;
        int titleLight = 0;
        std::array<int, 3> battlePictures{};
        std::array<int, 2> battleLayerY{};
        int dupMaxPlayers = 0;
        int dupGuildOnly = 0;
        int dupMarkNo = 0;
        int dupMinLevel = 0;
        int dupTimeLimit = 0;
    };

}

#endif
