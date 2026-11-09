#ifndef NORTH_ISLAND_CLIENT_RULES_STAT_BUFF_ITEMS_H
#define NORTH_ISLAND_CLIENT_RULES_STAT_BUFF_ITEMS_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief Rules for the stat buff items (Chili Sauce, Body Lotion, ...).
    /// @details Original: UseGridItem sends them as 23/119 3 with the bag place (instead of the plain use 23/15), unless the buff of their stat is already running (Se_EquipUI.pas:5958-5978).
    class StatBuffItems {
        public:
            /// @brief Gives the buff code of an item.
            /// @details Original: GetStatBuffItemCode (AC_Item.pas:1867). The hundreds are the place of the stat's four bits in FStrengthenBuffMask (3 ATK, 4 DEF, 5 MATK, 6 MDEF), the rest the item's level.
            /// @param itemId Id of the item.
            /// @return The buff code, or 0 for every other item.
            static int code(int itemId);

            /// @brief Checks whether such an item may be used now.
            /// @details Original: UseGridItem's checks: "Unable to use in combat" in a battle, "Unable to use" while GetBitField(FStrengthenBuffMask, 4, code div 100) is not 0.
            /// @param itemId Id of the item.
            /// @param fighting True while in a battle.
            /// @param buffMask The FStrengthenBuffMask value.
            /// @return The refusal text, or empty when the item goes to the server.
            static std::string refusal(int itemId, bool fighting, std::uint32_t buffMask);
    };

}

#endif
