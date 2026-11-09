#include "client/rules/DisconnectNotice.h"

#include <map>

namespace client {

    namespace {

        /// @brief The reasons of DZ_Connect's action 0 that the game words.
        /// @return Table from reason number to the game's words.
        const std::map<int, const char*>& reasons() {
            // disconnect reason numbers and the game's words for them
            static const std::map<int, const char*> table{
                {1, "Disconnected"},
                {2, "Failed, offline"},
                {3, "Login failed 3 times"},
                {4, "Disconnected"},
                {5, "Foul found,offline"},
                {6, "Foul found,offline"},
                {7, "No event,offline"},
                {8, "Offline, Please Check"},
                {9, "False Edition"},
                {10, "False acts, offline"},
                {11, "On barrier, offline"},
                {12, "Offline, Pls Check"},
                {13, "Offline, Pls Check"},
                {14, "Offline, Pls Check"},
                {15, "Please restart"},
                {16, "Invalid IP,offline"},
                {17, "Update the edition"},
                {18, "Disconnected"},
                {19, "Repeated login,logoff"},
                {20, "Server error, offline"},
                {21, "Server error, offline"},
                {22, "Invalid packet format."},
                {23, "Offline when renaming"},
                {24, "Short password,logoff"},
                {25, "Name existed, offline"},
                {26, "Foul found, offline"},
                {27, "login error, offline"},
                {28, "Offline, Pls Check"},
                {29, "Too much info"},
                {30, "Account locked,logoff"},
                {31, "Invalid ID, offline"},
                {32, "Battle over a scene"},
                {33, "Invalid scene,offline"},
                {34, "Repeated login"},
                {35, "Login Agreement"},
                {36, "Invalid ID"},
                {37, "Invalid scene,offline"},
                {38, "Invalid scene,offline"},
                {40, "Offline, Pls Check"},
                {41, "Recharge after login"},
                {42, "Modify combat packet"},
                {43, "error of mode 0"},
                {44, "Not agree with scene"},
                {45, "Account is closed"},
                {46, "Foul found"},
                {47, "Battle not finished"},
                {48, "Illegally use spell"},
                {49, "Login after curfew"},
                {50, "login to the server"},
                {51, "Unable to login"},
                {52, "Unable to login"},
                {53, "Server not released"},
                {54, "Competition Over"},
                {55, "doesn't use vehicle"},
                {56, "Long password,offline"},
                {57, "Same password,offline"},
                {58, "Close beta on Nov. 18"},
                {59, "Kicked out byRelaySer"},
                {60, "Disconnected"},
                {61, "Server is busy now."},
                {62, "Invalid code,offline"},
                {63, "Disconnected"},
                {64, "Invalid program"},
                {65, " Invalid item version"},
                {66, "Invalid IP zone"},
                {67, "Disconnected"},
                {68, "Invalid IP Role"},
                {69, "Using no war contract"},
                {70, "Safe protection off"},
                {71, "Illegal vehicle"},
                {72, "Invalid item,offline"},
                {73, "Reset Instance"},
                {74, "PvP Server Contact failed"},
                {75, "The amount of users on the same IP address has reached the upper limit"},
                {150, "Check Account Fault"},
            };
            return table;
        }

        /// @brief Tells whether a reason puts its words behind the number.
        /// @param reason The disconnect reason.
        /// @return True for the reasons that name themselves.
        bool namesItself(int reason) {
            switch (reason) {
                case 3:
                case 15:
                case 17:
                case 19:
                case 31:
                case 41:
                case 45:
                case 51:
                case 53:
                case 54:
                case 58:
                case 73:
                case 255: // reasons 3, 15, 17, 19, 31, 41, 45, 51, 53, 54, 58, 73 and 255 name themselves
                    return true;
                default: // reasons 60..68 name themselves too
                    return reason >= 60 && reason <= 68;
            }
        }

        /// @brief Tells whether a reason offers the Prev. and Quit buttons.
        /// @param reason The disconnect reason.
        /// @return True when the form offers Prev. and Quit.
        bool offersPrevious(int reason) {
            switch (reason) {
                case 2:
                case 19:
                case 25:
                case 37:
                case 38:
                case 41:
                case 43:
                case 44:
                case 47:
                case 48:
                case 70: // reasons 2, 19, 25, 37, 38, 41, 43, 44, 47, 48 and 70 offer Prev. and Quit
                    return true;
                default: // reasons 4..15, 50..51, 53..55, 58..62 and 72..73 offer it too
                    return (reason >= 4 && reason <= 15) || (reason >= 50 && reason <= 51) || (reason >= 53 && reason <= 55) ||
                           (reason >= 58 && reason <= 62) || (reason >= 72 && reason <= 73);
            }
        }

    }

    DisconnectNotice disconnectNotice(int reason, std::int32_t playerId) {
        constexpr int kOther = 255; // reason 255: unknown reason, "Intermit to connect to the Server"
        const auto found = reasons().find(reason);
        std::string words;
        if (found != reasons().end()) {
            words = found->second;
        } else {
            words = "Intermit to connect to the Server";
            reason = kOther;
        }
        DisconnectNotice notice;
        // player ids 5..16 are the game's own ids, which always get the reason's words
        notice.text =
            (playerId >= 5 && playerId <= 16) || namesItself(reason) ? std::to_string(reason) + ":" + words : "Disconnection:" + std::to_string(reason);
        if (offersPrevious(reason)) {
            notice.buttons = DisconnectNotice::Buttons::PreviousAndQuit;
        } else if (reason == 17) { // reason 17: update the edition
            notice.buttons = DisconnectNotice::Buttons::UpdateAndQuit;
        }
        return notice;
    }

}
