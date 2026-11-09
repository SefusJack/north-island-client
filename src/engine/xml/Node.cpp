#include "xml/Node.h"

namespace xml {

    std::optional<std::string> Node::attribute(const std::string& name) const {
        for (const auto& attribute : m_attributes) {
            if (attribute.first == name) {
                return attribute.second;
            }
        }
        return std::nullopt;
    }

    void Node::addAttribute(std::string name, std::string value) { m_attributes.emplace_back(std::move(name), std::move(value)); }

}
