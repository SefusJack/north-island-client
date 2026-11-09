#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_PILLARS_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_PILLARS_H

#include "client/models/house/HomeFurniture.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "formats/ItemTable.h"

#include <array>
#include <functional>
#include <string>
#include <utility>

namespace client {

    /// @brief The pillars of the two floors: the cells of the 16 x 24 grid of a room that hold one.
    /// @details Original: DZ_Thing.pas TPillarManage. A wall's pieces stand between pillars; a pillar fills its cell for the other furniture (value 201).
    class HousePillars {
        public:
            static constexpr int kColumns = 16; // cell columns of a floor's pillar map
            static constexpr int kRows = 24; // cell rows of a floor's pillar map
            static constexpr int kFloors = 2; // floors with pillar maps
            static constexpr int kBytes = 48; // bytes of one floor's map, a bit per cell (16 x 24 / 8)

            /// @brief Creates an empty pillar set.
            /// @param furniture The house's furniture whose cells pillars fill.
            /// @param items Item table.
            /// @param notice Receives the reason when a removal is refused.
            HousePillars(
                HomeFurniture& furniture,
                const formats::ItemTable& items,
                std::function<void(const std::string&)> notice
            )
                : m_furniture(furniture),
                  m_items(items),
                  m_notice(std::move(notice)) {}

            /// @brief Tells whether there is a pillar in a cell of a floor.
            /// @param cellX Column 1..16.
            /// @param cellY Row 1..24.
            /// @param floor Floor 0 or 1.
            /// @return True when there is one.
            bool has(int cellX, int cellY, int floor) const;

            /// @brief Adds a pillar.
            /// @details Original: AddPillarAt.
            /// @param cellX Column 1..16.
            /// @param cellY Row 1..24.
            /// @param floor Floor 0 or 1.
            /// @return True when added (false when the cell is blocked or outside).
            bool add(int cellX, int cellY, int floor);

            /// @brief Removes a pillar; refused while a wall piece leans on it.
            /// @details Original: RemovePillarAt.
            /// @param cellX Column 1..16.
            /// @param cellY Row 1..24.
            /// @param floor Floor 0 or 1.
            /// @return True when removed.
            bool remove(int cellX, int cellY, int floor);

            /// @brief Removes all pillars.
            /// @details Original: ClearAllPillars.
            void clear();

            /// @brief Replaces the pillars from a pillar list message.
            /// @details Original: server action 65/11 (ApplyPillarList): for each floor a mode (0 none, 1 bytes in order, 2 pairs of byte number and bits).
            /// @param payload The whole message payload.
            void applyList(const core::Bytes& payload);

            /// @brief Applies the changed cells of a pillar diff message.
            /// @details Original: server action 65/13 (ApplyPillarDiff): a floor and a mode (1 all bytes, 2 pairs of byte number and bits).
            /// @param payload The whole message payload.
            void applyDiff(const core::Bytes& payload);

            /// @brief The map point of a cell's floor.
            /// @details Original: WordGrid of TPillarManage's constructor.
            /// @param cellX Column 1..16.
            /// @param cellY Row 1..24.
            /// @return The map point.
            static core::Point worldOf(int cellX, int cellY);

            /// @brief The cell of a place number, row by row.
            /// @details Original: PillarIndexToCell.
            /// @param index Place 1..384.
            /// @return The cell (column, row).
            static core::Point cellOfIndex(int index);

            /// @brief The owner's pillar editing mode: 0 none, 1 add, 2 remove, 3 move.
            /// @details Original: SetPillarEditMode, FPillars.PlaceMode.
            /// @return The mode.
            int editMode() const { return m_editMode; }

            /// @brief Starts editing: from none, keeps the pillars to compare with and to go back to.
            /// @param mode 1 add, 2 remove, 3 move (others are ignored).
            /// @param floor Floor 0 or 1.
            void startEdit(int mode, int floor);

            /// @brief Stops editing (mode 0).
            /// @param floor Floor 0 or 1.
            /// @return True when the floor's pillars differ from the ones kept (the owner is asked).
            bool stopEdit(int floor);

