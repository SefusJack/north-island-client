#include "client/models/house/FurnitureStalls.h"

#include "client/rules/FurnitureKinds.h"

#include <string>

namespace client {

    namespace {

        constexpr int kShelf = 1; // shelf kind 1: Shelf (item 39059), the shelf an owner sells from
        constexpr int kStepX = 32; // px across between neighbouring places on a piece
        constexpr int kStepY = 15; // px down between neighbouring places on a piece
        constexpr core::Point kFacingLeft{0, -30}; // shelf item offset from the piece, px, piece facing left
        constexpr core::Point kFacingRight{-25, -15}; // shelf item offset from the piece, px, piece facing right
        constexpr core::Point kCaseFacingLeft{-18, 6 - 30}; // display case item offset, px, facing left (shelf offset plus -18, 6)
        constexpr core::Point kCaseFacingRight{10 - 25, 6 - 15}; // display case item offset, px, facing right (shelf offset plus 10, 6)
        constexpr int kCasePlaces = 2; // places on a display case
        constexpr int kDisplayCase = 3; // shelf kind 3: display case (Food Shelf, item 39086)
        // refusal texts by server code 1..7
        constexpr const char* kRefusals[] = {
            "The number of this furniture meets upper limit",
            "There's no egg in it",
            "The price has changed, cannot be used",
            "It is broken for a while",
            "It's full",
            "Not enough space, the pet cannot use the bed",
            "It's full!",
        };

    }

    FurnitureStalls::FurnitureStalls(
        gfx::PictureLibrary& pictures,
        const formats::ItemTable& items,
        const HomeFurniture& furniture,
        HouseUtensils& utensils,
        Hooks hooks
    )
        : m_pictures(pictures),
          m_items(items),
          m_furniture(furniture),
          m_utensils(utensils),
          m_hooks(std::move(hooks)) {}

    void FurnitureStalls::clear() {
        m_shelf = {};
        m_vending = {};
        m_cases = {};
        m_contents = {};
    }

    const StallInFurItem& FurnitureStalls::shelf(int number) const {
        static const StallInFurItem none;
        return number >= 1 && number <= kShelfItems ? m_shelf[static_cast<std::size_t>(number)] : none;
    }

    const StallInFurItem& FurnitureStalls::vending(int number) const {
        static const StallInFurItem none;
        return number >= 1 && number <= kVendingItems ? m_vending[static_cast<std::size_t>(number)] : none;
    }

    const StallInFurItem& FurnitureStalls::displayed(int number) const {
        static const StallInFurItem none;
        return number >= 1 && number <= kCases ? m_cases[static_cast<std::size_t>(number)] : none;
    }

    const StallCaseRecord& FurnitureStalls::ownCase(int number) const {
        static const StallCaseRecord none;
        return number >= 1 && number <= kCases ? m_ownCases[static_cast<std::size_t>(number)] : none;
    }

