#ifndef NORTH_ISLAND_UI_SPOT_H
#define NORTH_ISLAND_UI_SPOT_H

#include "core/Color.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "core/Size.h"
#include "ui/Element.h"

#include <optional>
#include <string>

namespace ui {

    /// @brief A named place in a layout that its view model paints at itself: where a text or picture goes, its colour, and the step and count of a grid.
    /// @details It draws nothing and never takes the mouse, so the area it sits in behaves as without it. Position, Size and Bounds are relative to the parent, like any element's.
    class Spot : public Element {
        public:
            /// @brief Reads the Spot attributes (Color, Pitch, Rows, Columns); others go to Element.
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Gives the spot's position in its parent.
            /// @return The top left of the spot.
            core::Point at() const { return {bounds().x, bounds().y}; }

            /// @brief Gives the spot's rectangle in its parent.
            /// @return The bounds.
            const core::Rect& rect() const { return bounds(); }

            /// @brief Gives the spot's size.
            /// @return The width and height of the bounds.
            core::Size size() const { return {bounds().width, bounds().height}; }

            /// @brief Gives the spot's colour.
            /// @return The Color attribute; throws LayoutError when the layout gave none.
            core::Color color() const;

            /// @brief Gives the step from one grid cell to the next.
            /// @return The Pitch attribute (x, y); throws LayoutError when the layout gave none.
            core::Point pitch() const;

            /// @brief Gives the grid's row count.
            /// @return The Rows attribute; throws LayoutError when the layout gave none.
            int rows() const;

            /// @brief Gives the grid's column count.
            /// @return The Columns attribute; throws LayoutError when the layout gave none.
            int columns() const;

        private:
            std::optional<core::Color> m_color;
            std::optional<core::Point> m_pitch;
            std::optional<int> m_rows;
            std::optional<int> m_columns;
    };

}

#endif
