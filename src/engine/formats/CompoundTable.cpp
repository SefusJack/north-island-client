#include "formats/CompoundTable.h"

#include "core/ByteReader.h"

#include <unordered_map>

namespace formats {

    namespace {

        constexpr std::uint16_t kWordKey = 64444; // XOR key for stored 16-bit words (0xFBBC)
        constexpr std::uint8_t kByteKey = 211; // XOR key for stored bytes (0xD3)
        constexpr int kBias = 3; // subtracted from every value after the XOR
        constexpr int kHeaderProduct = 6; // the first (header) record holds the plain value 6 where recipes hold a product id
        constexpr int kGasStorage = 0x98D3; // product item 39123: Gas Storage (its recipe gets no facility)

        /// @brief Reads a stored 16-bit word and decodes it ((stored XOR key) - 3).
        /// @param reader Reader positioned at the word.
        /// @return The decoded value.
        int word(core::ByteReader& reader) { return static_cast<std::uint16_t>((reader.u16() ^ kWordKey) - kBias); }

        /// @brief Reads a stored byte and decodes it ((stored XOR 211) - 3).
        /// @param reader Reader positioned at the byte.
        /// @return The decoded value.
        int byte(core::ByteReader& reader) { return static_cast<std::uint8_t>((reader.u8() ^ kByteKey) - kBias); }

    }

    std::optional<CompoundTable> CompoundTable::parse(const core::Bytes& data) {
        // at least the header record plus one recipe
        if (data.size() < kRecordSize * 2 || data.size() % kRecordSize != 0) {
            return std::nullopt;
        }
        CompoundTable table;
        const std::size_t count = data.size() / kRecordSize;
        table.m_recipes.resize(count);
        for (std::size_t index = 0; index < count; ++index) {
            core::ByteReader reader(data, index * kRecordSize);
            CompoundRecipe& recipe = table.m_recipes[index];
            const int product = index == 0 ? static_cast<int>(reader.u16()) : word(reader);
            if (index == 0) {
                if (product != kHeaderProduct) {
                    return std::nullopt;
                }
                continue;
            }
            recipe.productId = product;
            recipe.blueprintItemId = word(reader);
            byte(reader);
            recipe.facilityId = word(reader);
            recipe.productCount = byte(reader);
            for (std::size_t material = 0; material < recipe.materials.size(); ++material) {
                recipe.materials[material].itemId = word(reader);
                recipe.materials[material].count = byte(reader);
            }
            recipe.materials[0] = {};
            recipe.materialCount = byte(reader);
            recipe.makeMinutes = word(reader);
            if (recipe.productId == kGasStorage) {
                recipe.facilityId = 0;
            }
        }
        return table;
    }

    const CompoundRecipe& CompoundTable::at(int index) const {
        static const CompoundRecipe none;
        return index >= 1 && index <= size() ? m_recipes[static_cast<std::size_t>(index)] : none;
    }

    void CompoundTable::setFacility(int index, int facilityId) {
        if (index >= 1 && index <= size()) {
            m_recipes[static_cast<std::size_t>(index)].facilityId = facilityId;
        }
    }

    int CompoundTable::indexOfProduct(int productId) const {
        for (int index = 1; index <= size(); ++index) {
            if (m_recipes[static_cast<std::size_t>(index)].productId == productId) {
                return index;
            }
        }
        return 0;
    }

    int CompoundTable::productOfBlueprint(int blueprintItemId) const {
        int product = 0;
        for (int index = 1; index <= size(); ++index) {
            if (m_recipes[static_cast<std::size_t>(index)].blueprintItemId == blueprintItemId) {
                product = m_recipes[static_cast<std::size_t>(index)].productId;
            }
        }
        return product;
    }

    int CompoundTable::findByFacility(int facilityId, int startIndex) const {
        for (int index = startIndex < 1 ? 1 : startIndex; index <= size(); ++index) {
            if (m_recipes[static_cast<std::size_t>(index)].facilityId == facilityId) {
                return index;
            }
        }
        return 0;
    }

}
