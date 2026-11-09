#ifndef NORTH_ISLAND_PROTOCOL_REPAIR_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_REPAIR_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/84: repair an item (TMZ_RepairForm.ConfirmRepair).
    /// @param itemId Item id (u16).
    /// @param itemSlot Bag place of the item (u8).
    /// @param toolSlot Bag place of the Repairing Spanner or Potion (u8).
    /// @return The encoded payload.
    core::Bytes repairItemRequest(int itemId, int itemSlot, int toolSlot);

}

#endif
