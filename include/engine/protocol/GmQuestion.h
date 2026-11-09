#ifndef NORTH_ISLAND_PROTOCOL_GM_QUESTION_H
#define NORTH_ISLAND_PROTOCOL_GM_QUESTION_H

#include "core/Bytes.h"

#include <optional>
#include <string>
#include <vector>

namespace protocol {

    /// @brief S->C 16/1: the GM's question to a player (u8 kind, u8 length and the question, three answers of u8 length and text, the rest is the fourth answer).
    /// @details Original: DZ_Connect.pas, TAC_Question; the kind is Player.FGmQuestion.
    struct GmQuestion {
        int kind = 0;
        std::string question;
        std::vector<std::string> choices;
    };

    /// @brief Decodes S->C 16/1.
    /// @param payload Packet payload.
    /// @return The question, or nothing when malformed.
    std::optional<GmQuestion> decodeGmQuestion(const core::Bytes& payload);

    /// @brief Builds C->S 16/1 (original SendServerMsg): u8 option and the answer as i32.
    /// @details The kind 1 asks as option 1, the kind 199 as option 2; any other kind sends nothing, so the bytes are empty.
    /// @param kind The question's kind.
    /// @param answer The choice's number from 1, or 255 when the question was closed unanswered.
    /// @return The encoded request.
    core::Bytes gmAnswerRequest(int kind, int answer);

}

#endif
