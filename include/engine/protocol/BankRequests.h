#ifndef NORTH_ISLAND_PROTOCOL_BANK_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_BANK_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief Builds client 30/1: take the marked items out (original WithdrawMarkedItems).
    /// @param bankSlots Bank places (u8 each) of the items.
    /// @return The encoded request.
    core::Bytes bankWithdrawRequest(const std::vector<int>& bankSlots);

    /// @brief Builds client 30/2: put the marked items in (original StoreMarkedItems).
    /// @param bagSlots Bag places (u8 each) of the items.
    /// @return The encoded request.
    core::Bytes bankStoreRequest(const std::vector<int>& bagSlots);

    /// @brief Builds client 30/3: a bank item moved to another place (original MoveBankItem).
    /// @param fromSlot Bank place it leaves.
    /// @param toSlot Bank place it goes to.
    /// @return The encoded request.
    core::Bytes bankMoveRequest(int fromSlot, int toSlot);

    /// @brief Builds client 30/4: a bag item dropped on the bank (original DropBagItemToBank).
    /// @param bankSlot Bank place it is dropped on.
    /// @param bagSlot Bag place of the item.
    /// @return The encoded request.
    core::Bytes bankDropRequest(int bankSlot, int bagSlot);

    /// @brief Builds client 30/5: a bank item dropped on the bag (original WithdrawBankItemTo).
    /// @param bagSlot Bag place it is dropped on.
    /// @param bankSlot Bank place of the item.
    /// @return The encoded request.
    core::Bytes bankTakeRequest(int bagSlot, int bankSlot);

    /// @brief Builds client 29/1: take gold out of the account (original TMoneyAccountForm).
    /// @param amount Gold (i32).
    /// @return The encoded request.
    core::Bytes bankGoldWithdrawRequest(std::int32_t amount);

    /// @brief Builds client 29/2: put gold in the account (original TMoneyAccountForm).
    /// @param amount Gold (i32).
    /// @return The encoded request.
    core::Bytes bankGoldDepositRequest(std::int32_t amount);

    /// @brief Builds client 29/3: carried gold into the ATM (original TATMDepositForm.ATMDeposit).
    /// @param amount Gold (i32).
    /// @return The encoded request.
    core::Bytes atmDepositRequest(std::int32_t amount);

    /// @brief Builds client 29/4: the ATM's gold into the carried gold (original ATMWithdraw).
    /// @param amount Gold (i32).
    /// @return The encoded request.
    core::Bytes atmWithdrawRequest(std::int32_t amount);

    /// @brief Builds client 29/5: ATM gold to another player's ATM (original TATMTransferForm.SendATMTransfer).
    /// @param playerId Receiving player id (i32).
    /// @param amount Gold (i32).
    /// @return The encoded request.
    core::Bytes atmTransferRequest(std::int32_t playerId, std::int32_t amount);

}

#endif
