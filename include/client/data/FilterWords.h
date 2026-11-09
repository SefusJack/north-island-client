#ifndef NORTH_ISLAND_CLIENT_DATA_FILTER_WORDS_H
#define NORTH_ISLAND_CLIENT_DATA_FILTER_WORDS_H

#include <array>

namespace client {

    /// @brief Gives the parts a name may not hold anywhere (BadNameParts, 8; wb_texmgr.pas).
    /// @return The list of refused name parts.
    const std::array<const char*, 8>& nameParts(); // 8 = number of name-part entries

    /// @brief Gives the words a name may not hold (BadNameWords, 195; wb_texmgr.pas).
    /// @return The list of refused name words.
    const std::array<const char*, 195>& nameWords(); // 195 = number of BadNameWords entries

    /// @brief Gives the words a text may not hold (BadTextWords, 146; wb_texmgr.pas).
    /// @return The list of refused text words.
    const std::array<const char*, 146>& textWords(); // 146 = number of BadTextWords entries

}

#endif
