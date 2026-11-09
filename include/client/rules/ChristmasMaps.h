#ifndef NORTH_ISLAND_CLIENT_RULES_CHRISTMAS_MAPS_H
#define NORTH_ISLAND_CLIENT_RULES_CHRISTMAS_MAPS_H

namespace client {

    /// @brief Tells whether a map snows and plays the Christmas music while the Christmas event is on.
    /// @details Original: CH_TTrafficManage.IsXmasMap.
    /// @param mapId Map id.
    /// @return True for a Christmas map.
    bool isChristmasMap(int mapId);

}

#endif
