#ifndef NORTH_ISLAND_UI_ELEMENT_H
#define NORTH_ISLAND_UI_ELEMENT_H

#include "core/Point.h"
#include "core/Rect.h"
#include "ui/CursorKind.h"
#include "ui/DrawContext.h"
#include "ui/KeyEvent.h"
#include "ui/LoadContext.h"
#include "ui/MouseButton.h"
#include "ui/UiContext.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ui {

    /// @brief Base of every element of a layout: name, bounds relative to the parent, children and named events with layout-assigned handlers.
    class Element {
        public:
            /// @brief Which edge or corner of a window bigger than 800 x 600 an element keeps its distance from.
            enum class Anchor { None, TopLeft, Top, TopRight, Left, Center, Right, BottomLeft, Bottom, BottomRight, Fill };

            /// @brief Creates an element with default attributes.
            Element();
            virtual ~Element() = default;
            Element(const Element&) = delete;
            Element& operator=(const Element&) = delete;

            /// @brief The element's Name attribute.
            /// @return The name.
            const std::string& name() const { return m_name; }

            /// @brief The element's numeric id (used to find it in the tree).
            /// @return The id.
            int id() const { return m_id; }

            /// @brief Sets the element's numeric id.
            /// @param id The new id.
            void setId(int id) { m_id = id; }

            /// @brief The bounds relative to the parent.
            /// @return The bounds rectangle.
            const core::Rect& bounds() const { return m_bounds; }

            /// @brief Sets the bounds relative to the parent.
            /// @param bounds The new bounds.
            void setBounds(const core::Rect& bounds) { m_bounds = bounds; }

            /// @brief Moves the element, keeping its size.
            /// @param position The new top left relative to the parent.
            void setPosition(core::Point position) {
                m_bounds.x = position.x;
                m_bounds.y = position.y;
            }

            /// @brief Whether the element (and so its children) is drawn.
            /// @return True when visible.
            bool isVisible() const { return m_visible; }

            /// @brief Shows or hides the element.
            /// @param visible True to show it.
            virtual void setVisible(bool visible) { m_visible = visible; }

            /// @brief Whether the element reacts to input.
            /// @return True when enabled.
            bool isEnabled() const { return m_enabled; }

            /// @brief The mouse cursor shown over the element.
            /// @return The cursor kind.
            CursorKind cursor() const { return m_cursor; }

            /// @brief Sets the mouse cursor shown over the element.
            /// @param cursor The cursor kind.
            void setCursor(CursorKind cursor) { m_cursor = cursor; }

            /// @brief The sound file played on mouse down ("" for none).
            /// @return The sound file name.
            const std::string& clickSound() const { return m_clickSound; }

            /// @brief The sound file played when the left button comes up over the element.
            /// @details Original: TSe_Component.FSoundOnUp.
            /// @return The sound file name ("" for none).
            const std::string& releaseSound() const { return m_releaseSound; }

            /// @brief Whether a press on the element or anything in it brings its layout on top (game forms).
            /// @return True when it raises on press.
            virtual bool raisesOnPress() const { return m_raisesOnPress; }

            /// @brief Whether the element with that id is this one or lies in one that raises on press.
            /// @param id The element id to look for.
            /// @return True when the id lies in a raising element.
            bool raisesOnPressAt(int id) const;

            /// @brief Whether that press brings the layout over the popups too (a form whose click the game's popups give way to).
            /// @return True when it raises above popups.
            virtual bool raisesAbovePopups() const { return false; }

            /// @brief Whether the children are drawn and take the mouse only inside the element (a scrolled panel's).
            /// @return True when children are clipped.
            virtual bool clipsChildren() const { return false; }

            /// @brief Whether the element with that id is this one or lies in one that raises above the popups.
            /// @param id The element id to look for.
            /// @return True when the id lies in an element that raises above popups.
            bool raisesAbovePopupsAt(int id) const;

            /// @brief The hint shown above the element while the mouse is on it.
            /// @return The hint text ("" for none).
            const std::string& hint() const { return m_hint; }

            /// @brief The hint for the mouse at a point of the element (by default the hint everywhere).
            /// @param local The mouse position relative to the element.
            /// @return The hint text.
            virtual const std::string& hintAt(core::Point local) const;

            /// @brief What the hint box stands over (by default the element's top left).
            /// @param local The mouse position relative to the element.
            /// @return The hint origin relative to the element.
            virtual core::Point hintOrigin(core::Point local) const;

            /// @brief Sets the element's hint text.
            /// @param hint The hint ("" for none).
            void setHint(const std::string& hint) { m_hint = hint; }

            /// @brief Appends a child, drawn after the element and over earlier children.
            /// @param child The child to take over.
            void addChild(std::unique_ptr<Element> child) { m_children.push_back(std::move(child)); }

            /// @brief The children in draw order.
            /// @return The child list.
            const std::vector<std::unique_ptr<Element>>& children() const { return m_children; }

            /// @brief Whether the layout gave the event a handler.
            /// @param event The event name.
            /// @return True when a handler is set.
            bool hasHandler(const std::string& event) const { return m_handlers.count(event) != 0; }

            /// @brief Whether the element or one below it has that id.
            /// @param id The element id.
            /// @return True when found.
            bool contains(int id) const;

            /// @brief Draws the child holding that element last of its siblings, below the topmost ones.
            /// @param id The element id to raise.
            void raiseChildHolding(int id);

            /// @brief The same over the topmost ones as well (a form clicked over the popup that was shown first).
            /// @param id The element id to raise.
            void raiseChildHoldingAbovePopups(int id);

            /// @brief Whether the element stays above siblings that are raised (popups over windows).
            /// @return True when topmost.
            bool isTopmost() const { return m_topmost; }

            /// @brief The element's anchor.
            /// @return The anchor.
            Anchor anchor() const { return m_anchor; }

            /// @brief HudScale="true": a HUD bar's layout, drawn at the screen's HUD size around its anchor point.
            /// @return True when HUD scaled.
            bool isHudScaled() const { return m_hudScaled; }

            /// @brief The point of its bounds its anchor holds to (the top right corner for TopRight, the middle for None and Center).
            /// @return The anchor point in parent coordinates.
            core::Point anchorPoint() const;

            /// @brief Places the element and those in it for a window that is `view` in the screen's coordinates (the 800 x 600 screen in its middle).
            /// @param view The window rectangle in screen coordinates.
            /// @param parentShift How far its parent has been moved from where the layout put it.
            void applyView(const core::Rect& view, core::Point parentShift);

            /// @brief How far its anchor moved it in a bigger window: code that places an anchored element adds it.
            /// @return The shift.
            core::Point viewShift() const { return m_viewShift; }

            /// @brief Reads one attribute of the layout; throws LayoutError for unknown ones.
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            virtual void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context);

            /// @brief Called once all attributes and children are read.
            /// @param context The load context.
            virtual void finishLoading(const LoadContext& context);

            /// @brief Updates the visible children each frame.
            /// @param context The UI context.
            virtual void update(UiContext& context);

            /// @brief Draws this element and then its visible children.
            /// @param context The draw context.
            /// @param origin The parent's screen position.
            void drawTree(DrawContext& context, core::Point origin);

            /// @brief The second pass: what must lie above every element (open drop-down lists).
            /// @param context The draw context.
            /// @param origin The parent's screen position.
            void drawOverlayTree(DrawContext& context, core::Point origin);

            /// @brief Whether a point belongs to the element itself; plain containers let the mouse through.
            /// @param local The point relative to the element.
            /// @param pictures The picture library, for pixel-exact tests.
            /// @return True when the element takes the mouse there.
            virtual bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const;

            /// @brief The same for what the element draws in the overlay pass (tested before everything else).
            /// @param local The point relative to the element.
            /// @return True when the overlay takes the mouse there.
            virtual bool hitTestOverlay(core::Point local) const;

            /// @brief A mouse button went down on the element.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the element.
            virtual void onMouseDown(UiContext& context, MouseButton button, core::Point local);

            /// @brief A mouse button came up over the element.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the element.
            virtual void onMouseUp(UiContext& context, MouseButton button, core::Point local);

            /// @brief Called while the left button that went down on this element is held.
            /// @param context The UI context.
            /// @param local The mouse relative to the element.
            virtual void onMouseDrag(UiContext& context, core::Point local);

            /// @brief The left button went down on this element and came up elsewhere.
            /// @param context The UI context.
            virtual void onCaptureReleased(UiContext& context);

            /// @brief A double click on the element.
            /// @param context The UI context.
            /// @param local The point relative to the element.
            virtual void onDoubleClick(UiContext& context, core::Point local);

            /// @brief The icon dragged from this element's cell was let go.
            /// @param context The UI context.
            /// @param sourceIndex The cell the icon was dragged from.
            /// @param target The element it was let go over (nothing: over no element).
            /// @param targetIndex The cell of the target it went into (-1: none).
            /// @param screenPoint Where the mouse was, in screen coordinates.
            /// @param grab Where the mouse held the icon, from its top left.
            virtual void onDropped(
                UiContext& context,
                int sourceIndex,
                std::optional<std::reference_wrapper<const Element>> target,
                int targetIndex,
                core::Point screenPoint,
                core::Point grab
            );

            /// @brief The cell a dragged icon would go into at a point of this element.
            /// @param local The point relative to the element.
            /// @return The cell number (-1: none).
            virtual int dropIndexAt(core::Point local) const;

            /// @brief The mouse wheel turned over the element.
            /// @param context The UI context.
            /// @param steps The wheel steps.
            virtual void onMouseWheel(UiContext& context, int steps);

            /// @brief A key went down.
            /// @param context The UI context.
            /// @param key The key event.
            virtual void onKeyDown(UiContext& context, const KeyEvent& key);

            /// @brief A key came up.
            /// @param context The UI context.
            /// @param key The key event.
            virtual void onKeyUp(UiContext& context, const KeyEvent& key);

            /// @brief Typed text arrived.
            /// @param context The UI context.
            /// @param text The typed text.
            virtual void onTextInput(UiContext& context, const std::string& text);

            /// @brief Whether the element takes typed text (text fields).
            /// @return True for text fields.
            virtual bool takesText() const { return false; }

        protected:
            /// @brief Draws the element itself (children are drawn by drawTree).
            /// @param context The draw context.
            /// @param screenPosition The element's top left on the screen.
            virtual void draw(DrawContext& context, core::Point screenPosition);

            /// @brief Draws what must lie above every element.
            /// @param context The draw context.
            /// @param screenPosition The element's top left on the screen.
            virtual void drawOverlay(DrawContext& context, core::Point screenPosition);

            /// @brief Declares an event the layout may give a handler for (an attribute of that name).
            /// @param event The event name.
            void declareEvent(const std::string& event) { m_events.insert(event); }

            /// @brief Raises an event to its handler, if the layout gave one.
            /// @param context The UI context.
            /// @param event The event name.
            /// @param index The list item or cell index (-1: none).
            /// @param position The point relative to the element.
            void raise(UiContext& context, const std::string& event, int index = -1, core::Point position = {}) const;

            /// @brief The same with all the arguments (the source is filled in).
            /// @param context The UI context.
            /// @param event The event name.
            /// @param args The event arguments.
            void raise(UiContext& context, const std::string& event, EventArgs args) const;

            /// @brief Sets the sound played on mouse down.
            /// @param sound The sound file name.
            void setClickSound(const std::string& sound) { m_clickSound = sound; }

            /// @brief Whether the layout gave the element a size (Bounds or Size).
            /// @return True when a size was given.
            bool sizeWasGiven() const { return m_sizeGiven; }

        private:
            std::string m_name;
            int m_id = 0;
            core::Rect m_bounds;
            bool m_sizeGiven = false;
            bool m_visible = true;
            bool m_enabled = true;
            bool m_topmost = false;
            bool m_hudScaled = false;
            Anchor m_anchor = Anchor::None;
            core::Point m_viewShift;
            core::Point m_viewGrowth;
            bool m_raisesOnPress = false;
            CursorKind m_cursor = CursorKind::Normal;
            std::string m_clickSound;
            std::string m_releaseSound;
            std::string m_hint;
            std::set<std::string> m_events;
            std::map<std::string, std::string> m_handlers;
            std::vector<std::unique_ptr<Element>> m_children;
    };

}

#endif
