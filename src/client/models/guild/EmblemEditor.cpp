#include "client/models/guild/EmblemEditor.h"

#include "client/models/guild/EmblemCells.h"

#include <cstddef>

namespace client {

    namespace {

        /// @brief The cell of a grid at a point.
        /// @param grid The grid.
        /// @param at Cell position (must be inside).
        /// @return Reference to the cell's palette index.
        std::uint8_t& cellOf(EmblemGrid& grid, core::Point at) { return grid.cells[static_cast<std::size_t>(at.x)][static_cast<std::size_t>(at.y)]; }

        /// @brief Whether a point is a cell of the grid.
        /// @param at Point to test.
        /// @return True when inside.
        bool inside(core::Point at) { return at.x >= 0 && at.y >= 0 && at.x < EmblemGrid::kSide && at.y < EmblemGrid::kSide; }

    }

    EmblemEditor::EmblemEditor() { pushUndo(); }

    void EmblemEditor::paint(const std::vector<core::Point>& cells) {
        for (const core::Point& at : cells) {
            if (inside(at)) {
                cellOf(m_grid, at) = static_cast<std::uint8_t>(m_color);
            }
        }
    }

    void EmblemEditor::pushUndo() {
        if (static_cast<int>(m_undo.size()) + 1 > kMostUndo) {
            m_undo.erase(m_undo.begin());
        }
        m_undo.push_back(m_grid);
        m_undoPosition = static_cast<int>(m_undo.size()) - 1;
    }

    void EmblemEditor::fill(core::Point cell) {
        const std::uint8_t old = cellOf(m_grid, cell);
        if (old == m_color) {
            return;
        }
        std::vector<core::Point> open{cell};
        cellOf(m_grid, cell) = static_cast<std::uint8_t>(m_color);
        while (!open.empty()) {
            const core::Point at = open.back();
            open.pop_back();
            for (const core::Point next :
                 {core::Point{at.x + 1, at.y}, core::Point{at.x, at.y + 1}, core::Point{at.x - 1, at.y}, core::Point{at.x, at.y - 1}}) {
                if (inside(next) && cellOf(m_grid, next) == old) {
                    cellOf(m_grid, next) = static_cast<std::uint8_t>(m_color);
                    open.push_back(next);
                }
            }
        }
    }

    void EmblemEditor::press(core::Point cell) {
        if (m_tool == Tool::Fill) {
            fill(cell);
        } else {
            m_lineStart = cell;
            m_lineEnd = cell;
        }
        m_dragging = true;
    }

    void EmblemEditor::drag(core::Point cell) {
        if (!m_dragging) {
            return;
        }
        m_lineEnd = cell;
        if (m_tool == Tool::Pen) {
            paint(lineCells(m_lineStart, m_lineEnd));
            m_lineStart = m_lineEnd;
        }
    }

    void EmblemEditor::cancelStroke() {
        m_dragging = false;
        m_lineStart = {-1, -1};
        m_lineEnd = m_lineStart;
    }

    void EmblemEditor::release() {
        if (!m_dragging) {
            return;
        }
        const bool oneCell = m_lineStart.x == m_lineEnd.x && m_lineStart.y == m_lineEnd.y;
        if (m_tool == Tool::Line || m_tool == Tool::Rectangle || m_tool == Tool::Ellipse) {
            if (oneCell) {
                paint({m_lineStart});
            } else if (m_tool == Tool::Line) {
                paint(lineCells(m_lineStart, m_lineEnd));
            } else if (m_tool == Tool::Rectangle) {
                paint(rectangleCells(m_lineStart, m_lineEnd));
            } else {
                paint(ellipseCells(m_lineStart, m_lineEnd));
            }
        }
        m_undo.resize(static_cast<std::size_t>(m_undoPosition) + 1);
        pushUndo();
        cancelStroke();
    }

    void EmblemEditor::undo() {
        if (m_undoPosition - 1 >= 0) {
            --m_undoPosition;
            m_grid = m_undo[static_cast<std::size_t>(m_undoPosition)];
        }
    }

    void EmblemEditor::redo() {
        if (static_cast<int>(m_undo.size()) - 1 >= m_undoPosition + 1) {
            ++m_undoPosition;
            m_grid = m_undo[static_cast<std::size_t>(m_undoPosition)];
        }
    }

    void EmblemEditor::clearSamples() {
        m_samples.clear();
        m_names.clear();
    }

    void EmblemEditor::addSample(const EmblemGrid& grid, const std::string& name) {
        if (static_cast<int>(m_samples.size()) != kMostSamples) {
            m_samples.push_back(grid);
            m_names.push_back(name);
        }
    }

    void EmblemEditor::scrollSamplesLeft() {
        if (m_scroll - 1 >= 0) {
            --m_scroll;
        }
    }

    void EmblemEditor::scrollSamplesRight() {
        if (static_cast<int>(m_samples.size()) - 1 - 1 >= m_scroll + kSamplesShown) {
            ++m_scroll;
        }
    }

    void EmblemEditor::pickSample(int place) {
        const int index = m_scroll + place;
        if (index < 0 || index >= static_cast<int>(m_samples.size())) {
            return;
        }
        pushUndo();
        m_grid = m_samples[static_cast<std::size_t>(index)];
        pushUndo();
    }

}
