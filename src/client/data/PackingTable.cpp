#include "client/data/PackingTable.h"

#include <array>
#include <cstdint>

namespace client {

    namespace {

        constexpr int kColumns = 12; // 1 shop picture id + 11 items (+0..+10)

        // 20 chains: shop picture id, then up to 11 item ids (0 = end of the chain)
        constexpr std::array<std::array<std::uint16_t, kColumns>, 20> kChains{{
            // each row below: shop picture id (an icon id, not an item), then the item at +0, +1, ... up to +10
            {19681, 11101, 11103, 11105, 11107, 11110, 11048, 11117, 11130, 11133, 11138, 11146}, // Sky Sword +0..+10
            {19682, 11102, 11104, 11106, 11108, 11111, 11050, 11118, 11131, 11134, 11139, 11147}, // Lord Sword +0..+10
            {20301, 12072, 12074, 12076, 12078, 12080, 12082, 12504, 12093, 12096, 12099, 12116}, // Beast Fork +0..+10
            {20302, 12073, 12075, 12077, 12079, 12081, 12083, 12505, 12094, 12097, 12100, 12117}, // Spirit Fork +0..+10
            {19891, 14076, 14079, 14081, 14083, 14085, 14120, 14122, 14131, 14136, 14144, 14161}, // Polished Wand +0..+10
            {19891, 14078, 14080, 14082, 14084, 14086, 14121, 14123, 14132, 14137, 14145, 14162}, // Light Wand +0..+10
            {19651, 18063, 18066, 18067, 18068, 18069, 0, 0, 0, 0, 0, 0}, // Attack Pestle +0..+4
            {20807, 13061, 13062, 13063, 13060, 13068, 13059, 13069, 0, 0, 0, 0}, // Crescent Bow +0..+6
            {19601, 22135, 22163, 22171, 22175, 22210, 22216, 22223, 22240, 22246, 22250, 22507}, // Energy Halo +0..+10
            {13822, 22157, 22160, 22168, 22172, 22207, 22213, 22220, 0, 0, 0, 0}, // Gold Ears +0..+6
            {13901, 22158, 22161, 22169, 22173, 22208, 22214, 22221, 22238, 22244, 22248, 22505}, // Stand Ears +0..+6, then +7..+9 (named Gold Ears), +10
            {13831, 22159, 22162, 22170, 22174, 22209, 22215, 22222, 22239, 22245, 22249, 22506}, // Jade Ears +0..+6, then +7..+9 (Bunny/Jade Rabbit Ears), +10
            {19571, 25191, 25204, 25212, 25222, 25589, 25592, 25605, 25611, 25614, 25621, 25633}, // Dark Evil Wings +0..+10
            {19561, 25192, 25205, 25213, 25223, 25590, 25593, 25606, 25612, 25615, 25622, 25634}, // Angel Wings +0..+10
            {19561, 25215, 25216, 25217, 25224, 25591, 25595, 25607, 0, 0, 0, 0}, // Archangel Wings +0..+6
            {28841, 15050, 15053, 15054, 15055, 15056, 15057, 15058, 15060, 15061, 15063, 15066}, // Golden Rings +0..+10
            {21871, 21174, 21175, 21209, 21210, 21211, 21212, 21223, 21283, 0, 0, 0}, // Purple Wing Robe +0..+7
            {29101, 21213, 21214, 21215, 21216, 21217, 21218, 21224, 21284, 0, 0, 0}, // Kung-Fu Suit +0..+7
            {20321, 25207, 25599, 25600, 25601, 25602, 25603, 25608, 25613, 25616, 25623, 25635}, // Friendship Brooch +0..+10
            {20808, 12101, 12102, 12103, 12104, 12105, 12106, 12107, 12108, 12109, 12110, 12115}, // Thunder Spear +0..+10
        }};

    }

    std::optional<PackingTable::Step> PackingTable::find(int shopPicId, int itemId) {
        for (const auto& chain : kChains) {
            if (chain[0] != shopPicId) {
                continue;
            }
            for (int column = 1; column < kColumns; ++column) {
                if (chain[static_cast<std::size_t>(column)] == itemId) {
                    const bool more = column + 1 < kColumns && chain[static_cast<std::size_t>(column) + 1] != 0;
                    return Step{column - 1, more};
                }
            }
        }
        return std::nullopt;
    }

}
