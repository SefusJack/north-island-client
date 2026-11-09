#ifndef NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCES_H
#define NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCES_H

#include "client/models/instances/InstanceCreateRow.h"
#include "client/models/instances/InstanceGoal.h"
#include "client/models/instances/InstanceInfo.h"
#include "client/models/instances/InstanceMember.h"
#include "client/models/instances/InstanceRow.h"
#include "core/Bytes.h"
#include "formats/MarkTable.h"
#include "formats/NpcTable.h"
#include "formats/SceneInfo.h"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Model of the instance windows: open instances, the player's own instance, its members, goals and time left.
    /// @details Original: MZ_Menu.pas TDupMisForm, TCreateDupMisForm, TDupDetailForm (without windows) and the player's FDupState
    /// (0 none, 1 waiting, 2 started). Sends the requests of action 85 and takes in the server's answers; DupMisManage (MZ_DupMis.pas) is the table of instance scenes.
    class Instances {
        public:
            static constexpr int kRowsPerPage = 5; // instances listed on one page
            static constexpr int kMembersPerPage = 5; // members listed on one page
            static constexpr int kGoals = 11; // goal lines of an instance, numbered 1..11

            /// @brief Tab of the instance window.
            /// @details Original: FCurTab.
            enum class Tab { Info = 1, Members = 2, Choose = 3 }; // tab numbers of the instance window

            /// @brief Callbacks into the rest of the client (network, scenes, own player, windows).
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<std::optional<formats::SceneInfo>(int)> scene;
                std::function<std::vector<int>()> scenes;
                std::function<bool(int)> isInstanceMap;
                std::function<std::int32_t()> ownId;
                std::function<std::string()> ownName;
                std::function<int()> ownMap;
                std::function<bool()> fighting;
                std::function<bool()> inGuild;
                std::function<bool()> cannotTeleport;
                std::function<std::optional<std::string>(std::int32_t)> playerName;
                std::function<void(const std::string&, int)> message;
                std::function<void(const std::string&)> chat;
                std::function<bool()> detailOpen;
                std::function<void()> openDetail;
                std::function<void()> closeDetail;
                std::function<bool()> windowOpen;
                std::function<void()> showWindow;
                std::function<void()> createListChanged;
            };

            /// @brief Formats minutes as "1 Hour 5 minute" or "15 minute".
            /// @details Original: FormatMinutesText.
            /// @param minutes Number of minutes.
            /// @return The text.
            static std::string minutesText(int minutes);

            /// @brief Creates the model.
            /// @param marks Mark table (names and texts of instances).
            /// @param npcs NPC table (names of goals).
            /// @param hooks Callbacks into the client.
            Instances(
                const formats::MarkTable& marks,
                const formats::NpcTable& npcs,
                Hooks hooks
            );

            /// @brief Tells whether the player is in the map of an instance.
            /// @details Original: IsDupMissionMap(Player.FMapId).
            /// @return True when in an instance map.
            bool here() const;

            /// @brief Tells whether the player is fighting.
            /// @return True when fighting.
            bool fighting() const;

            /// @brief State of the player's instance.
            /// @details Original: Player.FDupState; 0 none, 1 waiting for the start, 2 started.
            /// @return The state.
            int state() const { return m_state; }

            /// @brief Info of the player's own instance.
            /// @details Original: DupMisForm.FDupDetail.
            /// @return The info.
            const InstanceInfo& own() const { return m_own; }

            /// @brief Info of the instance being looked at.
            /// @details Original: DupDetailForm.FDupDetail.
            /// @return The info.
            const InstanceInfo& detail() const { return m_detail; }

            /// @brief Players listed in the detail window.
            /// @return The members.
            const std::vector<InstanceMember>& detailMembers() const { return m_detailMembers; }

            /// @brief A goal of the own instance (goals 1..11 at 1..11).
            /// @param index Goal number.
            /// @return The goal.
            const InstanceGoal& goal(int index) const { return m_goals[static_cast<std::size_t>(index)]; }

            /// @brief NPC id of the boss.
            /// @return The NPC id.
            int bossNpc() const { return m_bossNpc; }

            /// @brief Number of boss kills.
            /// @return The count.
            int bossKills() const { return m_bossKills; }

            /// @brief Tab shown in the instance window.
            /// @return The tab.
            Tab tab() const { return m_tab; }

            /// @brief Rows of the open instances list page.
            /// @return The rows.
            const std::vector<InstanceRow>& rows() const { return m_rows; }

            /// @brief Current page of the list.
            /// @return Page number.
            int orderPage() const { return m_orderPage; }

            /// @brief Number of pages of the list.
            /// @return Page count.
            int orderPages() const { return m_orderPages; }

            /// @brief Start number of the own instance, as the server sent it.
            /// @details Original: RecvDupStartNum.
            /// @return The number.
            int startNumber() const { return m_startNumber; }

            /// @brief Ids of the members on the current members page.
            /// @return The ids.
            const std::vector<std::int32_t>& memberIds() const { return m_memberIds; }

            /// @brief Current page of the members list.
            /// @return Page number.
            int memberPage() const { return m_memberPage; }

            /// @brief Number of pages of the members list.
            /// @return Page count.
            int memberPages() const { return m_memberPages; }

            /// @brief Instances to create, the lowest level first.
            /// @details Original: LoadCreateDupList.
            /// @return The rows of the create form.
            std::vector<InstanceCreateRow> createRows() const;

            /// @brief Resets the window and the player's state at the start of a game.
            /// @details Original: ResetDupMisForm.
            void reset();

            /// @brief The window was shown: asks for the list once.
            /// @details Original: TDupMisForm.ShowCtrl.
            void windowShown();

            /// @brief Switches the tab; the members tab asks for the members.
            /// @details Original: SelectDupMisTab.
            /// @param tab Tab to show.
            void selectTab(Tab tab);

            /// @brief Tells whether the create form may open.
            /// @details Original: TDupMisForm.OpenCreateDupForm.
            /// @return True when the player is not in an instance.
            bool mayCreate();

            /// @brief Goes to the previous page of the list.
            /// @details Original: PrevDupOrderPage.
            void previousOrderPage();

            /// @brief Goes to the next page of the list.
            /// @details Original: NextDupOrderPage.
            void nextOrderPage();

            /// @brief Asks for the detail of a listed instance, if the player may join it.
            /// @details Original: JoinDupFromList.
            /// @param row Row 0..4.
            void joinFromList(int row);

            /// @brief Asks for the detail of the instance of a clicked row.
            /// @param row Row index.
            void showDetail(int row);

            /// @brief Joins the instance shown in the detail window.
            /// @details Original: TDupDetailForm.JoinDupFromDetail.
            void joinFromDetail();

            /// @brief Checks the request and sends the create request (85/3).
            /// @details Original: SendCreateDup.
            /// @param sceneId Scene of the instance to create.
            /// @param message Creator's message.
            /// @return True when sent.
            bool create(int sceneId, const std::string& message);

            /// @brief Checks that an instance may be created (none waited for or played; a guild's instance needs a guild), with the messages.
            /// @details Original: ConfirmCreateDup's checks.
            /// @param sceneId Scene of the instance.
            /// @return True when allowed.
            bool mayCreateScene(int sceneId);

            /// @brief Sends the request to leave the own instance.
            /// @details Original: SendLeaveDup.
            void leave();

            /// @brief Sends the request to enter the own instance.
            /// @details Original: SendEnterDup.
            void enter();

            /// @brief Goes to the previous page of the members.
            /// @details Original: PrevDupMemberPage.
            void previousMembersPage();

            /// @brief Goes to the next page of the members.
            /// @details Original: NextDupMemberPage.
            void nextMembersPage();

            /// @brief Tells whether the player may send a member away.
            /// @details Original: ConfirmDupKickMember's checks.
            /// @param memberId Id of the member.
            /// @return True when allowed.
            bool maySend(std::int32_t memberId);

            /// @brief Sends the request to send a member away.
            /// @details Original: SendDupKickMember.
            /// @param memberId Id of the member.
            void sendAway(std::int32_t memberId);

            /// @brief Counts down the remaining minutes of a started instance.
            /// @details Original: TickDupRemainTime.
            /// @param nowMs Current time in ms.
            void tick(std::uint64_t nowMs);

            /// @brief Takes in the server's action 85, dispatching on the sub id.
            /// @param payload The message.
            void receive(const core::Bytes& payload);

        private:
            /// @brief Handles 85/1, the page of open instances.
            /// @details Original: RecvDupOrderList.
            /// @param payload The message.
            void receiveList(const core::Bytes& payload);

            /// @brief Handles 85/2, the result of creating an instance.
            /// @details Original: RecvDupInitResult.
            /// @param payload The message.
            void receiveInitResult(const core::Bytes& payload);

            /// @brief Handles 85/3, the result of joining an instance.
            /// @details Original: RecvDupJoinResult.
            /// @param payload The message.
            void receiveJoinResult(const core::Bytes& payload);

            /// @brief Handles 85/4, the detail of an instance.
            /// @details Original: RecvDupDetail.
            /// @param payload The message.
            void receiveDetail(const core::Bytes& payload);

            /// @brief Handles 85/5, the state of the player's own instance.
            /// @details Original: RecvDupState.
            /// @param payload The message.
            void receiveState(const core::Bytes& payload);

            /// @brief Handles 85/6, a page of the members of the own instance.
            /// @details Original: RecvDupMemberList.
            /// @param payload The message.
            void receiveMemberList(const core::Bytes& payload);

            /// @brief Handles 85/7, a member joined.
            /// @details Original: RecvDupMemberJoined.
            /// @param payload The message.
            void receiveMemberJoined(const core::Bytes& payload);

            /// @brief Handles 85/8, one more row for the last list page.
            /// @details Original: RecvDupOrderRow.
            /// @param payload The message.
            void receiveRow(const core::Bytes& payload);

            /// @brief Handles 85/9, the remaining time.
            /// @details Original: RecvDupRemainTime.
            /// @param payload The message.
            void receiveRemainingTime(const core::Bytes& payload);

            /// @brief Handles 85/10, goal and boss state.
            /// @details Original: RecvDupGoalState.
            /// @param payload The message.
            void receiveGoal(const core::Bytes& payload);

            /// @brief Handles 85/11, an open instance started or was cancelled.
            /// @details Original: RecvDupOrderClosed.
            /// @param payload The message.
            void receiveClosed(const core::Bytes& payload);

            /// @brief Handles 85/12, a member left.
            /// @details Original: RecvDupMemberLeft.
            /// @param payload The message.
            void receiveMemberLeft(const core::Bytes& payload);

            /// @brief Handles 85/14, the instances the server turns off or on for creating.
            /// @details Original: RecvDupMissionFlags.
            /// @param payload The message.
            void receiveFlags(const core::Bytes& payload);

            /// @brief Scene data of a map.
            /// @param mapId Map id.
            /// @return The scene, or an empty one.
            formats::SceneInfo sceneOf(int mapId) const;

            /// @brief Title and text of an instance: the mark's name and text, or the scene's name.
            /// @param scene Scene of the instance.
            /// @param title Receives the title.
            /// @param text Receives the text.
            void titleAndText(const formats::SceneInfo& scene, std::string& title, std::string& text) const;

            /// @brief Fills the texts of an instance's rules.
            /// @details Original: SetDupDetailInfo and SetDupInnerInfo.
            /// @param mapId Map of the instance.
            /// @param info Receives the texts.
            void describe(int mapId, InstanceInfo& info) const;

            /// @brief Adds a row to the list of open instances (only runtime maps of instances).
            /// @details Original: AddDupOrderRow.
            /// @param mapId Runtime map.
            /// @param creator Creator's name.
            /// @param message Creator's message.
            /// @param guild Guild's name for a guild instance.
            /// @param players Player count.
            void addRow(
                int mapId,
                const std::string& creator,
                const std::string& message,
                const std::string& guild,
                int players
            );

            /// @brief Shows a new player count in the list row of a scene.
            /// @details Original: UpdateDupOrderPlayerCount.
            /// @param sceneId Scene of the row.
            /// @param players New player count.
            /// @param started 1 when the instance started.
            void updateRowPlayers(int sceneId, int players, int started);

            /// @brief Adds a player to the detail window's list.
            /// @details Original: AddDupDetailMember.
            /// @param id Player id.
            /// @param members New member count.
            /// @param sceneId Scene of the instance.
            void addDetailMember(std::int32_t id, int members, int sceneId);

            /// @brief Removes a player from the detail window's list.
            /// @details Original: RemoveDupDetailMember.
            /// @param id Player id.
            /// @param members New member count.
            /// @param sceneId Scene of the instance.
            void removeDetailMember(std::int32_t id, int members, int sceneId);

            /// @brief Sets the info of the own instance.
            /// @details Original: SetDupInnerInfo.
            /// @param mapId Map of the instance.
            /// @param members Member count.
            /// @param creatorId Creator's player id.
            void setInnerInfo(int mapId, int members, std::int32_t creatorId);

            /// @brief Clears the own instance (the original zeroes FDupDetail, 147 bytes).
            void clearOwn();

            /// @brief Loads the time limit of the own instance as the remaining time.
            /// @details Original: LoadDupTimeLimit.
            void loadTimeLimit();

            /// @brief Mission flag of an instance scene: 1 unless the server turned it off.
            /// @param sceneId Scene id.
            /// @return The flag.
            int flagOf(int sceneId) const;

            /// @brief Shows a short message through the hook.
            /// @param text Message text.
            /// @param durationMs Display time in ms.
            void say(const std::string& text, int durationMs) const { m_hooks.message(text, durationMs); }

            const formats::MarkTable& m_marks;
            const formats::NpcTable& m_npcs;
            Hooks m_hooks;
            int m_state = 0;
            InstanceInfo m_own;
            InstanceInfo m_detail;
            std::vector<InstanceMember> m_detailMembers;
            std::array<InstanceGoal, kGoals + 1> m_goals{};
            int m_bossNpc = 0;
            int m_bossKills = 0;
            Tab m_tab = Tab::Choose;
            bool m_refreshOnShow = true;
            std::vector<InstanceRow> m_rows;
            int m_orderPage = 0;
            int m_orderPages = 0;
            int m_startNumber = 0;
            std::vector<std::int32_t> m_memberIds;
            int m_memberPage = 0;
            int m_memberPages = 0;
            std::map<int, int> m_flags;
            std::optional<std::uint64_t> m_lastMinuteMs;
    };

}

#endif
