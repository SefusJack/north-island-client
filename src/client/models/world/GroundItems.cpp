#include "client/models/world/GroundItems.h"

#include "gfx/TextStyle.h"

#include <algorithm>
#include <vector>

namespace client {

    namespace {

        constexpr int kMostThings = 255; // most things lying on the ground (indexes 1..255)
        constexpr int kCellSize = 32; // bag cell size, px
        constexpr int kCellGap = 2; // gap between bag cells, px
        constexpr int kGoldKind = 2; // record kind 2: a pile of gold (otherwise an item)
        constexpr int kHoverLight = 15; // brightness added to the thing under the mouse
        constexpr int kGlintLight = 20; // brightness of the glint
        constexpr int kGlintFrames = 11; // frames in the glint picture
        constexpr int kGlintCycle = 101; // glint cycle length in steps (frames plus a pause)
        constexpr std::uint64_t kGlintStepMs = 25; // time per glint step, ms
        const core::Color kHintColor = core::Color::fromRgb565(65504); // RGB565 0xFFE0: yellow

        /// @brief Picks the picture name of a gold pile by its amount.
        /// @details Original: InitGdMoney.
        /// @param amount Gold amount.
        /// @return The picture name, GdMoney1 to GdMoney5.
        const char* goldPicture(std::int32_t amount) {
            // pile thresholds: under 100, 1000, 10000, 100000 give GdMoney1..4, otherwise GdMoney5
            if (amount < 100) {
                return "GdMoney1";
            }
            if (amount < 1000) {
                return "GdMoney2";
            }
            if (amount < 10000) {
                return "GdMoney3";
            }
            if (amount < 100000) {
                return "GdMoney4";
            }
            return "GdMoney5";
        }

    }

    void GroundItems::setList(const std::vector<protocol::GroundItemRecord>& records) {
        m_things.clear();
        for (const protocol::GroundItemRecord& record : records) {
            if (record.index < 1 || record.index > kMostThings) {
                continue;
            }
            Item item;
            item.position = record.position;
            item.extra = record.extra;
            if (record.kind == kGoldKind) {
                item.gold = true;
                item.amount = record.itemOrGold;
            } else {
                item.itemId = record.itemOrGold;
            }
            place(record.index, item);
        }
    }

    int GroundItems::addItem(int itemId, core::Point position, std::int32_t extra) {
        const int index = freeIndex();
        if (index != 0) {
            Item item;
            item.itemId = itemId;
            item.position = position;
            item.extra = extra;
            place(index, item);
        }
        return m_things.count(index) != 0 ? index : 0;
    }

    int GroundItems::addGold(std::int32_t amount, core::Point position) {
        const int index = freeIndex();
        if (index != 0) {
            Item item;
            item.gold = true;
            item.amount = amount;
            item.position = position;
            place(index, item);
        }
        return m_things.count(index) != 0 ? index : 0;
    }

    void GroundItems::setActive(int index, bool active) {
        if (const auto found = m_things.find(index); found != m_things.end()) {
            found->second.active = active;
        }
    }

    std::optional<GroundItems::Item> GroundItems::remove(int index) {
        const auto found = m_things.find(index);
        if (found == m_things.end()) {
            return std::nullopt;
        }
        const Item removed = found->second;
        m_things.erase(found);
        return removed;
    }

    int GroundItems::freeIndex() const {
        for (int index = 1; index <= kMostThings; ++index) {
            if (m_things.count(index) == 0) {
                return index;
            }
        }
        return 0;
    }

