#ifndef NORTH_ISLAND_UI_SCREEN_H
#define NORTH_ISLAND_UI_SCREEN_H

#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"
#include "ui/CursorKind.h"
#include "ui/Element.h"
#include "ui/KeyEvent.h"
#include "ui/LayoutError.h"
#include "ui/LayoutSource.h"
#include "ui/LoadContext.h"
#include "ui/MouseButton.h"
#include "ui/UiContext.h"
#include "ui/Window.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ui {

    /// @brief The window's UI: layout files stacked in load order, with mouse/keyboard routing between them.
    class Screen {
        public:
            /// @brief Creates an empty screen.
            /// @param pictures Picture library used for hit tests and drawing.
            explicit Screen(
                gfx::PictureLibrary& pictures
            );
            Screen(const Screen&) = delete;
            Screen& operator=(const Screen&) = delete;

            /// @brief Adds a layout on top of the others; throws LayoutError (or xml::ParseError) on failure.
            /// @param layout Layout name and XML text.
            /// @param loadContext Context used to resolve pictures and attributes while building.
            /// @return The layout's root element.
            Element& load(const LayoutSource& layout, const LoadContext& loadContext);

            /// @brief Reads a layout file and adds it on top of the others.
            /// @param layoutFile Path of the layout XML file.
            /// @param loadContext Context used to resolve pictures and attributes while building.
            /// @return The layout's root element.
            Element& load(const std::filesystem::path& layoutFile, const LoadContext& loadContext);

            /// @brief Gives the UI context shared by the elements.
            /// @return The context.
            UiContext& context() { return m_context; }

            /// @brief Registers the handler for an event name used by the layouts.
            /// @param handlerName Event handler name as written in the layout.
            /// @param handler Function called when the event is raised.
            void on(const std::string& handlerName, UiContext::Handler handler) { m_context.setHandler(handlerName, std::move(handler)); }

            /// @brief Finds the element of that name and type; throws LayoutError when there is none.
            /// @tparam T Expected element type.
            /// @param name Element name.
            /// @return The element.
            template <typename T>
            T& get(const std::string& name) {
                Element& element = findByName(name);
                T* typed = dynamic_cast<T*>(&element);
                if (typed == nullptr) {
                    throw LayoutError("element " + name + " has another type");
                }
                return *typed;
            }

            /// @brief Gives the keyboard to the element of that name.
            /// @param name Element name.
            void focus(const std::string& name);

            /// @brief Moves the mouse: updates hover and drags the captured element.
            /// @param position Mouse position in screen coordinates.
            /// @param leftHeld Whether the left button is held.
            void mouseMove(core::Point position, bool leftHeld);

            /// @brief Presses a mouse button on the element under the mouse and gives it the keyboard.
            /// @param position Mouse position in screen coordinates.
            /// @param button Button pressed.
            /// @param clicks Click count of the press (SDL counts on; folded to 1/2 like WM_LBUTTONDBLCLK).
            /// @param ctrl Whether Ctrl is held (handed on with the events raised).
            void mouseDown(core::Point position, MouseButton button, int clicks, bool ctrl = false);

            /// @brief Releases a mouse button: finishes drags and tells the element under the mouse.
            /// @param position Mouse position in screen coordinates.
            /// @param button Button released.
            /// @param ctrl Whether Ctrl is held (handed on with the events raised).
            void mouseUp(core::Point position, MouseButton button, bool ctrl = false);

            /// @brief Makes the next button release go nowhere.
            /// @details Original: EventManage.FClickConsumed, the press started something (an NPC's event);
            /// TForm1.DXDraw1MouseUp skips DispatchMouseUp once (DZ_Game.pas:2071-2074).
            void consumeNextRelease() { m_releaseConsumed = true; }

            /// @brief Sends a wheel turn to the element with the keyboard.
            /// @param steps Wheel steps (positive: up).
            void mouseWheel(int steps);

            /// @brief Handles a key press: the key preview first, then the element with the keyboard.
            /// @param key The key event.
            void keyDown(const KeyEvent& key);

            /// @brief Sets a hook that sees every key press before the element with the keyboard.
            /// @details Returning true keeps the key from the element (the shortcuts of a scene: Ctrl+I, Escape).
            /// @param preview The hook.
            void setKeyPreview(std::function<bool(const KeyEvent&)> preview) { m_keyPreview = std::move(preview); }

            /// @brief Handles a key release.
            /// @param key The key event.
            void keyUp(const KeyEvent& key);

            /// @brief Handles typed text: the text fallback first, then the element with the keyboard.
            /// @param text The typed text.
            void textInput(const std::string& text);

            /// @brief Sets a hook called before a typed character goes on when the element with the keyboard is no text box or memo.
            /// @details Original: TForm1.HandleCharMsg. The hook may give the keyboard to a text field (the chat line).
            /// @param fallback The hook.
            void setTextFallback(std::function<void()> fallback) { m_textFallback = std::move(fallback); }

            /// @brief Gives the topmost shown window that closes on Escape its Escape event.
            /// @details Original: CloseTopForm.
            /// @return False when there is no such window.
            bool closeTopOnEscape();

            /// @brief Tells whether a key is down now, whatever has the keyboard (for walking with the arrows).
            /// @param key The key.
            /// @return True when the key is held.
            bool isKeyHeld(Key key) const { return m_heldKeys.count(key) != 0; }

            /// @brief Tells whether the keyboard goes to a text field.
            /// @return True when it does.
            bool isTyping();

            /// @brief Tells whether an element of that name is there and shown (it and the elements holding it are visible); test scripts ask it.
            /// @param name Element name.
            /// @return True when shown.
            bool isShown(const std::string& name);

            /// @brief Tells whether a press at the point lands on the element of that name (the topmost one there); test scripts ask it.
            /// @param name Element name.
            /// @param position Point in screen coordinates.
            /// @return True when that element is the topmost one there.
            bool isTopmostAt(const std::string& name, core::Point position);

            /// @brief Sets the window in the screen's coordinates (the game's 800 x 600 screen in its middle); anchored elements go to its edges.
            /// @param view The window rectangle.
            void setView(const core::Rect& view);

            /// @brief Gives the window rectangle.
            /// @return The view.
            const core::Rect& view() const { return m_view; }

            /// @brief Sets how big the HUD bars' layouts (HudScale="true") are drawn, around their anchor point; the mouse is mapped back for them.
            /// @param scale Scale factor (1: as laid out).
            void setHudScale(float scale) { m_hudScale = scale; }

            /// @brief Gives the HUD scale.
            /// @return The scale factor.
            float hudScale() const { return m_hudScale; }

            /// @brief Restricts drawing and mouse hits to the layouts the filter keeps (none set: all of them).
            /// @details The qol client's party members show their windows over the leader's screen this way.
            /// @param filter Predicate on a layout root.
            void setLayoutFilter(std::function<bool(const Element& layout)> filter) { m_layoutFilter = std::move(filter); }

            /// @brief Tells whether an element of the kept layouts catches the mouse at a point of the screen.
            /// @param position Point in screen coordinates.
            /// @return True when one does.
            bool catchesAt(core::Point position) { return hitAt(position) != 0; }

            /// @brief Lists the windows (ui::Window) shown in the kept layouts.
            /// @return Their rectangles in the screen's coordinates.
            std::vector<core::Rect> shownWindows() const;

            /// @brief Runs one frame of updates and refreshes the hover.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Draws all kept layouts, the drag picture or the hovered element's hint.
            /// @param canvas Target canvas.
            /// @param nowMs Current time in milliseconds.
            void draw(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Tells what the mouse cursor should look like now.
            /// @return The cursor kind.
            CursorKind cursor();

            /// @brief Gives the element under the mouse.
            /// @return Its id (0: none).
            int hoveredId() const { return m_hoveredId; }

            /// @brief Gives where the mouse is.
            /// @return The mouse position.
            core::Point mouse() const { return m_mouse; }

            /// @brief Gives where the mouse is in the coordinates of the layout that holds an element (HUD bars are drawn at the HUD size).
            /// @param element An element of the layout.
            /// @return The mouse position in that layout's coordinates, or on the screen when the element is not shown.
            core::Point mouseIn(const Element& element);

        private:
            /// @brief Draws the hovered element's hint box.
            /// @param canvas Target canvas.
            void drawHint(gfx::Canvas& canvas);

            /// @brief Ends a drag: the source element learns what is under the mouse.
            /// @param position Mouse position in screen coordinates.
            void dropDragged(core::Point position);

            using LayoutRef = std::optional<std::reference_wrapper<const Element>>;

            /// @brief An element found by id, with where it is and which layout holds it.
            struct Located {
                std::reference_wrapper<Element> element;
                core::Point screenPosition;
                LayoutRef layout;
            };

            /// @brief Gives the HUD size a layout is drawn at.
            /// @param layout The layout (or none).
            /// @return The scale (1 for those that are not HUD bars).
            float scaleOf(LayoutRef layout) const;

            /// @brief Maps a point of the screen into a layout's own coordinates.
            /// @param point Point in screen coordinates.
            /// @param layout The layout.
            /// @return The point in the layout's coordinates.
            core::Point toLayout(core::Point point, LayoutRef layout) const;

            /// @brief Maps a point of a layout's own coordinates back to the screen.
            /// @param point Point in the layout's coordinates.
            /// @param layout The layout.
            /// @return The point in screen coordinates.
            core::Point fromLayout(core::Point point, LayoutRef layout) const;

            /// @brief Gives the mouse relative to a located element.
            /// @param located The element.
            /// @param mouse Mouse position in screen coordinates.
            /// @return The mouse in the element's coordinates.
            core::Point localMouse(const Located& located, core::Point mouse) const { return toLayout(mouse, located.layout) - located.screenPosition; }

            /// @brief Finds an element by name; throws LayoutError when there is none.
            /// @param name Element name.
            /// @return The element.
            Element& findByName(const std::string& name);

            /// @brief Finds the topmost shown window below a parent that closes on Escape.
            /// @param parent Element to search below.
            /// @return The window, or nothing.
            std::optional<std::reference_wrapper<Window>> topEscapeWindow(Element& parent);

            /// @brief Finds a visible element by id.
            /// @param id Element id (0: none).
            /// @return Where it is, or nothing.
            std::optional<Located> locate(int id);

            /// @brief Finds the topmost element that catches the mouse at a point.
            /// @param position Point in screen coordinates.
            /// @return The element id, 0 for none.
            int hitAt(core::Point position);

            /// @brief Recomputes the hovered element from the mouse position.
            void refreshHover();

            /// @brief Syncs every list box with its scroll bar, below an element.
            /// @param element Element to start at.
            void syncScrollBars(Element& element);

            std::unique_ptr<Element> m_root;
            // the game's 800 x 600 screen
            core::Rect m_view{0, 0, 800, 600};
            float m_hudScale = 1.0f;
            gfx::PictureLibrary& m_pictures;
            UiContext m_context;
            core::Point m_mouse;
            std::set<Key> m_heldKeys;
            std::function<bool(const KeyEvent&)> m_keyPreview;
            std::function<void()> m_textFallback;
            int m_hoveredId = 0;
            int m_capturedId = 0;
            bool m_pressWasDouble = false;
            bool m_releaseConsumed = false;
            int m_nextId = 1;
            std::function<bool(const Element& layout)> m_layoutFilter;

            /// @brief Tells whether the layout filter keeps a layout.
            /// @param layout The layout root.
            /// @return True when it is drawn and catches the mouse.
            bool kept(const Element& layout) const { return !m_layoutFilter || m_layoutFilter(layout); }
    };

}

#endif
