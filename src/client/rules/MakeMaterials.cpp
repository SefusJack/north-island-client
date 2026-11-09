#include "client/rules/MakeMaterials.h"

#include "core/ByteWriter.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kMostProducts = 50; // most products one make request may yield
        constexpr int kMostMaterials = 999; // most of one material one make request may use
        constexpr int kMostHave = 9999; // owned count shown for a material is capped at 9999

    }

    int MakeMaterials::bagSlotOf(int itemId) const {
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId == itemId && item.count > 0) {
                return slot;
            }
        }
        return 0;
    }

    MakeMaterials::BlueprintAt MakeMaterials::blueprintAt(int itemId) const {
        constexpr int kBag = 1; // blueprint place 1: the bag
        constexpr int kBookcase = 3; // blueprint place 3: the home bookcase
        constexpr int kDesk = 4; // blueprint place 4: the home desk
        if (const int slot = bagSlotOf(itemId); slot != 0) {
            return {kBag, slot};
        }
        if (const int place = m_shelves.placeOf(false, itemId); place != 0) {
            return {kBookcase, place};
        }
        if (const int place = m_shelves.placeOf(true, itemId); place != 0) {
            return {kDesk, place};
        }
        return {};
    }

    MakeCheck MakeMaterials::check(const formats::CompoundRecipe& recipe, int quantity) const {
        MakeCheck result;
        if (recipe.productCount > 0 && quantity * recipe.productCount > kMostProducts) {
            quantity = kMostProducts / recipe.productCount;
        }
        quantity = std::max(quantity, 0);
        for (int material = 1; material <= recipe.materialCount; ++material) {
            const int each = recipe.material(material).count;
            if (each > 0 && quantity * each > kMostMaterials) {
                quantity = kMostMaterials / each;
            }
        }
        result.quantity = quantity;
        for (int material = 1; material <= recipe.materialCount; ++material) {
            result.need[static_cast<std::size_t>(material)] = quantity * recipe.material(material).count;
        }
        std::array<int, formats::CompoundRecipe::kMaterials> left = result.need;

        core::ByteWriter items;
        int itemParts = 0;
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId == 0) {
                continue;
            }
            for (int material = 1; material <= recipe.materialCount; ++material) {
                const auto index = static_cast<std::size_t>(material);
                if (recipe.material(material).itemId != item.itemId || item.count <= 0 || !m_marks.isAvailable(slot)) {
                    continue;
                }
                result.have[index] += item.count;
                if (left[index] > 0) {
                    const int taken = std::min(left[index], item.count);
                    left[index] -= taken;
                    items.u8(static_cast<std::uint8_t>(slot)).u8(static_cast<std::uint8_t>(taken));
                    ++itemParts;
                }
            }
        }

        core::ByteWriter pieces;
        int pieceParts = 0;
        int seen = 0;
        for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
            const FurnitureRecord& record = m_furniture.piece(piece);
            if (record.itemId == 0 || record.parent != 0 || record.retained != 0) {
                continue;
            }
            for (int material = 1; material <= recipe.materialCount; ++material) {
                const auto index = static_cast<std::size_t>(material);
                if (recipe.material(material).itemId != record.itemId) {
                    continue;
                }
                if (m_furniture.coveredByAnother(piece)) {
                    continue;
                }
                result.have[index] += 1;
                if (left[index] > 0) {
                    left[index] -= 1;
                    pieces.u16(static_cast<std::uint16_t>(piece));
                    ++pieceParts;
                }
            }
            ++seen;
            if (m_furniture.count() <= seen) {
                break;
            }
        }

        result.ok = true;
        for (int material = 1; material <= recipe.materialCount; ++material) {
            const auto index = static_cast<std::size_t>(material);
            result.have[index] = std::min(result.have[index], kMostHave);
            result.enough[index] = left[index] == 0;
            if (left[index] != 0) {
                result.ok = false;
            }
        }

        core::ByteWriter request;
        request.u8(static_cast<std::uint8_t>(quantity));
        request.u8(0).u16(0).u8(0).u16(0); // u8, u16, u8, u16 request fields sent as zero
        request.u8(static_cast<std::uint8_t>(itemParts)).bytes(items.take());
        request.u8(static_cast<std::uint8_t>(pieceParts)).bytes(pieces.take());
        result.request = request.take();
        return result;
    }

}
