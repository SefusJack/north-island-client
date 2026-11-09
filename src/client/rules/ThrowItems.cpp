#include "client/rules/ThrowItems.h"

namespace client {

    int ThrowItems::kindOf(int itemId) {
        switch (itemId) {
            case 34251: // throw item, kind 1
            case 34261: // throw item, kind 1
                return 1;
            case 34273: // throw item, kind 2
            case 34324: // throw item, kind 2
            case 34393: // throw item, kind 2
                return 2;
            case 34291: // throw item, kind 3
                return 3;
            default: // not a thrown item
                return 0;
        }
    }

}
