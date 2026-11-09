#ifndef NORTH_ISLAND_UI_LAYOUT_ERROR_H
#define NORTH_ISLAND_UI_LAYOUT_ERROR_H

#include <stdexcept>
#include <string>

namespace ui {

    /// @brief Thrown for a layout file the toolkit cannot use (unknown element or attribute, bad value, missing element).
    class LayoutError : public std::runtime_error {
        public:
            /// @brief Creates the error.
            /// @param message What is wrong with the layout.
            explicit LayoutError(
                const std::string& message
            )
                : std::runtime_error(message) {}
    };

}

#endif
