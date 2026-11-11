#ifndef NORTH_ISLAND_UI_LAYOUT_LOADER_H
#define NORTH_ISLAND_UI_LAYOUT_LOADER_H

#include "ui/Element.h"
#include "ui/LoadContext.h"
#include "xml/Node.h"

#include <memory>

namespace ui {

    /// @brief Builds an element tree from a layout document.
    class LayoutLoader {
        public:
            /// @brief Builds the element for a node and, recursively, its children.
            /// @param node The layout node.
            /// @param context The load context.
            /// @return The new element; throws LayoutError (with the line) for anything unknown.
            static std::unique_ptr<Element> build(const xml::Node& node, const LoadContext& context);

        private:
            /// @brief Creates an empty element for a node name.
            /// @param type The node name (Panel, Image, ImageButton, ...).
            /// @return The element, or null for an unknown name.
            static std::unique_ptr<Element> create(const std::string& type);
    };

}

#endif
