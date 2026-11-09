#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_PET_GEAR_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_PET_GEAR_H

#include "client/services/GameServices.h"
#include "ui/IconGrid.h"

#include <vector>

namespace client {

    /// @brief Shows a pet's gear in the cells of a trade or stall form: slots 1..5 in a row, the sixth apart; each item in the cell of the slot Item.dat gives it.
    /// @param services Game services (items).
    /// @param five Grid of slots 1 to 5.
    /// @param sixth Grid of slot 6.
    /// @param equipment Item ids of the gear.
    void showPetGear(GameServices& services, ui::IconGrid& five, ui::IconGrid& sixth, const std::vector<int>& equipment);

}

#endif
