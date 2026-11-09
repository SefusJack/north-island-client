#ifndef NORTH_ISLAND_CLIENT_DATA_PACKING_ITEMS_H
#define NORTH_ISLAND_CLIENT_DATA_PACKING_ITEMS_H

namespace client {

    /// @brief Tells whether a mall picture is one of the gear a Present Case takes.
    /// @details Original: PackingItemTable (DZ_String.pas), the first of each of the table's 20 rows.
    /// @param shopPicId The mall picture id.
    /// @return True when packable.
    bool isPackableShopPicture(int shopPicId);

}

#endif
