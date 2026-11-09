#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_MAKE_THING_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_MAKE_THING_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HomeFurniture.h"
#include "client/models/crafting/MakeJobs.h"
#include "client/rules/MakeMaterials.h"
#include "client/rendering/painters/ScaledItemIcon.h"
#include "client/viewmodels/crafting/MakeThingInfoWindow.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "gfx/PictureId.h"
#include "ui/DrawingArea.h"
#include "ui/ScrollBar.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Manufacture tree window listing the recipes of a furniture piece as foldable groups.
    /// @details Original: MakeThingWindow.xml, RE_Menu.pas TRe_MakeThingForm with se_UI.pas TRe_TreeList.
    class MakeThingWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<bool()> inFloorRoom;
                std::function<void(int, const std::string&, int, int, int)> askContinue;
            };

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services (layout, pictures, items, compounds).
            /// @param screen Screen the window is loaded into.
            /// @param furniture The player's furniture (for make bonuses).
            /// @param jobs Manufacture job slots.
            /// @param materials Material and blueprint lookup.
            /// @param info Recipe info window opened when a recipe is picked.
            /// @param messages Player message line.
            /// @param hooks Owner callbacks.
            MakeThingWindow(
                GameServices& services,
                ui::Screen& screen,
                const HomeFurniture& furniture,
                MakeJobs& jobs,
                const MakeMaterials& materials,
                MakeThingInfoWindow& info,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window and fills the tree.
            /// @details Original: ShowCtrl and LoadMakeThingTree.
            /// @param piece The piece's number.
            /// @param facilityIds Items of the pieces that make things (the piece's own first).
            void show(int piece, const std::vector<int>& facilityIds);

            /// @brief Closes the window.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief One recipe row.
            struct Recipe {
                int index = 0;
                std::string name;
                std::string minutes;
                ScaledItemIcon icon;
            };

            /// @brief A foldable group of recipes, one per furniture piece.
            struct Group {
                std::string name;
                bool expanded = false;
                std::vector<Recipe> recipes;
            };

            /// @brief A shown row: a group (recipe -1) or one of its recipes.
            struct Line {
                int group = 0;
                int recipe = -1;
            };

            /// @brief Builds the groups and recipes.
            /// @details Original: LoadMakeThingTree.
            /// @param facilityIds Items of the pieces that make things.
            void load(const std::vector<int>& facilityIds);

            /// @brief Formats a batch's minutes with the furniture's savings.
            /// @param recipe The recipe.
            /// @return The minutes text.
            std::string minutesText(const formats::CompoundRecipe& recipe) const;

            /// @brief Lists the rows the unfolded groups show.
            /// @return The rows in order.
            std::vector<Line> lines() const;

            /// @brief Counts the rows that fit in the tree.
            /// @return Visible row count.
            int visibleRows() const;

            /// @brief Scrolls the tree to a top row and updates the scroll bar.
            /// @details Original: GetTreeRowCount, TreeLineUp, TreeLineDown, the scroll bar.
            /// @param top Wanted top row (clamped).
            void scrollTo(int top);

            /// @brief Finds the row under the mouse as RenderWithChildren works it out.
            /// @return Row index within the view, or -1 for none.
            int hoveredRow() const;

            /// @brief Handles a click on the tree: folds a group or picks a recipe.
            /// @details Original: HandleClick.
            void clicked();

            /// @brief Picks a recipe: continues a waiting job or opens the manufacture window.
            /// @details Original: PickMakeThingRecipe.
            /// @param recipeIndex Recipe number in Compound2.dat.
            void pick(int recipeIndex);

            /// @brief Paints the tree.
            /// @param canvas Target canvas.
            /// @param at Top-left of the tree.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints a group row.
            /// @param canvas Target canvas.
            /// @param at Top-left of the row.
            /// @param group The group.
            void paintGroup(gfx::Canvas& canvas, core::Point at, const Group& group);

            /// @brief Paints a recipe row.
            /// @param canvas Target canvas.
            /// @param at Top-left of the row.
            /// @param recipe The recipe.
            void paintRecipe(gfx::Canvas& canvas, core::Point at, const Recipe& recipe);

            /// @brief Tiles a picture over a rectangle.
            /// @details Original: DrawTiledPic.
            /// @param canvas Target canvas.
            /// @param area Rectangle to cover.
            /// @param picture Tile picture.
            void paintTiled(gfx::Canvas& canvas, core::Rect area, gfx::PictureId picture);

            GameServices& m_services;
            ui::Screen& m_screen;
            const HomeFurniture& m_furniture;
            MakeJobs& m_jobs;
            const MakeMaterials& m_materials;
            MakeThingInfoWindow& m_info;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::DrawingArea& m_tree;
            ui::ScrollBar& m_scroll;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_indentSpot;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_groupSpot;
            gfx::PictureId m_fold;
            gfx::PictureId m_unfold;
            gfx::PictureId m_minuteIcon;
            gfx::PictureId m_hoverTile;
            int m_piece = 0;
            std::vector<Group> m_groups;
            int m_top = 0;
    };

}

#endif
