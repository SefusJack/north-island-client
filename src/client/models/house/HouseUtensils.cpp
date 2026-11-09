#include "client/models/house/HouseUtensils.h"

#include "client/models/house/HouseGrid.h"

#include <array>
#include <string>

namespace client {

    namespace {

        constexpr int kOnWall = 4; // bit 4 of an item's furnPlaceFlags: the piece hangs on a wall
        constexpr int kCoffeeTable = 39120; // Coffee Table
        constexpr int kSprayTub = 39025; // Shower
        constexpr int kLift = 39014; // Elevator
        constexpr int kFireplace = 39126; // Wall Furnace
        constexpr int kCracker = 39149; // New Year Cracker
        constexpr int kClock = 39152; // Gugu Clock

        /// @brief The animation a piece carries in one facing: its picture, offset, frame count and size, and start state.
        /// @details Original: AddHomeUtensil.
        struct AnimationSpec {
            int itemId;
            int rotation;
            const char* picture;
            int offsetX;
            int offsetY;
            int frames;
            int width;
            int height;
            int state;
        };

        // row: {item, facing (0 = left, 1 = right), picture name, x offset px, y offset px, frame count, width px, height px, start state}
        constexpr std::array<AnimationSpec, 8> kAnimations{{
            {kLift, 0, "Elevator_LB", 5, -112, 1, 74, 154, 0}, // Elevator, left facing
            {kFireplace, 0, "Fire_LB", 9, 2, 4, 30, 25, 2}, // Wall Furnace, left facing
            {kCracker, 0, "BianPao_LB", -8, -43, 8, 54, 82, 3}, // New Year Cracker, left facing
            {kClock, 0, "GuGuClock_L", -48, -55, 4, 78, 82, 0}, // Gugu Clock, left facing
            {kLift, 1, "Elevator_RB", -79, -112, 1, 74, 154, 0}, // Elevator, right facing
            {kFireplace, 1, "Fire_RB", -39, 2, 4, 30, 25, 2}, // Wall Furnace, right facing
            {kCracker, 1, "BianPao_RB", -46, -43, 8, 54, 82, 3}, // New Year Cracker, right facing
            {kClock, 1, "GuGuClock_R", -30, -55, 4, 78, 82, 0}, // Gugu Clock, right facing
        }};

        /// @brief How far down a piece stands on top of the given item.
        /// @details Original: GetFurnitureDrawOffset.
        /// @param itemId The item below.
        /// @return The offset in pixels (0, 0 for items not in the table).
        core::Point drawOffset(int itemId) {
            switch (itemId) {
                // the cases below give how many pixels (y) a piece stands lower on the item
                case 39078: // item 39078 (Gas Furnace): 25 down
                case 39150: return {0, 25}; // item 39150 (Cute Round Table): 25 down
                case 39020: // item 39020 (Desk): 20 down
                case 39085: return {0, 20}; // item 39085 (Glaze Table): 20 down
                case 39015: // item 39015 (Locker): 15 down
                case 39018: // item 39018 (Wardrobe): 15 down
                case 39023: // item 39023 (Fridge): 15 down
                case 39089: return {0, 15}; // item 39089 (Blue Window): 15 down
                case 39057: // item 39057 (Fortune Egg): 10 down
                case 39084: return {0, 10}; // item 39084 (Mailbox): 10 down
                case 39081: // item 39081 (large shelf, Big5 name): 5 down
                case 39112: return {0, 5}; // item 39112 (TV Cabinet): 5 down
                case 39157: return {0, -10}; // item 39157 (Potion Cabinet): 10 up
                default: return {0, 0}; // any other item: no offset
            }
        }

    }

    HouseUtensils::HouseUtensils(
        gfx::PictureLibrary& pictures,
        const formats::ItemTable& items,
        const HomeFurniture& furniture
    )
        : m_pictures(pictures),
          m_items(items),
          m_furniture(furniture) {}

    int HouseUtensils::slotOfPiece(int furnIndex) const {
        if (furnIndex < 1 || furnIndex > kMost) {
            return 0;
        }
        for (int slot = 1; slot <= kMost; ++slot) {
            if (m_utensils[static_cast<std::size_t>(slot)].furnIndex == furnIndex) {
                return slot;
            }
        }
        return 0;
    }

    bool HouseUtensils::standsIn(int itemId) const {
        for (std::size_t slot = 1; slot < m_utensils.size(); ++slot) {
            if (m_utensils[slot].itemId == itemId) {
                return true;
            }
        }
        return false;
    }

    core::Rect HouseUtensils::pictureArea(int slot, core::Point origin) const {
        constexpr int kPictureDrop = 15; // px the picture area sits lower than the piece's world point
        const HouseUtensil& utensil = m_utensils[static_cast<std::size_t>(slot)];
        if (!utensil.picture.isValid()) {
            return {};
        }
        const gfx::Picture& data = m_pictures.picture(utensil.picture);
        return {utensil.world.x - origin.x - data.width() / 2, utensil.world.y - origin.y - data.height() / 2 + kPictureDrop, data.width(), data.height()};
    }

    gfx::PictureId HouseUtensils::facingPicture(int itemId, int rotation) const {
        const auto item = m_items.find(itemId);
        if (!item || (rotation != 0 && rotation != 1)) {
            return {};
        }
        return m_pictures.find(std::to_string(rotation == 0 ? item->furnPicture0 : item->furnPicture1)).value_or(gfx::PictureId{});
    }

    core::Point HouseUtensils::pictureSize(gfx::PictureId picture) const {
        if (!picture.isValid()) {
            return {};
        }
        return {m_pictures.picture(picture).width(), m_pictures.picture(picture).height()};
    }

