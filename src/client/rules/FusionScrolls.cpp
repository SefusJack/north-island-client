#include "client/rules/FusionScrolls.h"

namespace client {

    bool isFusionScroll(int itemId) {
        switch (itemId) {
            case 34238: // Seraph scroll
            case 34239: // Demon scroll
            case 34243: // Cherubim scroll
            case 34244: // Ofanim scroll
            case 34245: // Virtues scroll
            case 34246: // Principality scroll
            case 34247: // Small Demon scroll
            case 34255: return true; // Archangel scroll
            default: return false; // not a fusion scroll
        }
    }

}
