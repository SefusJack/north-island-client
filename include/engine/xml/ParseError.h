#ifndef NORTH_ISLAND_XML_PARSE_ERROR_H
#define NORTH_ISLAND_XML_PARSE_ERROR_H

#include <stdexcept>
#include <string>

namespace xml {

    /// @brief Error thrown by Parser for malformed documents; what() names the line.
    class ParseError : public std::runtime_error {
        public:
            /// @brief Creates the error.
            /// @param message What is wrong.
            /// @param line Source line of the problem.
            ParseError(
                const std::string& message,
                int line
            )
                : std::runtime_error("line " + std::to_string(line) + ": " + message) {}
    };

}

#endif
