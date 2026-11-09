#ifndef NORTH_ISLAND_CLIENT_MODELS_ITEMS_GIFT_CARD_FILE_H
#define NORTH_ISLAND_CLIENT_MODELS_ITEMS_GIFT_CARD_FILE_H

#include "client/models/items/GiftCard.h"

#include <filesystem>
#include <vector>

namespace client {

    /// @brief The lists of the gift cards the client keeps on disk.
    /// @details Original: Data\Gec.Dat, HL_Menu.pas LoadGiftCards and SaveGiftCards. Layout: u16 count, then per card u16 id, u8 version, u8 number of choices, and per choice u16 item and u8 count.
    struct GiftCardFile {
        /// @brief Reads the cards of a file; a file that is too short ends the list where it stops.
        /// @details Original: LoadGiftCards.
        /// @param path File to read.
        /// @return The cards; none for a missing file.
        static std::vector<GiftCard> read(const std::filesystem::path& path);

        /// @brief Writes the cards that have an id.
        /// @details Original: SaveGiftCards.
        /// @param path File to write.
        /// @param cards Cards to save.
        /// @return True when the file was written.
        static bool write(const std::filesystem::path& path, const std::vector<GiftCard>& cards);
    };

}

#endif
