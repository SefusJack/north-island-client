#include "ui/IconGrid.h"

#include "gfx/TextStyle.h"

namespace ui {

    namespace {

        constexpr core::Point kCountOffset{2, 20}; // px offset of the item count text from the cell's top left
        constexpr int kCountWidth = 32; // px width of the item count text area

        /// @brief Whether a point lies in a rectangle (right and bottom edges excluded).
        /// @param box The rectangle.
        /// @param point The point.
        /// @return True when inside.
        bool inside(const core::Rect& box, core::Point point) {
            return point.x >= box.x && point.y >= box.y && point.x < box.x + box.width && point.y < box.y + box.height;
        }

    }

    IconGrid::IconGrid() {
        setClickSound("sound/wav0151.wav"); // the click sound of a grid cell (wav0151)
        declareEvent("DoubleClick");
        declareEvent("Drop");
        declareEvent("Click");
        declareEvent("RightClick");
        declareEvent("Press");
        declareEvent("MouseWheel");
    }

    void IconGrid::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Columns") {
            m_columns = LoadContext::parseInt(value);
        } else if (name == "Rows") {
            m_rows = LoadContext::parseInt(value);
        } else if (name == "Pitch") {
            m_pitch = LoadContext::parseInt(value);
            m_pitchY = m_pitch;
        } else if (name == "PitchY") {
            m_pitchY = LoadContext::parseInt(value);
        } else if (name == "CellSize") {
            m_cellSize = LoadContext::parseInt(value);
        } else if (name == "FirstIndex") {
            m_firstIndex = LoadContext::parseInt(value);
        } else if (name == "HoverColor") {
            m_hoverColor = LoadContext::parseColor(value);
        } else if (name == "CountColor") {
            m_countColor = LoadContext::parseColor(value);
        } else if (name == "Draggable") {
            m_draggable = LoadContext::parseBool(value);
        } else if (name == "CellPicture") {
            m_cellPictureName = value;
        } else if (name == "OutlineColor") {
            m_outlineColor = LoadContext::parseColor(value);
        } else if (name == "OutlineGapColor") {
            m_outlineGapColor = LoadContext::parseColor(value);
        } else if (name == "CellSource") {
            m_cellSource = LoadContext::parseRect(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void IconGrid::finishLoading(const LoadContext& context) {
        if (!sizeWasGiven()) {
            setBounds({bounds().x, bounds().y, (m_columns - 1) * m_pitch + m_cellSize, (m_rows - 1) * m_pitchY + m_cellSize});
        }
        if (!m_cellPictureName.empty()) {
            m_cellPicture = context.picture(m_cellPictureName);
        }
        Element::finishLoading(context);
    }

    void IconGrid::setColumns(int columns, int rows) {
        m_columns = columns;
        m_rows = rows;
        setBounds({bounds().x, bounds().y, (m_columns - 1) * m_pitch + m_cellSize, (m_rows - 1) * m_pitchY + m_cellSize});
    }

    core::Point IconGrid::cellPosition(int index) const {
        const int number = index - m_firstIndex;
        return {(number % m_columns) * m_pitch, (number / m_columns) * m_pitchY};
    }

    int IconGrid::cellAt(core::Point local) const {
        for (auto cell = m_cells.rbegin(); cell != m_cells.rend(); ++cell) {
            const core::Point at = cellPosition(cell->first);
            if (inside({at.x, at.y, cell->second.source.width, cell->second.source.height}, local)) {
                return cell->first;
            }
        }
        return 0;
    }

    bool IconGrid::hitTest(core::Point local, const gfx::PictureLibrary&) const { return inside({0, 0, bounds().width, bounds().height}, local); }

    const std::string& IconGrid::hintAt(core::Point local) const {
        static const std::string none;
        const auto cell = m_cells.find(cellAt(local));
        return cell != m_cells.end() ? cell->second.hint : none;
    }

    core::Point IconGrid::hintOrigin(core::Point local) const { return cellPosition(cellAt(local)); }

    void IconGrid::onDoubleClick(UiContext& context, core::Point local) {
        m_doubleClicked = true;
        if (const int index = cellAt(local)) {
            raise(context, "DoubleClick", index);
        }
    }

    void IconGrid::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left) {
            m_pressedCell = cellAt(local);
            if (m_pressedCell != 0) {
                raise(context, "Press", m_pressedCell);
            }
        }
        const bool doubleClicked = m_doubleClicked;
        m_doubleClicked = false;
        if (!m_draggable || doubleClicked || button != MouseButton::Left) {
            return;
        }
        const int index = cellAt(local);
        const auto cell = m_cells.find(index);
        if (cell == m_cells.end() || !cell->second.picture.isValid()) {
            return;
        }
        context.drag = DragItem{id(), index, cell->second.picture, cell->second.source, local - cellPosition(index)};
    }

    void IconGrid::onMouseWheel(UiContext& context, int steps) { raise(context, "MouseWheel", steps); }

    void IconGrid::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        const int index = cellAt(local);
        if (index == 0) {
            return;
        }
        if (button == MouseButton::Left && index == m_pressedCell) {
            raise(context, "Click", index);
        } else if (button == MouseButton::Right) {
            raise(context, "RightClick", index);
        }
    }

    void IconGrid::onDropped(
        UiContext& context,
        int sourceIndex,
        std::optional<std::reference_wrapper<const Element>> target,
        int targetIndex,
        core::Point screenPoint,
        core::Point grab
    ) {
        EventArgs args;
        args.index = sourceIndex;
        args.position = screenPoint;
        args.target = target ? target->get().name() : std::string();
        args.targetIndex = targetIndex;
        args.grab = grab;
        raise(context, "Drop", args);
    }

    int IconGrid::dropIndexAt(core::Point local) const {
        if (const int index = cellAt(local)) {
            return index;
        }
        if (local.x < 0 || local.y < 0 || local.x % m_pitch >= m_cellSize || local.y % m_pitchY >= m_cellSize) {
            return -1;
        }
        const int column = local.x / m_pitch;
        const int row = local.y / m_pitchY;
        if (column >= m_columns || row >= m_rows) {
            return -1;
        }
        return m_firstIndex + row * m_columns + column;
    }

    void IconGrid::draw(DrawContext& context, core::Point screenPosition) {
        if (m_outlineColor) {
            const int width = m_columns * m_pitch;
            const int height = m_rows * m_pitchY;
            if (m_outlineGapColor) {
                // the bag's frame: line, gap, line on the top and left; the cells' own lines, then one line on the right; a gap row,
                // then one line below
                context.canvas.fillRect({screenPosition.x - 3, screenPosition.y - 3, width + 4, height + 5}, *m_outlineColor);
                context.canvas.fillRect({screenPosition.x - 2, screenPosition.y - 2, width + 2, height + 3}, *m_outlineGapColor);
                context.canvas.fillRect({screenPosition.x - 1, screenPosition.y - 1, width, height}, *m_outlineColor);
            } else {
                context.canvas.drawRectOutline({screenPosition.x - 1, screenPosition.y - 1, width + 2, height + 2}, *m_outlineColor);
            }
        }
        if (m_cellPicture.isValid()) {
            for (int index = m_firstIndex; index < m_firstIndex + m_columns * m_rows; ++index) {
                context.canvas.drawPicture(m_cellPicture, screenPosition + cellPosition(index), m_cellSource);
            }
        }
        for (const auto& [index, cell] : m_cells) {
            if (cell.frame.isValid()) {
                context.canvas.drawPicture(cell.frame, screenPosition + cellPosition(index));
            }
        }
        const int hovered = context.hoveredId == id() ? cellAt(context.mouse - screenPosition) : 0;
        for (auto entry = m_cells.rbegin(); entry != m_cells.rend(); ++entry) {
            const auto& [index, cell] = *entry;
            if (!cell.picture.isValid()) {
                continue;
            }
            const core::Point at = screenPosition + cellPosition(index);
            if (index == hovered) {
                const core::Size hoverSize = cell.size ? core::Size{m_cellSize, m_cellSize} : core::Size{cell.source.width, cell.source.height};
                context.canvas.fillRectAlpha({at.x, at.y, hoverSize.width, hoverSize.height}, m_hoverColor);
            }
            if (cell.size) {
                const core::Rect
                    box{at.x + (m_cellSize - cell.size->width) / 2, at.y + (m_cellSize - cell.size->height) / 2, cell.size->width, cell.size->height};
                context.canvas.drawPictureScaled(cell.picture, box, cell.source);
            } else {
                context.canvas.drawPicture(cell.picture, at, cell.source);
            }
            if (cell.count) {
                const int rowsDown = (cell.source.height - m_cellSize) / m_pitchY;
                context.canvas.drawText(
                    {at.x + kCountOffset.x, at.y + rowsDown * m_pitchY + kCountOffset.y},
                    std::to_string(*cell.count),
                    {m_countColor, gfx::TextEffect::Plain, std::nullopt},
                    kCountWidth
                );
            }
            for (const IconOverlay& overlay : cell.overlays) {
                context.canvas.drawPicture(overlay.picture, at + overlay.offset, overlay.source);
            }
        }
    }

}
