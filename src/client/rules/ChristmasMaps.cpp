#include "client/rules/ChristmasMaps.h"

namespace client {

    bool isChristmasMap(int mapId) {
        // Christmas map ids: 11003..11005, 12000, 12050 and 60000..60012
        return (mapId >= 11003 && mapId <= 11005) || mapId == 12000 || mapId == 12050 || (mapId >= 60000 && mapId <= 60012);
    }

}
