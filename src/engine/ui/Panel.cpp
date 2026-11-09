#include "ui/Panel.h"

namespace ui {

    void Panel::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Skin") {
            m_skinName = value;
        } else if (name == "SkinSource") {
            m_skinSource = LoadContext::parseRect(value);
        } else if (name == "Borders") {
            const std::vector<int> borders = LoadContext::parseInts(value, 4); // left, top, right, bottom
            for (int i = 0; i < 4; ++i) { // the four borders
                m_borders[i] = borders[static_cast<std::size_t>(i)];
            }
        } else if (name == "HitTest") {
            m_hitTest = LoadContext::parseBool(value);
        } else if (name == "ClipChildren") {
            m_clipChildren = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void Panel::finishLoading(const LoadContext& context) {
        m_skinPicture = context.picture(m_skinName);
        if (m_skinPicture.isValid()) {
            const gfx::Picture& picture = context.pictureData(m_skinPicture);
            const core::Rect source = m_skinSource.value_or(core::Rect{0, 0, picture.width(), picture.height()});
            m_skin = TiledFrame(m_skinPicture, source, m_borders[0], m_borders[1], m_borders[2], m_borders[3]);
        }
        Element::finishLoading(context);
    }

    void Panel::setSkinPicture(gfx::PictureId picture, const gfx::PictureLibrary& pictures) {
        if (!picture.isValid() || picture == m_skinPicture) {
            return;
        }
        m_skinPicture = picture;
        const gfx::Picture& data = pictures.picture(m_skinPicture);
        const core::Rect source = m_skinSource.value_or(core::Rect{0, 0, data.width(), data.height()});
        m_skin = TiledFrame(m_skinPicture, source, m_borders[0], m_borders[1], m_borders[2], m_borders[3]);
    }

    bool Panel::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        if (m_hitTest) {
            return core::Rect{0, 0, bounds().width, bounds().height}.contains(local);
        }
        if (!m_skin.isValid()) {
            return false;
        }
        return m_skin.isOpaqueAt(pictures.picture(m_skinPicture), local, bounds().width, bounds().height);
    }

    void Panel::draw(DrawContext& context, core::Point screenPosition) {
        m_skin.draw(context.canvas, {screenPosition.x, screenPosition.y, bounds().width, bounds().height});
    }

}
