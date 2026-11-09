#include "client/models/pets/Pet.h"

namespace client {

    std::vector<int> Pet::equipmentIds() const {
        std::vector<int> ids;
        for (const protocol::ItemInstance& item : worn) {
            if (!item.empty()) {
                ids.push_back(item.itemId);
            }
        }
        return ids;
    }

}
