#include "client/models/instances/Instances.h"

#include "client/rules/TextFilter.h"
#include "core/ByteReader.h"
#include "core/StringUtil.h"
#include "protocol/InstanceRequests.h"
#include "protocol/MessageId.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kShortMs = 1500; // short on-screen message time, ms
        constexpr int kLongMs = 5000; // long on-screen message time, ms
        constexpr int kMessageLength = 28; // longest instance info text the player may enter, characters
        constexpr int kNoMinimum = 9999; // start value when searching for the lowest minimum level
        constexpr int kDefeatedBossKills = 1; // kill count that marks a boss as defeated
        constexpr int kTownWithoutTeleport = 11094; // map 11094: Carnie (teleport not allowed there)
        constexpr int kMinutesAfterDone = 10; // minutes left before the finished instance is deleted
        constexpr int kMinutesAfterWarning = 5; // minutes left at the final deletion warning
        const char* const kNone = "None";

        /// @brief Formats a number, or "None" for 0.
        /// @param value The number.
        /// @return The text.
        std::string numberOrNone(int value) { return value == 0 ? kNone : std::to_string(value); }

    }

    std::string Instances::minutesText(int minutes) {
        const int hours = minutes / 60;
        if (hours != 0) {
            return std::to_string(hours) + " Hour " + std::to_string(minutes % 60) + " minute";
        }
        return std::to_string(minutes % 60) + " minute";
    }

    bool Instances::here() const { return m_hooks.isInstanceMap(m_hooks.ownMap()); }

    bool Instances::fighting() const { return m_hooks.fighting(); }

    Instances::Instances(
        const formats::MarkTable& marks,
        const formats::NpcTable& npcs,
        Hooks hooks
    )
        : m_marks(marks),
          m_npcs(npcs),
          m_hooks(std::move(hooks)) {}

    formats::SceneInfo Instances::sceneOf(int mapId) const { return m_hooks.scene(mapId).value_or(formats::SceneInfo{}); }

    void Instances::titleAndText(const formats::SceneInfo& scene, std::string& title, std::string& text) const {
        if (scene.dupMarkNo != 0) {
            if (const auto mark = m_marks.find(scene.dupMarkNo - 1)) {
                title = mark->get().name;
                text = mark->get().text;
                return;
            }
        }
        title = scene.name;
        text = scene.name;
    }

    int Instances::flagOf(int sceneId) const {
        const auto found = m_flags.find(sceneId);
        return found == m_flags.end() ? 1 : found->second;
    }

    std::vector<InstanceCreateRow> Instances::createRows() const {
        const std::vector<int> scenes = m_hooks.scenes();
        std::vector<bool> taken(scenes.size(), false);
        std::vector<InstanceCreateRow> rows;
        for (std::size_t round = 0; round < scenes.size(); ++round) {
            int lowest = kNoMinimum;
            std::optional<std::size_t> pick;
            for (std::size_t i = 0; i < scenes.size(); ++i) {
                if (!taken[i] && flagOf(scenes[i]) == 1 && sceneOf(scenes[i]).dupMinLevel <= lowest) {
                    lowest = sceneOf(scenes[i]).dupMinLevel;
                    pick = i;
                }
            }
            if (lowest == 0 || !pick) {
                continue;
            }
            const formats::SceneInfo scene = sceneOf(scenes[*pick]);
            InstanceCreateRow row;
            row.sceneId = scenes[*pick];
            row.level = numberOrNone(scene.dupMinLevel);
            row.time = scene.dupTimeLimit == 0 ? kNone : minutesText(scene.dupTimeLimit);
            row.players = numberOrNone(scene.dupMaxPlayers);
            row.guildOnly = scene.dupGuildOnly == 0 ? kNone : "Yes";
            titleAndText(scene, row.title, row.description);
            rows.push_back(row);
            taken[*pick] = true;
        }
        return rows;
    }

    void Instances::reset() {
        m_state = 0;
        clearOwn();
        m_lastMinuteMs.reset();
        m_refreshOnShow = true;
        m_orderPage = 0;
        m_tab = Tab::Choose;
        m_startNumber = 0;
        m_orderPages = 0;
        m_rows.clear();
        m_memberPage = 0;
        m_memberPages = 0;
    }

    void Instances::clearOwn() {
        m_own = InstanceInfo{};
        m_goals.fill(InstanceGoal{});
        m_bossNpc = 0;
        m_bossKills = 0;
    }

    void Instances::windowShown() {
        if (m_refreshOnShow) {
            m_refreshOnShow = false;
            m_hooks.send(protocol::instanceListRequest());
        }
    }

    void Instances::selectTab(Tab tab) {
        if (tab == Tab::Members && m_tab != tab) {
            m_hooks.send(protocol::instanceMembersRequest());
        }
        m_tab = tab;
    }

    bool Instances::mayCreate() {
        if (m_state != 0) {
            say("Please quite the instance you have entered", kShortMs);
            return false;
        }
        return true;
    }

    void Instances::previousOrderPage() {
        if (m_orderPage - 1 >= 1) {
            --m_orderPage;
            m_hooks.send(protocol::instancePageRequest(m_orderPage));
        }
    }

    void Instances::nextOrderPage() {
        if (m_orderPage + 1 <= m_orderPages) {
            ++m_orderPage;
            m_hooks.send(protocol::instancePageRequest(m_orderPage));
        }
    }

    void Instances::joinFromList(int row) {
        if (row < 0 || row >= static_cast<int>(m_rows.size())) {
            return;
        }
        const int mapId = m_rows[static_cast<std::size_t>(row)].mapId;
        const auto scene = m_hooks.scene(mapId);
        if (!scene) {
            return;
        }
        if (scene->dupGuildOnly == 1 && !m_hooks.inGuild()) {
            say("Does not have the organization coca not to enter", 1000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::instanceDetailRequest(mapId));
    }

    void Instances::showDetail(int row) {
        if (row >= 0 && row < static_cast<int>(m_rows.size())) {
            m_hooks.send(protocol::instanceDetailRequest(m_rows[static_cast<std::size_t>(row)].mapId));
        }
    }

    void Instances::joinFromDetail() {
        if (m_own.mapId == m_detail.mapId) {
            say("You are already in the instance", kShortMs);
            return;
        }
        if (m_state != 0) {
            say("Please quite the instance you have entered", kShortMs);
            return;
        }
        m_hooks.send(protocol::instanceJoinRequest(m_detail.mapId));
    }

    bool Instances::mayCreateScene(int sceneId) {
        if (m_state != 0) {
            say("Please quite the instance you have entered", kShortMs);
            return false;
        }
        const auto scene = m_hooks.scene(sceneId);
        if (!scene) {
            return false;
        }
        if (scene->dupGuildOnly == 1 && !m_hooks.inGuild()) {
            say("Limits the organization to create ", 1000); // message display time in ms
            return false;
        }
        return true;
    }

    bool Instances::create(int sceneId, const std::string& message) {
        if (m_state != 0) {
            say("Please quite the instance you have entered", kShortMs);
            return false;
        }
        const std::string text = core::trim(message);
        if (static_cast<int>(text.size()) > kMessageLength) {
            say("The information you entered is too long", kShortMs);
            return false;
        }
        if (!isTextClean(text)) {
            say("Invalid letters", 1000); // message display time in ms
            return false;
        }
        m_hooks.send(protocol::instanceCreateRequest(sceneId, text));
        return true;
    }

    void Instances::leave() {
        if (m_hooks.fighting()) {
            say("Unable to use in combat", kShortMs);
            return;
        }
        m_hooks.send(protocol::instanceLeaveRequest());
    }

    void Instances::enter() {
        if (m_hooks.cannotTeleport()) {
            return;
        }
        if (m_hooks.ownMap() == kTownWithoutTeleport) {
            say("Cannot teleport", kShortMs);
            return;
        }
        m_hooks.send(protocol::instanceEnterRequest());
    }

    void Instances::previousMembersPage() {
        if (m_memberPage - 1 >= 1) {
            --m_memberPage;
            m_hooks.send(protocol::instanceMembersPageRequest(m_memberPage));
        }
    }

    void Instances::nextMembersPage() {
        if (m_memberPage + 1 <= m_memberPages) {
            ++m_memberPage;
            m_hooks.send(protocol::instanceMembersPageRequest(m_memberPage));
        }
    }

    // m_state 1: waiting for the start (creator may send members away)
    bool Instances::maySend(std::int32_t memberId) { return m_hooks.ownId() == m_own.creatorId && memberId != m_own.creatorId && m_state == 1; }

    void Instances::sendAway(std::int32_t memberId) { m_hooks.send(protocol::instanceKickRequest(m_own.mapId, memberId)); }

    void Instances::tick(std::uint64_t nowMs) {
        constexpr std::uint64_t kMinuteMs = 60000; // one minute, ms
        if (!m_lastMinuteMs) {
            m_lastMinuteMs = nowMs;
            return;
        }
        if (nowMs - *m_lastMinuteMs > kMinuteMs) {
            m_lastMinuteMs = nowMs;
            if (m_state == 2 && m_own.remainingMinutes - 1 >= 0) { // state 2: the instance has started
                --m_own.remainingMinutes;
            }
        }
    }

    void Instances::receive(const core::Bytes& payload) {
        switch (protocol::MessageId::of(payload).sub) {
            case 1: receiveList(payload); break; // 85/1: page of open instances (RecvDupOrderList)
            case 2: receiveInitResult(payload); break; // 85/2: result of creating an instance (RecvDupInitResult)
            case 3: receiveJoinResult(payload); break; // 85/3: result of joining an instance (RecvDupJoinResult)
            case 4: receiveDetail(payload); break; // 85/4: detail of an instance (RecvDupDetail)
            case 5: receiveState(payload); break; // 85/5: state of the own instance (RecvDupState)
            case 6: receiveMemberList(payload); break; // 85/6: page of members of the own instance (RecvDupMemberList)
            case 7: receiveMemberJoined(payload); break; // 85/7: a member joined (RecvDupMemberJoined)
            case 8: receiveRow(payload); break; // 85/8: one more list row (RecvDupOrderRow)
            case 9: receiveRemainingTime(payload); break; // 85/9: remaining time (RecvDupRemainTime)
            case 10: receiveGoal(payload); break; // 85/10: goal and boss state (RecvDupGoalState)
            case 11: receiveClosed(payload); break; // 85/11: an open instance started or was cancelled (RecvDupOrderClosed)
            case 12: receiveMemberLeft(payload); break; // 85/12: a member left (RecvDupMemberLeft)
            case 13: { // 85/13: start number (u16), RecvDupStartNum
                core::ByteReader reader(payload, 2);
                const int number = reader.u16();
                if (!reader.failed()) {
                    m_startNumber = number;
                }
                break;
            }
            case 14: receiveFlags(payload); break; // 85/14: instances turned off or on for creating (RecvDupMissionFlags)
            default: break; // other sub ids are ignored
        }
    }

    void Instances::addRow(
        int mapId,
        const std::string& creator,
        const std::string& message,
        const std::string& guild,
        int players
    ) {
        if (!m_hooks.isInstanceMap(mapId)) {
            return;
        }
        const formats::SceneInfo scene = sceneOf(mapId);
        InstanceRow row;
        row.mapId = mapId;
        row.message = message;
        row.level = std::to_string(scene.dupMinLevel);
        row.guildLimit = scene.dupGuildOnly == 0 ? kNone : "Yes";
        row.creator = scene.dupGuildOnly == 0 ? creator : guild;
        std::string text;
        titleAndText(scene, row.title, text);
        row.players = std::to_string(players) + "/" + std::to_string(scene.dupMaxPlayers);
        m_rows.push_back(row);
    }

    void Instances::receiveList(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int pages = reader.u8();
        const int page = reader.u8();
        const int count = reader.u8();
        if (reader.failed()) {
            return;
        }
        m_orderPages = pages;
        m_orderPage = page;
        m_rows.clear();
        std::string guild;
        for (int i = 0; i < count; ++i) {
            const int mapId = reader.u16();
            const std::string creator = reader.shortText();
            const std::string message = reader.shortText();
            const int players = reader.u8();
            if (!reader.atEnd()) {
                guild = reader.shortText();
            }
            if (reader.failed()) {
                return;
            }
            addRow(mapId, creator, message, guild, players);
        }
    }

    void Instances::receiveRow(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int pages = reader.u8();
        if (reader.failed()) {
            return;
        }
        m_orderPages = pages;
        if (m_orderPages != m_orderPage) {
            return;
        }
        const int mapId = reader.u16();
        const std::string creator = reader.shortText();
        const std::string message = reader.shortText();
        const int players = reader.u8();
        const std::string guild = reader.shortText();
        if (reader.failed()) {
            return;
        }
        const bool known = std::any_of(m_rows.begin(), m_rows.end(), [mapId](const InstanceRow& row) { return row.mapId == mapId; });
        if (!known) {
            addRow(mapId, creator, message, guild, players);
        }
    }

    void Instances::receiveInitResult(const core::Bytes& payload) {
        const int code = core::ByteReader(payload, 2).u8();
        switch (code) {
            case 0: // 85/2 code 0: created, ask for the list again
                m_hooks.send(protocol::instanceListRequest());
                say("Instance initiated successfully", kShortMs);
                break;
            case 1: say("The number of the instance has reached the upper limit", kShortMs); break; // code 1: too many instances
            case 2: say("Level is too low", kShortMs); break; // code 2: level too low
            case 3: say("Has already used the instance today", kShortMs); break; // code 3: already used today
            case 4: // code 4: content done, deleted after 10 minutes
                if (here()) {
                    say("The instance content has been completed, now it's time to redeem your rewards!\rSystem will delete the instance after 10 minutes",
                        kLongMs);
                    m_hooks.chat("The instance content has been completed, now it's time to redeem your rewards!");
                    m_hooks.chat("System will delete the instance after 10 minutes");
                }
                m_own.remainingMinutes = kMinutesAfterDone;
                m_own.remainPrefix = "And ";
                break;
            case 5: // code 5: deleted after 5 minutes, leave soon
                if (here()) {
                    say("System will delete the instance after 5 minutes\rPlease leave the instance as soon as possible", kLongMs);
                    m_hooks.chat("System will delete the instance after 5 minutes");
                    m_hooks.chat("Please leave the instance as soon as possible");
                }
                m_own.remainingMinutes = kMinutesAfterWarning;
                m_own.remainPrefix = "And ";
                break;
            case 6: say("The instance has been deleted, you can't be teleported", kShortMs); break; // code 6: instance deleted
            case 7: // code 7: time to finish expired
                if (here()) {
                    say("The time to finish the instance is expired ", kLongMs);
                    m_hooks.chat("The time to finish the instance is expired ");
                }
                m_own.remainingMinutes = kMinutesAfterDone;
                m_own.remainPrefix = "And ";
                break;
            case 8: // code 8: scene cannot be transmitted to the instance
            case 9: // code 9: players too busy to transmit
                m_hooks.chat(code == 8 ? "The scene system can't be transmitted to instance" : "Players are too busy to transmit instance");
                m_hooks.chat("Please log in the instance via the instance interface");
                if (m_hooks.windowOpen()) {
                    m_tab = Tab::Info;
                    m_hooks.showWindow();
                }
                break;
            case 10: // code 10: instance reset
                // message display time in ms
                say("Reset Instance", 3000);
                m_hooks.chat("Reset Instance");
                break;
            // code 11: instance not opened for now; 3000 is the message display time in ms
            case 11: say("The instance will not be opened temporarily", 3000); break;
            case 255: say("Instance initiating failed", kShortMs); break; // code 255: creating failed
            default: break; // other codes are ignored
        }
    }

    void Instances::receiveJoinResult(const core::Bytes& payload) {
        const auto refresh = [this] { m_hooks.send(protocol::instancePageRequest(m_orderPage)); };
        switch (core::ByteReader(payload, 2).u8()) {
            case 0: // 85/3 code 0: joined
                say("The instance added successfully", kShortMs);
                m_hooks.closeDetail();
                break;
            case 1: say("The number of Players in the instance has reached the maximum capacity.", kShortMs); break; // code 1: instance full
            case 2: say("Level is too low", kShortMs); break; // code 2: level too low
            case 3: // code 3: instance started, refresh the list
                refresh();
                say("The instance starts", kShortMs);
                m_hooks.closeDetail();
                break;
            case 4: // code 4: instance cancelled, refresh the list
                refresh();
                say("The instance is cancelled", kShortMs);
                m_hooks.closeDetail();
                break;
            case 5: say("Has already used the instance today", kShortMs); break; // code 5: already used today
            case 6: // code 6: instance ended, refresh the list
                refresh();
                say("The instance has ended", kShortMs);
                m_hooks.closeDetail();
                break;
            case 7: // code 7: guild-only instance and the player has no guild
                say("Joins the transcription the organization not symbol ", kShortMs);
                m_hooks.closeDetail();
                break;
            case 255: say("Instance adding failed", kShortMs); break; // code 255: joining failed
            default: break; // other codes are ignored
        }
    }

    void Instances::describe(int mapId, InstanceInfo& info) const {
        const formats::SceneInfo scene = sceneOf(mapId);
        info.mapId = mapId;
        info.minLevel = numberOrNone(scene.dupMinLevel);
        info.timeLimit = scene.dupTimeLimit == 0 ? kNone : minutesText(scene.dupTimeLimit);
        info.maxPlayers = numberOrNone(scene.dupMaxPlayers);
        info.guildOnly = scene.dupGuildOnly == 0 ? kNone : "Yes";
        titleAndText(scene, info.title, info.description);
    }

    void Instances::receiveDetail(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int sceneId = reader.u16();
        const int count = reader.u8();
        std::vector<InstanceMember> members;
        for (int i = 0; i < count; ++i) {
            InstanceMember member;
            member.name = reader.shortText();
            member.id = reader.i32();
            members.push_back(member);
        }
        if (reader.failed()) {
            return;
        }
        m_detail = InstanceInfo{};
        m_detailMembers = members;
        if (m_hooks.isInstanceMap(sceneId)) {
            describe(sceneId, m_detail);
            m_detail.members = count;
        }
        m_hooks.openDetail();
    }

    void Instances::setInnerInfo(int mapId, int members, std::int32_t creatorId) {
        if (!m_hooks.isInstanceMap(mapId)) {
            return;
        }
        m_tab = Tab::Info;
        describe(mapId, m_own);
        m_own.members = members;
        m_own.creatorId = creatorId;
    }

    void Instances::loadTimeLimit() {
        if (const auto scene = m_hooks.scene(m_own.mapId)) {
            m_own.remainingMinutes = scene->dupTimeLimit;
        }
    }

    void Instances::receiveState(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        switch (reader.u8()) {
            case 1: { // 85/5 sub 1: own instance info (u8 state, u16 map, u8 members, text, i32 creator)
                const int state = reader.u8();
                const int mapId = reader.u16();
                const int members = reader.u8();
                reader.shortText();
                const std::int32_t creatorId = reader.i32();
                if (reader.failed()) {
                    return;
                }
                m_state = state;
                setInnerInfo(mapId, members, creatorId);
                if (state == 1) {
                    m_goals.fill(InstanceGoal{});
                    m_bossNpc = 0;
                    m_bossKills = 0;
                    m_own.remainingMinutes = 0;
                    m_own.remainPrefix.clear();
                }
                break;
            }
            case 2: // sub 2: left the instance
                m_state = 0;
                clearOwn();
                say("Has quited the current instance", kShortMs);
                break;
            case 3: // sub 3: the instance started
                m_state = 2;
                loadTimeLimit();
                m_hooks.chat("The instance started");
                break;
            case 4: // sub 4: the creator refused the player
                m_state = 0;
                clearOwn();
                say("Creator refuses to add you to the instance", kLongMs);
                say("Has quited the current instance", kLongMs);
                break;
            default: break; // other sub ids are ignored
        }
    }

    void Instances::receiveMemberList(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int pages = reader.u8();
        const int page = reader.u8();
        const int members = reader.u8();
        const int count = reader.u8();
        std::vector<std::int32_t> ids;
        for (int i = 0; i < count; ++i) {
            const std::int32_t id = reader.i32();
            if (id != 0 && !reader.failed()) {
                ids.push_back(id);
            }
        }
        if (reader.failed()) {
            return;
        }
        m_memberPages = pages;
        m_memberPage = page;
        m_own.members = members;
        m_memberIds = ids;
    }

    void Instances::addDetailMember(std::int32_t id, int members, int sceneId) {
        if (m_detail.mapId != sceneId) {
            return;
        }
        const auto listed = [id](const InstanceMember& member) { return member.id == id; };
        if (std::any_of(m_detailMembers.begin(), m_detailMembers.end(), listed)) {
            return;
        }
        InstanceMember member;
        member.id = id;
        if (id == m_hooks.ownId()) {
            member.name = m_hooks.ownName();
        } else if (const auto name = m_hooks.playerName(id)) {
            member.name = *name;
        } else {
            return;
        }
        m_detail.members = members;
        m_detailMembers.push_back(member);
    }

    void Instances::removeDetailMember(std::int32_t id, int members, int sceneId) {
        if (m_detail.mapId != sceneId) {
            return;
        }
        const auto found = std::find_if(m_detailMembers.begin(), m_detailMembers.end(), [id](const InstanceMember& member) { return member.id == id; });
        if (found != m_detailMembers.end()) {
            m_detail.members = members;
            m_detailMembers.erase(found);
        }
    }

    void Instances::updateRowPlayers(int sceneId, int players, int started) {
        if (sceneId == 0) {
            return;
        }
        const formats::SceneInfo scene = sceneOf(sceneId);
        for (InstanceRow& row : m_rows) {
            if (row.mapId != sceneId) {
                continue;
            }
            row.players = std::to_string(players) + "/" + std::to_string(scene.dupMaxPlayers);
            if (scene.dupGuildOnly != 1 && started == 1) {
                row.creator.clear();
            }
            return;
        }
    }

    void Instances::receiveMemberJoined(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int pages = reader.u8();
        const int members = reader.u8();
        const std::int32_t id = reader.i32();
        const int sceneId = reader.u16();
        if (reader.failed()) {
            return;
        }
        const bool listed = std::find(m_memberIds.begin(), m_memberIds.end(), id) != m_memberIds.end();
        if (id != 0 && sceneId != 0 && !listed && m_own.mapId == sceneId) {
            m_own.members = members;
            m_memberPages = pages;
            m_memberIds.push_back(id);
        }
        addDetailMember(id, members, sceneId);
        updateRowPlayers(sceneId, members, 0);
    }

    void Instances::receiveMemberLeft(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int members = reader.u8();
        const std::int32_t id = reader.i32();
        const int sceneId = reader.u16();
        const int started = reader.u8();
        if (reader.failed()) {
            return;
        }
        if (std::find(m_memberIds.begin(), m_memberIds.end(), id) != m_memberIds.end()) {
            m_own.members = members;
            if (started == 1) {
                m_own.creatorId = 0;
            }
            m_hooks.send(protocol::instanceMembersPageRequest(m_memberPage));
        }
        removeDetailMember(id, members, sceneId);
        updateRowPlayers(sceneId, members, started);
    }

    void Instances::receiveRemainingTime(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        switch (reader.u8()) {
            case 1: { // 85/9 sub 1: remaining time (i32)
                const int seconds = reader.i32();
                if (!reader.failed()) {
                    m_own.remainingMinutes = seconds;
                }
                break;
            }
            // sub 2: waiting for being cleared; 1300 is the message display time in ms
            case 2: say("The instance is waiting for being cleared", 1300); break;
            default: break; // other sub ids are ignored
        }
    }

    void Instances::receiveGoal(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const auto npcName = [this](int npcId) {
            const auto npc = m_npcs.find(npcId);
            return npc ? npc->name : std::string();
        };
        switch (reader.u8()) {
            case 1: { // 85/10 sub 1: kills of a goal (u8 goal, u8 kills, text player)
                const int index = reader.u8();
                const int kills = reader.u8();
                const std::string player = reader.shortText();
                if (reader.failed() || index > kGoals) {
                    return;
                }
                InstanceGoal& goal = m_goals[static_cast<std::size_t>(index)];
                if (here() && kills >= goal.need && goal.have < goal.need && goal.npcId != 0 && goal.need == 1) {
                    m_hooks.chat("Enemies " + npcName(goal.npcId) + " Have been beat by " + player);
                }
                goal.have = kills;
                break;
            }
            case 2: { // sub 2: boss kills (u8 kills, text player)
                const int kills = reader.u8();
                const std::string player = reader.shortText();
                if (reader.failed()) {
                    return;
                }
                if (here() && m_bossKills == 0 && m_bossNpc != 0 && kills == kDefeatedBossKills) {
                    m_hooks.chat("Enemies " + npcName(m_bossNpc) + " Have been beat by " + player);
                }
                m_bossKills = kills;
                break;
            }
            case 3: { // sub 3: goal definition (u8 goal, u8 need, u16 npc)
                const int index = reader.u8();
                const int need = reader.u8();
                const int npcId = reader.u16();
                if (reader.failed() || index > kGoals) {
                    return;
                }
                m_goals[static_cast<std::size_t>(index)].need = need;
                m_goals[static_cast<std::size_t>(index)].npcId = npcId;
                break;
            }
            case 4: { // sub 4: boss definition (u16 npc)
                const int npcId = reader.u16();
                if (!reader.failed()) {
                    m_bossNpc = npcId;
                }
                break;
            }
            default: break; // other sub ids are ignored
        }
    }

    void Instances::receiveClosed(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int reason = reader.u8();
        const int sceneId = reader.u16();
        if (reader.failed()) {
            return;
        }
        if (m_hooks.detailOpen() && m_detail.mapId == sceneId) {
            // reason 1: the instance started, 2: it was cancelled
            if (reason == 1) {
                say("The instance starts", kShortMs);
            } else if (reason == 2) {
                say("The instance is cancelled", kShortMs);
            }
            m_hooks.closeDetail();
        }
        if (std::any_of(m_rows.begin(), m_rows.end(), [sceneId](const InstanceRow& row) { return row.mapId == sceneId; })) {
            m_hooks.send(protocol::instancePageRequest(m_orderPage));
        }
    }

    void Instances::receiveFlags(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int count = reader.u8();
        const std::vector<int> scenes = m_hooks.scenes();
        bool changed = false;
        for (int i = 0; i < count && !reader.failed(); ++i) {
            const int sceneId = reader.u16();
            const int flag = reader.u8();
            if (std::find(scenes.begin(), scenes.end(), sceneId) != scenes.end() && flagOf(sceneId) != flag) {
                m_flags[sceneId] = flag;
                changed = true;
            }
        }
        if (changed) {
            m_hooks.createListChanged();
        }
    }

}
