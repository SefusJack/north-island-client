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
#include <unordered_map>
#include <vector>

namespace xml {
    class Node;
}

namespace ui {

    class ListBox;
    class TextBox;

    /// @brief A pointing hand drawn at the element a game pad highlights.
    struct PadPointer {
        const gfx::Texture* picture = nullptr; // none: a frame is drawn around the element instead
        core::Point tip; // the fingertip in the picture
        bool below = false; // the hand stands below the element and points up; otherwise above it, pointing down
    };

    /// @brief A place in the topmost window a game pad's pointer steps to: a control, a row of a list or a cell of a grid.
    struct PadSpot {
        core::Point point; // where the pointer stands, in screen coordinates
        int elementId = 0;
        bool cell = false; // a row or a cell
        bool scrollsUp = false; // going up from here first scrolls the list
        bool scrollsDown = false; // going down from here first scrolls the list
        int scrollBarId = 0; // the scroll bar that scrolls it; 0 when the mouse wheel over the place does
        bool slider = false; // left and right move along it (within reach) and click there, instead of leaving it
        int reach = 0; // how far the place goes on to the left and right of its point, px: what lies above or below any part of it is straight on
        bool loose = false; // one of the places a view model names, which need not stand in rows and columns: the nearest that way is the next
    };

    /// @brief What a game pad finds under its pointer in a grid of things.
    struct PadGridCell {
        bool filled = false; // the cell shows a thing
        bool draggable = false; // the grid's things can be picked up
        bool doubleClick = false; // a double click uses a thing
    };

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

            /// @brief Holds or lets go a key for isKeyHeld alone: no element and no key preview hears of it (a game pad's stick walking as the arrows do).
            /// @param key The key.
            /// @param held True to hold it.
            void holdKey(Key key, bool held) {
                if (held) {
                    m_heldKeys.insert(key);
                } else {
                    m_heldKeys.erase(key);
                }
            }

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

            /// @brief Gives where a shown element lies on the screen, at the size it is drawn (HUD bars at the HUD size), without its layout's slide.
            /// @param name Element name.
            /// @return The rectangle in screen coordinates, nothing when there is no such element or it is not shown.
            std::optional<core::Rect> screenRectOf(const std::string& name);

            /// @brief Slides a layout off its place (Element::setSlide), by the layout's name.
            /// @param layoutName The layout root's name.
            /// @param slide The shift in screen pixels.
            /// @param away True when it is off the screen altogether.
            void slideLayout(const std::string& layoutName, core::Point slide, bool away);

            /// @brief Gives how far a layout is slid off its place now (Element::slide), by the layout's name.
            /// @param layoutName The layout's root element name.
            /// @return The shift in screen pixels; none for a layout this screen does not have.
            core::Point layoutSlide(const std::string& layoutName) const;

            /// @brief Whether a layout has slid off the screen altogether (Element::isSlidAway), by the layout's name.
            /// @param layoutName The layout's root element name.
            /// @return True when it is away; false for a layout this screen does not have.
            bool layoutAway(const std::string& layoutName) const;

            /// @brief Highlights an element for a game pad: it shows its mouse-over look and its hint, with a hand pointing at it, wherever the mouse is.
            /// @param name Element name; empty takes the highlight away.
            /// @param pointer The hand to draw; it must stay alive while the highlight is shown. Without a picture a frame is drawn around the element.
            void setPadFocus(const std::string& name, const PadPointer& pointer = {});

            /// @brief Presses the highlighted element as a click would (its click sound, then its Click).
            /// @return False when nothing is highlighted or it is no button.
            bool pressPadFocus();

            /// @brief Gives the topmost shown window, the one a game pad's pointer steps through.
            /// @return The window's element id, 0 without a window.
            int padWindowId();

            /// @brief Lists the places of the topmost window a game pad's pointer steps to: its buttons, check boxes and text
            /// fields, the shown rows of its lists and the cells of its grids, each where the mouse would reach it.
            /// @return The places; empty without a window.
            std::vector<PadSpot> padSpots();

            /// @brief Gives where a game pad's pointer starts in the topmost window: the window's PadStart element, else its first row or cell, else its first control.
            /// @return The point in screen coordinates; nothing without a window or a place in it.
            std::optional<core::Point> padStartPoint();

            /// @brief Tells what grid of things lies under a point, for a game pad that picks things up and uses them.
            /// @param point The point in screen coordinates.
            /// @return The cell there; nothing when the mouse would not reach a grid at the point.
            std::optional<PadGridCell> padGridAt(core::Point point);

            /// @brief Backs out of the topmost window for a game pad when the Escape key would not: presses its own cancel, close or quit button.
            /// @return True when the window was dealt with here; false when Escape is to be tried (it closes the window, or there is none).
            bool pressPadCancel();

