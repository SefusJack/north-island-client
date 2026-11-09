#ifndef NORTH_ISLAND_CLIENT_RULES_FORM_DRAG_H
#define NORTH_ISLAND_CLIENT_RULES_FORM_DRAG_H

#include "core/Point.h"
#include "core/Rect.h"
#include "core/Size.h"

namespace client {

    /// @brief How the HUD forms move when the left button holds them, for the forms a view model lays out itself (the small map, the shortcut bar).
    /// @details Original: se_UI.pas, se_mainUI.pas. The screen is the window in the form's coordinates (the game's 800 x 600 screen, or a bigger window round it), whose edges stand for the screen's.
    struct FormDrag {
        static constexpr core::Rect kGameScreen{0, 0, 800, 600}; // the game screen: x, y, width, height in px

        /// @brief Moves a form (not collapsed) where the mouse takes it, at the top within 10 pixels to 0, its bottom past 540 to 550.
        /// @details Original: TSe_Form.HandleMouseMove (se_UI.pas:2343-2365).
        /// @param area The form's area where the mouse takes it.
        /// @param screen The window in the form's coordinates.
        /// @return The area after the snapping.
        static core::Rect dragForm(core::Rect area, const core::Rect& screen = kGameScreen);

        /// @brief Pulls a form back onto the screen every frame, and while dragged (not collapsed) also to the edges it comes near.
        /// @details Original: TSe_Form.RenderWithChildren (se_UI.pas:2394-2423).
        /// @param area The form's area.
        /// @param dragging Whether the form is being dragged.
        /// @param screen The window in the form's coordinates.
        /// @return The fitted area.
        static core::Rect fitForm(core::Rect area, bool dragging, const core::Rect& screen = kGameScreen);

        /// @brief The shortcut bar's layout and area while it is dragged.
        /// @details Original: TSe_HotKeyForm: layout 1 upright (Form_HotKey_V, 56 x 274), 2 lying (Form_HotKey_H, 274 x 56), 3 minimized (Form_HotKey_Mini, 39 x 38; ToggleHotKeyMinimize).
        struct HotKeyMove {
            int layout = 1;
            int previousLayout = 0;
            core::Point grab;
            core::Rect area;
        };

        /// @brief Size of the shortcut bar in a layout.
        /// @param layout 1 upright, 2 lying, 3 minimized.
        /// @return The bar's size.
        static core::Size hotKeySize(int layout);

        /// @brief Makes the bar follow the mouse; near an edge it turns (a lying bar stands up, an upright one lies down) and the grab's x and y change places.
        /// @details Original: TSe_HotKeyForm.HandleMouseMove (se_mainUI.pas:15591-15666), then RenderWithChildren (se_mainUI.pas:18244-18265) via keepHotKeyBar.
        /// @param layout Current layout.
        /// @param previousLayout The layout before the last change (FState.PrevLayout).
        /// @param grab Where the mouse holds the bar.
        /// @param mouse Mouse position.
        /// @param screen The window in the form's coordinates.
        /// @return The new layout, previous layout, grab and area.
        static HotKeyMove dragHotKeyBar(
            int layout,
            int previousLayout,
            core::Point grab,
            core::Point mouse,
            const core::Rect& screen = kGameScreen
        );

        /// @brief Pulls the bar onto the screen and above the bottom 50 pixels.
        /// @details Original: RenderWithChildren while dragged, ToggleHotKeyMinimize.
        /// @param area The bar's area.
        /// @param screen The window in the form's coordinates.
        /// @return The kept area.
        static core::Rect keepHotKeyBar(core::Rect area, const core::Rect& screen = kGameScreen);
    };

}

#endif
