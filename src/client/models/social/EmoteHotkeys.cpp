#include "client/models/social/EmoteHotkeys.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "core/FileSystem.h"
#include "core/Log.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kSittingAction = 13; // action 13: sit down (default for hotkey slot 1)
        constexpr std::size_t kPhraseBytes = 30; // bytes per saved phrase in the file (length byte plus text)

    }

    bool EmoteHotkeys::load(const std::filesystem::path& file) {
        m_slots.fill({});
        m_phrases.fill({});
        const auto data = core::readFile(file);
        if (!data) {
            m_slots[0] = {EmoteSlot::Kind::Action, kSittingAction};
            return false;
        }
        core::ByteReader reader(*data);
        for (EmoteSlot& slot : m_slots) {
            slot.number = reader.i32();
            const std::int32_t kind = reader.i32();
            // file kind 1: emotion, 2: action, else empty
            slot.kind = kind == 1 ? EmoteSlot::Kind::Emotion : kind == 2 ? EmoteSlot::Kind::Action : EmoteSlot::Kind::Empty;
        }
        for (std::string& phrase : m_phrases) {
            const std::size_t start = reader.position();
            const std::size_t length = reader.u8();
            phrase = reader.text(std::min(length, kPhraseLength));
            reader.seek(start + kPhraseBytes);
        }
        if (!reader.atEnd()) {
            m_heartRadar = reader.u8();
        }
        if (reader.failed()) {
            core::logWarning("the hotkey file ", file.string(), " is short");
        }
        return true;
    }

    void EmoteHotkeys::save(const std::filesystem::path& file) const {
        core::ByteWriter writer;
        for (const EmoteSlot& slot : m_slots) {
            writer.i32(slot.number).i32(static_cast<std::int32_t>(slot.kind));
        }
        for (const std::string& phrase : m_phrases) {
            writer.u8(static_cast<std::uint8_t>(phrase.size()));
            for (std::size_t i = 0; i < kPhraseLength; ++i) {
                writer.u8(i < phrase.size() ? static_cast<std::uint8_t>(phrase[i]) : 0);
            }
        }
        writer.u8(m_heartRadar);
        if (!core::writeFile(file, writer.take())) {
            core::logWarning("cannot write ", file.string());
        }
    }

    void EmoteHotkeys::setPhrase(int index, const std::string& text) { m_phrases[static_cast<std::size_t>(index - 1)] = text.substr(0, kPhraseLength); }

}