            /// @brief Names the grids whose drags work the modern way: while one of their things is dragged a right click puts one
            /// down and the drag goes on (a Drop with EventArgs::partial).
            /// @param gridNames The grids' element names; empty for none.
            void setModernDrag(std::vector<std::string> gridNames) { m_modernDragGrids = std::move(gridNames); }

            /// @brief Starts a drag of a grid's cell that follows the mouse with no button held, until a left click puts it down.
            /// @param gridName The grid's element name.
            /// @param index The cell.
            /// @param count How many of the cell's things it carries.
            void startStickyDrag(const std::string& gridName, int index, int count);

            /// @brief Gives the text field the left button was last pressed on since this was last asked, once.
            /// @return The field's element id; none when no field was pressed.
            std::optional<int> takeTextClick() {
                const int id = m_textClickId;
                m_textClickId = 0;
                return id != 0 ? std::optional<int>(id) : std::nullopt;
            }

            /// @brief Gives the element that has the keyboard.
            /// @return Its id; 0 for none.
            int focusedId() const { return m_context.focusedId; }

            /// @brief Gives the keyboard to an element by its id.
            /// @param id The element's id.
            void focusId(int id) { m_context.focusedId = id; }

            /// @brief Finds a text field that is shown, by its id.
            /// @param id The element's id.
            /// @return The field; null when there is none of that id or it is not shown.
            TextBox* textBoxOf(int id);

            /// @brief Whether a thing is dragged now.
            /// @return True during a drag.
            bool isDragging() const { return m_context.drag.has_value(); }

            /// @brief Ends the drag without putting the thing anywhere.
            void cancelDrag() { m_context.drag.reset(); }

            /// @brief Scrolls by a scroll bar for a game pad, as the mouse wheel over the bar does.
            /// @param scrollBarId The scroll bar's element id (a PadSpot's scrollBarId).
            /// @param up True for a line up, false for a line down.
            void padScroll(int scrollBarId, bool up);

            /// @brief Presses what a game pad's shoulder button stands for in the topmost window: the tab before or after the open one, or the window's two PadShoulders buttons.
            /// @param next True for the right shoulder.
            /// @return True when the window gives its shoulder buttons a use.
            bool pressPadShoulder(bool next);

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
            /// @param layout If given, gets the layout holding the element (nullptr for none).
            /// @return The element id, 0 for none.
            int hitAt(core::Point position, const Element** layout = nullptr);

            /// @brief Recomputes the hovered element from the mouse position.
            void refreshHover();

            /// @brief Syncs every list box with its scroll bar.
            void syncScrollBars();

            /// @brief Adds the list boxes below an element, in tree order, to m_listBoxes.
            /// @param element Element to start at.
            void collectListBoxes(Element& element);

            std::unique_ptr<Element> m_root;
            // findByName's answers, until any element tree changes (Element::treeChanges); elements are never removed
            std::unordered_map<std::string, Element*> m_named;
            std::uint64_t m_namedAt = 0;
            // every list box in tree order, until any element tree changes
            std::vector<ListBox*> m_listBoxes;
            std::uint64_t m_listBoxesAt = UINT64_MAX;
            // the game's 800 x 600 screen
            core::Rect m_view{0, 0, 800, 600};
            float m_hudScale = 1.0f;
            /// @brief Starts decoding the pictures a layout names on workers, before the layout is built.
            /// @param node A layout node; its children are visited too.
            /// @param started Gets the names of the pictures started.
            void prefetchPictures(const xml::Node& node, std::vector<std::string>& started);

            gfx::PictureLibrary& m_pictures;
            UiContext m_context;
            core::Point m_mouse;
            std::set<Key> m_heldKeys;
            std::function<bool(const KeyEvent&)> m_keyPreview;
            std::function<void()> m_textFallback;
            int m_hoveredId = 0;
            /// @brief Finds the topmost shown window.
            /// @return The window, or nothing.
            std::optional<Located> padWindow();

            /// @brief Presses a button as a click would (its click sound, then its Click).
            /// @param element The element.
            /// @return False when it is no button or not enabled.
            bool pressButton(Element& element);

            /// @brief Gives where the game pad's pointing hand is drawn.
            /// @return The hand's place on the screen; nothing without a highlighted element or without a hand.
            std::optional<core::Rect> padPointerPlace();

            /// @brief Brings the windows that opened since the last update to the front.
            void raiseOpenedWindows();

            int m_textClickId = 0; // the text field last pressed on (takeTextClick)
            std::vector<std::string> m_modernDragGrids; // the grids whose drags a right click puts one thing down of
            bool m_rightUpConsumed = false; // the right button's release belongs to a press that put a dragged thing down
            std::set<int> m_openWindows; // the windows shown at the last update: one that opens comes to the front
            bool m_openWindowsKnown = false; // m_openWindows was filled once
            int m_padFocusId = 0; // the element a game pad highlights (0: none); it counts as the one under the mouse
            PadPointer m_padPointer; // the hand drawn at it
            const Element* m_hoveredLayout = nullptr; // the layout holding m_hoveredId
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
