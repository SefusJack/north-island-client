#ifndef NORTH_ISLAND_XML_NODE_H
#define NORTH_ISLAND_XML_NODE_H

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace xml {

    /// @brief One XML element: name, attributes in document order, child elements and its direct text.
    class Node {
        public:
            /// @brief Creates an element.
            /// @param name Element name.
            explicit Node(
                std::string name = {}
            )
                : m_name(std::move(name)) {}

            /// @brief Gives the element name.
            /// @return The name.
            const std::string& name() const { return m_name; }

            /// @brief Gives the attributes in document order.
            /// @return Name/value pairs.
            const std::vector<std::pair<std::string, std::string>>& attributes() const { return m_attributes; }

            /// @brief Gives the child elements.
            /// @return The children.
            const std::vector<Node>& children() const { return m_children; }

            /// @brief Gives the text directly inside the element.
            /// @return The text.
            const std::string& text() const { return m_text; }

            /// @brief Gives the source line of the start tag.
            /// @return The line number.
            int line() const { return m_line; }

            /// @brief Looks up an attribute.
            /// @param name Attribute name.
            /// @return Its value, or nothing.
            std::optional<std::string> attribute(const std::string& name) const;

            /// @brief Tells whether an attribute is present.
            /// @param name Attribute name.
            /// @return True when present.
            bool hasAttribute(const std::string& name) const { return attribute(name).has_value(); }

            /// @brief Sets the source line.
            /// @param line The line number.
            void setLine(int line) { m_line = line; }

            /// @brief Appends an attribute.
            /// @param name Attribute name.
            /// @param value Attribute value.
            void addAttribute(std::string name, std::string value);

            /// @brief Appends a child element.
            /// @param child The child.
            void addChild(Node child) { m_children.push_back(std::move(child)); }

            /// @brief Appends text to the element.
            /// @param text Text to append.
            void appendText(const std::string& text) { m_text += text; }

        private:
            std::string m_name;
            std::vector<std::pair<std::string, std::string>> m_attributes;
            std::vector<Node> m_children;
            std::string m_text;
            int m_line = 0;
    };

}

#endif
