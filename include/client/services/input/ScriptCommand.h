#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_COMMAND_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_COMMAND_H

#include "client/services/input/ScriptClickSpec.h"

#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The reading of the script lines that name objects of the scene (see InputScript).
    struct ScriptCommand {
        /// @brief Splits a line into words; "double quotes" keep spaces inside a word.
        /// @param line The script line.
        /// @return The words.
        static std::vector<std::string> words(const std::string& line);

        /// @brief Reads a whole decimal number (optional minus, at most 9 digits).
        /// @param word The word.
        /// @return The number, empty when the word is not one.
        static std::optional<int> number(const std::string& word);

        /// @brief Finds the object kind of a name: self, npc, npcindex, player, ground, furn, furnitem.
        /// @param name The kind name.
        /// @return The kind, empty when unknown.
        static std::optional<ScriptObjectKind> kindNamed(const std::string& name);

        /// @brief Gives the script name of an object kind.
        /// @param kind The kind.
        /// @return The name ("?" for an unknown kind).
        static std::string kindName(ScriptObjectKind kind);

        /// @brief Reads the object named by the words from `first` on ("furnitem 39057"; "self" has no number).
        /// @param words The line's words.
        /// @param first Index of the kind word.
        /// @param error Receives what is wrong when the result is empty.
        /// @return The object, empty on an error.
        static std::optional<ScriptObjectRef> object(const std::vector<std::string>& words, std::size_t first, std::string& error);

        /// @brief Reads clickobj KIND ID and clicknpc ID, with a d (double click), r (right button) or ctrl prefix (dclickobj, rclicknpc, ctrlclickobj ...).
        /// @param words The line's words.
        /// @param error Receives what is wrong with a click command.
        /// @return The click, empty for another command or on an error.
        static std::optional<ScriptClickSpec> click(const std::vector<std::string>& words, std::string& error);
    };

}

#endif
