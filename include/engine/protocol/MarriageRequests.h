#ifndef NORTH_ISLAND_PROTOCOL_MARRIAGE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_MARRIAGE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds 45/4: the spouse card is asked for (the social window's Marry tab).
    /// @details Original: client action 45, marriage (SendServerMsg 45/x in DZ_Connect.pas).
    /// @return The encoded payload.
    core::Bytes spouseCardRequest();

    /// @brief Builds 45/6: a mail to the spouse from the Marry card (the card's Mail button).
    /// @param ownId Own player id (i32).
    /// @param spouseName The spouse's name (pstr8).
    /// @param text Mail text.
    /// @return The encoded payload.
    core::Bytes spouseMailRequest(std::int32_t ownId, const std::string& spouseName, const std::string& text);

    /// @brief Builds 45/8: the pet hidden for the wedding is shown again (a walk starts; ClearFlag211CAndSend).
    /// @param ownId Own player id (i32).
    /// @return The encoded payload.
    core::Bytes petShownRequest(std::int32_t ownId);

    /// @brief Builds 45/11: the answer to a marriage application between two players.
    /// @param ownId Own player id.
    /// @param partnerId Partner player id.
    /// @param accept Whether it was accepted.
    /// @return The encoded payload.
    core::Bytes marryAnswerRequest(std::int32_t ownId, std::int32_t partnerId, bool accept);

    /// @brief Builds 45/12: the answer to a divorce application.
    /// @param ownId Own player id.
    /// @param partnerId Partner player id.
    /// @param accept Whether it was accepted.
    /// @return The encoded payload.
    core::Bytes divorceAnswerRequest(std::int32_t ownId, std::int32_t partnerId, bool accept);

    /// @brief Builds 45/9: the answer to a spouse's divorce request.
    /// @param agree Whether the divorce was agreed.
    /// @return The encoded payload.
    core::Bytes divorceConsentRequest(bool agree);

}

#endif
