#include "client/rules/FurnitureBonus.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kNearRadius = 6; // neighbour distance in cells (|dx| + |dy|)
        constexpr int kBonusStep = 10; // extra products gained per neighbouring piece

        /// @brief Tells whether a product is one that no neighbour changes.
        /// @param itemId Product item id.
        /// @return True for the fixed products.
        // product item ids that no neighbour changes
        // 30256 Halloween Pumpkin, 30383 2010 Halloween Pumpk(in), 55012 2011 Pumpkin, 55052 2012 Spooky Pumpkin, 55090 2013 Spooky Pumpkin
        bool isFixedProduct(int itemId) { return itemId == 30256 || itemId == 30383 || itemId == 55012 || itemId == 55052 || itemId == 55090; }

    }

    int FurnitureBonus::flagOf(int itemId) {
        switch (itemId) {
            case 39053: return 1; // item 39053 Air Condition: flag 1 (15 per cent less time)
            case 39007: return 2; // item 39007 Wood Bell: flag 2 (2 per cent less time)
            case 39045: return 4; // item 39045 Stand Bell: flag 4 (3 per cent less time)
            case 37202: return 8; // item 37202 Windmill: flag 8 (3 per cent less time)
            default: return 0; // other pieces: no flag
        }
    }

    int FurnitureBonus::flags(const HomeFurniture& furniture) {
        int result = 0;
        for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
            const int flag = flagOf(furniture.piece(piece).itemId);
            if (flag != 0 && (result & flag) == 0) {
                result += flag;
            }
        }
        return result;
    }

    int FurnitureBonus::minutes(int baseMinutes, int flags) {
        if (flags == 0) {
            return baseMinutes;
        }
        // percent of the time left: flag 1 takes off 15, flag 2 takes off 2, flags 8 and 4 take off 3 each
        const int percent = 100 - (flags & 1) * 15 - ((flags & 2) / 2) * 2 - ((flags & 8) / 8) * 3 - ((flags & 4) / 4) * 3;
        const int scaled = percent * baseMinutes;
        return scaled / 100 * 100 == scaled ? scaled / 100 : scaled / 100 + 1; // percent to minutes, rounded up
    }

    int FurnitureBonus::findById(const HomeFurniture& furniture, int itemId, bool settledOnly) {
        if (itemId == 0) {
            return 0;
        }
        for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
            if (furniture.piece(piece).itemId == itemId && (!settledOnly || furniture.piece(piece).retained == 0)) {
                return piece;
            }
        }
        return 0;
    }

    int FurnitureBonus::findNear(const HomeFurniture& furniture, int piece, int itemId, int radius) {
        if (piece < 1 || piece > HomeFurniture::kMost) {
            return 0;
        }
        const FurnitureRecord& from = furniture.piece(piece);
        for (int other = 1; other <= HomeFurniture::kMost; ++other) {
            const FurnitureRecord& near = furniture.piece(other);
            if (near.itemId == itemId && near.room == from.room && std::abs(from.cellX - near.cellX) + std::abs(from.cellY - near.cellY) <= radius) {
                return other;
            }
        }
        return 0;
    }

    int FurnitureBonus::extraProducts(const HomeFurniture& furniture, const formats::CompoundTable& compounds, int id, int mode) {
        if (isFixedProduct(id)) {
            return 0;
        }
        const int facility = mode == 1 ? id : compounds.at(compounds.indexOfProduct(id)).facilityId;
        const int piece = findById(furniture, facility);
        if (piece == 0) {
            return 0;
        }
        int bonus = 0;
        const auto near = [&](int itemId) { return findNear(furniture, piece, itemId, kNearRadius) != 0; };
        // facilities 39075 and 39082 gain a step for each of the other two (39075/39082, 39077) and for 39098 or 39172 near them
        // 39075 Turf, 39082 Parterre, 39077 Fence, 39098 and 39172 French Window
        if (facility == 39075) {
            bonus += near(39082) ? kBonusStep : 0;
            bonus += near(39077) ? kBonusStep : 0;
            bonus += near(39098) || near(39172) ? kBonusStep : 0;
        } else if (facility == 39082) {
            bonus += near(39075) ? kBonusStep : 0;
            bonus += near(39077) ? kBonusStep : 0;
            bonus += near(39098) || near(39172) ? kBonusStep : 0;
        }
        return bonus;
    }

}
