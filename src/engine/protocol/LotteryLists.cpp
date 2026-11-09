#include "protocol/LotteryLists.h"

#include "core/ByteReader.h"

#include <algorithm>

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 3;
        constexpr int kMostPrizes = 20; // at most 20 prizes in a lottery list

    }

    std::optional<LotteryInfo> decodeLotteryInfo(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        LotteryInfo info;
        info.drawsUsed = reader.u8();
        const int count = std::min<int>(reader.u8(), kMostPrizes);
        for (int i = 0; i < count; ++i) {
            LotteryPrize prize;
            prize.itemId = reader.u16();
            prize.count = reader.u8();
            info.prizes.push_back(prize);
        }
        if (reader.failed()) {
            return std::nullopt;
        }
        return info;
    }

    std::optional<LotteryResult> decodeLotteryResult(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        LotteryResult result;
        result.prize = reader.u8();
        result.drawsUsed = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return result;
    }

}
