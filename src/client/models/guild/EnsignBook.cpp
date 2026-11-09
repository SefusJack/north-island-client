#include "client/models/guild/EnsignBook.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "core/FileSystem.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::size_t kRecordSize = 296; // file record: id 4 + badge block 288 + revision 1 + 3 unused bytes
        constexpr std::uint64_t kSendIntervalMs = 1000; // ms between requests for missing badges
        constexpr std::size_t kMostAsked = 10; // most badge ids asked for in one request
        constexpr std::uint8_t kEmptyByte = 255; // byte value of an empty badge pixel pair

        /// @brief A badge block with every pixel empty.
        /// @return The empty block.
        formats::EmblemBlock emptyBlock() { return formats::EmblemBlock::parse(core::Bytes(formats::EmblemBlock::kBytes, kEmptyByte)); }

    }

    int EnsignBook::indexOf(std::uint32_t id) const {
        for (std::size_t index = 0; index < m_ensigns.size(); ++index) {
            if (m_ensigns[index].id == id) {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    std::optional<std::reference_wrapper<const EnsignBook::Ensign>> EnsignBook::find(std::uint32_t id) const {
        const int index = indexOf(id);
        if (index < 0) {
            return std::nullopt;
        }
        return std::cref(m_ensigns[static_cast<std::size_t>(index)]);
    }

    void EnsignBook::setList(const std::vector<std::pair<std::uint32_t, int>>& listed, const std::filesystem::path& file) {
        m_ensigns.clear();
        for (const auto& [id, revision] : listed) {
            m_ensigns.push_back({id, revision, emptyBlock()});
        }
        loadFile(file);
    }

    void EnsignBook::loadFile(const std::filesystem::path& file) {
        m_queue.clear();
        for (const Ensign& ensign : m_ensigns) {
            m_queue.push_back(ensign.id);
        }
        const auto bytes = core::readFile(file);
        if (!bytes) {
            return;
        }
        core::ByteReader reader(*bytes);
        while (reader.remaining() >= kRecordSize) {
            const std::uint32_t id = reader.u32();
            const formats::EmblemBlock data = formats::EmblemBlock::parse(reader.bytes(formats::EmblemBlock::kBytes));
            const int revision = reader.u8();
            reader.skip(3); // three unused bytes at the end of a record
            const int index = indexOf(id);
            if (index < 0) {
                continue;
            }
            if (m_ensigns[static_cast<std::size_t>(index)].revision == revision) {
                m_ensigns[static_cast<std::size_t>(index)].data = data;
                m_queue.erase(std::remove(m_queue.begin(), m_queue.end(), id), m_queue.end());
            } else {
                m_ensigns.erase(m_ensigns.begin() + index);
            }
        }
    }

    void EnsignBook::save(const std::filesystem::path& file) const {
        if (m_ensigns.empty()) {
            return;
        }
        core::ByteWriter writer;
        for (const Ensign& ensign : m_ensigns) {
            writer.u32(ensign.id)
                .bytes(ensign.data.bytes())
                .u8(static_cast<std::uint8_t>(ensign.revision))
                .u8(0)
                .u8(0)
                .u8(0); // three unused bytes at the end of a record
        }
        std::error_code error;
        std::filesystem::create_directories(file.parent_path(), error);
        core::writeFile(file, writer.take());
    }

    void EnsignBook::received(std::uint32_t id, int revision, const formats::EmblemBlock& data, const std::filesystem::path& file) {
        const int index = indexOf(id);
        if (index >= 0) {
            m_ensigns[static_cast<std::size_t>(index)].revision = revision;
            m_ensigns[static_cast<std::size_t>(index)].data = data;
        } else {
            m_ensigns.push_back({id, revision, data});
        }
        save(file);
    }

    void EnsignBook::setOwn(std::uint32_t guildId, int revision, const formats::EmblemBlock& data) {
        const int index = indexOf(guildId);
        if (index >= 0) {
            m_ensigns[static_cast<std::size_t>(index)].revision = revision;
            m_ensigns[static_cast<std::size_t>(index)].data = data;
        } else {
            m_ensigns.push_back({guildId, revision, data});
        }
    }

    void EnsignBook::replaceData(std::uint32_t id, const formats::EmblemBlock& data) {
        const int index = indexOf(id);
        if (index >= 0) {
            m_ensigns[static_cast<std::size_t>(index)].data = data;
        }
    }

    std::vector<std::uint32_t> EnsignBook::takeRequest(std::uint64_t nowMs) {
        std::vector<std::uint32_t> ids;
        if (m_queue.empty()) {
            return ids;
        }
        if (m_tickMs == 0) {
            m_tickMs = nowMs;
        }
        if (nowMs - m_tickMs < kSendIntervalMs) {
            return ids;
        }
        m_tickMs = nowMs;
        const std::size_t count = std::min(m_queue.size(), kMostAsked);
        ids.assign(m_queue.begin(), m_queue.begin() + static_cast<std::ptrdiff_t>(count));
        m_queue.erase(m_queue.begin(), m_queue.begin() + static_cast<std::ptrdiff_t>(count));
        return ids;
    }

}
