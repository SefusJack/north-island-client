#ifndef NORTH_ISLAND_FORMATS_SCENE_SURFACE_H
#define NORTH_ISLAND_FORMATS_SCENE_SURFACE_H

#include <vector>

namespace formats {

    /// @brief What an event shows (eve.Emg's "interacts", TSurface).
    /// @details Its kind (1 a talk with choices, 2 and 3 a store), a value, and its entries in order: each a talk line (source 1, a Talk.Dat id)
    /// or an item (source 2, an Item.dat id), shown as the prompt (kind 1), counted as a Confirm / Cancel answer (2), offered as a choice (3) or not shown (4).
    struct SceneSurface {
        /// @brief One talk line or item of the event.
        struct Entry {
            int number = 0;
            int id = 0;
            int source = 0;
            int kind = 0;
        };

        int index = 0;
        int kind = 0;
        int value = 0;
        std::vector<Entry> entries;
    };

}

#endif
