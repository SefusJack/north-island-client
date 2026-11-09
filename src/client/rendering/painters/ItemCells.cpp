#include "client/rendering/painters/ItemCells.h"

#include "client/rules/ItemInfo.h"
#include "client/data/VehicleKinds.h"
#include "client/rules/VehiclePlacing.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr int kCellSize = 32; // px: side of one inventory cell / item icon
        constexpr int kCellGap = 2; // px: gap between neighbouring cells of a multi-cell item
        constexpr int kRailTop = 5; // px: y of the durability rail/bar on a vehicle icon
        constexpr int kFuelRailTop = 10; // px: y of the fuel rail/bar on a vehicle icon

    }

    ui::IconCell itemIconCell(
        GameServices& services,
        const protocol::ItemInstance& item,
        bool inBag,
        bool inUse,
        bool inVehicleBox
    ) {
        ui::IconCell cell;
        const auto record = services.items.find(item.itemId);
        const auto look = item.lookItemId != 0 ? services.items.find(item.lookItemId) : record;
        const int iconId = look ? look->iconId : 0;
        cell.picture = services.pictures.find(std::to_string(iconId)).value_or(services.pictures.find("1000").value_or(gfx::PictureId{}));
        cell.source = {0, 0, kCellSize, kCellSize};
        const int columns = record ? record->cellWidth : 1;
        const int rows = record ? record->cellHeight : 1;
        if ((columns > 1 || rows > 1) && cell.picture.isValid()) {
            const gfx::Picture& picture = services.pictures.picture(cell.picture);
            const int width = columns * kCellSize + (columns - 1) * kCellGap;
            const int height = rows * kCellSize + (rows - 1) * kCellGap;
            // 2: pixels of the picture's border that are not part of the middle
            cell.source = {std::max(0, (picture.width() - 2) / 2 - width / 2), std::max(0, (picture.height() - 2) / 2 - height / 2), width, height};
        }
        const std::string size = std::to_string(columns) + "x" + std::to_string(rows) + "Grid";
        if (inUse) {
            cell.frame = services.pictures.find(size + "_2").value_or(gfx::PictureId{});
        } else if (columns > 1 || rows > 1) {
            cell.frame = services.pictures.find(size).value_or(gfx::PictureId{});
        }
        if (inBag) {
            cell.count = item.count;
        }
        constexpr int kVehicleType = 39; // item type 39: vehicles (Hot-air Balloon, Airship, Boat, ...)
        if (record && record->type == kVehicleType) {
            const std::string suffix = VehicleKinds::isCapsule(record->id) && !inVehicleBox ? "3" : "2";
            const int limit = vehicleWearLimit(*record);
            const auto strip = [&](const std::string& name, int top, double part) {
                if (const auto picture = services.pictures.find(name)) {
                    const gfx::Picture& data = services.pictures.picture(*picture);
                    const int width = part < 0 ? data.width() : static_cast<int>(std::lround(data.width() * part));
                    cell.overlays.push_back({*picture, {0, 0, width, data.height()}, {0, top}});
                }
            };
            strip("icon_rail" + suffix, kRailTop, -1);
            strip("icon_HP" + suffix, kRailTop, limit > 0 ? static_cast<double>(limit - item.usedAmount) / limit : 0.0);
            if (item.extraAmount > 0) {
                strip("icon_rail" + suffix, kFuelRailTop, -1);
                strip("icon_SP" + suffix, kFuelRailTop, limit > 0 ? static_cast<double>(item.extraAmount) / limit : 0.0);
            }
        }
        if (record) {
            cell.hint = record->name + hintSuffix(*record, item);
        }
        return cell;
    }

    ui::IconCell itemNameCell(GameServices& services, const protocol::ItemInstance& item) {
        ui::IconCell cell;
        const auto record = services.items.find(item.itemId);
        cell.picture = services.pictures.find(std::to_string(record ? record->iconId : 0)).value_or(services.pictures.find("1000").value_or(gfx::PictureId{}));
        cell.source = {0, 0, kCellSize, kCellSize};
        cell.count = item.count;
        if (record) {
            cell.hint = record->name;
        }
        return cell;
    }

}
