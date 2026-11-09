#ifndef NORTH_ISLAND_PROTOCOL_SHOP_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_SHOP_REQUESTS_H

#include "core/Bytes.h"

#include <utility>
#include <vector>

namespace protocol {

    /// @brief Builds request 27/1: buy the cart.
    /// @details Original: TRe_NpcStoreForm.BuyCartItems. Layout: u8 line count, then u8 store row (from 1) and u8 count per line.
    /// @param rowsAndCounts Pairs of store row and count to buy.
    /// @return The encoded request payload.
    core::Bytes npcStoreBuyRequest(const std::vector<std::pair<int, int>>& rowsAndCounts);

    /// @brief Builds request 27/2: sell marked items to the NPC.
    /// @details Original: TRe_SaleToNpcForm.SellMarkedItems. Layout: u8 bag slot per item, then u8 sale kind.
    /// @param bagSlots Bag slots of the items sold.
    /// @param saleKind 0 for equipment, 1 for goods.
    /// @return The encoded request payload.
    core::Bytes npcSaleRequest(const std::vector<int>& bagSlots, int saleKind);

    /// @brief Builds request 31/1: take the inn's rest.
    /// @details Original: TINNAskForm.AcceptInnRest.
    /// @return The encoded request payload.
    core::Bytes innRestRequest();

    /// @brief Builds request 31/2: take pets out of the inn's pet storage.
    /// @details Original: TRe_InnForm. Layout: u8 place per pet taken out.
    /// @param places Storage places of the pets taken out.
    /// @return The encoded request payload.
    core::Bytes petStorageTakeRequest(const std::vector<int>& places);

    /// @brief Builds request 31/3: store pets in the inn's pet storage.
    /// @details Original: TRe_InnForm. Layout: u8 pet slot per pet stored.
    /// @param petSlots Pet slots of the pets stored.
    /// @return The encoded request payload.
    core::Bytes petStorageStoreRequest(const std::vector<int>& petSlots);

    /// @brief Builds request 31/4: store some pets and take others out in one step.
    /// @details Original: TRe_InnForm. Layout: u8 n, the pet slots stored, u8 m, the places taken out.
    /// @param petSlots Pet slots of the pets stored.
    /// @param places Storage places of the pets taken out.
    /// @return The encoded request payload.
    core::Bytes petStorageSwapRequest(const std::vector<int>& petSlots, const std::vector<int>& places);

}

#endif
