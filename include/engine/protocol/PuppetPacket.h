#ifndef NORTH_ISLAND_PROTOCOL_PUPPET_PACKET_H
#define NORTH_ISLAND_PROTOCOL_PUPPET_PACKET_H

#include "core/Bytes.h"
#include "protocol/PuppetPerson.h"

#include <cstdint>
#include <optional>

namespace protocol {

    /// @brief A server 23/104 packet of the Memory Card and puppet training; one struct for all options.
    /// @details Original: TInsteadEcisForm, TMemCardForm1; the Recv routines of CJ_Menu.pas. The fields each option fills are listed above.
    struct PuppetPacket {
        int option = 0;
        std::int32_t id = 0;
        std::int32_t other = 0;
        int slot = 0;
        int code = 0;
        std::int32_t exp = 0;
        int step = 0;
        bool online = false;
        bool extended = false;
        PuppetPerson person;

        /// @brief Decodes a 23/104 packet.
        /// @param payload The packet payload.
        /// @return The packet, or nothing when malformed.
        static std::optional<PuppetPacket> decode(const core::Bytes& payload);
    };

}

#endif
