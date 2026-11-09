#include "protocol/GmQuestion.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    std::optional<GmQuestion> decodeGmQuestion(const core::Bytes& payload) {
        GmQuestion result;
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        result.kind = reader.u8();
        result.question = reader.shortText();
        for (int i = 0; i < 3; ++i) { // three answers carry a length byte; the fourth is the rest
            result.choices.push_back(reader.shortText());
        }
        result.choices.push_back(reader.rest());
        if (reader.failed()) {
            return std::nullopt;
        }
        return result;
    }

    core::Bytes gmAnswerRequest(int kind, int answer) {
        const int option = kind == 1 ? 1 : kind == 199 ? 2 : 0; // 199: question kind answered with option 2
        if (option == 0) {
            return {};
        }
        core::ByteWriter writer;
        writer.u8(16).u8(1).u8(static_cast<std::uint8_t>(option)).i32(answer); // C->S 16/1: answer to the GM question
        return writer.take();
    }

}