    void GroundItems::place(int index, Item item) {
        if (item.gold) {
            item.picture = m_pictures.find(goldPicture(item.amount)).value_or(m_pictures.find("1000").value_or(gfx::PictureId{}));
            item.flightPicture = item.picture;
            item.source = {0, 0, kCellSize, kCellSize};
            item.glints = true;
        } else {
            const auto record = m_items.find(item.itemId);
            if (!record) {
                return;
            }
            item.picture = m_pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{});
            item.flightPicture = m_pictures.find(std::to_string(record->iconId)).value_or(m_pictures.find("1000").value_or(gfx::PictureId{}));
            item.source = {0, 0, kCellSize, kCellSize};
            if ((record->cellWidth > 1 || record->cellHeight > 1) && item.picture.isValid()) {
                const gfx::Picture& picture = m_pictures.picture(item.picture);
                const int width = record->cellWidth * kCellSize + (record->cellWidth - 1) * kCellGap;
                const int height = record->cellHeight * kCellSize + (record->cellHeight - 1) * kCellGap;
                // the picture's 2-pixel border is left out when centring the item's box in it
                item.source = {std::max(0, (picture.width() - 2) / 2 - width / 2), std::max(0, (picture.height() - 2) / 2 - height / 2), width, height};
                item.pixelHit = true;
            }
            item.glints = record->type >= 1 && record->type <= 14; // item types 1..14 (weapons, armour) glint
        }
        m_things[index] = item;
    }

    core::Point GroundItems::topLeft(const Item& item, const Camera& camera) const {
        const core::Point at = camera.toScreen(item.position);
        return {at.x - item.source.width / 2, at.y - item.source.height / 2};
    }

    core::Rect GroundItems::areaOf(const Item& item, const Camera& camera) const {
        const core::Point at = topLeft(item, camera);
        return {at.x, at.y, item.source.width, item.source.height};
    }

    int GroundItems::indexAt(core::Point screenPoint, const Camera& camera) const {
        int found = 0;
        int foundY = 0;
        for (const auto& [index, item] : m_things) {
            if (!item.active) {
                continue;
            }
            const core::Point at = topLeft(item, camera);
            const core::Rect box{at.x, at.y, item.source.width, item.source.height};
            if (!box.contains(screenPoint)) {
                continue;
            }
            if (item.pixelHit && item.picture.isValid() &&
                !m_pictures.picture(item.picture).isOpaque(item.source.x + screenPoint.x - at.x, item.source.y + screenPoint.y - at.y)) {
                continue;
            }
            const int sortY = item.position.y + item.extra;
            if (found == 0 || sortY >= foundY) {
                found = index;
                foundY = sortY;
            }
        }
        return found;
    }

    void GroundItems::draw(
        gfx::Canvas& canvas,
        const Item& item,
        const Camera& camera,
        bool hovered,
        std::uint64_t nowMs
    ) const {
        if (!item.active) {
            return;
        }
        const core::Point at = topLeft(item, camera);
        canvas.drawPicture(item.picture, at, item.source);
        if (hovered) {
            canvas.drawPictureLight(item.picture, at, kHoverLight, item.source);
        }
        if (!item.glints) {
            return;
        }
        const int step = static_cast<int>((nowMs / kGlintStepMs + static_cast<std::uint64_t>(item.position.x)) % kGlintCycle);
        if (step < kGlintFrames) {
            if (const auto glint = m_pictures.find("blink")) {
                canvas.drawPictureLight(*glint, at, kGlintLight, core::Rect{0, step * kCellSize, kCellSize, kCellSize});
            }
        }
    }

    void GroundItems::drawHint(gfx::Canvas& canvas, int index, core::Point mouse) const {
        const auto found = m_things.find(index);
        if (found == m_things.end()) {
            return;
        }
        std::string text;
        if (found->second.gold) {
            text = std::to_string(found->second.amount) + " Gold";
        } else if (const auto record = m_items.find(found->second.itemId)) {
            text = record->name;
        }
        // hint sits 15 px right of the mouse; text width is 8 px per character, text height 12
        // hint sits 15 px right of the mouse; text width is 8 px per character; 12 is the font height
        canvas.drawText({mouse.x + 15, mouse.y}, text, {kHintColor, gfx::TextEffect::Shadow, std::nullopt}, static_cast<int>(text.size()) * 8, 12);
    }

}
