#include "client/data/PackingItems.h"

#include <algorithm>
#include <iterator>

namespace client {

    bool isPackableShopPicture(int shopPicId) {
        // shop picture ids: the first column of each of the 20 rows of PackingItemTable
        // in row order: Sky Sword, Lord Sword, Beast Fork, Spirit Fork, Polished Wand, Light Wand, Attack Pestle, Crescent Bow,
        // Energy Halo, Gold Ears, Stand Ears, Jade Ears, Dark Evil Wings, Angel Wings, Archangel Wings, Golden Rings,
        // Purple Wing Robe, Kung-Fu Suit, Friendship Brooch, Thunder Spear
        constexpr int kPictures[] = {19681, 19682, 20301, 20302, 19891, 19891, 19651, 20807, 19601, 13822,
                                     13901, 13831, 19571, 19561, 19561, 28841, 21871, 29101, 20321, 20808};
        return std::find(std::begin(kPictures), std::end(kPictures), shopPicId) != std::end(kPictures);
    }

}
