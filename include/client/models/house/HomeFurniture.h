#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOME_FURNITURE_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOME_FURNITURE_H

#include "client/models/house/FurnitureRecord.h"
#include "client/models/house/HouseRoomRect.h"
#include "formats/ItemTable.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace client {

    /// @brief The furniture of the house the player is in: pieces by number (1..130), the cells they fill in the two floors' grids, and the room boxes.
    /// @details Original: RE_Furniture.pas TRE_FurnitureMng. HouseInterior applies the messages 62/x with PlaceFurniture, RemoveFurniture and LinkFurnitureItem.
    class HomeFurniture {
        public:
            static constexpr int kMost = 130; // most furniture pieces a house holds
            static constexpr int kRooms = 5; // rooms (floor plans) of a house
            static constexpr int kPillarCell = 201; // cell value that marks a pillar-filled cell
            static constexpr int kLevels = 4; // levels (z, 1 the floor) a cell stacks

            /// @brief The cells a piece covers: across (x), deep (y) and high (z), by its facing.
            struct Span {
                int x = 1;
                int y = 1;
                int z = 1;
            };

            /// @brief Creates an empty house (first floor room size 3, second size 1).
            /// @param items Item table giving piece sizes and flags.
            explicit HomeFurniture(
                const formats::ItemTable& items
            );

            /// @brief The id of the house's owner (62/4 list sets it).
            /// @return Owner id.
            std::int32_t ownerId() const { return m_ownerId; }

            /// @brief Sets the id of the house's owner.
            /// @param ownerId Owner id.
            void setOwnerId(std::int32_t ownerId) { m_ownerId = ownerId; }

            /// @brief Replaces a room's box; a box not inside the 57 x 57 cells is ignored.
            /// @details Original: SetRoomArea.
            /// @param area The new box.
            /// @param index Room index 0..kRooms-1.
            void setRoomArea(const HouseRoomRect& area, int index);

            /// @brief The box of a room.
            /// @param index Room index (clamped to the valid range).
            /// @return The room's box.
            const HouseRoomRect& room(int index) const;

            /// @brief The piece with a number.
            /// @param index Piece number 1..130.
            /// @return The piece, or an empty record (item 0) for none or an index out of range.
            const FurnitureRecord& piece(int index) const;

            /// @brief Makes a piece take another item where it stands.
            /// @details Original: RecvMaidAction 3 (the Wit Maid's new costume).
            /// @param index Piece number.
            /// @param itemId The new item.
            void setItemId(int index, int itemId);

            /// @brief Sets how much of a Magic Sandglass piece is used up.
            /// @details Original: HandleSandglassUsedPacket.
            /// @param index Piece number.
            /// @param value Used amount.
            void setUsedCount(int index, int value);

            /// @brief The number of pieces in the house.
            /// @return Piece count.
            int count() const { return m_count; }

            /// @brief Finds the first free piece number.
            /// @details Original: FindFreeFurnitureSlot.
            /// @return The number, or 0 when none is free.
            int freeSlot() const;

            /// @brief The cells a piece fills, by its item and facing.
            /// @param itemId The item.
            /// @param rotation The facing (0 or 1; 1 swaps across and deep).
            /// @return The span, or nothing for an item the table lacks.
            std::optional<Span> spanOf(int itemId, int rotation) const;

            /// @brief Tells whether an item is one of the two carpets, which fill no cells.
            /// @details Original: IsCarpetItem.
            /// @param itemId The item.
            /// @return True for a carpet.
            bool isCarpet(int itemId) const;

            /// @brief Puts a piece into a slot and marks its cells.
            /// @details Original: PlaceFurniture.
            /// @param slot Piece number, 0 for the first free one.
            /// @param record The piece.
            /// @param strict Also require a wall piece to lean on pillars.
            /// @return True when the piece was put there.
            bool place(int slot, const FurnitureRecord& record, bool strict);

            /// @brief Removes a piece and frees its cells.
            /// @details Original: RemoveFurniture.
            /// @param slot Piece number.
            /// @return True when removed (false when missing or another piece stands on it).
            bool remove(int slot);

            /// @brief Makes a child piece stand on a parent piece.
            /// @details Original: LinkFurnitureItem.
            /// @param child The piece standing on.
            /// @param parent The piece it stands on.
            /// @return The parent's link place used, 0 for none.
            int link(int child, int parent);

            /// @brief Removes all pieces and empties the grids (the rooms stay).
            /// @details Original: ClearFurnitureMng.
            void clear();

            /// @brief Tells whether another piece stands on a piece.
            /// @details Original: HasFurnitureOnTop.
            /// @param slot Piece number.
            /// @return True when covered.
            bool coveredByAnother(int slot) const { return hasFurnitureOnTop(slot); }

            /// @brief Tells whether a piece may be moved to a place.
            /// @details Original: ValidateFurnitureMove.
            /// @param slot Piece number.
            /// @param place A record with the new cell, facing and room.
            /// @return True when allowed.
            bool moveAllowed(int slot, const FurnitureRecord& place) const;

            /// @brief What fills a cell: a piece's number, 201 a pillar, 0 nothing, 255 outside the grid.
            /// @details Original: GetHouseCell.
            /// @param cellX Cell column.
            /// @param cellY Cell row.
            /// @param cellZ Level.
            /// @param room Floor room index.
            /// @return The cell value.
            int houseCell(int cellX, int cellY, int cellZ, int room) const;

            /// @brief Tells whether a piece cannot stand with its first cell there.
            /// @details Original: IsPlacementBlocked.
            /// @param cellX First cell column.
            /// @param cellY First cell row.
            /// @param cellZ First cell level.
            /// @param record The piece (item, facing, room).
            /// @param allowedCell A cell value that does not block (the piece's own number).
            /// @return True when blocked.
            bool placementBlocked(
                int cellX,
                int cellY,
                int cellZ,
                const FurnitureRecord& record,
                int allowedCell
            ) const;

            /// @brief Sets the cells a piece covers to a value.
            /// @details Original: MarkFurnitureCells.
            /// @param record The piece.
            /// @param value Value to write (its number, or 0).
            /// @param room Floor room index.
            /// @return True when all cells were inside the grid.
            bool markCells(const FurnitureRecord& record, int value, int room);

        private:
            /// @brief Checks whether a piece may go to a slot.
            /// @details Original: ValidateFurniturePlace.
            /// @param record The piece.
            /// @param slot Piece number, 0 for the first free one.
            /// @param strict Also require a wall piece to lean on pillars.
            /// @return 0 when allowed, else a refusal code (1 outside the room, 2 blocked, 3 no second facing, 255 invalid).
            int validatePlace(const FurnitureRecord& record, int slot, bool strict) const;

            /// @brief Tells whether another piece stands on a piece (true also for a piece that does not exist).
            /// @details Original: HasFurnitureOnTop.
            /// @param slot Piece number.
            /// @return True when covered.
            bool hasFurnitureOnTop(int slot) const;

            /// @brief Writes one cell of a floor's grid.
            /// @param cellX Cell column.
            /// @param cellY Cell row.
            /// @param cellZ Level.
            /// @param value Value to write.
            /// @param room Floor room index.
            /// @return False when the cell is outside the grid.
            bool setHouseCell(
                int cellX,
                int cellY,
                int cellZ,
                int value,
                int room
            );

            /// @brief Tells whether a wall piece leans on pillars (or is on the room's first row or column).
            /// @param record The wall piece.
            /// @param span The cells it covers.
            /// @return True when it leans on pillars.
            bool leansOnPillars(const FurnitureRecord& record, const Span& span) const;

            /// @brief The index of a cell in a floor's grid.
            /// @param cellX Cell column.
            /// @param cellY Cell row.
            /// @param cellZ Level.
            /// @param room Floor room index.
            /// @return The index, -1 outside.
            static int gridIndex(int cellX, int cellY, int cellZ, int room);

            const formats::ItemTable& m_items;
            std::int32_t m_ownerId = 0;
            int m_count = 0;
            std::array<FurnitureRecord, kMost + 1> m_pieces{};
            std::array<HouseRoomRect, kRooms> m_rooms{};
            std::vector<std::uint8_t> m_grids;
    };

}

#endif
