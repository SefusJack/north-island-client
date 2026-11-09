#include "ui/Spot.h"

#include "ui/LayoutError.h"

namespace ui {

    namespace {

        /// @brief Gives a value the layout must have given.
        /// @tparam T The value type.
        /// @param value The value, if given.
        /// @param spot The spot's name.
        /// @param attribute The attribute that gives it.
        /// @return The value; throws LayoutError when it was not given.
        template <typename T>
        T required(const std::optional<T>& value, const std::string& spot, const char* attribute) {
            if (!value) {
                throw LayoutError("spot \"" + spot + "\" has no " + attribute);
            }
            return *value;
        }

    }

    void Spot::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Color") {
            m_color = LoadContext::parseColor(value);
        } else if (name == "Pitch") {
            m_pitch = LoadContext::parsePoint(value);
        } else if (name == "Rows") {
            m_rows = LoadContext::parseInt(value);
        } else if (name == "Columns") {
            m_columns = LoadContext::parseInt(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    core::Color Spot::color() const { return required(m_color, name(), "Color"); }

    core::Point Spot::pitch() const { return required(m_pitch, name(), "Pitch"); }

    int Spot::rows() const { return required(m_rows, name(), "Rows"); }

    int Spot::columns() const { return required(m_columns, name(), "Columns"); }

}
