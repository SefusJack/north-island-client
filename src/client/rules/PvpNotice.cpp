#include "client/rules/PvpNotice.h"

#include <map>

namespace client {

    namespace {

        /// @brief One worded notice: chat or short message, duration, text.
        struct Entry {
            bool chat;
            int durationMs;
            const char* text;
        };

        /// @brief The notices of ShowPvpEventNotice by code.
        /// @return The table.
        const std::map<int, Entry>& pvpEvents() {
            // table: code -> {chat line?, duration in ms (0 for chat), text}
            static const std::map<int, Entry> table{
                {1, {true, 0, "Players Level 10+ please go to the 2F, the capitol in Welling Village. The team PK will start in 6 minutes."}},
                {2,
                 {true,
                  0,
                  "Players Level 10+ please go to the 3F, the Capitol in Welling Village. You can register your name. The top 10 winners will get different badges. The top 3 will get special equipment Players Level 10+ please go to the 4F,the Capitol in Welling Village and register"}},
                {3, {true, 0, "Weekly team PK will end in 5 minutes, please try your best to get good marks"}},
                {4, {true, 0, "Weekly team PK is over. Hope you join in the event next time"}},
                {5, {false, 2000, "Player less than Level 10 cannot join in the PK"}},
                {6, {false, 2000, "Some of your team haven't reached Level 10, cannot join in the PK"}},
                {7, {false, 2000, "Please to go to the other servers"}},
                {8, {false, 2000, "Ready for PK"}},
                {9, {false, 2000, "The PK field has been cleared now and at 21:30 will be open for all"}},
                {10, {false, 2000, "Target is making a match, cannot join in"}},
                {11, {false, 2000, "Making a match, cannot join in"}},
                {12, {true, 0, "Cancel team PK this week"}},
                {13, {true, 0, "Resume team PK this week"}},
                {14, {false, 2000, "Suspend team PK once and delay to next week"}},
                {15, {true, 0, "Your item bar is full. Please guarantee enough space for our reissuing next maintainace"}},
                {16, {false, 2000, "Pet's amity is less than 40, cannot join in the PK"}},
                {17, {false, 2000, "The PK event haven't begun yet, please wait"}},
                {18, {false, 2000, "4 player PK event is canceled. We will post the time of the next one on our official website"}},
                {19, {false, 2000, "Please go to branch server 1 to join in the Team PK event"}},
                {20, {false, 0, "Auto matching after single PK....."}},
                {21, {false, 0, "Auto matching after party PK....."}},
                {22, {true, 0, "More great new rewards from the Group PvP!  Come join in now for some great prizes!"}},
                {23, {true, 0, "Group PvP has ended! Good luck to everyone next year!"}},
                {51, {true, 0, "The PvP in the PvP Server will begin in 5 minutes, all players whose level are 30+ can join in"}},
                {52, {true, 0, "The PvP in the PvP Server begins! Please go to the PvP Clerk to register."}},
                {53, {true, 0, "The PvP in the PvP Server will be ending in 5 minutes. Good luck! "}},
                {54, {true, 0, "The PvP in the PvP Server has ended, thanks for your stopping by!"}},
                {55, {false, 2000, "Players less than level 30 cannot join the PvP Server "}},
                {56, {false, 2000, "Prepares the scene for PvP Server "}},
                {57, {false, 2000, "The event has ended, thanks for stopping by!"}},
                {58, {false, 2000, "The PvP on the PvP Server this week has been canceled"}},
                {59, {false, 2000, "The PvP on the PvP Server will be held as usual"}},
                {60, {false, 2000, "The PvP on the PvP Server will be on hold. It will be held in next week"}},
                {61, {true, 0, "The PvP has begun! Please go to branch 1 in each server and find the PvP registrar to sign up. "}},
                {62, {true, 0, "The PvP has begun! If your level is 30+, you can go to the 2F of the Capitol to visit the PvP Clerk and register"}},
                {63, {false, 2000, "Today, the team cannot join the PvP Server"}},
                {64, {false, 2000, "Players whose levels are less than 30 cannot join the PvP Server"}},
                {65, {false, 2000, "One or more team members are below level 30, cannot join PvP"}},
                {66, {true, 0, "Attention please, the system will be disconnected in 5 minutes"}},
                {67, {true, 0, "The PvP on the PvP Server has ended, thanks for stopping by! Please go to your original server and wait for the results."}},
                {68,
                 {true,
                  0,
                  "The PvP on the PvP Server will begin at 4:00pm. Players can go to the 2F of the Capitol to visit the PvP Clerk to register and go to the PK grounds"}},
                {69, {false, 3000, "This scene is full, please try again later"}},
                {70, {false, 3000, "The server is busy right now, please try again later"}},
                {71,
                 {true,
                  0,
                  "The PvP Clerk and the PvP event has released in appointed scene, if you want to join it, you can go to the 2F OF THE Capitol to visit the PvP Clerk and then go to the event scene"}},
                {72, {true, 0, "Please go back to the original server to get your rewards"}},
                {73, {true, 0, "The PvP ranks have been released, go have a look."}},
            };
            return table;
        }

