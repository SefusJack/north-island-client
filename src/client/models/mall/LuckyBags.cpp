#include "client/models/mall/LuckyBags.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "core/FileSystem.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kLuckyBagType = 41; // item type 41: lucky bags (2011 Big Box, 2011 Halloween Box, ...)
        constexpr std::uint64_t kRequestGapMs = 1000; // least gap between two content requests, ms
        constexpr std::uint8_t kLuckyBags = 91; // packet action 91: lucky bag contents
        constexpr std::uint8_t kAsk = 1; // sub 1: client asks for a bag's contents
        constexpr std::uint8_t kInfo = 2; // sub 2: server sends a bag's contents

    }

    LuckyBags::LuckyBags(
        std::filesystem::path file,
        std::function<void(const core::Bytes&)> send
    )
        : m_file(std::move(file)),
          m_send(std::move(send)) {}

    void LuckyBags::load() {
        m_bags.clear();
        m_lastRequestMs.reset();
        const auto data = core::readFile(m_file);
        // a file shorter than the 2-byte bag count is empty
        if (!data || data->size() < 2) {
            return;
        }
        core::ByteReader reader(*data);
        const int count = reader.u16();
        std::vector<int> rows;
        for (int i = 0; i < count; ++i) {
            rows.push_back(reader.u8());
        }
        for (const int row : rows) {
            Bag bag;
            bag.bagId = reader.u16();
            bag.itemCount = reader.u8();
            for (int j = 0; j < row; ++j) {
                const int itemId = reader.u16();
                const int itemCountOfRow = reader.u8();
                bag.items.push_back({itemId, itemCountOfRow});
            }
            m_bags.push_back(std::move(bag));
        }
    }

    void LuckyBags::save() const {
        const auto named = static_cast<int>(std::count_if(m_bags.begin(), m_bags.end(), [](const Bag& bag) { return bag.bagId > 0; }));
        if (named == 0) {
            return;
        }
        core::ByteWriter writer;
        writer.u16(static_cast<std::uint16_t>(named));
        for (const Bag& bag : m_bags) {
            if (bag.bagId > 0) {
                writer.u8(static_cast<std::uint8_t>(bag.items.size()));
            }
        }
        for (const Bag& bag : m_bags) {
            if (bag.bagId > 0) {
                writer.u16(static_cast<std::uint16_t>(bag.bagId)).u8(static_cast<std::uint8_t>(bag.itemCount));
                for (const GiftBoxContents::Entry& entry : bag.items) {
                    writer.u16(static_cast<std::uint16_t>(entry.itemId)).u8(static_cast<std::uint8_t>(entry.count));
                }
            }
        }
        core::writeFile(m_file, writer.take());
    }

    std::optional<std::reference_wrapper<const LuckyBags::Bag>> LuckyBags::find(int bagId) const {
        const auto found = std::find_if(m_bags.begin(), m_bags.end(), [bagId](const Bag& bag) { return bag.bagId == bagId; });
        if (found == m_bags.end()) {
            return std::nullopt;
        }
        return std::cref(*found);
    }

    bool LuckyBags::request(const formats::ItemTable& items, int bagId, std::uint64_t nowMs) {
        const auto item = items.find(bagId);
        if (!item || item->type != kLuckyBagType) {
            return false;
        }
        const auto known = find(bagId);
        if (known && known->get().current) {
            return true;
        }
        if (!m_lastRequestMs || nowMs - *m_lastRequestMs >= kRequestGapMs) {
            m_lastRequestMs = nowMs;
            core::ByteWriter writer;
            writer.u8(kLuckyBags).u8(kAsk).u16(static_cast<std::uint16_t>(bagId)).u8(static_cast<std::uint8_t>(known ? known->get().itemCount : 0));
            m_send(writer.take());
        }
        return false;
    }

    void LuckyBags::received(const core::Bytes& payload) {
        constexpr std::size_t kHeader = 4;
        constexpr std::size_t kRowSize = 3;
        // 4 is the header size: message id, sub id and the u16 bag id
        if (payload.size() < 4 || payload[1] != kInfo) {
            return;
        }
        // bag id is a little-endian u16 at bytes 2..3 (shift by 8 bits)
        const int bagId = payload[2] | (payload[3] << 8);
        auto bag = std::find_if(m_bags.begin(), m_bags.end(), [bagId](const Bag& candidate) { return candidate.bagId == bagId; });
        if (payload.size() < kHeader + 1) {
            if (bag != m_bags.end()) {
                bag->current = true;
            }
            return;
        }
        const std::size_t rows = (payload.size() - (kHeader + 1)) / kRowSize;
        if (rows == 0) {
            return;
        }
        if (bag == m_bags.end()) {
            auto free = std::find_if(m_bags.begin(), m_bags.end(), [](const Bag& candidate) { return candidate.bagId == 0; });
            if (free == m_bags.end()) {
                m_bags.emplace_back();
                free = m_bags.end() - 1;
            }
            bag = free;
        }
        bag->bagId = bagId;
        bag->itemCount = payload[kHeader];
        bag->items.clear();
        for (std::size_t row = 0; row < rows; ++row) {
            const std::size_t at = kHeader + 1 + row * kRowSize;
            // row: little-endian u16 item id (bytes 0..1, shift 8) and u8 count (byte 2)
            bag->items.push_back({payload[at] | (payload[at + 1] << 8), payload[at + 2]});
        }
        bag->current = true;
        save();
    }

}
