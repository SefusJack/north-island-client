#include "client/models/house/HousePillars.h"

#include "client/models/house/FurnitureRecord.h"
#include "client/models/house/HouseGrid.h"
#include "core/ByteReader.h"

namespace client {

    namespace {

        constexpr int kPillarItem = 39063; // item 39063: Pillar
        constexpr int kOnWall = 4; // furnPlaceFlags bit 4: the piece hangs on a wall
        constexpr int kLevels = 4; // levels (z) a cell stacks

        /// @brief Tells whether a bit of a byte is set.
        /// @param bit Bit number 1..8 (1 is the lowest).
        /// @param byte The byte value.
        /// @return True when set.
        bool bitSet(int bit, int byte) { return ((byte >> (bit - 1)) & 1) != 0; }

        /// @brief The furniture record of a pillar standing in a room cell.
        /// @param cellX Pillar column (1..16).
        /// @param cellY Pillar row (1..24).
        /// @param floor Floor room index.
        /// @return A record for the pillar item at level 1.
        FurnitureRecord pillarRecord(int cellX, int cellY, int floor) {
            FurnitureRecord record;
            record.itemId = kPillarItem;
            record.cellX = cellX + HouseGrid::kGridColumn - 1;
            record.cellY = cellY + HouseGrid::kGridRow - 1;
            record.cellZ = 1;
            record.room = floor;
            return record;
        }

    }

    bool HousePillars::has(int cellX, int cellY, int floor) const {
        return floor >= 0 && floor < kFloors && cellX >= 1 && cellX <= kColumns && cellY >= 1 && cellY <= kRows &&
               m_cells[static_cast<std::size_t>(floor)][static_cast<std::size_t>(cellX)][static_cast<std::size_t>(cellY)];
    }

    core::Point HousePillars::worldOf(int cellX, int cellY) {
        return HouseGrid::cellToScreen(cellX + HouseGrid::kGridColumn - 1, cellY + HouseGrid::kGridRow - 1, 1);
    }

    core::Point HousePillars::cellOfIndex(int index) {
        const int y = (index - 1) / kColumns + 1;
        return {index - (y - 1) * kColumns, y};
    }

    bool HousePillars::blocked(int cellX, int cellY, int floor) const {
        const FurnitureRecord record = pillarRecord(cellX, cellY, floor);
        return m_furniture.placementBlocked(record.cellX, record.cellY, record.cellZ, record, 0);
    }

    bool HousePillars::add(int cellX, int cellY, int floor) {
        if (cellX < 1 || cellX > kColumns || cellY < 1 || cellY > kRows || floor < 0 || floor >= kFloors) {
            return false;
        }
        const FurnitureRecord record = pillarRecord(cellX, cellY, floor);
        if (blocked(cellX, cellY, floor)) {
            return false;
        }
        slot(cellX, cellY, floor) = true;
        m_furniture.markCells(record, HomeFurniture::kPillarCell, floor);
        return true;
    }

    bool HousePillars::remove(int cellX, int cellY, int floor) {
        if (cellX < 1 || cellX > kColumns || cellY < 1 || cellY > kRows || floor < 0 || floor >= kFloors) {
            return false;
        }
        const auto leaning = [&](int x, int y, int facing) {
            for (int level = 1; level <= kLevels; ++level) {
                const int piece = m_furniture.houseCell(x + HouseGrid::kGridColumn - 1, y + HouseGrid::kGridRow - 1, level, floor);
                if (piece >= 1 && piece <= HomeFurniture::kMost) {
                    const FurnitureRecord& record = m_furniture.piece(piece);
                    const auto item = m_items.find(record.itemId);
                    if (item && (item->furnPlaceFlags & kOnWall) != 0 && record.rotation == facing) {
                        return true;
                    }
                }
            }
            return false;
        };
        if ((cellX < kColumns && leaning(cellX + 1, cellY, 1)) || (cellY < kRows && leaning(cellX, cellY + 1, 0))) {
            m_notice("Please remove the pillar instrument first");
            return false;
        }
        slot(cellX, cellY, floor) = false;
        m_furniture.markCells(pillarRecord(cellX, cellY, floor), 0, floor);
        return true;
    }

    void HousePillars::clear() {
        for (int floor = 0; floor < kFloors; ++floor) {
            for (int x = 1; x <= kColumns; ++x) {
                for (int y = 1; y <= kRows; ++y) {
                    if (has(x, y, floor)) {
                        remove(x, y, floor);
                    }
                }
            }
        }
    }

    std::array<int, HousePillars::kBytes + 1> HousePillars::bytesOf(const Cells& cells, int floor) {
        std::array<int, kBytes + 1> bytes{};
        for (int index = 1; index <= kColumns * kRows; ++index) {
            const core::Point cell = cellOfIndex(index);
            if (cells[static_cast<std::size_t>(floor)][static_cast<std::size_t>(cell.x)][static_cast<std::size_t>(cell.y)]) {
                bytes[static_cast<std::size_t>((index - 1) / 8 + 1)] |= 1 << ((index - 1) % 8); // 8 cells per byte
            }
        }
        return bytes;
    }

