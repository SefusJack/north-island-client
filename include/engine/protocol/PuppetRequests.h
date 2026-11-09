#ifndef NORTH_ISLAND_PROTOCOL_PUPPET_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_PUPPET_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds 23/104 option 1: invite a helper (SendPuppetTrainRequest).
    /// @param helperId Helper player id (i32).
    /// @param bagSlot Bag place of the card (u8).
    /// @return The encoded payload.
    core::Bytes puppetInviteRequest(std::int32_t helperId, int bagSlot);

    /// @brief Builds 23/104 option 2: the helper agrees.
    /// @param requesterId Requester player id (i32).
    /// @return The encoded payload.
    core::Bytes puppetAcceptRequest(std::int32_t requesterId);

    /// @brief Builds 23/104 option 3: the helper refuses.
    /// @param reason 6 refuses, 7 no answer, 17 busy (u8).
    /// @param requesterId Requester player id (i32).
    /// @return The encoded payload.
    core::Bytes puppetRefuseRequest(int reason, std::int32_t requesterId);

    /// @brief Builds 23/104 option 4: the requester refuses the helper's wish to stop.
    /// @param reason 10 refuses, 12 no answer (u8).
    /// @return The encoded payload.
    core::Bytes puppetKeepRequest(int reason);

    /// @brief Builds 23/104 option 5: the requester asks to stop.
    /// @return The encoded payload.
    core::Bytes puppetAskStopRequest();

    /// @brief Builds 23/104 option 6: the helper agrees to stop.
    /// @param requesterId Requester player id (i32).
    /// @return The encoded payload.
    core::Bytes puppetHelperStopRequest(std::int32_t requesterId);

    /// @brief Builds 23/104 option 7: the requester spends the card's exp.
    /// @return The encoded payload.
    core::Bytes puppetUseExpRequest();

    /// @brief Builds 23/104 option 8: the helper asks to stop.
    /// @param requesterId Requester player id (i32).
    /// @return The encoded payload.
    core::Bytes puppetHelperAskStopRequest(std::int32_t requesterId);

    /// @brief Builds 23/104 option 9: the requester agrees to stop.
    /// @return The encoded payload.
    core::Bytes puppetAgreeStopRequest();

    /// @brief Builds 23/104 option 10: the helper refuses to stop.
    /// @param reason 11 refuses, 13 no answer (u8).
    /// @param requesterId Requester player id (i32).
    /// @return The encoded payload.
    core::Bytes puppetHelperKeepRequest(int reason, std::int32_t requesterId);

    /// @brief Builds 23/104 option 12: open the requester's form (the answer is 14).
    /// @return The encoded payload.
    core::Bytes puppetOpenRequest();

    /// @brief Builds 23/104 option 13: a mail to the partner.
    /// @param ownId Own player id (i32).
    /// @param partnerName The partner's name (pstr8).
    /// @param text The words.
    /// @return The encoded payload.
    core::Bytes puppetMailRequest(std::int32_t ownId, const std::string& partnerName, const std::string& text);

    /// @brief Builds 23/104 option 14: ask the time left of a card.
    /// @param slot The card (u8; the requester's own is 1).
    /// @return The encoded payload.
    core::Bytes puppetTimeLeftRequest(int slot);

    /// @brief Builds 23/104 option 15: grow the card by one step.
    /// @return The encoded payload.
    core::Bytes puppetGrowRequest();

}

#endif
