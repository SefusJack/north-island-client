#include "ui/ImageButton.h"

#include "gfx/TextRenderer.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>

namespace ui {

    namespace {

        /// @brief Parses the IconLines attribute into polylines.
        /// @param value Lines separated by ';', each a list of "x,y" points; lines with fewer than two points are dropped.
        /// @return The polylines.
        std::vector<std::vector<core::Point>> parseIconLines(const std::string& value) {
            std::vector<std::vector<core::Point>> lines;
            std::istringstream all(value);
            std::string line;
            while (std::getline(all, line, ';')) {
                std::vector<core::Point> points;
                std::istringstream words(line);
                std::string word;
                while (words >> word) {
                    const core::Point point = LoadContext::parsePoint(word);
                    points.push_back(point);
                }
                if (points.size() >= 2) { // a line needs two points at least
                    lines.push_back(std::move(points));
                }
            }
            return lines;
        }

    }

    ImageButton::ImageButton() {
        declareEvent("Click");
        declareEvent("Press");
        setCursor(CursorKind::Hand);
        setClickSound("sound/wav0151.wav"); // the default button click sound (wav0151)
    }

    void ImageButton::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Picture") {
            m_pictureName = value;
        } else if (name == "Frames") {
            m_frames = std::max(1, LoadContext::parseInt(value));
        } else if (name == "Toggle") {
            m_toggleMode = LoadContext::parseBool(value);
        } else if (name == "RectHit") {
            m_rectHit = LoadContext::parseBool(value);
        } else if (name == "IconLines") {
            m_iconLines = parseIconLines(value);
        } else if (name == "IconWidth") {
            m_iconWidth = std::max(1, LoadContext::parseInt(value));
        } else if (name == "Narrow") {
            m_narrow = LoadContext::parseBool(value);
        } else if (name == "Animation") {
            m_animationName = value;
        } else if (name == "AnimationFirst") {
            m_animationFirst = std::max(0, LoadContext::parseInt(value));
        } else if (name == "FrameHeight") {
            m_givenFrameHeight = std::max(1, LoadContext::parseInt(value));
        } else if (name == "DrawHeight") {
            m_drawHeight = std::max(1, LoadContext::parseInt(value));
        } else if (name == "AnimationFrames") {
            m_animationFrames = std::max(1, LoadContext::parseInt(value));
        } else if (name == "AnimationInterval") {
            m_animationIntervalMs = std::max(1, LoadContext::parseInt(value));
        } else if (name == "Text") {
            m_text = value;
        } else if (name == "FontColor") {
            m_textStyle.color = LoadContext::parseColor(value);
        } else if (name == "Effect") {
            m_textStyle.effect = LoadContext::parseEffect(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void ImageButton::finishLoading(const LoadContext& context) {
        m_picture = context.picture(m_pictureName);
        if (m_picture.isValid()) {
            const gfx::Picture& picture = context.pictureData(m_picture);
            m_pictureWidth = picture.width();
            m_frameHeight = m_givenFrameHeight > 0 ? m_givenFrameHeight : picture.height() / m_frames;
            if (!sizeWasGiven()) {
                setBounds({bounds().x, bounds().y, picture.width(), m_frameHeight});
            }
        }
        if (m_frameHeight == 0) {
            m_frameHeight = bounds().height;
        }
        if (!m_animationName.empty()) {
            m_animation = context.picture(m_animationName);
        }
        Element::finishLoading(context);
    }

    void ImageButton::setPicture(gfx::PictureId picture, const gfx::PictureLibrary& pictures) {
        m_picture = picture;
        if (!m_picture.isValid()) {
            return;
        }
        const gfx::Picture& data = pictures.picture(m_picture);
        m_pictureWidth = data.width();
        m_frameHeight = m_givenFrameHeight > 0 ? m_givenFrameHeight : data.height() / m_frames;
        setBounds({bounds().x, bounds().y, data.width(), m_frameHeight});
    }

    bool ImageButton::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        if (!core::Rect{0, 0, bounds().width, bounds().height}.contains(local)) {
            return false;
        }
        return m_rectHit || !m_picture.isValid() || pictures.picture(m_picture).isOpaque(pictureX(local.x), local.y);
    }

    int ImageButton::pictureX(int x) const {
        const int width = bounds().width;
        if (!m_narrow || width >= m_pictureWidth || x < width / 2) {
            return x;
        }
        return m_pictureWidth - (width - x);
    }

    void ImageButton::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        (void)local;
        if (button == MouseButton::Left && isEnabled()) {
            m_pressed = true;
            m_state = 2; // state 2: pressed
            raise(context, "Press");
        }
    }

    void ImageButton::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        (void)local;
        if (button != MouseButton::Left || !isEnabled() || !m_pressed) {
            return;
        }
        m_pressed = false;
        m_state = 1; // state 1: mouse over
        if (m_toggleMode) {
            m_toggled = !m_toggled;
        }
        clicked(context);
    }

    void ImageButton::click(UiContext& context) {
        if (!isVisible() || !isEnabled()) {
            return;
        }
        m_state = 2; // state 2: pressed
        clicked(context);
    }

    void ImageButton::clicked(UiContext& context) { raise(context, "Click"); }

    int ImageButton::updateFrame(int hoveredId) {
        if (hoveredId == id() && !m_blocked) {
            if (m_state != 2) { // state 2: pressed
                m_state = 1; // state 1: mouse over
            }
        } else {
            m_state = 0;
            m_pressed = false;
        }
        // a selected or toggled button shows frame 2, the pressed look
        return m_selected || (m_toggleMode && m_toggled) ? std::min(2, m_frames - 1) : std::min(m_state, m_frames - 1);
    }

    void ImageButton::draw(DrawContext& context, core::Point screenPosition) {
        const int state = updateFrame(context.hoveredId);
        if (m_animation.isValid() && m_animationIntervalMs > 0 && m_animating) {
            if (m_animationFrameMs == 0) {
                m_animationFrameMs = context.nowMs;
            } else if (context.nowMs - m_animationFrameMs > static_cast<std::uint64_t>(m_animationIntervalMs)) {
                m_animationFrame = (m_animationFrame + 1) % m_animationFrames;
                m_animationFrameMs = context.nowMs;
            }
            context.canvas.drawPicture(
                m_animation,
                screenPosition,
                core::Rect{0, (m_animationFirst + m_animationFrame) * bounds().height, bounds().width, bounds().height}
            );
            return;
        }
        const int rows = m_drawHeight > 0 ? m_drawHeight : std::min(m_frameHeight, bounds().height);
        if (m_narrow && bounds().width < m_pictureWidth) {
            const int left = bounds().width / 2;
            const int right = bounds().width - left;
            context.canvas.drawPicture(m_picture, screenPosition, core::Rect{0, state * m_frameHeight, left, rows});
            context.canvas
                .drawPicture(m_picture, {screenPosition.x + left, screenPosition.y}, core::Rect{m_pictureWidth - right, state * m_frameHeight, right, rows});
        } else {
            context.canvas.drawPicture(m_picture, screenPosition, core::Rect{0, state * m_frameHeight, bounds().width, rows});
        }
        if (!m_iconLines.empty()) {
            int right = 0;
            int bottom = 0;
            for (const auto& line : m_iconLines) {
                for (const core::Point& point : line) {
                    right = std::max(right, point.x);
                    bottom = std::max(bottom, point.y);
                }
            }
            // pressed (state 2) and not selected: the icon sits one pixel lower
            const core::Point at{
                screenPosition.x + (bounds().width - right) / 2,
                screenPosition.y + (bounds().height - bottom) / 2 + (state == 2 && !m_selected ? 1 : 0)
            };
            for (const auto& line : m_iconLines) {
                for (std::size_t i = 1; i < line.size(); ++i) {
                    context.canvas.drawLine({at.x + line[i - 1].x, at.y + line[i - 1].y}, {at.x + line[i].x, at.y + line[i].y}, m_iconWidth, m_textStyle.color);
                }
            }
        } else if (!m_text.empty()) {
            const int width = gfx::TextRenderer::textWidth(m_text, m_textStyle.effect);
            // 12: capital height in rows; pressed (state 2) and not selected: one pixel lower
            const core::Point at{
                screenPosition.x + (bounds().width - width) / 2,
                screenPosition.y + (bounds().height - 12) / 2 + (state == 2 && !m_selected ? 1 : 0)
            };
            context.canvas.drawText(at, m_text, m_textStyle);
        }
    }

}
