#include "protocol/StoreTankRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kStoreTank = 134; // 23/134: Store Tank (fuel)

        /// @brief Builds a 23/134 request with two byte arguments and an optional third.
        /// @param option Option byte (1 put in, 2 take out, 3 use).
        /// @param first First byte argument.
        /// @param second Second byte argument.
        /// @param third Third byte argument, left out when negative.
        /// @return The encoded request payload.
        core::Bytes make(std::uint8_t option, int first, int second, int third) {
            core::ByteWriter writer;
            writer.u8(kItems).u8(kStoreTank).u8(option).u8(static_cast<std::uint8_t>(first)).u8(static_cast<std::uint8_t>(second));
            if (third >= 0) {
                writer.u8(static_cast<std::uint8_t>(third));
            }
            return writer.take();
        }

    }

    core::Bytes putInStoreTankRequest(int tankSlot, int fuelSlot, int row) { return make(1, tankSlot, fuelSlot, row); } // option 1 = put fuel in

    core::Bytes takeFromStoreTankRequest(int tankSlot, int row, int count) { return make(2, tankSlot, row, count); } // option 2 = take fuel out

    core::Bytes useStoreTankRequest(int tankSlot, int row) { return make(3, tankSlot, row, -1); } // option 3 = pour fuel into the ridden vehicle

}
