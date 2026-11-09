#include "client/models/mail/SavedMail.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

#include <algorithm>

namespace client {

    namespace {

        /// @brief Reads a Delphi short string of a fixed size.
        /// @param reader Reader positioned at the length byte.
        /// @param size Capacity of the string field.
        /// @return The text.
        std::string readShortString(core::ByteReader& reader, std::size_t size) {
            const std::size_t length = std::min<std::size_t>(reader.u8(), size);
            std::string text = reader.text(length);
            reader.skip(size - length);
            return text;
        }

        /// @brief Writes a Delphi short string of a fixed size, padded with zeros.
        /// @param writer Writer to append to.
        /// @param text Text to write (cut to size).
        /// @param size Capacity of the string field.
        void writeShortString(core::ByteWriter& writer, const std::string& text, std::size_t size) {
            const std::string kept = text.substr(0, size);
            writer.u8(static_cast<std::uint8_t>(kept.size())).text(kept).text(std::string(size - kept.size(), '\0'));
        }

    }

    SavedMail SavedMail::decode(const core::Bytes& data, std::size_t offset) {
        core::ByteReader reader(data, offset);
        SavedMail mail;
        mail.kind = reader.u8();
        mail.read = reader.u8() != 0;
        mail.targetId = reader.i32();
        mail.targetName = readShortString(reader, kLongestName);
        mail.text = readShortString(reader, kLongestText);
        mail.sendTime = reader.f64();
        return mail;
    }

    core::Bytes SavedMail::encode() const {
        core::ByteWriter writer;
        writer.u8(static_cast<std::uint8_t>(kind)).u8(read ? 1 : 0).i32(targetId);
        writeShortString(writer, targetName, kLongestName);
        writeShortString(writer, text, kLongestText);
        writer.f64(sendTime);
        return writer.take();
    }

}
