#ifndef NORTH_ISLAND_CLIENT_SERVICES_PARTY_PARTY_MEMBERS_H
#define NORTH_ISLAND_CLIENT_SERVICES_PARTY_PARTY_MEMBERS_H

#include "audio/AudioDevice.h"
#include "client/models/character/CharacterLook.h"
#include "client/services/party/AccountBundle.h"
#include "client/services/GameServices.h"
#include "gfx/Canvas.h"
#include "ui/Screen.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace client {

    /// @brief A party member's account: a line of the --accounts file.
    struct PartyAccount {
        std::string account;
        std::string password;
        int slot = 1;
        bool autoBattle = true;

        /// @brief Compares all fields of two accounts.
        /// @param other The account to compare with.
        /// @return True when account, password, slot and auto battle all match.
        bool operator==(const PartyAccount& other) const {
            return account == other.account && password == other.password && slot == other.slot && autoBattle == other.autoBattle;
        }
    };

    /// @brief Parses the --accounts file: "account password [place] [auto|manual]" a line; blank and # lines are skipped; at most kMostMembers accounts.
    /// @param lines The lines of the file.
    /// @param error Receives what is wrong with the file (nothing is returned then).
    /// @return The accounts, or nothing when a line is invalid.
    std::optional<std::vector<PartyAccount>> parsePartyAccounts(const std::vector<std::string>& lines, std::string& error);

    /// @brief What the party panels show of a member.
    struct PartyMemberView {
        /// @brief Where the member is in its login and play.
        enum class State { LoggingIn, InWorld, InBattle, Gone };
        std::string account;
        std::string name;
        std::int32_t id = 0;
        State state = State::LoggingIn;
        int level = 0;
        int hp = 0;
        int maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        int element = 0;
        int job = 0;
        std::optional<CharacterLook> look;
        std::string reason;
        bool inTeam = false;
        bool autoBattle = true;
        bool commanding = false;
        bool selected = false;
    };

    /// @brief A member's chat line for the leader's chat window.
    struct PartyChatLine {
        int channel = 0;
        std::string line;
        std::string member;
    };

    /// @brief A member's message for the played account's top box (centre: its middle one).
    struct PartyMessage {
        bool centre = false;
        std::string text;
        int durationMs = 0;
    };

    class PartyPanels;
    class Scene;
    class WorldScene;

    /// @brief The qol client's several accounts at once: the played account leads, each member runs the login and world scenes unseen and follows through the server's own team.
    /// @details Covers team invitations (13/7), auto battle, manual battle turns shown in the leader's place, member windows over the leader's screen, chat relay, leader swap and log in/out from the group window.
    class PartyMembers {
        public:
            static constexpr int kMostMembers = 3; // most members (extra accounts) in the party

            /// @brief Sets up the members' accounts (not yet logged in).
            /// @param played The played account's bundle, which the members share tables and settings of and which a member's replaces when it takes the lead.
            /// @param device The open sound device (the played account's).
            /// @param setUpPlayed What the Application gives the played account's services (message box, screenshots, Video tab); applied again for a member that takes the lead.
            /// @param accounts The members' accounts.
            PartyMembers(
                std::unique_ptr<AccountBundle>& played,
                const audio::AudioDevice& device,
                std::function<void(GameServices&)> setUpPlayed,
                std::vector<PartyAccount> accounts
            );

            /// @brief Removes the party hooks from the played account's services.
            ~PartyMembers();
            PartyMembers(const PartyMembers&) = delete;
            PartyMembers& operator=(const PartyMembers&) = delete;

            /// @brief Tells whether there are no members.
            /// @return True when no member is in the list.
            bool empty() const { return m_members.empty(); }

            /// @brief Counts the members (rows), including those logging out or gone.
            /// @return The number of members.
            int size() const { return static_cast<int>(m_members.size()); }

            /// @brief Finds the member whose battle commands are given now, shown and steered in the leader's place.
            /// @param leaderDone The leader is in a fight and its own fighters have their commands.
            /// @return The member's scene, or nothing for the leader.
            std::optional<std::reference_wrapper<Scene>> commandingScene(bool leaderDone);

            /// @brief Gives the member whose menus and shortcuts stand in for the leader's.
            /// @return The member's row, or -1 when the leader has them.
            int selected() const { return m_selected; }

            /// @brief Takes the members' chat lines heard a moment ago (the leader's own copy of a shared line has come by then), each once.
            /// @param nowMs The current time in milliseconds.
            /// @return The lines for the leader's chat window.
            std::vector<PartyChatLine> takeChatLines(std::uint64_t nowMs);

            /// @brief Takes the members' messages since the last call, for the played account's boxes (PlayerMessages::showGrouped).
            /// @return The messages.
            std::vector<PartyMessage> takeMessages() { return std::exchange(m_messages, {}); }

            /// @brief Leaves the leader's HUD layouts out of its screen (drawing and mouse) while a member is selected, whose own stand in.
            /// @param leader The leader's screen.
            void coverLeaderHud(ui::Screen& leader) const;

            /// @brief Draws the members' windows and popups over the leader's screen, each tagged with the member's name.
            /// @param canvas The canvas, at the game screen's origin.
            /// @param nowMs The current time in milliseconds.
            /// @param hudScale The HUD scale.
            void drawWindows(gfx::Canvas& canvas, std::uint64_t nowMs, float hudScale);

            /// @brief Draws the members' rows under the own status (PartyPanels).
            /// @param canvas The canvas, at the game screen's origin.
            /// @param hudScale The HUD scale.
            /// @param leaderWorld The played account's world scene, whose battle's own panels show the members fighting in it.
            void drawPanels(gfx::Canvas& canvas, float hudScale, const WorldScene& leaderWorld);

            /// @brief Handles an input event for the members: a click on a member's row, the mouse over a member's window, the selected member's Ctrl shortcuts and Escape, typing into a member's text box.
            /// @param event The SDL event.
            /// @param origin The game screen's place in the window.
            /// @param hudScale The HUD scale.
            /// @param memberShown A member's whole scene is shown (its battle commands), which takes the input itself.
            /// @return True when a member took the event.
            bool handleEvent(const SDL_Event& event, core::Point origin, float hudScale, bool memberShown);

            /// @brief Tells whether the members have been logged in.
            /// @return True once start() has run.
            bool started() const { return m_started; }

            /// @brief Logs every member in to the server the leader is on (once the leader is in the world); one added later logs in at once.
            void start();

            /// @brief Adds another member (the group window's Log in); rows of members that could not log in go.
            /// @param account The account to log in.
            /// @return Empty, or why the account cannot be added.
            std::string add(PartyAccount account);

            /// @brief Tells whether a member plays this character (not one logging out).
            /// @param playerId The character's id.
            /// @return True when a member plays it.
            bool isMember(std::int32_t playerId) const;

            /// @brief Makes that member leave the leader's team and log out; its row goes once the leave has been sent.
            /// @param playerId The member's character id.
            void logOut(std::int32_t playerId);

            /// @brief Updates every member once a beat.
            /// @param nowMs The current time in milliseconds.
            /// @param leaderWorld The played account's world scene (nothing: not in the world).
            void update(std::uint64_t nowMs, std::optional<std::reference_wrapper<const WorldScene>> leaderWorld);

            /// @brief Moves the members' scenes before every frame.
            /// @param nowMs The current time in milliseconds.
            void move(std::uint64_t nowMs);

            /// @brief Describes every member for the party panels.
            /// @return One view per member.
            std::vector<PartyMemberView> views() const;

        private:
            static constexpr int kMostInvites = 3; // most team invites a member sends before it stops asking
            static constexpr std::uint64_t kInviteGapMs = 5000; // ms between two invites
            static constexpr std::uint64_t kChatDelayMs = 1500; // ms a chat line a member heard waits before it is relayed to the leader
            static constexpr std::uint64_t kChatRepeatMs = 5000; // ms a sent line is remembered against repeats

            static constexpr std::uint64_t kLeaveWaitMs = 2000; // ms a logging-out member waits for its sends to finish before it is removed
            static constexpr std::uint64_t kAwayMs = 10000; // ms a member may stay on another map than the leader before it is logged out

            struct Member;

            /// @brief Gives the played account's services.
            /// @return The services.
            GameServices& played() { return m_played->services; }

            /// @brief Gives the played account's services (read only).
            /// @return The services.
            const GameServices& played() const { return m_played->services; }

            /// @brief Sets the party hooks of the played account's services.
            /// @param services The played account's services.
            void setUpPlayed(GameServices& services);

            /// @brief Sets the hooks of a member's services.
            /// @param member The member.
            void setUpMember(Member& member);

            /// @brief Makes that member the played account and the played account that member (the sound and the hooks change hands).
            /// @param index The member's row.
            void takeLead(int index);

            /// @brief Starts a member's login scene on the leader's server.
            /// @param member The member.
            void logIn(Member& member);

            /// @brief Counts the places of the leader's team taken by others than the members (the played account under another leader, other players).
            /// @param leaderWorld The played account's world scene.
            /// @return The number of such places.
            int otherTeammates(const WorldScene& leaderWorld) const;

            /// @brief Sends the member's leave of the team, after which it is removed.
            /// @param member The member.
            /// @param reason What its row says meanwhile.
            void beginLogOut(Member& member, const std::string& reason);

            /// @brief Drops a member's row (its connection closes) and moves the rows after it up.
            /// @param index The member's row.
            void remove(int index);

            /// @brief Switches a member to the scene its current scene asked for.
            /// @param member The member.
            void changeScene(Member& member);

            /// @brief Sends the leader's team invitation to a member, a few tries and kInviteGapMs apart.
            /// @param member The member to invite.
            /// @param leaderWorld The played account's world scene.
            /// @param nowMs The current time in milliseconds.
            void inviteToTeam(Member& member, const WorldScene& leaderWorld, std::uint64_t nowMs);

            /// @brief Runs a call on the member's screen with only its windows, popups (and, selected, its menus) kept.
            /// @tparam Call The callable type; it takes the member's ui::Screen.
            /// @param index The member's row.
            /// @param hudScale The HUD scale.
            /// @param call What to run.
            template <typename Call>
            void withWindows(int index, float hudScale, const Call& call);

            /// @brief Finds the member whose kept layouts catch the mouse at a point of the game screen.
            /// @param point The point, in the game screen's coordinates.
            /// @param hudScale The HUD scale.
            /// @return The member's row, or -1 for none.
            int memberAt(core::Point point, float hudScale);

            /// @brief Runs a call on each member in the world on the leader's map, while the NPC mirror is on.
            /// @tparam Call The callable type; it takes the Member and its WorldScene.
            /// @param call What to run.
            template <typename Call>
            void forMirroringMembers(const Call& call);

            /// @brief Marks a member as left the party: closes its scene and connection.
            /// @param member The member.
            /// @param reason Why it left (shown on its row).
            static void gone(Member& member, const std::string& reason);

            std::unique_ptr<AccountBundle>& m_played;
            const audio::AudioDevice& m_device;
            std::function<void(GameServices&)> m_setUpPlayed;
            const audio::AudioDevice m_silent{audio::AudioDevice::Silent{}};
            std::vector<std::unique_ptr<Member>> m_members;
            bool m_started = false;
            std::uint64_t m_nowMs = 0;
            std::int32_t m_teamLeaderId = 0;
            int m_otherTeammates = 0;
            int m_commanding = -1;
            bool m_mirrorNpcs = false;
            int m_selected = -1;
            std::unique_ptr<PartyPanels> m_panels;
            std::vector<PartyMessage> m_messages;
            bool m_panelPressed = false;
            std::optional<core::Point> m_mouse;
            int m_mouseOwner = -1;
            int m_keyboardOwner = -1;
            std::vector<SDL_Keycode> m_routedKeys;

            /// @brief A chat line already handed to the leader, and when.
            struct SentLine {
                int channel;
                std::string line;
                std::uint64_t atMs;
            };

            std::vector<SentLine> m_sentLines;
    };

}

#endif
