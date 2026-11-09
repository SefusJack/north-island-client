#include "client/rules/Radio.h"

namespace client {

    int radioKind(int itemId) {
        switch (itemId) {
            case 39070: return 1; // item 39070: the house radio, kind 1
            case 34076: return 2; // item 34076: radio kind 2
            case 34132: return 3; // item 34132: radio kind 3 (lines show as "(All)")
            default: return 0; // not a radio
        }
    }

}
