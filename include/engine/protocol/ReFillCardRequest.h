#ifndef NORTH_ISLAND_PROTOCOL_RE_FILL_CARD_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_RE_FILL_CARD_REQUEST_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Client 35/3: a prepaid card to recharge the account's points with.
    /// @details Layout: 23 03, pstr8 card number, pstr8 password, u8 0.
    struct ReFillCardRequest {
        std::string cardNumber;
        std::string password;

        /// @brief Encodes the request.
        /// @return The encoded payload.
        core::Bytes encode() const;
    };

}

#endif
