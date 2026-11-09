#include "client/models/items/GiftCardFile.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "core/FileSystem.h"

namespace client {

    std::vector<GiftCard> GiftCardFile::read(const std::filesystem::path& path) {
        std::vector<GiftCard> cards;
        const auto bytes = core::readFile(path);
        if (!bytes) {
            return cards;
        }
        core::ByteReader reader(*bytes);
        const int count = reader.u16();
        for (int index = 0; index < count && !reader.failed(); ++index) {
            GiftCard card;
            card.id = reader.u16();
            card.version = reader.u8();
            const int choices = reader.u8();
            for (int choice = 0; choice < choices && !reader.failed(); ++choice) {
                GiftCard::Choice entry;
                entry.itemId = reader.u16();
                entry.count = reader.u8();
                card.choices.push_back(entry);
            }
            if (reader.failed()) {
                break;
            }
            cards.push_back(std::move(card));
        }
        return cards;
    }

    bool GiftCardFile::write(const std::filesystem::path& path, const std::vector<GiftCard>& cards) {
        core::ByteWriter writer;
        int count = 0;
        for (const GiftCard& card : cards) {
            if (card.id != 0) {
                ++count;
            }
        }
        writer.u16(static_cast<std::uint16_t>(count));
        for (const GiftCard& card : cards) {
            if (card.id == 0) {
                continue;
            }
            writer.u16(static_cast<std::uint16_t>(card.id)).u8(static_cast<std::uint8_t>(card.version)).u8(static_cast<std::uint8_t>(card.choices.size()));
            for (const GiftCard::Choice& choice : card.choices) {
                writer.u16(static_cast<std::uint16_t>(choice.itemId)).u8(static_cast<std::uint8_t>(choice.count));
            }
        }
        return core::writeFile(path, writer.take());
    }

}
