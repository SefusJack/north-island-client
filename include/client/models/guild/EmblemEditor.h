#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_EMBLEM_EDITOR_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_EMBLEM_EDITOR_H

#include "client/models/guild/EmblemGrid.h"
#include "core/Point.h"

#include <string>
#include <vector>

namespace client {

    /// @brief State of the badge editor: the grid being drawn, tool, colour, strokes in progress, undo steps and samples.
    /// @details Original: TSe_TeamForm's emblem fields; twenty steps to undo and the samples to pick from.
    class EmblemEditor {
        public:
            /// @brief The drawing tools.
            /// @details Original: FEmblemTool.
            enum class Tool { Pen = 0, Fill = 1, Line = 2, Rectangle = 3, Ellipse = 4 }; // FEmblemTool values

            static constexpr int kMostUndo = 20; // undo steps kept
            static constexpr int kMostSamples = 25; // sample emblems in the list
            static constexpr int kSamplesShown = 5; // samples visible at once

            /// @brief Creates the editor with an empty grid and the first undo step.
            /// @details Original: ResetEmblemEditor.
            EmblemEditor();

            /// @brief The grid being drawn.
            /// @return The grid.
            const EmblemGrid& grid() const { return m_grid; }

            /// @brief The tool selected.
            /// @return The tool.
            Tool tool() const { return m_tool; }

            /// @brief Selects a tool.
            /// @param tool The tool.
            void selectTool(Tool tool) { m_tool = tool; }

            /// @brief The palette index painted with.
            /// @return Colour index.
            int paintColor() const { return m_color; }

            /// @brief Selects the palette index to paint with.
            /// @param color Colour index.
            void selectColor(int color) { m_color = color; }

            /// @brief Whether a stroke is in progress.
            /// @return True while the button is held.
            bool dragging() const { return m_dragging; }

            /// @brief Start cell of the stroke.
            /// @return The cell, (-1, -1) for none.
            core::Point lineStart() const { return m_lineStart; }

            /// @brief Current end cell of the stroke.
            /// @return The cell, (-1, -1) for none.
            core::Point lineEnd() const { return m_lineEnd; }

            /// @brief The button went down on a cell of the grid (Fill fills, other tools start a stroke).
            /// @details Original: HandleMouseDown.
            /// @param cell Cell pressed.
            void press(core::Point cell);

            /// @brief The mouse moved with the button held inside the grid.
            /// @details Original: HandleMouseMove.
            /// @param cell Cell under the mouse.
            void drag(core::Point cell);

            /// @brief The mouse left the grid: the stroke is dropped.
            void cancelStroke();

            /// @brief The button came up: the shape is painted and an undo step is kept.
            /// @details Original: HandleMouseUp.
            void release();

            /// @brief Steps back one undo step.
            /// @details Original: UndoEmblemEdit.
            void undo();

            /// @brief Steps forward one undo step.
            /// @details Original: RedoEmblemEdit.
            void redo();

            /// @brief Empties the grid, leaving no step to undo.
            /// @details Original: ClearEmblemGrid.
            void clear() { m_grid = EmblemGrid(); }

            /// @brief The samples to pick from.
            /// @return The sample grids.
            const std::vector<EmblemGrid>& samples() const { return m_samples; }

            /// @brief The names of the samples.
            /// @return The sample names.
            const std::vector<std::string>& sampleNames() const { return m_names; }

            /// @brief The first sample shown.
            /// @details Original: FSampleScroll.
            /// @return Index of the first sample shown.
            int sampleScroll() const { return m_scroll; }

            /// @brief Empties the sample list.
            /// @details Original: ClearEmblemSamples.
            void clearSamples();

            /// @brief Adds a sample (at most 25).
            /// @details Original: AddEmblemSample.
            /// @param grid Sample grid.
            /// @param name Sample name.
            void addSample(const EmblemGrid& grid, const std::string& name);

            /// @brief Scrolls the samples one to the left.
            void scrollSamplesLeft();

            /// @brief Scrolls the samples one to the right.
            void scrollSamplesRight();

            /// @brief A click on the sample in one of the five places: it becomes the grid.
            /// @param place Place clicked (0..4).
            void pickSample(int place);

        private:
            /// @brief Keeps the grid as a new undo step.
            /// @details Original: PushEmblemUndo.
            void pushUndo();

            /// @brief Flood-fills the cell and the cells of its colour that touch it.
            /// @details Original: FloodFillEmblem.
            /// @param cell Cell to start from.
            void fill(core::Point cell);

            /// @brief Paints cells with the colour selected (cells outside the grid are skipped).
            /// @param cells Cells to paint.
            void paint(const std::vector<core::Point>& cells);

            EmblemGrid m_grid;
            std::vector<EmblemGrid> m_undo;
            int m_undoPosition = 0;
            Tool m_tool = Tool::Pen;
            int m_color = 0;
            bool m_dragging = false;
            core::Point m_lineStart{-1, -1};
            core::Point m_lineEnd{-1, -1};
            std::vector<EmblemGrid> m_samples;
            std::vector<std::string> m_names;
            int m_scroll = 0;
    };

}

#endif
