#include "client/rules/PianoSeat.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kSeatSide = 45; // seat x offset from the piano's world point, px
        constexpr int kSeatDrop = 20; // seat y offset below the piano's world point, px
        constexpr int kFacingEven = 3; // facing direction of the player on an even-picture piano
        constexpr int kFacingOdd = 5; // facing direction of the player on an odd-picture piano

    }

    bool PianoSeat::isFree(const HomeFurniture& furniture, const formats::ItemTable& items, int furnIndex) {
        const FurnitureRecord& piece = furniture.piece(furnIndex);
        const auto item = items.find(piece.itemId);
        if (!item) {
            return true;
        }
        int sizeX = item->furnSizeX;
        int sizeY = item->furnSizeY;
        if (piece.rotation == 1) {
            std::swap(sizeX, sizeY);
        }
        const int offsetX = piece.rotation == 0 ? -1 : 0;
        const int offsetY = piece.rotation == 0 ? 0 : -1;
        const int count = piece.rotation == 0 ? 1 : sizeY;
        const int count2 = piece.rotation == 0 ? sizeX : 1;
        const HouseRoomRect& area = furniture.room(piece.room);
        for (int i = 1; i <= count; ++i) {
            for (int j = 1; j <= count2; ++j) {
                const int x = piece.cellX + offsetX + i - 1;
                const int y = piece.cellY + offsetY + j - 1;
                if (!area.contains(x, y) || piece.cellZ < 1 || piece.cellZ > HomeFurniture::kLevels) {
                    return false;
                }
                const int held = furniture.houseCell(x, y, piece.cellZ, piece.room);
                if (held != 0 && held != furnIndex) {
                    return false;
                }
            }
        }
        return true;
    }

    PianoSeat::Place PianoSeat::place(core::Point pieceWorld, bool evenPicture) {
        return evenPicture ? Place{{pieceWorld.x + kSeatSide, pieceWorld.y + kSeatDrop}, kFacingEven}
                           : Place{{pieceWorld.x - kSeatSide, pieceWorld.y + kSeatDrop}, kFacingOdd};
    }

}
