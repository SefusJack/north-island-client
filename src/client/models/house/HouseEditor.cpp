#include "client/models/house/HouseEditor.h"

#include "client/rules/FurnitureKinds.h"
#include "protocol/HouseRequests.h"
#include "protocol/ShowPlatformRequests.h"
#include "protocol/StoreroomRequests.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kRecyclingBin = 38055; // item 38055: Resource Recycling (bin)
        constexpr int kKittyBin = 38059; // item 38059: Kitty Recycling Bin
        constexpr int kWorkPlatform = 38049; // item 38049: Work Platform
        constexpr int kSweetDoor = 39111; // item 39111: Sweet Door (a rented door)
        constexpr int kJobItemType = 20; // item type 20 (e.g. Angel's Feather, Beast Pill, Black Tea): cannot be recycled
        constexpr int kReach = 170; // max distance in px (each axis) from the player to a piece to work with it
        constexpr int kNoticeMs = 1500; // notice display time, ms
        constexpr int kLongNoticeMs = 2000; // long notice display time, ms
        constexpr int kQueueNoticeMs = 1000; // "cancel the movement" notice display time, ms
        constexpr int kMostLinks = 10; // link places of a piece for pieces standing on it

        /// @brief Tells whether an item is a recycling bin (Recycling Bin or Kitty Bin).
        /// @param itemId The item.
        /// @return True for a bin.
        bool isBin(int itemId) { return itemId == kRecyclingBin || itemId == kKittyBin; }

        /// @brief Tells whether an item is a rented furniture kind (the doors); giving one up gives up the right to it.
        /// @details Original: GetRentFurnitureKind.
        /// @param itemId The item.
        /// @return True for a rented door.
        bool isRented(int itemId) {
            return itemId == 39107 || itemId == 39108 || itemId == kSweetDoor;
        } // 39107 Cherry Door and 39108 Beach Door are two rented doors; 39111 is the Sweet Door

    }

    HouseEditor::HouseEditor(
        HouseInterior& interior,
        const formats::ItemTable& items,
        Hooks hooks
    )
        : m_interior(interior),
          m_items(items),
          m_hooks(std::move(hooks)),
          m_walls(interior, items) {}

    bool HouseEditor::isWallPiece(int itemId) const {
        const auto item = m_items.find(itemId);
        return item && (item->furnPlaceFlags & FurnitureKinds::kOnWall) != 0;
    }

    std::optional<HouseWallDrag::Place> HouseEditor::placeOnWall(
        int itemId,
        core::Point mouse,
        core::Point middle,
        int direction,
        const Camera& camera
    ) {
        const core::Point rowCol = m_walls.hitCell(mouse, camera);
        const HouseGrid::Cell under = HouseGrid::screenToCell(camera.toMap(middle));
        std::optional<HouseWallDrag::Place> found = HouseWallDrag::Place{under, direction};
        if (under.x - HouseGrid::kGridColumn + 1 <= HouseGrid::kGridWidth && under.y - HouseGrid::kGridRow + 1 <= HouseGrid::kGridDepth) {
            found = m_walls.place(itemId, rowCol, under, camera.toMap(mouse));
        }
        m_light = HouseWallLight{rowCol, itemId, found ? found->direction : direction};
        return found;
    }

    void HouseEditor::update(std::optional<int> bagItem, core::Point mouse, const Camera& camera) {
        if (m_drag) {
            if (isWallPiece(m_drag->itemId)) {
                dragTo(mouse, camera);
            } else {
                m_light.reset();
            }
            return;
        }
        m_light.reset();
        if (bagItem && owner() && m_interior.inFloorRoom() && isWallPiece(*bagItem)) {
            placeOnWall(*bagItem, mouse, mouse, 0, camera);
        }
    }

    HouseGrid::Cell HouseEditor::cellUnder(core::Point mouse, const Camera& camera) { return HouseGrid::screenToCell(camera.toMap(mouse)); }

    bool HouseEditor::press(int slot, core::Point mouse, const Camera& camera) {
        const HouseUtensil& utensil = m_interior.utensils().utensil(slot);
        const auto item = m_items.find(utensil.itemId);
        if (!owner() || !item || !utensil.picture.isValid()) {
            return false;
        }
        const core::Rect area = m_interior.utensils().pictureArea(slot, camera.origin());
        HouseDrag drag;
        drag.furnIndex = utensil.furnIndex;
        drag.slot = slot;
        drag.itemId = utensil.itemId;
        drag.direction = m_interior.furniture().piece(utensil.furnIndex).rotation;
        drag.picture = utensil.picture;
        if (isWallPiece(utensil.itemId)) {
            drag.turned = m_interior.utensils().facingPicture(utensil.itemId, 1);
        }
        drag.size = {area.width, area.height};
        drag.grab = {mouse.x - area.x, mouse.y - area.y};
        drag.click = camera.toMap(mouse);
        m_drag = drag;
        return true;
    }

    bool HouseEditor::pressRetained(int furnIndex, core::Point mouse, const Camera& camera) {
        const FurnitureRecord& piece = m_interior.furniture().piece(furnIndex);
        if (!owner() || piece.itemId == 0 || piece.retained == 0 || m_hooks.locked()) {
            return false;
        }
        const auto item = m_items.find(piece.itemId);
        if (!item) {
            return false;
        }
        HouseDrag drag;
        drag.furnIndex = furnIndex;
        drag.slot = 0;
        drag.itemId = piece.itemId;
        drag.direction = piece.rotation;
        drag.picture = m_interior.utensils().facingPicture(piece.itemId, piece.rotation);
        if (isWallPiece(piece.itemId)) {
            drag.turned = m_interior.utensils().facingPicture(piece.itemId, 1);
        }
        drag.size = m_interior.utensils().pictureSize(drag.picture);
        drag.grab = {drag.size.x / 2, drag.size.y / 2};
        drag.click = camera.toMap(mouse);
        if (!drag.picture.isValid()) {
            return false;
        }
        m_drag = drag;
        return true;
    }

    int HouseEditor::roomOfHeld() const {
        const FurnitureRecord& piece = m_interior.furniture().piece(m_drag->furnIndex);
        return piece.retained != 0 ? m_interior.room() : piece.room;
    }

    HouseGrid::Cell HouseEditor::placeCell(core::Point mouse, const Camera& camera) const {
        const HomeFurniture& furniture = m_interior.furniture();
        const core::Point middle = camera.toMap({mouse.x - m_drag->grab.x + m_drag->size.x / 2, mouse.y - m_drag->grab.y + m_drag->size.y / 2});
        HouseGrid::Cell cell = HouseGrid::screenToCell(middle);
        const auto item = m_items.find(m_drag->itemId);
        const int room = roomOfHeld();

        const int rotation = furniture.piece(m_drag->furnIndex).rotation;
        if (!item || (item->furnPlaceFlags & FurnitureKinds::kLaysOnPiece) == 0 || cell.x - HouseGrid::kGridColumn + 1 > HouseGrid::kGridWidth ||
            cell.y - HouseGrid::kGridRow + 1 > HouseGrid::kGridDepth) {
            return cell;
        }
        const auto span = furniture.spanOf(m_drag->itemId, rotation);
        const auto holds = [&](int piece) {
            const auto held = m_items.find(furniture.piece(piece).itemId);
            return held && (held->furnPlaceFlags & FurnitureKinds::kHoldsPieces) != 0;
        };
        for (int level = 1; level <= HomeFurniture::kLevels; ++level) {
            const int occupant = furniture.houseCell(cell.x, cell.y, level, room);
            if (occupant != 0 && occupant != m_drag->furnIndex && occupant >= 1 && occupant <= HomeFurniture::kMost) {
                if (!holds(occupant)) {
                    break;
                }
                bool fits = true;
                for (int x = cell.x; x <= cell.x + span->x - 1; ++x) {
                    for (int y = cell.y; y <= cell.y + span->y - 1; ++y) {
                        const int under = furniture.houseCell(x, y, level, room);
                        if (under < 1 || under > HomeFurniture::kMost || !holds(under)) {
                            fits = false;
                        }
                    }
                }
                if (!fits) {
                    break;
                }
            }
            if (occupant == 0 || occupant == m_drag->furnIndex) {
                bool fits = true;
                for (int x = cell.x; occupant == 0 && x <= cell.x + span->x - 1; ++x) {
                    for (int y = cell.y; y <= cell.y + span->y - 1; ++y) {
                        if (furniture.houseCell(x, y, level, room) != 0) {
                            fits = false;
                        }
                    }
                }
                if (fits) {
                    cell.z = level;
                }
                break;
            }
        }
        return cell;
    }

    void HouseEditor::dragTo(core::Point mouse, const Camera& camera) {
        if (!m_drag) {
            return;
        }
        if (isWallPiece(m_drag->itemId)) {
            const core::Point middle{mouse.x - m_drag->grab.x + m_drag->size.x / 2, mouse.y - m_drag->grab.y + m_drag->size.y / 2};
            if (const auto place = placeOnWall(m_drag->itemId, mouse, middle, m_drag->direction, camera)) {
                m_drag->drop = place->cell;
                m_drag->direction = place->direction;
            }
            return;
        }
        m_drag->drop = placeCell(mouse, camera);
    }

    bool HouseEditor::toolFitsPlatform(int toolItem, int platformPiece) {
        const auto item = m_items.find(toolItem);
        if (!item || item->cellWidth != 1 || item->cellHeight != 1) {
            m_hooks.notice("This item cannot be placed", 1000); // notice length in ms
            return false;
        }
        const HomeFurniture& furniture = m_interior.furniture();
        for (int place = 1; place <= kMostLinks; ++place) {
            const int linked = furniture.piece(platformPiece).links[static_cast<std::size_t>(place)];
            if (linked != 0 && furniture.piece(linked).itemId == kWorkPlatform) {
                m_hooks.notice("It cannot be placed because there is already one there", 1000); // notice length in ms
                return false;
            }
        }
        return true;
    }

    std::string HouseEditor::destroyText(int itemId) const {
        const auto item = m_items.find(itemId);
        if (item && item->rank > 1 && item->combineKind != 0) {
            return "This item will be #R#BRegenerated/#R/#B, are you sure? ";
        }
        if (isRented(itemId)) {
            return "The item will be #R#Bdestroyed by /#R/#B, and you will give up the right to have the furniture, are you sure? ";
        }
        return "This item will be #R#Bdestroyed/#R/#B now, are you sure? ";
    }

    void HouseEditor::dropPieceOnBin(int piece, int bin) {
        const int itemId = m_interior.furniture().piece(piece).itemId;
        const auto item = m_items.find(itemId);
        if (itemId == kWorkPlatform || itemId == kSweetDoor) {
            m_hooks.notice((item ? item->name : std::string()) + " Cannot take back", kNoticeMs);
            return;
        }
        m_hooks.confirm(destroyText(itemId), [this, piece, bin] { m_hooks.send(protocol::recycleFurnitureRequest(piece, bin)); });
    }

    void HouseEditor::dropItemOnBin(int bagSlot, int itemId, int count, int bin) { askRecycle(itemId, protocol::recycleItemRequest(bagSlot, count, bin)); }

    void HouseEditor::askRecycle(int itemId, core::Bytes request) {
        const auto item = m_items.find(itemId);
        if (itemId == kWorkPlatform || (item && item->type == kJobItemType)) {
            m_hooks.notice((item ? item->name : std::string()) + " Cannot take back", kNoticeMs);
            return;
        }
        m_hooks.confirm(destroyText(itemId), [this, request = std::move(request)] { m_hooks.send(request); });
    }

    bool HouseEditor::dropStoreroomItem(
        int cell,
        int itemId,
        int count,
        int piece,
        int targetSlot
    ) {
        const HouseUtensil& target = m_interior.utensils().utensil(targetSlot);
        if (!owner() || !m_items.find(itemId) || targetSlot == 0 || !isBin(target.itemId)) {
            return false;
        }
        askRecycle(itemId, protocol::storeroomRecycleRequest(cell, count, target.furnIndex, piece));
        return true;
    }

    void HouseEditor::release(core::Point mouse, int targetSlot, const Camera& camera) {
        if (!m_drag) {
            return;
        }
        const HouseDrag drag = *m_drag;
        m_drag.reset();
        const HomeFurniture& furniture = m_interior.furniture();
        const FurnitureRecord piece = furniture.piece(drag.furnIndex);
        if (m_hooks.locked() || piece.itemId == 0) {
            return;
        }
        if (m_hooks.queueBlocks(piece.itemId)) {
            m_hooks.notice("Cancel the movement", kQueueNoticeMs);
            return;
        }
        const HouseUtensil& target = m_interior.utensils().utensil(targetSlot);
        if (targetSlot != 0 && target.furnIndex != drag.furnIndex) {
            if (furniture.coveredByAnother(drag.furnIndex)) {
                m_hooks.notice("Please remove the upper instrument first", kLongNoticeMs);
                return;
            }
            if (target.itemId == kWorkPlatform) {
                if (toolFitsPlatform(piece.itemId, target.furnIndex)) {
                    m_hooks.send(protocol::toolOnPlatformRequest(drag.furnIndex, target.furnIndex));
                }
                return;
            }
            if (isBin(target.itemId)) {
                dropPieceOnBin(drag.furnIndex, target.furnIndex);
                return;
            }
        }
        const bool dropped = drag.drop.x != 0 || drag.drop.y != 0 || drag.drop.z != 0;
        if (!dropped && piece.retained != 0) {
            return;
        }
        const HouseGrid::Cell cell = dropped ? drag.drop : HouseGrid::Cell{piece.cellX, piece.cellY, piece.cellZ};
        const int rotation = isWallPiece(piece.itemId) ? drag.direction : piece.rotation;
        if (cell.x != piece.cellX || cell.y != piece.cellY || cell.z != piece.cellZ || rotation != piece.rotation) {
            if (furniture.coveredByAnother(drag.furnIndex)) {
                m_hooks.notice("Please remove the upper instrument first", kLongNoticeMs);
                return;
            }
            if (piece.itemId == FurnitureCarCases::kShowPlatformItem && m_interior.exhibit().itemId() != 0) {
                m_hooks.notice("Cannot move, there is an item on platform", kLongNoticeMs);
                return;
            }
            FurnitureRecord place = piece;
            place.cellX = cell.x;
            place.cellY = cell.y;
            place.cellZ = cell.z;
            place.rotation = rotation;
            if (piece.retained != 0) {
                place.room = m_interior.room();
            }
            if (furniture.moveAllowed(drag.furnIndex, place)) {
                m_hooks.send(protocol::moveFurnitureRequest(drag.furnIndex, cell.x, cell.y, cell.z, place.rotation));
            }
            return;
        }
        if (camera.toMap(mouse) != drag.click) {
            return;
        }
        const core::Point feet = m_hooks.playerFeet();
        const core::Point world = m_interior.utensils().utensil(drag.slot).world;
        if (std::abs(world.x - feet.x) > kReach || std::abs(world.y - feet.y) > kReach) {
            m_hooks.notice("too far away", kNoticeMs);
            return;
        }
        m_hooks.used(drag.furnIndex);
    }

    bool HouseEditor::wallStands(const HouseGrid::Cell& cell, int direction) const {
        const HouseRoomRect& room = m_interior.furniture().room(m_interior.room());
        if (room.left > cell.x || room.right <= cell.x || room.top > cell.y || room.bottom <= cell.y) {
            return false;
        }
        const int column = cell.x - HouseGrid::kGridColumn + 1;
        const int row = cell.y - HouseGrid::kGridRow + 1;
        if (direction == 0 && room.top < cell.y) {
            return m_interior.pillars().has(column, row - 1, m_interior.room());
        }
        if (direction == 1 && room.left < cell.x) {
            return m_interior.pillars().has(column - 1, row, m_interior.room());
        }
        return true;
    }

    void HouseEditor::gather(int slot) {
        const HomeFurniture& furniture = m_interior.furniture();
        const int index = m_interior.utensils().utensil(slot).furnIndex;
        const FurnitureRecord piece = furniture.piece(index);
        if (!owner()) {
            m_hooks.notice("Unable to operate and unable to put it at home", kLongNoticeMs);
            return;
        }
        if (piece.itemId == 0) {
            return;
        }
        if (furniture.coveredByAnother(index)) {
            m_hooks.notice("Please remove the upper instrument first", kLongNoticeMs);
            return;
        }
        std::vector<int> taken{piece.itemId};
        if (piece.itemId == kWorkPlatform) {
            for (int place = 1; place <= kMostLinks; ++place) {
                const int linked = piece.links[static_cast<std::size_t>(place)];
                if (linked != 0) {
                    taken.push_back(furniture.piece(linked).itemId);
                }
            }
        }
        if (!m_hooks.roomInBag(taken)) {
            m_hooks.notice("Unable to lay items any more.", 1000); // notice length in ms
            return;
        }
        m_hooks.send(protocol::takeBackFurnitureRequest(index));
    }

    void HouseEditor::rotate(int slot) {
        const HomeFurniture& furniture = m_interior.furniture();
        const int index = m_interior.utensils().utensil(slot).furnIndex;
        if (!owner()) {
            m_hooks.notice("Unable to operate and unable to put it at home", kLongNoticeMs);
            return;
        }
        if (m_interior.utensils().utensil(slot).itemId == 0) {
            return;
        }
        if (furniture.coveredByAnother(index)) {
            m_hooks.notice("Please remove the upper instrument first", kLongNoticeMs);
            return;
        }
        FurnitureRecord place = furniture.piece(index);
        place.rotation = place.rotation + 1 > 1 ? 0 : place.rotation + 1;
        if (furniture.moveAllowed(index, place)) {
            m_hooks.send(protocol::moveFurnitureRequest(index, place.cellX, place.cellY, place.cellZ, place.rotation));
        }
    }

    void HouseEditor::rightClick(int slot) {
        const HouseUtensil& utensil = m_interior.utensils().utensil(slot);
        const auto holds = [&](int places) {
            for (int place = 1; place <= places; ++place) {
                if (utensil.contentSlots[static_cast<std::size_t>(place)] != 0) {
                    return true;
                }
            }
            return false;
        };
        constexpr int kPutAway = 1500; // notice display time, ms
        const char* const putAway = "Put away the item and then bring the tool back ";
        const char* const clear = "Please clear it before taking it back.";
        if (FurnitureKinds::shelfKind(m_items, utensil.itemId) != 0 && holds(2)) { // places checked for a shelf or case
            m_hooks.notice(putAway, kPutAway);
            return;
        }
        if (FurnitureKinds::bedKind(m_items, utensil.itemId) != 0 && holds(4)) { // places checked for a bed
            m_hooks.notice(clear, kPutAway);
            return;
        }
        if (FurnitureKinds::cupboardKind(m_items, utensil.itemId) != 0 && holds(10)) { // places checked for a cupboard
            m_hooks.notice(clear, kPutAway);
            return;
        }
        if (FurnitureKinds::isVendingMachine(m_items, utensil.itemId) && holds(5)) { // places checked for a vending machine
            m_hooks.notice(putAway, kPutAway);
            return;
        }
        if (FurnitureKinds::isFortuneEgg(utensil.itemId) && holds(10)) { // places checked for a Fortune Egg machine
            m_hooks.notice(putAway, kPutAway);
            return;
        }
        if (utensil.itemId == FurnitureCarCases::kShowPlatformItem && m_interior.exhibit().cell() != 0) {
            m_hooks.send(protocol::takeOffPlatformRequest(m_interior.exhibit().cell()));
            return;
        }
        const auto item = m_items.find(utensil.itemId);
        if (!owner() || !item || m_hooks.locked() || (m_drag && m_drag->slot == slot)) {
            return;
        }
        const core::Point feet = m_hooks.playerFeet();
        if (std::abs(utensil.world.x - feet.x) > kReach || std::abs(utensil.world.y - feet.y) > kReach) {
            m_hooks.notice("too far away", kNoticeMs);
            return;
        }
        const bool canRotate = item->furnPicture1 != 0 && (item->furnPlaceFlags & FurnitureKinds::kOnWall) == 0;
        m_hooks.choose(
            "The selected instrument is #R#B" + item->name + "/#R/#B, You want? ",
            canRotate,
            [this, slot] { rotate(slot); },
            [this, slot] { gather(slot); }
        );
    }

    bool HouseEditor::dropBagItem(
        int bagSlot,
        int itemId,
        int count,
        core::Point mouse,
        int targetSlot,
        const Camera& camera
    ) {
        const HomeFurniture& furniture = m_interior.furniture();
        if (!owner() || !m_items.find(itemId)) {
            return false;
        }
        const auto kind = FurnitureKinds::checkHomeDiscard(m_items, itemId);
        const bool floor = FurnitureKinds::isFloorItem(m_items, itemId);
        const bool wallpaper = FurnitureKinds::isWallpaperItem(m_items, itemId);
        const HouseUtensil& target = m_interior.utensils().utensil(targetSlot);
        if (kind != FurnitureKinds::Discard::Placeable && targetSlot == 0 && !floor && !wallpaper) {
            const auto item = m_items.find(itemId);
            switch (kind) {
                case FurnitureKinds::Discard::Common:
                    m_hooks.notice("Common items cannot be discarded on the ground at home", 3000);
                    break; // common item: refused, notice 3000 ms
                case FurnitureKinds::Discard::SmallInstrument:
                    m_hooks.notice(item->name + " Please use it on the work platform", 3000);
                    break; // small instrument: use the work platform, notice 3000 ms
                case FurnitureKinds::Discard::SmallAccessory:
                    m_hooks.notice("Small parts cannot be discarded on the ground at home", 3000);
                    break; // small accessory: refused, notice 3000 ms
                default: break; // any other kind: nothing to say
            }
            return true;
        }
        if (targetSlot != 0) {
            if (m_hooks.shelfDrop && m_hooks.shelfDrop(bagSlot, target.furnIndex)) {
                return true;
            }
            if (target.itemId == kWorkPlatform) {
                const auto item = m_items.find(itemId);
                if (item->type != FurnitureKinds::kInstrumentType) {
                    m_hooks.notice("It is not an instrument", 1000); // notice length in ms
                } else if (toolFitsPlatform(itemId, target.furnIndex)) {
                    bool already = false;
                    for (int place = 1; place <= kMostLinks; ++place) {
                        const int linked = furniture.piece(target.furnIndex).links[static_cast<std::size_t>(place)];
                        already = already || (linked != 0 && furniture.piece(linked).itemId == itemId);
                    }
                    if (already) {
                        m_hooks.notice("It cannot be placed because there is already one there", 1000); // notice length in ms
                    } else {
                        m_hooks.send(protocol::toolOnPlatformRequest(bagSlot, target.furnIndex));
                    }
                }
                return true;
            }
            if (isBin(target.itemId)) {
                dropItemOnBin(bagSlot, itemId, count, target.furnIndex);
                return true;
            }
            return true;
        }
        if (floor) {
            m_hooks.send(protocol::useFloorItemRequest(bagSlot));
            return true;
        }
        if (wallpaper) {
            m_hooks.send(protocol::useWallItemRequest(bagSlot));
            return true;
        }
        if (furniture.freeSlot() == 0) {
            m_hooks.notice("The ground instruments have reached the limit", kNoticeMs);
            return true;
        }
        HouseGrid::Cell cell = cellUnder(mouse, camera);
        cell.z = 1;
        int direction = 0;
        const bool wall = isWallPiece(itemId);
        if (wall) {
            const auto place = placeOnWall(itemId, mouse, mouse, 0, camera);
            if (!place) {
                return true;
            }
            cell = place->cell;
            direction = place->direction;
        }
        FurnitureRecord record;
        record.itemId = itemId;
        record.room = m_interior.room();
        record.rotation = direction;
        if (furniture.placementBlocked(cell.x, cell.y, cell.z, record, 0)) {
            return true;
        }
        if (wall && !wallStands(cell, direction)) {
            return true;
        }
        m_hooks.send(protocol::placeFurnitureRequest(bagSlot, cell.x, cell.y, cell.z, direction));
        return true;
    }

}
