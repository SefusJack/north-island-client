#include "ui/ImageView.h"

#include <algorithm>

namespace ui {

    void ImageView::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Picture") {
            m_pictureName = value;
        } else if (name == "Source") {
            m_sourceFile = value;
        } else if (name == "SourceRect") {
            m_sourceRect = LoadContext::parseRect(value);
        } else if (name == "HitTest") {
            m_hitTest = LoadContext::parseBool(value);
        } else if (name == "Frames") {
            m_frames = std::max(1, LoadContext::parseInt(value));
        } else if (name == "FrameInterval") {
            m_frameIntervalMs = LoadContext::parseInt(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void ImageView::finishLoading(const LoadContext& context) {
        m_picture = m_sourceFile.empty() ? context.picture(m_pictureName) : context.pictureFile(m_sourceFile);
        if (m_picture.isValid()) {
            const gfx::Picture& picture = context.pictureData(m_picture);
            m_frameHeight = picture.height() / m_frames;
            if (!sizeWasGiven()) {
                const core::Rect area = m_sourceRect.value_or(core::Rect{0, 0, picture.width(), m_frameHeight});
                setBounds({bounds().x, bounds().y, area.width, area.height});
            }
        }
        Element::finishLoading(context);
    }

    bool ImageView::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        if (!m_hitTest || !m_picture.isValid() || !core::Rect{0, 0, bounds().width, bounds().height}.contains(local)) {
            return false;
        }
        return pictures.picture(m_picture).isOpaque(local.x, local.y);
    }

    void ImageView::draw(DrawContext& context, core::Point screenPosition) {
        if (!m_picture.isValid()) {
            return;
        }
        if (m_frames > 1 && m_frameIntervalMs > 0) {
            if (m_frameStartMs == 0) {
                m_frameStartMs = context.nowMs;
            } else if (context.nowMs - m_frameStartMs > static_cast<std::uint64_t>(m_frameIntervalMs)) {
                m_frame = (m_frame + 1) % m_frames;
                m_frameStartMs = context.nowMs;
            }
        }
        const int frameHeight = m_frameHeight > 0 ? m_frameHeight : bounds().height;
        core::Rect source = m_sourceRect.value_or(core::Rect{0, 0, bounds().width, frameHeight});
        source.y += m_frame * frameHeight;
        context.canvas.drawPicture(m_picture, screenPosition, source);
    }

}
