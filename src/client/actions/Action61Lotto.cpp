#include "client/actions/Action61Lotto.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <optional>
#include <string>

namespace client {

    namespace {

        /// @brief Looks up the short Lotto notice text of 61/5.
        /// @param code Message code read from the payload (1..9).
        /// @param durationMs Receives how long the notice is shown, in ms.
        /// @return The notice text, or nullopt for an unknown code.
        std::optional<std::string> lottoMessage(int code, int& durationMs) {
            durationMs = 1200; // default notice duration in ms
            switch (code) {
                case 1: // code 1: draw-time notice (long)
                    durationMs = 2200; // longer notice duration in ms
                    return "Prizes can be claimed from 20:00 on draw day until 18:00 on the next draw day!";
                // code 2: notice text
                case 2: return "You are carrying too much money.";
                // code 3: notice text
                case 3: return "You did not win this Lotto draw.";
                // code 4: notice text
                case 4: return "This draw has not been held yet.";
                // code 5: notice text
                case 5: return "Betting has closed. Please come back for the next draw!";
                // code 6: notice text
                case 6: return "You have no tickets left to claim. Please wait for the next draw.";
                // code 7: notice text
                case 7: return "Please claim your tickets from the last draw before betting.";
                // code 8: notice text
                case 8: return "The Lotto machine is under maintenance. Please wait!";
                case 9: return "Send presents sometimes because your inventory is full."; // code 9: English notice, inventory full
                default: return std::nullopt; // unknown code: no notice
            }
        }

    }

    void handleAction61Lotto(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: // 61/1: RecvLottoTicket, fills the unseen board
            case 2: // 61/2: RecvLottoValue, fills the unseen board
            case 3: // 61/3: RecvLottoBetResult, fills the unseen board
            case 7: // 61/7: RecvLottoHistoryRow, fills the unseen board
            case 8: // 61/8: RecvLottoPrizeTier, fills the unseen board
            case 9: // 61/9: RecvLottoNumbers, fills the unseen board
                return;
            case 4: // 61/4: chat line, the draw numbers are out
                context.events.onChat(0, 0, "The Lotto numbers are out. Check them at the casino.");
                return;
            case 5: { // 61/5: u8 code, a short notice (lottoMessage)
                const int code = reader.u8();
                int durationMs = 0;
                if (!reader.failed()) {
                    if (const auto text = lottoMessage(code, durationMs)) {
                        context.events.onNoticeMessage(*text, durationMs);
                    }
                    return;
                }
                break;
            }
            // 61/10: RecvLottoJackpotWinner, chat line with the winner's name
            case 10: context.events.onChat(0, 0, "Congratulations to this draw's Lotto jackpot winner: " + reader.rest()); return;
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
