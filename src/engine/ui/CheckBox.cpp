#include "ui/CheckBox.h"

namespace ui {

    void CheckBox::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "CheckedPicture") {
            m_checkedName = value;
        } else if (name == "Checked") {
            m_checked = LoadContext::parseBool(value);
        } else {
            ImageButton::applyAttribute(name, value, context);
        }
    }

    void CheckBox::finishLoading(const LoadContext& context) {
        ImageButton::finishLoading(context);
        m_uncheckedPicture = picture();
        m_checkedPicture = context.picture(m_checkedName);
        setChecked(m_checked);
    }

    void CheckBox::setChecked(bool checked) {
        m_checked = checked;
        setPicture(m_checked ? m_checkedPicture : m_uncheckedPicture);
    }

    void CheckBox::clicked(UiContext& context) {
        setChecked(!m_checked);
        ImageButton::clicked(context);
    }

}