    void FurnitureStalls::place(StallInFurItem& item, int utensilSlot, int piece, const Offsets& offsets) {
        const auto pieceItem = m_items.find(m_furniture.piece(piece).itemId);
        if (utensilSlot != 0 && pieceItem) {
            const HouseUtensil& utensil = m_utensils.utensil(utensilSlot);
            const int across = (item.posInFurn - 1) * kStepX;
            const int down = (item.posInFurn - 1) * kStepY;
            const auto picture = [this](int number) { return m_pictures.find(std::to_string(number)).value_or(gfx::PictureId{}); };
            if (picture(pieceItem->furnPicture0) == utensil.picture) {
                item.world = {utensil.world.x + across + offsets.left.x, utensil.world.y + down + offsets.left.y};
            }
            if (picture(pieceItem->furnPicture1) == utensil.picture) {
                item.world = {utensil.world.x + across + offsets.right.x, utensil.world.y - down + offsets.right.y};
            }
        }
        const auto record = m_items.find(item.itemId);
        if (!record) {
            return;
        }
        item.picture = m_pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{});
        if (item.picture.isValid()) {
            const gfx::Picture& data = m_pictures.picture(item.picture);
            item.size = {data.width(), data.height()};
        }
    }

    void FurnitureStalls::put(const protocol::FurnitureStallItem& stalled) {
        const int pieceItem = m_furniture.piece(stalled.furnIndex).itemId;
        const int utensilSlot = m_utensils.slotOfPiece(stalled.furnIndex);
        const bool own = m_hooks.ownId() == stalled.ownerId;
        const auto fill = [&](StallInFurItem& item) {
            item = {};
            item.itemId = stalled.item.itemId;
            item.price = stalled.price;
            item.count = stalled.count;
            item.item = stalled.item;
            item.echoItem = stalled.echoItem;
            item.echoPrice = stalled.echoPrice;
            item.furnIndex = stalled.furnIndex;
            item.stallIndex = stalled.stallSlot;
            item.posInFurn = stalled.posInFurn;
        };
        const auto lock = [&](StallInFurItem& item) {
            if (utensilSlot != 0 && stalled.posInFurn >= 1 && stalled.posInFurn <= 10) { // a piece has at most 10 content places
                m_utensils.utensil(utensilSlot).contentSlots[static_cast<std::size_t>(stalled.posInFurn)] = stalled.stallSlot;
            }
            if (own) {
                item.gridSlot = stalled.gridCell;
                m_hooks.markBag(stalled.gridCell, true);
            }
        };
        if (FurnitureKinds::shelfKind(m_items, pieceItem) == kShelf && stalled.stallSlot >= 1 && stalled.stallSlot <= kShelfItems) {
            StallInFurItem& item = m_shelf[static_cast<std::size_t>(stalled.stallSlot)];
            fill(item);
            place(item, utensilSlot, stalled.furnIndex, {kFacingLeft, kFacingRight});
            lock(item);
        }
        if (FurnitureKinds::isVendingMachine(m_items, pieceItem) && stalled.stallSlot >= 1 && stalled.stallSlot <= kVendingItems) {
            StallInFurItem& item = m_vending[static_cast<std::size_t>(stalled.stallSlot)];
            fill(item);
            lock(item);
        }
    }

    void FurnitureStalls::empty(StallInFurItem& item, int utensilSlot, std::int32_t ownerId) {
        if (m_hooks.ownId() == ownerId && item.gridSlot != 0) {
            m_hooks.markBag(item.gridSlot, false);
            item.gridSlot = 0;
        }
        if (utensilSlot != 0 && item.posInFurn >= 1 && item.posInFurn <= 10) { // a piece has at most 10 content places
            m_utensils.utensil(utensilSlot).contentSlots[static_cast<std::size_t>(item.posInFurn)] = 0;
        }
        item.picture = {};
        item.itemId = 0;
        item.price = 0;
        item.count = 0;
    }

    void FurnitureStalls::remove(std::int32_t ownerId, int number, int furnIndex) {
        const int utensilSlot = m_utensils.slotOfPiece(furnIndex);
        const int pieceItem = utensilSlot != 0 ? m_utensils.utensil(utensilSlot).itemId : 0;
        const bool onShelf = FurnitureKinds::shelfKind(m_items, pieceItem) == kShelf && number >= 1 && number <= kShelfItems;
        const bool inMachine = !onShelf && FurnitureKinds::isVendingMachine(m_items, pieceItem) && number >= 1 && number <= kVendingItems;
        if (!onShelf && !inMachine) {
            return;
        }
        StallInFurItem& item = onShelf ? m_shelf[static_cast<std::size_t>(number)] : m_vending[static_cast<std::size_t>(number)];
        const int itemPiece = onShelf ? m_utensils.slotOfPiece(item.furnIndex) : utensilSlot;
        empty(item, itemPiece, ownerId);
        if (m_hooks.ownId() != ownerId) {
            m_hooks.reclaimed(furnIndex, item.posInFurn);
        }
    }

    void FurnitureStalls::take(std::int32_t ownerId, int number, int count, int furnIndex) {
        const int utensilSlot = m_utensils.slotOfPiece(furnIndex);
        const int pieceItem = utensilSlot != 0 ? m_utensils.utensil(utensilSlot).itemId : 0;
        const bool onShelf = FurnitureKinds::shelfKind(m_items, pieceItem) == kShelf && number >= 1 && number <= kShelfItems;
        const bool inMachine = !onShelf && FurnitureKinds::isVendingMachine(m_items, pieceItem) && number >= 1 && number <= kVendingItems;
        if (!onShelf && !inMachine) {
            return;
        }
        StallInFurItem& item = onShelf ? m_shelf[static_cast<std::size_t>(number)] : m_vending[static_cast<std::size_t>(number)];
        if (item.count <= count) {
            empty(item, onShelf ? m_utensils.slotOfPiece(item.furnIndex) : utensilSlot, ownerId);
        } else {
            item.count -= count;
        }
    }

    void FurnitureStalls::setCase(int number, const StallCaseRecord& record) {
        if (record.furnIndex < 1 || record.furnIndex > HouseUtensils::kMost) {
            return;
        }
        StallInFurItem& item = m_cases[static_cast<std::size_t>(number)];
        const int slot = m_utensils.slotOfPiece(record.furnIndex);
        item.itemId = record.itemId;
        item.price = record.price;
        item.count = record.count;
        item.furnIndex = record.furnIndex;
        item.stallIndex = number;
        item.posInFurn = record.posInFurn;
        item.item.itemId = record.itemId;
        item.item.count = record.count;
        if (slot != 0 && record.posInFurn >= 1 && record.posInFurn <= 10) { // a piece has at most 10 content places
            m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(record.posInFurn)] = number;
        }
        place(item, slot, record.furnIndex, {kCaseFacingLeft, kCaseFacingRight});
    }

    void FurnitureStalls::updateCase(const protocol::FurnitureCaseUpdate& update, bool inHouse) {
        const int number = update.caseNo;
        if (number < 1 || number > kCases) {
            return;
        }
        const bool own = m_hooks.ownId() == update.ownerId;
        const StallCaseRecord after{update.furnIndex, update.posInFurn, update.itemId, update.count, update.price};
        StallCaseRecord& kept = m_ownCases[static_cast<std::size_t>(number)];
        StallInFurItem& item = m_cases[static_cast<std::size_t>(number)];
        StallCaseRecord before;
        if (inHouse) {
            if (own) {
                before = kept.furnIndex != 0 ? kept : StallCaseRecord{};
            } else {
                before = {item.furnIndex, item.posInFurn, item.itemId, item.count, item.price};
            }
            if (after.itemId != 0) {
                const int slot = m_utensils.slotOfPiece(after.furnIndex);
                const int pieceItem = slot != 0 ? m_utensils.utensil(slot).itemId : m_furniture.piece(after.furnIndex).itemId;
                if (FurnitureKinds::shelfKind(m_items, pieceItem) == kDisplayCase) {
                    setCase(number, after);
                }
            } else {
                const int slot = m_utensils.slotOfPiece(kept.furnIndex);
                if (slot != 0 && kept.posInFurn >= 1 && kept.posInFurn <= 10) { // a piece has at most 10 content places
                    m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(kept.posInFurn)] = 0;
                }
                item = {};
            }
        }
        if (own) {
            kept = after;
        }
        if (m_hooks.caseChanged) {
            m_hooks.caseChanged({number, own, before, after});
        }
    }

    void FurnitureStalls::sellCase(const protocol::FurnitureCaseSale& sale, bool inHouse) {
        const int number = sale.caseNo;
        if (number < 1 || number > kCases) {
            return;
        }
        const bool own = m_hooks.ownId() == sale.ownerId;
        StallCaseRecord& kept = m_ownCases[static_cast<std::size_t>(number)];
        StallInFurItem& item = m_cases[static_cast<std::size_t>(number)];
        if (inHouse) {
            const int slot = m_utensils.slotOfPiece(sale.furnIndex);
            if (slot != 0 && FurnitureKinds::shelfKind(m_items, m_utensils.utensil(slot).itemId) == kDisplayCase) {
                const bool theirs = m_furniture.ownerId() == sale.ownerId;
                if (item.count <= sale.count) {
                    if (own) {
                        kept = {};
                    }
                    if (theirs) {
                        if (item.posInFurn >= 1 && item.posInFurn <= 10) { // a piece has at most 10 content places
                            m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(item.posInFurn)] = 0;
                        }
                        item.picture = {};
                        item.itemId = 0;
                        item.price = 0;
                        item.count = 0;
                    }
                } else {
                    if (own) {
                        kept.count -= sale.count;
                    }
                    if (theirs) {
                        item.count -= sale.count;
                    }
                }
            }
        } else if (own) {
            kept.count -= sale.count;
            if (kept.count <= 0) {
                kept = {};
            }
        }
    }

    void FurnitureStalls::relink(int furnIndex) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (slot == 0) {
            return;
        }
        const int pieceItem = m_utensils.utensil(slot).itemId;
        const int kind = FurnitureKinds::shelfKind(m_items, pieceItem);
        if (kind == kShelf) {
            for (int number = 1; number <= kShelfItems; ++number) {
                StallInFurItem& item = m_shelf[static_cast<std::size_t>(number)];
                if (item.itemId != 0 && item.furnIndex == furnIndex) {
                    place(item, slot, furnIndex, {kFacingLeft, kFacingRight});
                }
            }
        }
        if (kind == kDisplayCase) {
            for (int number = 1; number <= kCases; ++number) {
                StallInFurItem& item = m_cases[static_cast<std::size_t>(number)];
                if (item.itemId != 0 && item.furnIndex == furnIndex && item.posInFurn <= kCasePlaces) {
                    place(item, slot, furnIndex, {kCaseFacingLeft, kCaseFacingRight});
                }
            }
        }
    }

    const FurnitureStalls::Content& FurnitureStalls::content(int number) const {
        static const Content none;
        return number >= 1 && number <= kContents ? m_contents[static_cast<std::size_t>(number)] : none;
    }

    void FurnitureStalls::putContent(
        std::int32_t sellerId,
        int furnIndex,
        int place,
        int itemId,
        int count,
        int number
    ) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (sellerId != m_furniture.ownerId() || furnIndex <= 0 || place < 1 || place > 10 || itemId <= 0 || count <= 0 || number < 1 ||
            number > kContents || // a machine has at most 10 places
            slot == 0) {
            return;
        }
        m_contents[static_cast<std::size_t>(number)] = {furnIndex, place, itemId, count};
        m_utensils.utensil(slot).contentSlots[static_cast<std::size_t>(place)] = number;
    }

    void FurnitureStalls::setSalePrice(int furnIndex, std::int32_t price) {
        if (const int slot = m_utensils.slotOfPiece(furnIndex); slot != 0) {
            m_utensils.utensil(slot).salePrice = price;
        }
        if (m_hooks.priceChanged) {
            m_hooks.priceChanged(furnIndex, price);
        }
    }

    void FurnitureStalls::eggSold(int furnIndex, int place) {
        const int slot = m_utensils.slotOfPiece(furnIndex);
        if (m_hooks.ownId() != m_furniture.ownerId() || slot == 0 || place < 1 || place > 10) { // a machine has at most 10 places
            return;
        }
        HouseUtensil& utensil = m_utensils.utensil(slot);
        const int number = utensil.contentSlots[static_cast<std::size_t>(place)];
        if (number < 1 || number > kContents) {
            return;
        }
        Content& stack = m_contents[static_cast<std::size_t>(number)];
        if (utensil.salePrice > 0) {
            --stack.count;
        }
        if (stack.count == 0 || utensil.salePrice == 0) {
            utensil.contentSlots[static_cast<std::size_t>(place)] = 0;
            stack = {};
        }
    }

    void FurnitureStalls::setEggCount(int furnIndex, int count) {
        if (const int slot = m_utensils.slotOfPiece(furnIndex); slot != 0) {
            m_utensils.utensil(slot).eggCount = count;
        }
    }

    void FurnitureStalls::refuse(int code) {
        if (code >= 1 && code <= static_cast<int>(sizeof kRefusals / sizeof *kRefusals)) {
            m_hooks.notice(kRefusals[code - 1]);
        }
    }

}
