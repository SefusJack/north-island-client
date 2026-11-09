#ifndef NORTH_ISLAND_UI_IMAGE_BUTTON_H
#define NORTH_ISLAND_UI_IMAGE_BUTTON_H

#include "gfx/TextStyle.h"
#include "ui/Element.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ui {

    /// @brief A button drawn from a picture holding its states (normal, mouse over, pressed) below each other; raises Click and Press.
    /// @details Original: SetPicAndBounds, TSe_FixedButton.FSelected, IsBlockedByModal, StartButtonAnim / StopButtonAnim.
    class ImageButton : public Element {
        public:
            /// @brief Creates a button with the hand cursor, the click sound and the Click and Press events.
            ImageButton();

            /// @brief Reads the ImageButton attributes (Picture, Frames, Toggle, RectHit, IconLines, Animation, Text and the others listed above).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Looks up the pictures and takes the picture's size when the layout gave none.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Whether the mouse is on the button: an opaque pixel, or the whole rectangle with RectHit.
            /// @param local The point relative to the button.
            /// @param pictures The picture library for the pixel test.
            /// @return True when the button takes the mouse there.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Presses the button on a left press and raises Press.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the button.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Releases the button and raises Click when it was pressed.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the button.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Sets the picture without changing sizes.
            /// @param picture The new picture.
            void setPicture(gfx::PictureId picture) { m_picture = picture; }

            /// @brief Shows another picture of as many states; a state's height and the button's size follow it.
            /// @details Original: SetPicAndBounds.
            /// @param picture The new picture.
            /// @param pictures The picture library to read its size from.
            void setPicture(gfx::PictureId picture, const gfx::PictureLibrary& pictures);

            /// @brief The button's picture.
            /// @return The picture id.
            gfx::PictureId picture() const { return m_picture; }

            /// @brief Sets the caption drawn in the middle of a blank button.
            /// @param text The caption.
            void setText(const std::string& text) { m_text = text; }

            /// @brief The caption.
            /// @return The caption text.
            const std::string& text() const { return m_text; }

            /// @brief A selected button shows its pressed look (the open tab).
            /// @details Original: TSe_FixedButton.FSelected.
            /// @param selected True to select it.
            void setSelected(bool selected) { m_selected = selected; }

            /// @brief RectHit from code: the whole rectangle catches the mouse (a button whose bounds move to an animation's frame).
            /// @param rectHit True for rectangle hits.
            void setRectHit(bool rectHit) { m_rectHit = rectHit; }

            /// @brief A question over the form takes the hover and press from the button (its frame stays 0).
            /// @details Original: IsBlockedByModal.
            /// @param blocked True when blocked by a modal.
            void setBlocked(bool blocked) { m_blocked = blocked; }

            /// @brief Turns the Animation frames on or off; a start begins at the first frame.
            /// @details Original: StartButtonAnim / StopButtonAnim.
            /// @param animating True to play the animation.
            void setAnimating(bool animating) {
                if (animating && !m_animating) {
                    m_animationFrame = 0;
                    m_animationFrameMs = 0;
                }
                m_animating = animating;
            }

            /// @brief The state the button shows for the element under the mouse (what draw does first).
            /// @param hoveredId The id of the element under the mouse.
            /// @return The frame row drawn.
            int updateFrame(int hoveredId);

            /// @brief Presses the button from code (Enter in a text box): the pressed look and Click.
            /// @param context The UI context.
            void click(UiContext& context);

        protected:
            /// @brief Draws the button state, its animation frame, icon lines or caption.
            /// @param context The draw context.
            /// @param screenPosition The button's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

            /// @brief Called when the button is clicked; raises Click.
            /// @param context The UI context.
            virtual void clicked(UiContext& context);

        private:
            /// @brief The picture's column that a column of the button shows.
            /// @param x The column in the button.
            /// @return The column in the picture (Narrow: the right half from the picture's right end).
            int pictureX(int x) const;

            gfx::PictureId m_picture;
            std::string m_pictureName;
            int m_frames = 3; // frames stacked in the picture: normal, hover/pressed, selected
            int m_frameHeight = 0;
            int m_state = 0;
            bool m_pressed = false;
            bool m_toggleMode = false;
            bool m_rectHit = false;
            bool m_narrow = false;
            std::vector<std::vector<core::Point>> m_iconLines;
            int m_iconWidth = 1;
            int m_pictureWidth = 0;
            bool m_toggled = false;
            bool m_selected = false;
            bool m_blocked = false;
            std::string m_animationName;
            gfx::PictureId m_animation;
            bool m_animating = true;
            int m_animationFrames = 1;
            int m_animationFirst = 0;
            int m_givenFrameHeight = 0;
            int m_drawHeight = 0;
            int m_animationIntervalMs = 0;
            int m_animationFrame = 0;
            std::uint64_t m_animationFrameMs = 0;
            std::string m_text;
            gfx::TextStyle m_textStyle{{0x10, 0x30, 0x70, 255}, gfx::TextEffect::Plain, std::nullopt}; // default caption colour: dark blue (RGB 16, 48, 112)
    };

}

#endif
