#ifndef NORTH_ISLAND_PROTOCOL_STOREROOM_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_STOREROOM_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 62/7: items from the bag into the Storeroom.
    /// @details Original: RequestBagToGarage. Layout: u8 bag place, u8 cell, u8 count, u16 piece.
    /// @param bagSlot Bag place of the items.
    /// @param cell Target Storeroom cell.
    /// @param count How many items.
    /// @param piece Furniture index of the Storeroom piece.
    /// @return The encoded request payload.
    core::Bytes storeroomPutRequest(int bagSlot, int cell, int count, int piece);

    /// @brief Builds request 62/8: items within the Storeroom.
    /// @details Original: RequestMoveInGarage. Layout: u8 cell, u8 cell, u8 count, u16 piece.
    /// @param from Source cell.
    /// @param to Target cell.
    /// @param count How many items.
    /// @param piece Furniture index of the Storeroom piece.
    /// @return The encoded request payload.
    core::Bytes storeroomMoveRequest(int from, int to, int count, int piece);

    /// @brief Builds request 62/9: items from the Storeroom into the bag.
    /// @details Original: RequestGarageToBag. Layout: u8 cell, u8 bag place, u8 count, u16 piece.
    /// @param cell Source Storeroom cell.
    /// @param bagSlot Target bag place.
    /// @param count How many items.
    /// @param piece Furniture index of the Storeroom piece.
    /// @return The encoded request payload.
    core::Bytes storeroomTakeRequest(int cell, int bagSlot, int count, int piece);

    /// @brief Builds request 62/12: items of the Storeroom let go on a Recycle Bin.
    /// @details Original: TUtensilManage.RecycleFromGarage. Layout: u8 cell, u8 count, u16 recycle bin piece, u16 piece.
    /// @param cell Storeroom cell.
    /// @param count How many items.
    /// @param bin Furniture index of the Recycle Bin piece.
    /// @param piece Furniture index of the Storeroom piece.
    /// @return The encoded request payload.
    core::Bytes storeroomRecycleRequest(int cell, int count, int bin, int piece);

    /// @brief Builds request 62/50: the Super Handbag was used; the answer shows the Storeroom wherever the player is.
    /// @return The encoded request payload.
    core::Bytes superHandbagRequest();

}

#endif
