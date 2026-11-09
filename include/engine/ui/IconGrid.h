#ifndef NORTH_ISLAND_UI_ICON_GRID_H
#define NORTH_ISLAND_UI_ICON_GRID_H

#include "core/Color.h"
#include "ui/Element.h"
#include "ui/IconCell.h"

#include <map>
#include <optional>
#include <string>

namespace ui {

    /// @brief Cells of icons in columns and rows (item bag, equipment slots) filled by the view model; supports hover, click, double click, right click and drag and drop.
    /// @details Original: drag and drop follows TSe_ItemImage.HandleMouseDown; frames and cells draw like TRe_ItemGridForm.DrawControl and TSe_ItemImage.DrawControl.
    class IconGrid : public Element {
        public:
            /// @brief Creates a grid and declares its events (DoubleClick, Drop, Click, RightClick, Press, MouseWheel).
            IconGrid();

            /// @brief Reads the IconGrid attributes (Columns, Rows, Pitch, PitchY, CellSize, FirstIndex, HoverColor, CountColor, Draggable, CellPicture, CellSource, OutlineColor, OutlineGapColor).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Takes the size of the cells when the layout gave none.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Fills a cell.
            /// @param index The cell number.
            /// @param cell What the cell shows.
            void setCell(int index, const IconCell& cell) { m_cells[index] = cell; }

            /// @brief Empties one cell.
            /// @param index The cell number.
            void clearCell(int index) { m_cells.erase(index); }

            /// @brief Empties every cell.
            void clearCells() { m_cells.clear(); }

            /// @brief Changes the number of columns and rows; the grid takes their size.
            /// @param columns The new column count.
            /// @param rows The new row count.
            void setColumns(int columns, int rows);

            /// @brief The distance from one cell's origin to the next.
            /// @return The Pitch (x) and PitchY (y) attributes, px.
            core::Point pitch() const { return {m_pitch, m_pitchY}; }

            /// @brief The size of one cell.
            /// @return The CellSize attribute as width and height, px.
            core::Size cellSize() const { return {m_cellSize, m_cellSize}; }

            /// @brief The number of the cell whose box holds a point of the grid.
            /// @param local The point relative to the grid.
            /// @return The cell number (0: none).
            int cellAt(core::Point local) const;

            /// @brief The top left of a cell in the grid.
            /// @param index The cell number.
            /// @return The cell's top left relative to the grid.
            core::Point cellPosition(int index) const;

            /// @brief Whether a point is inside the grid's bounds.
            /// @param local The point relative to the grid.
            /// @param pictures The picture library (unused).
            /// @return True when inside.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief The hint of the cell under a point.
            /// @param local The point relative to the grid.
            /// @return The cell's hint ("" when none).
            const std::string& hintAt(core::Point local) const override;

            /// @brief The hint box origin: the top left of the cell under the point.
            /// @param local The point relative to the grid.
            /// @return The cell's top left.
            core::Point hintOrigin(core::Point local) const override;

            /// @brief Raises DoubleClick with the cell number and suppresses the drag pick-up of its press.
            /// @param context The UI context.
            /// @param local The point relative to the grid.
            void onDoubleClick(UiContext& context, core::Point local) override;

            /// @brief Raises Press on a left press and starts dragging the icon when Draggable.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the grid.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises Click (left, same cell as the press) or RightClick (right) for the cell under the mouse.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the grid.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief The wheel over the grid: raises "MouseWheel" with the steps.
            /// @param context The UI context.
            /// @param steps The wheel steps.
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Raises Drop for an icon dragged from this grid.
            /// @param context The UI context.
            /// @param sourceIndex The cell the icon came from.
            /// @param target The element it was let go over.
            /// @param targetIndex The target's cell (-1: none).
            /// @param screenPoint Where the mouse was, in screen coordinates.
            /// @param grab Where the mouse held the icon, from its top left.
            void onDropped(
                UiContext& context,
                int sourceIndex,
                std::optional<std::reference_wrapper<const Element>> target,
                int targetIndex,
                core::Point screenPoint,
                core::Point grab
            ) override;

            /// @brief The cell a dragged icon would go into: an item's own cells first, else the grid cell there.
            /// @param local The point relative to the grid.
            /// @return The cell number (-1: none).
            int dropIndexAt(core::Point local) const override;

        protected:
            /// @brief Draws the outline round the cells, the cell picture under every cell, the cell frames, then each cell's hover fill, icon, count and overlays.
            /// @param context The draw context.
            /// @param screenPosition The grid's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            int m_columns = 1;
            int m_rows = 1;
            int m_pitch = 34; // horizontal distance between cell origins, px
            int m_pitchY = 34; // vertical distance between cell origins, px
            int m_cellSize = 32; // icon width and height, px
            int m_firstIndex = 1;
            bool m_draggable = false;
            std::string m_cellPictureName; // drawn under every cell, empty ones too; none when empty
            gfx::PictureId m_cellPicture;
            std::optional<core::Rect> m_cellSource; // the part of the cell picture drawn; the whole picture when none
            std::optional<core::Color> m_outlineColor; // a 1 px line round all the cells; none when unset
            std::optional<core::Color> m_outlineGapColor; // when set, the bag's frame: a gap of it and a second line on the top and left, a gap row below
            bool m_doubleClicked = false;
            int m_pressedCell = 0;
            core::Color m_hoverColor{255, 108, 1, 80}; // orange hover highlight, alpha 80
            core::Color m_countColor{8, 8, 8, 255}; // item count text: near black
            std::map<int, IconCell> m_cells;
    };

}

#endif
