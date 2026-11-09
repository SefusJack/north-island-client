#ifndef NORTH_ISLAND_PROTOCOL_LOTTERY_LISTS_H
#define NORTH_ISLAND_PROTOCOL_LOTTERY_LISTS_H

#include "core/Bytes.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief One prize of the online draw: item and count.
    struct LotteryPrize {
        int itemId = 0;
        int count = 0;
    };

    /// @brief The draw's info: draws used today and the prize list.
    /// @details Original: 104/1/1 (TLa_LotteryOLForm.RecvLotteryPacket), u8 draws used, u8 prize count (up to 20), then u16 item, u8 count each.
    struct LotteryInfo {
        int drawsUsed = 0;
        std::vector<LotteryPrize> prizes;
    };

    /// @brief Decodes the draw's info (104/1/1).
    /// @param payload The packet payload.
    /// @return The info, or nothing when malformed.
    std::optional<LotteryInfo> decodeLotteryInfo(const core::Bytes& payload);

    /// @brief The result of a draw: the prize won and the draws used.
    /// @details Original: 104/1/2, u8 prize (its place in the list, from 1), u8 draws used.
    struct LotteryResult {
        int prize = 0;
        int drawsUsed = 0;
    };

    /// @brief Decodes the result of a draw (104/1/2).
    /// @param payload The packet payload.
    /// @return The result, or nothing when malformed.
    std::optional<LotteryResult> decodeLotteryResult(const core::Bytes& payload);

}

#endif