        /// @brief The notices of Show4PlayerPkNotice by code.
        /// @return The table.
        const std::map<int, Entry>& fourPlayerPk() {
            // table: code -> {chat line?, duration in ms (0 for chat), text}
            static const std::map<int, Entry> table{
                {1, {true, 0, "4 player PK event will start in 5 minutes. Please go to the PK field"}},
                {2, {true, 0, "Wish all of you get good marks in the PK"}},
                {3, {true, 0, "4 player PK event will start in 5 minutes.Please try your best to get a good mark"}},
                {4, {true, 0, "4 player PK event is over. We will proclaim the list soon"}},
                {5, {false, 2000, "Special event for 4P, please choose another server"}},
                {6, {false, 2000, "The leader of the target team isn't the one registered"}},
                {7, {false, 2000, "The number of target team isn't the one registered"}},
                {8, {false, 2000, "You haven't registered"}},
                {9, {false, 2000, "Exclusive server, cannot delete character"}},
            };
            return table;
        }

        /// @brief The notices of RecvArenaNotice by code.
        /// @return The table.
        const std::map<int, Entry>& arena() {
            // table: code -> {chat line?, duration in ms (0 for chat), text}
            static const std::map<int, Entry> table{
                {1, {true, 0, "Dear players, the Arena Competition begins now. We welcome every player."}},
                {2, {true, 0, "Dear players, the Arena Competition is over now. Wait for the next event."}},
                {3, {true, 0, "Dear players, the Arena Competition is over now. Remember to get your rewards from the sexy Prize Girl."}},
                {4, {true, 0, "Today's Arena Battle event is over. Everyone is welcome to take on the challenge again next time."}},
                {5, {true, 0, "Today's Arena Battle event has begun. Please enter the arena from Zhuojun."}},
                {6, {true, 0, "--Arena Battle: Team PK-- has begun. Everyone is welcome to join!"}},
                {7, {true, 0, "--Arena Battle: Free-for-all-- has begun. Everyone is welcome to join!"}},
            };
            return table;
        }

        /// @brief Looks a code up in a notice table.
        /// @param table The table.
        /// @param code The code.
        /// @return The notice, none when absent.
        std::optional<PvpNotice> noticeOf(const std::map<int, Entry>& table, int code) {
            const auto found = table.find(code);
            if (found == table.end()) {
                return std::nullopt;
            }
            return PvpNotice{found->second.chat, found->second.durationMs, found->second.text};
        }

    }

    std::optional<PvpNotice> pvpEventNotice(int code) { return noticeOf(pvpEvents(), code); }

    std::optional<PvpNotice> fourPlayerPkNotice(int code) { return noticeOf(fourPlayerPk(), code); }

    std::optional<PvpNotice> arenaNotice(int code) { return noticeOf(arena(), code); }

}