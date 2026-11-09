#ifndef NORTH_ISLAND_UI_CHECK_BOX_H
#define NORTH_ISLAND_UI_CHECK_BOX_H

#include "ui/ImageButton.h"

namespace ui {

    /// @brief An image button that switches between two pictures on each click.
    /// @details Attributes: Picture (unchecked), CheckedPicture, Checked, Click handler.
    class CheckBox : public ImageButton {
        public:
            /// @brief Reads the CheckedPicture and Checked attributes; others go to ImageButton.
            /// @param name Attribute name.
            /// @param value Attribute value text.
            /// @param context Load context for parsing.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves both pictures and shows the initial state.
            /// @param context Load context for picture lookup.
            void finishLoading(const LoadContext& context) override;

            /// @brief Tells whether the box is checked.
            /// @return True when checked.
            bool isChecked() const { return m_checked; }

            /// @brief Sets the state and shows the matching picture.
            /// @param checked New state.
            void setChecked(bool checked);

        protected:
            /// @brief Toggles the state, then lets ImageButton raise its click.
            /// @param context UI context.
            void clicked(UiContext& context) override;

        private:
            std::string m_checkedName;
            gfx::PictureId m_uncheckedPicture;
            gfx::PictureId m_checkedPicture;
            bool m_checked = false;
    };

}

#endif
