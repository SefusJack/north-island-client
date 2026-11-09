#include "client/models/house/HouseRoomRect.h"

#include <cstdlib>

namespace client {

    bool HouseRoomRect::contains(int cellX, int cellY) const { return cellX >= left && cellX <= right && cellY >= top && cellY <= bottom; }

    HouseRoomRect HouseRoomRect::ofSize(int size) {
        switch (size) {
            case 1: return {29, 38, 44, 50}; // size 1: box left 29, top 38, right 44, bottom 50 (cells)
            case 2: return {29, 32, 44, 50}; // size 2: box left 29, top 32, right 44, bottom 50 (cells)
            case 3: return {29, 27, 44, 50}; // size 3: box left 29, top 27, right 44, bottom 50 (cells)
            default: return {}; // any other size: empty box
        }
    }

    int HouseRoomRect::spanX() const { return std::abs(right - left) + 1; }

    int HouseRoomRect::spanY() const { return std::abs(bottom - top) + 1; }

}