    bool HouseUtensils::isOpaque(gfx::PictureId picture, core::Point local) const {
        return picture.isValid() && m_pictures.picture(picture).isOpaque(local.x, local.y);
    }

    int HouseUtensils::add(const FurnitureRecord& record, int furnIndex, int slot, std::uint64_t nowMs) {
        if (furnIndex < 1 || furnIndex > kMost) {
            return 0;
        }
        const bool stands = record.parent == 0 && record.retained == 0;
        if (stands && slot <= 0) {
            for (int free = 1; free <= kMost && slot <= 0; ++free) {
                if (m_utensils[static_cast<std::size_t>(free)].itemId == 0) {
                    slot = free;
                }
            }
        }
        const auto item = m_items.find(record.itemId);
        if (stands && slot > 0 && item) {
            const core::Point at = HouseGrid::cellToScreen(record.cellX, record.cellY, record.cellZ);
            HouseUtensil& utensil = m_utensils[static_cast<std::size_t>(slot)];
            const HouseUtensil kept = utensil;
            utensil = {};
            utensil.contentSlots = kept.contentSlots;
            utensil.salePrice = kept.salePrice;
            utensil.eggCount = kept.eggCount;
            utensil.itemId = record.itemId;
            utensil.furnIndex = furnIndex;
            utensil.world = at;
            if (record.rotation == 0 || record.rotation == 1) {
                utensil.picture = facingPicture(record.itemId, record.rotation);
                if (record.itemId == kSprayTub) {
                    utensil.overlayPicture =
                        m_pictures.find(std::to_string(record.rotation == 0 ? item->furnPicture0 : item->furnPicture1) + "_a").value_or(gfx::PictureId{});
                }
                for (const AnimationSpec& spec : kAnimations) {
                    if (spec.itemId == record.itemId && spec.rotation == record.rotation) {
                        utensil.animPicture = m_pictures.find(spec.picture).value_or(gfx::PictureId{});
                        utensil.animOffset = {spec.offsetX, spec.offsetY};
                        utensil.animFrames = spec.frames;
                        utensil.animWidth = spec.width;
                        utensil.animHeight = spec.height;
                        utensil.animState = spec.state;
                        utensil.animMs = nowMs;
                        // the fireplace's far corner is 2 cells deep in the second facing
                        utensil.animCell = record.rotation == 1 ? core::Point{1, record.itemId == kFireplace ? 2 : item->furnSizeY}
                                                                : core::Point{item->furnSizeY, item->furnSizeX};
                    }
                }
            }
            m_slotOfPiece[static_cast<std::size_t>(furnIndex)] = slot;
        }
        if (slot > 0 && m_furniture.isCarpet(record.itemId)) {
            for (int place = 1; place <= kMost; ++place) {
                if (m_carpets[static_cast<std::size_t>(place)] == 0) {
                    m_carpets[static_cast<std::size_t>(place)] = slot;
                    break;
                }
            }
        }
        return stands && item ? slot : 0;
    }

    int HouseUtensils::remove(int furnIndex) {
        const int slot = slotOfPiece(furnIndex);
        if (slot == 0) {
            return 0;
        }
        const int itemId = m_utensils[static_cast<std::size_t>(slot)].itemId;
        m_slotOfPiece[static_cast<std::size_t>(furnIndex)] = 0;
        m_utensils[static_cast<std::size_t>(slot)] = {};
        if (m_furniture.isCarpet(itemId)) {
            for (int place = 1; place <= kMost; ++place) {
                if (m_carpets[static_cast<std::size_t>(place)] == slot) {
                    m_carpets[static_cast<std::size_t>(place)] = 0;
                    break;
                }
            }
        }
        return slot;
    }

    void HouseUtensils::clear() {
        m_utensils.fill({});
        m_slotOfPiece.fill(0);
        m_carpets.fill(0);
    }

    void HouseUtensils::setAnimState(int slot, int state) {
        if (slot >= 1 && slot <= kMost) {
            m_utensils[static_cast<std::size_t>(slot)].animState = state;
        }
    }

    void HouseUtensils::applyStackOffset(int slot) {
        if (slot < 1 || slot > kMost) {
            return;
        }
        HouseUtensil& utensil = m_utensils[static_cast<std::size_t>(slot)];
        if (utensil.itemId == 0 || utensil.furnIndex == 0) {
            return;
        }
        const FurnitureRecord& record = m_furniture.piece(utensil.furnIndex);
        const auto item = m_items.find(record.itemId);
        if (!item || (item->furnPlaceFlags & kOnWall) != 0 || record.cellZ <= 1) {
            return;
        }
        const auto span = m_furniture.spanOf(record.itemId, record.rotation);
        std::array<bool, kMost + 1> seen{};
        core::Point offset{0, 0};
        for (int x = record.cellX; span && x <= record.cellX + span->x - 1; ++x) {
            for (int y = record.cellY; y <= record.cellY + span->y - 1; ++y) {
                for (int layer = 1; layer <= record.cellZ - 1; ++layer) {
                    const int below = m_furniture.houseCell(x, y, layer, record.room);
                    if (below < 0 || below > kMost || seen[static_cast<std::size_t>(below)]) {
                        continue;
                    }
                    seen[static_cast<std::size_t>(below)] = true;
                    const int belowSlot = slotOfPiece(below);
                    const int belowItem = m_furniture.piece(below).itemId;
                    if (belowItem == kCoffeeTable) {
                        const core::Point lower = m_utensils[static_cast<std::size_t>(belowSlot)].world;
                        offset =
                            offset + core::Point{lower.x - utensil.world.x, lower.y - utensil.world.y + 7}; // the piece sits 7 pixels down on a coffee table
                    } else {
                        offset = offset + drawOffset(belowItem);
                    }
                }
            }
        }
        utensil.world = utensil.world + offset;
    }

}
