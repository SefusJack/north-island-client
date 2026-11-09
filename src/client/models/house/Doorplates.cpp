#include "client/models/house/Doorplates.h"

#include "core/ByteReader.h"

namespace client {

    namespace {

        constexpr int kMostRecords = 1000; // most doorplate records read from one list message
        constexpr std::size_t kMessageHead = 2; // bytes of main id and sub id before the records

    }

    void Doorplates::clear(std::int32_t playerId) { m_texts.erase(playerId); }

    void Doorplates::receiveList(const core::Bytes& payload) {
        core::ByteReader reader(payload, kMessageHead);
        for (int record = 0; record < kMostRecords && reader.remaining() > 0; ++record) {
            const std::int32_t playerId = reader.i32();
            const std::string text = reader.shortText();
            if (reader.failed()) {
                return;
            }
            m_texts[playerId] = text;
        }
    }

    const std::string& Doorplates::text(std::int32_t playerId) const {
        static const std::string kNone;
        const auto found = m_texts.find(playerId);
        return found != m_texts.end() ? found->second : kNone;
    }

}
