#include "ui/ProgressBar.h"

#include <algorithm>

namespace ui {

    void ProgressBar::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Picture") {
            m_pictureName = value;
        } else if (name == "Maximum") {
            m_maximum = std::max(1, LoadContext::parseInt(value));
        } else if (name == "Value") {
            m_value = LoadContext::parseInt(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void ProgressBar::finishLoading(const LoadContext& context) {
        m_picture = context.picture(m_pictureName);
        if (m_picture.isValid() && !sizeWasGiven()) {
            const gfx::Picture& picture = context.pictureData(m_picture);
            setBounds({bounds().x, bounds().y, picture.width(), picture.height()});
        }
        Element::finishLoading(context);
    }

    void ProgressBar::setValue(int value) { m_value = std::clamp(value, 0, m_maximum); }

    void ProgressBar::draw(DrawContext& context, core::Point screenPosition) {
        const int width = bounds().width / m_maximum * m_value;
        if (width > 0) {
            context.canvas.drawPicture(m_picture, screenPosition, core::Rect{0, 0, width, bounds().height});
        }
    }

}
