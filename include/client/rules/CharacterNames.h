#ifndef NORTH_ISLAND_CLIENT_RULES_CHARACTER_NAMES_H
#define NORTH_ISLAND_CLIENT_RULES_CHARACTER_NAMES_H

#include <optional>
#include <string>

namespace client {

    constexpr std::size_t kMaxCharacterNameLength = 10; // most bytes of a new character's name

    /// @brief Checks a new character name before it is sent to the server.
    /// @details Original: TRE_CreateCharacter.HandleCreateCharButton, HasNonAsciiChar, IsNameClean.
    /// @param name The name typed in the creation wizard.
    /// @return The message the wizard shows, or nothing when the name may be sent.
    std::optional<std::string> characterNameProblem(const std::string& name);

}

#endif
