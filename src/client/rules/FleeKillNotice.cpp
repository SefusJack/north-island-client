#include "client/rules/FleeKillNotice.h"

#include <map>

namespace client {

    namespace {

        /// @brief One table entry of a flee-killing notice.
        struct Entry {
            bool chat;
            int durationMs;
            bool resetsCursor;
            const char* text;
        };

        /// @brief The notices of the flee-killing event by code.
        /// @details Original: MZ_Menu.pas RecvFleeKillEvent.
        /// @return Table from event code to notice.
        const std::map<int, Entry>& notices() {
            // event codes 1..15; chat lines have no duration, messages show 3000 ms
            static const std::map<int, Entry> table{
                {1,
                 {true,
                  0,
                  false,
                  "Flee killing event begin\241ALet's cherish our life as it is\241APlayers more than Level 10+ please go to the 4th floor of the Congress Building in Weilin and register"}},
                {2, {true, 0, false, "10 minutes later, this event will be over"}},
                {3, {true, 0, false, "Flee Killing event is overYou're welcome to come to again\241AThanks for your support"}},
                // per row: event code, {chat line?, duration ms, resets cursor?, text}
                {4, {false, 3000, false, "Please go to branch server 1 to join in the Flee Killing event"}},
                {5, {false, 3000, false, "The time of flee killing event is: \rFrom 7:00-8:00 PM every Tuesday"}},
                {6, {false, 3000, false, "Flee Killing Event"}},
                {7, {false, 3000, false, "Cannot be registered in the last 10 minutes"}},
                {8, {false, 3000, true, "The revive cannot be the onlooker"}},
                {9, {false, 3000, true, "Failure, cannot join in PK"}},
                {10, {false, 3000, true, "Failure, cannot join in fighting"}},
                {11, {false, 3000, true, "Failure, cannot team up"}},
                {12, {false, 3000, true, "Cannot team up with a failure"}},
                {13, {false, 3000, false, "One team member's item bar is full"}},
                {14, {false, 3000, false, "Someone has opened the treasure box"}},
                {15, {false, 3000, false, "This weeks flee killing event has been suspended"}},
            };
            return table;
        }

    }

    std::optional<FleeKillNotice> fleeKillNotice(int code) {
        const auto found = notices().find(code);
        if (found == notices().end()) {
            return std::nullopt;
        }
        return FleeKillNotice{found->second.chat, found->second.durationMs, found->second.resetsCursor, found->second.text};
    }

}