    void HousePillars::startEdit(int mode, int floor) {
        if (mode < 1 || mode > 3 || floor < 0 || floor >= kFloors) { // edit modes 1 add, 2 remove, 3 move
            return;
        }
        if (m_editMode == 0) {
            m_kept = m_cells;
        }
        m_editMode = mode;
    }

    bool HousePillars::stopEdit(int floor) {
        const bool inMode = m_editMode >= 1 && m_editMode <= 3; // edit modes 1 add, 2 remove, 3 move
        m_editMode = 0;
        m_marked = {};
        m_lit = {};
        m_carried = {};
        m_carriedTo = {};
        return inMode && floor >= 0 && floor < kFloors && bytesOf(m_cells, floor) != bytesOf(m_kept, floor);
    }

    void HousePillars::revert(int floor) {
        for (int x = 1; x <= kColumns; ++x) {
            for (int y = 1; y <= kRows; ++y) {
                const bool wanted = m_kept[static_cast<std::size_t>(floor)][static_cast<std::size_t>(x)][static_cast<std::size_t>(y)];
                if (wanted != has(x, y, floor)) {
                    if (has(x, y, floor)) {
                        remove(x, y, floor);
                    } else {
                        add(x, y, floor);
                    }
                }
            }
        }
    }

    core::Bytes HousePillars::changeBody(int furnIndex, int floor) const {
        if (floor < 0 || floor >= kFloors) {
            return {};
        }
        const auto current = bytesOf(m_cells, floor);
        const auto kept = bytesOf(m_kept, floor);
        int differ = 0;
        for (int index = 1; index <= kBytes; ++index) {
            differ += current[static_cast<std::size_t>(index)] != kept[static_cast<std::size_t>(index)] ? 1 : 0;
        }
        if (differ == 0) {
            return {};
        }
        const int offset = floor == 0 ? 0 : 22; // second floor byte numbers start at 22 less
        // body: piece number as u16 (0xFF mask, shift 8), then mode 1 (all bytes) when more than 24 bytes differ, else mode 2 (changed bytes)
        core::Bytes body{
            static_cast<std::uint8_t>(furnIndex & 0xFF),
            static_cast<std::uint8_t>(furnIndex >> 8),
            static_cast<std::uint8_t>(differ > 24 ? 1 : 2) // mode 1 sends all 48 bytes (more than 24 differ), mode 2 only the changed ones
        };
        for (int index = 1; index <= kBytes; ++index) {
            if (differ > 24) {
                body.push_back(static_cast<std::uint8_t>(current[static_cast<std::size_t>(index)]));
            } else if (current[static_cast<std::size_t>(index)] != kept[static_cast<std::size_t>(index)]) {
                body.push_back(static_cast<std::uint8_t>(index - offset));
                body.push_back(static_cast<std::uint8_t>(current[static_cast<std::size_t>(index)]));
            }
        }
        return body;
    }

    bool HousePillars::move(
        int fromX,
        int fromY,
        int toX,
        int toY,
        int floor
    ) {
        if ((fromX == toX && fromY == toY) || !has(fromX, fromY, floor) || toX < 1 || toX > kColumns || toY < 1 || toY > kRows) {
            return false;
        }
        if (blocked(toX, toY, floor)) {
            return false;
        }
        return remove(fromX, fromY, floor) && add(toX, toY, floor);
    }

    void HousePillars::applyList(const core::Bytes& payload) {
        if (payload.size() <= 1) {
            return;
        }
        clear();
        core::ByteReader reader(payload, 2); // skips the first 2 payload bytes (action and sub ids)
        while (reader.remaining() > 1 && !reader.failed()) {
            const int floor = reader.u8();
            const int mode = reader.u8();
            if (mode == 0) {
                continue;
            }
            const int count = reader.u8();
            for (int i = 1; i <= count && !reader.failed(); ++i) {
                const int byteIndex = mode == 1 ? i : reader.u8();
                const int bits = reader.u8();
                for (int bit = 1; bit <= 8; ++bit) { // 8 cells per byte
                    if (bitSet(bit, bits)) {
                        const core::Point cell = cellOfIndex((byteIndex - 1) * 8 + bit); // 8 cells per byte
                        add(cell.x, cell.y, floor);
                    }
                }
            }
        }
    }

    void HousePillars::applyDiff(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the first 2 payload bytes (action and sub ids)
        const int floor = reader.u8();
        const int mode = reader.u8();
        if (reader.failed() || floor < 0 || floor >= kFloors) {
            return;
        }
        const auto apply = [&](int byteIndex, int bits) {
            for (int bit = 1; bit <= 8; ++bit) { // 8 cells per byte
                const core::Point cell = cellOfIndex((byteIndex - 1) * 8 + bit); // 8 cells per byte
                const bool wanted = bitSet(bit, bits);
                if (wanted != has(cell.x, cell.y, floor)) {
                    if (wanted) {
                        add(cell.x, cell.y, floor);
                    } else {
                        remove(cell.x, cell.y, floor);
                    }
                }
            }
        };
        if (mode == 1) {
            for (int byteIndex = 1; reader.remaining() > 0; ++byteIndex) {
                apply(byteIndex, reader.u8());
            }
        } else if (mode == 2) { // mode 2: pairs of a byte's number and its bits
            while (reader.remaining() >= 2) {
                const int byteIndex = reader.u8();
                apply(byteIndex, reader.u8());
            }
        }
    }

}
