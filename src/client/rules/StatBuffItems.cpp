#include "client/rules/StatBuffItems.h"

namespace client {

    int StatBuffItems::code(int itemId) {
        switch (itemId) {
            case 34236: return 301; // ATK buff item, level 1
            case 34329: return 302; // ATK buff item, level 2
            case 34264: return 401; // DEF buff item, level 1
            case 34327: return 402; // DEF buff item, level 2
            case 34249: return 501; // MATK buff item, level 1
            case 34326: return 502; // MATK buff item, level 2
            case 34265: return 601; // MDEF buff item, level 1
            case 34328: return 602; // MDEF buff item, level 2
            default: return 0; // not a stat buff item
        }
    }

    std::string StatBuffItems::refusal(int itemId, bool fighting, std::uint32_t buffMask) {
        const int buff = code(itemId);
        if (buff == 0) {
            return {};
        }
        if (fighting) {
            return "Unable to use in combat";
        }
        const int place = buff / 100; // hundreds = stat place in the buff mask
        // 4 bits per stat; 15 = mask of one 4-bit field
        if (((buffMask >> (4 * (place - 1))) & 15) != 0) {
            return "Unable to use";
        }
        return {};
    }

}
