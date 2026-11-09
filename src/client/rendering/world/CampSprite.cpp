#include "client/rendering/world/CampSprite.h"

#include "client/models/world/Camps.h"

namespace client {

    namespace {

        constexpr int kPlainStyle = 9001; // sprite 9001 (bank 009): plain tent style
        constexpr int kSign = 9010; // sprite 9010 (bank 009): camp sign shown for other players' camps
        constexpr int kFirstClayStyle = 9071; // first sprite id found in bank 009_c01 (Shape Clay styles)
        constexpr int kLastClayStyle = 9093; // last sprite id found in bank 009_c01

        /// @brief Picks the bank for a style sprite (the later clay styles are in 009_c01).
        /// @param id Sprite id.
        /// @return The sprite with its bank.
        CampSprite styled(int id) { return {id >= kFirstClayStyle && id <= kLastClayStyle ? "009_c01" : "009", id}; }

    }

    std::optional<CampSprite> campSprite(const Camp& camp, bool ownLook, const formats::ItemTable& items) {
        const int style = camp.kind2 == 0 ? Camps::kindId(camp.kind1, 1) : Camps::kindId(camp.kind2, 2);
        const bool plain = style == kPlainStyle || style == 0;
        if (ownLook) {
            if (plain) {
                const auto item = items.find(camp.tentItemId);
                return item ? std::optional<CampSprite>(CampSprite{"009", item->lookIds[0]}) : std::nullopt;
            }
            return styled(style);
        }
        return plain ? CampSprite{"009", kSign} : styled(style + 1);
    }

}
