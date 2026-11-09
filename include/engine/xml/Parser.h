#ifndef NORTH_ISLAND_XML_PARSER_H
#define NORTH_ISLAND_XML_PARSER_H

#include "xml/Node.h"

#include <cstddef>
#include <string>

namespace xml {

    /// @brief A small XML parser: elements, attributes, text, comments, CDATA, the XML declaration, predefined entities and numeric references.
    class Parser {
        public:
            /// @brief Parses a document; throws ParseError when malformed.
            /// @param document The XML text.
            /// @return The root element.
            static Node parse(const std::string& document);

        private:
            /// @brief Creates a parser over a document.
            /// @param document The XML text (must outlive the parser).
            explicit Parser(
                const std::string& document
            )
                : m_text(document) {}

            /// @brief Parses the whole document.
            /// @return The root element.
            Node parseDocument();

            /// @brief Parses an element and its content.
            /// @return The element.
            Node parseElement();

            /// @brief Parses the attributes of a start tag.
            /// @param node Element that receives them.
            void parseAttributes(Node& node);

            /// @brief Parses a name.
            /// @return The name.
            std::string parseName();

            /// @brief Replaces entities and character references.
            /// @param raw Text with entities.
            /// @return The decoded text.
            std::string decodeEntities(const std::string& raw);

            /// @brief Skips whitespace.
            void skipWhitespace();

            /// @brief Skips whitespace, comments, processing instructions and a DOCTYPE.
            void skipMisc();

            /// @brief Tells whether the text at the current position starts with a prefix.
            /// @param prefix The prefix.
            /// @return True when it does.
            bool lookingAt(const char* prefix) const;

            /// @brief Consumes a required character.
            /// @param c The character.
            void expect(char c);

            /// @brief Throws a ParseError at the current line.
            /// @param message What is wrong.
            [[noreturn]] void fail(const std::string& message) const;

            const std::string& m_text;
            std::size_t m_pos = 0;
            int m_line = 1;

            /// @brief Moves forward, counting lines.
            /// @param count Characters to skip.
            void advance(std::size_t count = 1);
    };

}

#endif