            /// @brief Goes back to the pillars kept.
            /// @details Original: RevertPillarChange.
            /// @param floor Floor 0 or 1.
            void revert(int floor);

            /// @brief Builds the body of the client's 65/11 request with the pillar changes.
            /// @details Original: SendPillarChanges: the Handcart's piece, mode 1 all 48 bytes when more than 24 differ, else mode 2 the bytes that differ.
            /// @param furnIndex The Handcart piece.
            /// @param floor Floor 0 or 1.
            /// @return The body, empty when nothing differs.
            core::Bytes changeBody(int furnIndex, int floor) const;

            /// @brief Moves one pillar to another cell of the floor.
            /// @details Original: MovePillar.
            /// @param fromX Old column.
            /// @param fromY Old row.
            /// @param toX New column.
            /// @param toY New row.
            /// @param floor Floor 0 or 1.
            /// @return True when moved.
            bool move(
                int fromX,
                int fromY,
                int toX,
                int toY,
                int floor
            );

            /// @brief The cell under the mouse while adding.
            /// @details Original: UpdatePillarHover (MarkedCell).
            /// @return The cell, (0, 0) for none.
            core::Point marked() const { return m_marked; }

            /// @brief Sets the cell under the mouse while adding.
            /// @param cell The cell.
            void setMarked(core::Point cell) { m_marked = cell; }

            /// @brief The pillar lit while removing or moving.
            /// @return The cell, (0, 0) for none.
            core::Point lit() const { return m_lit; }

            /// @brief Sets the pillar lit while removing or moving.
            /// @param cell The cell.
            void setLit(core::Point cell) { m_lit = cell; }

            /// @brief The pillar picked up while moving.
            /// @details Original: Se_DragItem.FDragSource.
            /// @return The cell, (0, 0) for none.
            core::Point carried() const { return m_carried; }

            /// @brief Sets the pillar picked up while moving.
            /// @param cell The cell.
            void setCarried(core::Point cell) { m_carried = cell; }

            /// @brief The cell the carried pillar would go to.
            /// @return The cell.
            core::Point carriedTo() const { return m_carriedTo; }

            /// @brief Sets the cell the carried pillar would go to.
            /// @param cell The cell.
            void setCarriedTo(core::Point cell) { m_carriedTo = cell; }

        private:
            using Cells = std::array<std::array<std::array<bool, kRows + 1>, kColumns + 1>, kFloors>;

            /// @brief Tells whether a pillar cannot stand in a cell.
            /// @details Original: IsPlacementBlocked for a pillar.
            /// @param cellX Column 1..16.
            /// @param cellY Row 1..24.
            /// @param floor Floor 0 or 1.
            /// @return True when blocked.
            bool blocked(int cellX, int cellY, int floor) const;

            /// @brief A floor's cells as bytes 1..48; bit n of byte i is the cell (i - 1) * 8 + n.
            /// @details Original: FPillars.Floors[].Bits.
            /// @param cells The cells of all floors.
            /// @param floor Floor 0 or 1.
            /// @return The bytes.
            static std::array<int, kBytes + 1> bytesOf(const Cells& cells, int floor);

            /// @brief The cell flag of a pillar position.
            /// @param cellX Column 1..16.
            /// @param cellY Row 1..24.
            /// @param floor Floor 0 or 1.
            /// @return A reference to the flag.
            bool& slot(int cellX, int cellY, int floor) {
                return m_cells[static_cast<std::size_t>(floor)][static_cast<std::size_t>(cellX)][static_cast<std::size_t>(cellY)];
            }

            HomeFurniture& m_furniture;
            const formats::ItemTable& m_items;
            std::function<void(const std::string&)> m_notice;
            Cells m_cells{};
            Cells m_kept{};
            int m_editMode = 0;
            core::Point m_marked;
            core::Point m_lit;
            core::Point m_carried;
            core::Point m_carriedTo;
    };

}

#endif
