#ifndef NORTH_ISLAND_UI_PROGRESS_BAR_H
#define NORTH_ISLAND_UI_PROGRESS_BAR_H

#include "ui/Element.h"

namespace ui {

    /// @brief Shows the left part of a picture in proportion to Value / Maximum, like the game's loading bar.
    class ProgressBar : public Element {
        public:
            /// @brief Reads the ProgressBar attributes (Picture, Maximum, Value).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Looks up the picture and takes its size when the layout gave none.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Sets the value, clamped to 0..Maximum.
            /// @param value The new value.
            void setValue(int value);

            /// @brief Sets the maximum.
            /// @param maximum The new maximum (at least 1).
            void setMaximum(int maximum) { m_maximum = maximum > 0 ? maximum : 1; }

            /// @brief The current value.
            /// @return The value.
            int value() const { return m_value; }

            /// @brief The maximum value.
            /// @return The maximum.
            int maximum() const { return m_maximum; }

        protected:
            /// @brief Draws the left part of the picture; its width step is the width divided by Maximum, rounded down.
            /// @param context The draw context.
            /// @param screenPosition The bar's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            gfx::PictureId m_picture;
            std::string m_pictureName;
            int m_maximum = 100; // value at which the bar is full (percent by default)
            int m_value = 0;
    };

}

#endif
