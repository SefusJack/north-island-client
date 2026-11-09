#include "client/actions/Action20Event.h"

#include "client/actions/Action54Christmas.h"
#include "client/actions/EventNotice.h"
#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    namespace {

        constexpr int kShortMs = 2000; // notice display time, ms
        const char* const kFourPeople = "Please make a team with four people before doing that";
        const char* const kServerBusy = "The server is busy, please try later";

        /// @brief Shows the notices for the TGC contest reply (20/42): exp, registration, cancel and level-up.
        /// @details Original: TEventManage.RecvTgcContestReply (20/42).
        /// @param payload Raw message bytes (action, sub id, command, fields).
        /// @param context Connection and event sinks of the current session.
        /// @return True when the message was understood.
        bool handleContestReply(const core::Bytes& payload, MessageContext& context) {
            core::ByteReader reader(payload, 2);
            const int command = reader.u8();
            switch (command) {
                case 1: { // command 1: i32 exp points gained
                    const std::int32_t value = reader.i32();
                    if (reader.failed()) {
                        return false;
                    }
                    context.events.onNoticeMessage("Exp rises " + std::to_string(value) + " points", kShortMs);
                    context.events.onFloatText(false, "ExpUP");
                    return true;
                }
                case 2: { // command 2: u8 code, the registration result
                    const int code = reader.u8();
                    if (reader.failed()) {
                        return false;
                    }
                    const char* text = "RegisterUnsuccessfully";
                    switch (code) {
                        case 0: text = "RegisterSuccessfully"; break; // code 0: registered
                        case 1:
                            text = "Someone in the team has signed up for the TGC contest and the register failed";
                            break; // code 1: someone in the team already signed up
                        case 2: text = kFourPeople; break; // code 2: the team needs four people
                        case 5: text = kServerBusy; break; // code 5: the server is busy
                        default: break; // other codes: registration failed
                    }
                    context.events.onNoticeMessage(text, kShortMs);
                    return true;
                }
                case 3: { // command 3: u8 code, the cancel result
                    const int code = reader.u8();
                    if (reader.failed()) {
                        return false;
                    }
                    const char* text = "CancelUnsuccessfully";
                    switch (code) {
                        case 0: text = "CancelSuccessfully"; break; // code 0: cancelled
                        case 1:
                            text = "Someone in the team hasn't signed up for the TGC contest before, failed to cancel";
                            break; // code 1: someone in the team never signed up
                        case 2: text = kFourPeople; break; // code 2: the team needs four people
                        case 3:
                            text = "Unable to cancel because it is different from the registered member";
                            break; // code 3: differs from the registered member
                        case 4: text = kServerBusy; break; // code 4: the server is busy
                        default: break; // other codes: cancel failed
                    }
                    context.events.onNoticeMessage(text, kShortMs);
                    return true;
                }
                case 4: // command 4: registration finished
                    context.events.onNoticeMessage("TGC registration finished", kShortMs);
                    return true;
                case 5: // command 5: i32 level, contest level up
                case 7: { // command 7: i32 level, the other level-up flavour
                    const std::int32_t level = reader.i32();
                    if (reader.failed()) {
                        return false;
                    }
                    context.events.onContestLevelUp(command == 7, level); // command 7 is the second flavour
                    return true;
                }
                case 6: // command 6: level has reached the upper limit
                    context.events.onNoticeMessage("Level has reached the upper limit", kShortMs);
                    return true;
                default: // unknown command: not handled
                    return false;
            }
        }

        /// @brief Shows an event message that only tells the player something, from the notice table or with numbers from the packet.
        /// @details Original: DZ_Connect.pas action 20, subs 19 to 61.
        /// @param sub Sub id of the action 20 message.
        /// @param payload Raw message bytes.
        /// @param context Connection and event sinks of the current session.
        /// @return False for a sub that needs more than a notice.
        bool handleEventNotice(int sub, const core::Bytes& payload, MessageContext& context) {
            core::ByteReader reader(payload, 2);
            const int first = reader.u8();
            if (const auto notice = EventNotice::find(sub, first)) {
                if (notice->get().kind == EventNotice::Kind::Short) {
                    context.events.onNoticeMessage(std::string(notice->get().text), notice->get().durationMs);
                } else {
                    context.events.onOwnSystemLine(std::string(notice->get().text));
                }
                return true;
            }
            switch (sub) {
                case 19: { // 20/19: u16 amount, recharge successful
                    reader.seek(2); // back to byte 2, after the action and sub id
                    const int amount = reader.u16();
                    if (reader.failed()) {
                        return false;
                    }
                    context.events.onSoundEffect("sound/wav0152.wav");
                    context.events.onNoticeMessage(std::to_string(amount) + " points. " + "Recharge successful ", kShortMs);
                    return true;
                }
                case 20: // 20/20: event announcement
                    context.events.onEventAnnounce();
                    return true;
                case 21: // 20/21: Team PK Competition notice
                    context.events.onNoticeMessage("There is Team PK Competition from 7:00pm-9:00 every Sunday.", kShortMs);
                    return true;
                case 22: // 20/22: u8 points, ability points obtained
                case 23: // 20/23: u8 points, spell points obtained
                    context.events.onSoundEffect("sound/wav0152.wav");
                    context.events.onNoticeMessage(
                        std::string(sub == 22 ? "Obtain ability points" : "Obtain spell points") + std::to_string(first) + " Points",
                        kShortMs
                    ); // 20/22 ability, 20/23 spell points
                    return true;
                case 26: { // 20/26: u8 place, i32 player, name: the Corton place
                    const std::int32_t playerId = reader.i32();
                    const std::string name = reader.shortText();
                    if (reader.failed()) {
                        return false;
                    }
                    context.events.onCortonPlace(first, playerId, name);
                    return true;
                }
                case 31: { // 20/31: u8 count, the competition's places come
                    std::string line = "Capture the Evil Corton: ";
                    for (int place = 1; place <= first; ++place) { // places are numbered from 1
                        line += "The " + std::to_string(place) + " place in the competition  ";
                    }
                    context.events.onOwnSystemLine(line + "Has come.");
                    return true;
                }
                case 53: // 20/53: a talk notice
                    // 31519 = talk text id, 5000 = duration in ms
                    context.events.onTalkNotice(31519, 5000);
                    return true;
                case 58: // 20/58: u8 first, buy-level flow (1/2 block forms, 3/5 ask, 6 result)
                    if (first == 1 || first == 2) { // 1 and 2: forms blocked and freed
                        return true;
                    }
                    if (first == 3 || first == 5) { // 3 and 5: the buy-level question (5 is the second flavour)
                        context.events.onBuyLevelAsked(first == 5);
                        return true;
                    }
                    if (first == 6) { // 6: u8 code, the buy-level result
                        const int code = reader.u8();
                        if (reader.failed()) {
                            return false;
                        }
                        context.events.onBuyLevelResult(code);
                        return true;
                    }
                    return false;
                case 54: { // 20/54: the Christmas event, forwarded to action 54
                    const core::Bytes shifted{54, static_cast<std::uint8_t>(first)}; // 54 = action id of the Christmas handler
                    handleAction54Christmas(shifted, context);
                    return true;
                }
                case 55: // 20/55: u8 first, the list of players who passed the 20th round (first 4)
                    if (first == 4) { // first 4: the list follows
                        reader.seek(3); // the list starts at byte 3, after action, sub id and first
                        context.events.onOwnSystemLine("Congratulations" + reader.rest() + "The above players have passed the 20th rounds tests");
                        return true;
                    }
                    return false;
                default: // other subs: not handled here
                    return false;
            }
        }

    }

    void handleAction20Event(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 20/1..6: one result of the running event
            case 2: // 20/2: event result
            case 3: // 20/3: event result
            case 4: // 20/4: event result
            case 5: // 20/5: event result
            case 6: { // 20/6: event result
                core::ByteReader reader(payload, 2);
                // skips u8, u16, u8 before the kind (4 bytes)
                reader.skip(1 + 2 + 1);
                EventResult result;
                result.kind = reader.u8();
                result.mode = reader.u8();
                result.target = reader.u16();
                result.option = reader.u8();
                result.value = reader.i32();
                result.dataId = reader.u16();
                if (!reader.failed()) {
                    context.events.onEventResult(result);
                }
                break;
            }
            case 8: // 20/8: the event is over
                context.events.onEventFinished();
                break;
            case 33: { // 20/33: u8 hidden flag (Player.FNpcsHidden)
                core::ByteReader reader(payload, 2);
                const bool hidden = reader.u8() == 1; // 1 = hide the NPC 38010
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onNpcsHidden(hidden);
                break;
            }
            case 34: { // 20/34: u16 previous map, u16 door word (Player.FPrevMapId, FDoorWord)
                core::ByteReader reader(payload, 2);
                const int previousMap = reader.u16();
                reader.u16();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPreviousMap(previousMap);
                break;
            }
            case 44: { // 20/44: i32 player, u8 mode (RecvWaitEventIcon)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                const int mode = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onWaitEventIcon(playerId, mode);
                break;
            }
            case 46: { // 20/46: u16 map NPC index (StartTreasureNpcJump)
                core::ByteReader reader(payload, 2);
                const int npcIndex = reader.u16();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onTreasureNpcJump(npcIndex);
                break;
            }
            // 20/48: the movie is finished (FinishMovie)
            case 48: context.events.onMovieFinish(); break;
            case 42: // 20/42: the TGC contest reply (RecvTgcContestReply)
                if (handleContestReply(payload, context)) {
                    break;
                }
                context.events.onUnhandled(id);
                break;
            case 19: // 20/19..61 (except those above): a notice from handleEventNotice; 20/19 recharge successful
            case 20: // 20/20: event announcement
            case 21: // 20/21: Team PK Competition notice
            case 22: // 20/22: ability points obtained
            case 23: // 20/23: spell points obtained
            case 24: // 20/24: event notice
            case 25: // 20/25: event notice
            case 26: // 20/26: the Corton place
            case 27: // 20/27: event notice
            case 28: // 20/28: event notice
            case 29: // 20/29: event notice
            case 30: // 20/30: event notice
            case 31: // 20/31: the competition's places
            case 32: // 20/32: event notice
            case 35: // 20/35: event notice
            case 36: // 20/36: event notice
            case 37: // 20/37: event notice
            case 38: // 20/38: event notice
            case 39: // 20/39: event notice
            case 40: // 20/40: event notice
            case 41: // 20/41: event notice
            case 43: // 20/43: event notice
            case 45: // 20/45: event notice
            case 47: // 20/47: event notice
            case 49: // 20/49: event notice
            case 50: // 20/50: event notice
            case 51: // 20/51: event notice
            case 52: // 20/52: event notice
            case 53: // 20/53: a talk notice
            case 54: // 20/54: the Christmas event
            case 55: // 20/55: players who passed the 20th round
            case 56: // 20/56: event notice
            case 57: // 20/57: event notice
            case 58: // 20/58: buy-level flow
            case 60: // 20/60: event notice
            case 61: // 20/61: event notice
                if (handleEventNotice(id.sub, payload, context)) {
                    break;
                }
                context.events.onUnhandled(id);
                break;
            case 7: // 20/7: go on (event signal)
            case 9: // 20/9: event signal (stop waiting, wait or a notice)
            case 10: // 20/10: event signal
            case 11: // 20/11: event signal
            case 12: // 20/12: event signal
            case 13: // 20/13: event signal
            case 14: // 20/14: event signal
            case 15: // 20/15: event signal
            case 16: // 20/16: event signal
            case 17: // 20/17: event signal
            case 18: { // 20/18: event signal
                context.events.onEventSignal(id.sub, 0);
                break;
            }
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
