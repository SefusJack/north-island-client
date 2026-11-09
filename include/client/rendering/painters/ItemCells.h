#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_ITEM_CELLS_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_ITEM_CELLS_H

#include "client/services/GameServices.h"
#include "protocol/ItemInstance.h"
#include "ui/IconCell.h"

namespace client {

    /// @brief Builds the cell of an item: its icon (or its look's), a larger item the middle of its picture on its frame, the count in the bag, the name as hint.
    /// @details Original: TCY_EquipImage.DrawControl. A cell in use by another window sits on the busy frame (WxHGrid_2). inVehicleBox is the large place of a vehicle form (the garage's repair space, the Space Capsule's, the parking lot's): a capsule's rails are the vehicle's there (FCtrlName FixTrafficSpace, Capsule, CarCase, ParkCase).
    /// @param services Game services (items, pictures).
    /// @param item The item.
    /// @param inBag True when shown in the bag (shows the count).
    /// @param inUse True when the cell is in use by another window.
    /// @param inVehicleBox True in the large place of a vehicle form.
    /// @return The icon cell.
    ui::IconCell itemIconCell(
        GameServices& services,
        const protocol::ItemInstance& item,
        bool inBag,
        bool inUse = false,
        bool inVehicleBox = false
    );

    /// @brief Builds the cell of a storage form: the icon at its top left, the count, the item's name as the hint.
    /// @details Original: TCY_ItemImage.
    /// @param services Game services (items, pictures).
    /// @param item The item.
    /// @return The icon cell.
    ui::IconCell itemNameCell(GameServices& services, const protocol::ItemInstance& item);

}

#endif
