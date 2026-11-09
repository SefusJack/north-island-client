#include "client/models/house/HouseGrid.h"

namespace client {

    core::Point HouseGrid::cellToScreen(int cellX, int cellY, int cellZ) {
        constexpr int kMost = 1000; // largest cell number accepted on any axis
        if (cellX < 0 || cellY < 0 || cellZ < 0 || cellX > kMost || cellY > kMost || cellZ > kMost) {
            return {-1, -1};
        }
        const int halfWidth = kCellWidth / 2;
        const int halfHeight = kCellHeight / 2;
        // the 2 turns the 1-based cell numbers into 0-based offsets
        return {kCells * halfWidth + (cellX - cellY) * halfWidth + kOrigin.x, (cellY + cellX - 2) * halfHeight + kOrigin.y - (cellZ - 1) * kLevelHeight};
    }

    HouseGrid::Cell HouseGrid::screenToCell(core::Point mapPoint) {
        constexpr double kSlope = 0.5; // slope of the isometric cell edges (30 px high per 60 px wide)
        const double x = mapPoint.x - kOrigin.x;
        const double y = mapPoint.y - kOrigin.y;
        const double halfWidth = kCellWidth / 2.0;
        const double halfHeight = kCellHeight / 2.0;
        const double topX = kCells * halfWidth;
        const double rightX = (kCells + kCells) * halfWidth;
        const double rightY = kCells * halfHeight;
        const double c1 = x * -kSlope - y;
        const double c2 = rightX * kSlope - rightY;
        const double cutX = c2 - c1;
        const double d1 = x * kSlope - y;
        const double d2 = topX * -kSlope - 0;
        const double cutY = (kSlope * d2 + kSlope * d1) / -1;
        const double fx = (cutX - topX) / halfWidth + 1;
        const double fy = cutY / halfHeight + 1;
        Cell cell;
        cell.x = static_cast<int>(fx) - (fx < 0 ? 1 : 0);
        cell.y = static_cast<int>(fy) - (fy < 0 ? 1 : 0);
        cell.z = 1;
        return cell;
    }

}
