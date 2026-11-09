#include "client/rules/GmIds.h"
#include "client/rules/CharacterActions.h"
#include "gfx/TextStyle.h"
#include "client/scenes/world/WorldScene.h"
#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "core/FileSystem.h"
#include "protocol/GuildRequests.h"

#include <array>
#include <cmath>
#include <ctime>

namespace client {

    namespace {

        constexpr int kQuestionMs = 5000; // question auto-answer time, ms
        constexpr int kNoticeMs = 1200; // message display time, ms
        constexpr int kInviteMouseMode = 18; // mouse mode 18: pick a player to invite to the guild
        constexpr int kDelphiEpochOffset = 25569; // days from the Delphi date epoch (1899-12-30) to the Unix epoch (1970-01-01)

        /// @brief Formats a Delphi TDateTime as year/month/day.
        /// @param date Delphi date value (days since 1899-12-30).
        /// @return The date text, like FormatDateTime('yyyy/m/d', date) of the original.
        std::string delphiDate(double date) {
            const std::time_t seconds = static_cast<std::time_t>((std::floor(date) - kDelphiEpochOffset) * 86400.0); // seconds per day
            std::tm utc{};
#ifdef _WIN32
            gmtime_s(&utc, &seconds);
#else
            gmtime_r(&seconds, &utc);
#endif
            return std::to_string(utc.tm_year + 1900) + "/" + std::to_string(utc.tm_mon + 1) + "/" + std::to_string(utc.tm_mday); // tm_year counts from 1900
        }

        /// @brief Builds the chat notice that the guild will be disbanded automatically.
        /// @param date Delphi date of the disbanding.
        /// @return The notice line.
        std::string disbandNotice(double date) {
            return "Pay attention. Because the number of members who reach level 10 is less than 15, your guild will  Be disbanded On " + delphiDate(date);
        }

        /// @brief Looks up the notice text for a guild position error code.
        /// @param code Error code of the server message.
        /// @return The notice text, or nothing for an unknown code.
        /// @details Original: RecvGuildPositionError (CY_Organ.pas).
        std::optional<std::string> positionError(int code) {
            switch (code) {
                case 1: return "have been in others' guild"; // error 1: already in another guild
                case 2: return "The guild positions are all full."; // error 2: all guild positions taken
                case 3: return "This guild is unavailable."; // error 3: guild unavailable
                case 4: return "The group is full."; // error 4: group full
                case 5: return "Unable to demote a teammate"; // error 5: cannot demote a teammate
                case 6: return "Unable to demote the Vice Guild Lord"; // error 6: cannot demote the Vice Guild Lord
                case 7: return "Not enough right"; // error 7: not enough right
                case 8: return "The player is offline."; // error 8: target offline
                case 9: return "The target refuses to be the Vice Guild Lord."; // error 9: target refuses to be Vice Guild Lord
                case 10: return "did not respond."; // error 10: no response
                case 11: return "The target refuses to be the Guild Lord."; // error 11: target refuses to be Guild Lord
                case 12: return " Agree to be the Vice Guild Lord."; // error 12: target agrees to be Vice Guild Lord
                case 13: return "Not enough money, fail to contribute."; // error 13: contribution lacks money
                case 14: return "Not enough quantity, contribution failed."; // error 14: contribution lacks quantity
                case 15: return "The material is full."; // error 15: material store full
                case 16: return "Cannot contribute item."; // error 16: item cannot be contributed
                case 17: return "The contribution is full."; // error 17: contribution full
                case 18: return "You must equip a hoe."; // error 18: a hoe must be equipped
                case 19: return "Not enough level, unable to produce."; // error 19: level too low to produce
                case 20: return "Unable to produce unless you join a guild."; // error 20: producing needs a guild
                case 21: return "You are forbidden to upload the guild badge."; // error 21: badge upload forbidden
                case 22: return "The target is busy now."; // error 22: target busy
                case 23: return "Unable to lay items any more."; // error 23: no more items can be laid
                case 24: return "Only a large city can do this."; // error 24: needs a large city
                case 25: return "Only the guild leader can do this."; // error 25: guild leader only
                case 26: return "Not enough contribution."; // error 26: contribution too low
                case 27: return "The positions are full."; // error 27: positions full
                case 100: return "Not enough right, unable to create."; // error 100: no right to create
                case 101: return "Not enough material, stop to create."; // error 101: not enough material to create
                case 254: return "Other reasons"; // error 254: other reasons
                case 255: return "Unable to use the function"; // error 255: function unusable
                default: return std::nullopt; // unknown code: no text
            }
        }

    }

    void WorldScene::onGuildInfo(const protocol::GuildInfo& info) {
        m_guild.replace(info);
        importMailLogs(MailLogImporter::Kind::Guild);
        m_player.title = info.name;
        m_guildWindow.refresh();
        layoutGuildInvite();
        m_groupWindow.refresh();
        if (info.disbandDate != 0) {
            m_chat.add(0, disbandNotice(info.disbandDate));
        }
    }

    void WorldScene::onGuildTitle(std::int32_t playerId, std::uint32_t guildId, const std::string& guild) {
        if (playerId == m_player.id) {
            m_player.title = guild;
            m_player.guildId = guildId;
            m_guild.setName(guild);
            if (guild.empty()) {
                m_groupWindow.guildGone();
            }
            m_groupWindow.refresh();
        } else if (const auto other = m_others.find(playerId); other != m_others.end()) {
            other->second.title = guild;
            other->second.guildId = guildId;
        }
    }

    std::filesystem::path WorldScene::ensignFile() const {
        // grouped servers share one folder per hundred server ids (id / 100)
        const int folder = m_services.session.serverGrouped ? m_services.session.serverId / 100 : m_services.session.serverId;
        return m_services.game.userRoot() / ("Server" + std::to_string(folder)) / "AllOrgEnsign.dat";
    }

    void WorldScene::onBadgeList(const std::vector<BadgeRecord>& listed) {
        std::vector<std::pair<std::uint32_t, int>> entries;
        for (const BadgeRecord& record : listed) {
            entries.emplace_back(record.guildId, record.version);
        }
        m_ensigns.setList(entries, ensignFile());
    }

    void WorldScene::onBadges(const std::vector<BadgeRecord>& badges) {
        for (const BadgeRecord& record : badges) {
            m_ensigns.received(record.guildId, record.version, formats::EmblemBlock::parse(record.pixels), ensignFile());
        }
    }

    void WorldScene::onOwnBadge(int version, const core::Bytes& pixels) {
        m_ensigns.setOwn(m_player.guildId, version, formats::EmblemBlock::parse(pixels));
        m_guildWindow.badgeChanged();
        m_messages.show("The group badge has been updated.", 3000); // message display time in ms
    }

    void WorldScene::onBadgeReset(std::uint32_t guildId, int sampleNumber) {
        const std::string name =
            std::string("sample") + (sampleNumber < 10 ? "0" : "") + std::to_string(sampleNumber) + ".ens"; // sample numbers below 10 are padded to two digits
        const std::filesystem::path file = m_services.game.userRoot() / name;
        if (const auto data = core::readFile(file); data && m_ensigns.find(guildId)) {
            m_ensigns.replaceData(guildId, formats::EmblemBlock::parse(*data));
            m_messages.show("The group badge must be modified.", 3000); // message display time in ms
        }
    }

    void WorldScene::guildCleared() {
        m_guildWeaponBank.hide();
        m_guild.clear();
        layoutGuildInvite();
        m_groupWindow.guildGone();
    }

    void WorldScene::layoutGuildInvite() { m_functionButtons.setGuildInvite(m_guild.mayInvite(m_player.id)); }

    void WorldScene::onGuildCreateForm() { m_guildCreate.open(); }

    void WorldScene::onGuildCreateResult(int result) {
        switch (result) {
            case 0: // result 0: the guild was founded
                m_guild.setName(m_guildCreate.takeName());
                m_guildBoard.resetPaging();
                if (!m_groupWindow.isOpen()) {
                    m_groupWindow.open();
                }
                m_groupWindow.refresh();
                break;
            case 1: m_messages.show("Failed, invalid guild name", 6000); break; // result 1: invalid name (message 6000 ms)
            case 2: m_messages.show("Failed, unqualified to found a guild", 6000); break; // result 2: not qualified (message 6000 ms)
            case 3: m_messages.show("Failed, you have founded a guild already.", 6000); break; // result 3: already founded one (message 6000 ms)
            case 4: m_messages.show("Failed, the guild is full.", 6000); break; // result 4: guild full (message 6000 ms)
            case 5: m_messages.show("Failed, repeated guild name", 6000); break; // result 5: name already used (message 6000 ms)
            case 255: m_messages.show("Fail, unknown reason", 6000); break; // result 255: unknown reason (message 6000 ms)
            default: break; // other results are ignored
        }
    }

    void WorldScene::onGuildInvite(std::int32_t inviterId) {
        const auto speaker = speakerOf(inviterId);
        if (!speaker) {
            return;
        }
        const auto answer = [this, inviterId](int value) {
            return [this, inviterId, value] { m_services.connection.send(protocol::guildInviteAnswerRequest(inviterId, value)); };
        };
        m_talk.ask(
            "Would you like to join my guild?",
            speaker,
            answer(1),
            answer(2),
            {40, kQuestionMs, answer(3)}
        ); // answers 1 accept, 2 refuse, 3 no answer; 40 characters per line
    }

    void WorldScene::onGuildInviteAnswer(std::int32_t targetId, int answer) {
        const auto found = m_others.find(targetId);
        if (found == m_others.end()) {
            return;
        }
        const std::string& name = found->second.name;
        switch (answer) {
            case 1: m_messages.show(name + " Agree to join the guild.", kNoticeMs); break; // answer 1: accepted
            case 2: m_messages.show(name + " Refuse to join the guild.", kNoticeMs); break; // answer 2: refused
            case 3: m_messages.show(name + " did not respond.", kNoticeMs); break; // answer 3: no response
            case 4: m_messages.show(name + " Invitation failed", kNoticeMs); break; // answer 4: invitation failed
            default: break; // other answers are ignored
        }
    }

    void WorldScene::onGuildMail(std::int32_t senderId, double sendTime, const std::string& text) {
        m_mailBox.receive(senderId, sendTime, text, 2); // mail kind 2: guild mail
        m_services.playSound("sound/wav0155.wav");
    }

    void WorldScene::onGuildMemberGone(std::int32_t id) {
        if (id == m_player.id) {
            m_guildWeaponBank.hide();
        }
        if (const auto name = m_guild.removeMember(id)) {
            m_messages.show("Player " + *name + " List of guild members who left", 2000); // message display time in ms
        }
        layoutGuildInvite();
        m_guildWindow.refresh();
        m_groupWindow.refresh();
    }

    void WorldScene::onGuildLeaveResult(int result, std::int32_t id) {
        switch (result) {
            case 0: // result 0: left (the own player, or the target)
                if (isOwn(id)) {
                    m_messages.show("You have left the guild.", kNoticeMs);
                    guildCleared();
                } else {
                    m_messages.show("The target has left the guild.", kNoticeMs);
                }
                break;
            case 1: m_messages.show("Not in the guild", kNoticeMs); break; // result 1: not in the guild
            case 2: m_messages.show("The Guild Lord is unable to leave.", kNoticeMs); break; // result 2: the lord cannot leave
            case 3: m_messages.show("Not enough right", kNoticeMs); break; // result 3: not enough right
            case 255: m_messages.show("Other reasons", kNoticeMs); break; // result 255: other reasons
            default: break; // other results are ignored
        }
    }

    void WorldScene::onGuildMemberJoined(std::int32_t id) {
        const auto record = m_otherRecords.find(id);
        const auto character = m_others.find(id);
        if (record == m_otherRecords.end() || character == m_others.end()) {
            return;
        }
        const formats::ColorPattern& colors = character->second.look.basePattern;
        // digit positions of the colour pattern that form the second colour code (colorCode5)
        constexpr std::array<int, 9> kSecondCodeDigits = {9, 10, 11, 12, 13, 14, 24, 25, 26};
        std::int32_t second = 0;
        for (int digit : kSecondCodeDigits) {
            second = second * 10 + colors.digit(static_cast<std::size_t>(digit)); // the digits are joined as decimal
        }
        protocol::GuildMemberRecord member;
        member.id = id;
        member.name = record->second.name;
        member.level = record->second.level;
        member.job = record->second.job;
        member.element = record->second.element;
        member.bodyType = character->second.look.bodyType;
        member.head = character->second.look.head;
        member.colorCode1 = colors.code(1);
        member.colorCode5 = second;
        member.nickname = record->second.nickname;
        m_guild.addMember(member);
    }

    void WorldScene::onGuildDisbanded(int result) {
        switch (result) {
            case 0: // result 0: the guild was disbanded
                m_messages.show("The guild is disbanded.", kNoticeMs);
                guildCleared();
                break;
            case 1: m_messages.show("Not in the guild", kNoticeMs); break; // result 1: not in the guild
            case 2: m_messages.show("The Guild Lord is unable to leave.", kNoticeMs); break; // result 2: the lord cannot leave
            case 3: m_messages.show("Not the Guild Lord", kNoticeMs); break; // result 3: not the Guild Lord
            case 255: m_messages.show("Other reasons", kNoticeMs); break; // result 255: other reasons
            default: break; // other results are ignored
        }
    }

    void WorldScene::onGuildRules(const std::string& rules) {
        m_guild.setRules(rules);
        m_guildWindow.showRules(rules);
    }

    void WorldScene::onGuildDisbandDate(double date) {
        if (date != 0) {
            m_chat.add(0, disbandNotice(date));
        } else {
            m_chat.add(0, "Your guild will not be disbanded automatically because the number of member who reach 15 level is more than 10.");
        }
    }

    void WorldScene::onGuildVice(std::int32_t id, bool appointed) {
        if (appointed) {
            if (const auto name = m_guild.appointVice(id)) {
                m_messages.show("[ " + *name + " ] is appointed as vice-leader of guild", 2500); // message display time in ms
            }
        } else {
            m_guild.deposeVice(id);
        }
        m_guildWindow.refresh();
    }

    void WorldScene::onGuildNewLord(int mode, std::int32_t id) {
        constexpr int kAbdicated = 3; // new-lord mode 3: the old lord abdicated
        if (mode == kAbdicated && m_guild.isLord(m_player.id)) {
            m_guildWeaponBank.hide();
        }
        m_guild.newLord(id, mode);
        layoutGuildInvite();
        m_guildWindow.refresh();
    }

    void WorldScene::onGuildPositionError(int code) {
        if (const auto text = positionError(code)) {
            m_messages.show(*text, kNoticeMs);
        }
    }

    void WorldScene::onGuildLordChanged(std::int32_t memberId) {
        if (const auto name = m_guild.swapLord(memberId)) {
            m_messages.show("Change Guild Lord[ " + *name + " ]", 2500); // message display time in ms
            m_guildWindow.refresh();
        }
    }

    void WorldScene::onGuildRenamed(const std::string& name) {
        m_guild.setName(name);
        for (auto& [id, other] : m_others) {
            if (other.guildId == m_player.guildId) {
                other.title = name;
            }
        }
        m_guildWindow.refresh();
    }

    void WorldScene::onGuildViceOffer(std::int32_t lordId) {
        const auto record = m_otherRecords.find(lordId);
        if (record == m_otherRecords.end()) {
            return;
        }
        const auto answer = [this, lordId](int value) {
            return [this, lordId, value] { m_services.connection.send(protocol::viceOfferAnswerRequest(lordId, value)); };
        };
        m_talk.ask(
            record->second.name + " Would you like to be the Vice Guild Lord?",
            std::nullopt,
            answer(1),
            answer(2),
            {40, kQuestionMs, answer(3)}
        ); // answers 1 accept, 2 refuse, 3 no answer; 40 characters per line
    }

    void WorldScene::onGuildLordOffer(std::int32_t lordId) {
        const int index = m_guild.indexOf(lordId);
        if (index < 0) {
            return;
        }
        const auto answer = [this, lordId](int value) {
            return [this, lordId, value] { m_services.connection.send(protocol::lordOfferAnswerRequest(lordId, value)); };
        };
        m_talk.ask(
            m_guild.at(index).record.name + " Would you like to be the Guild Lord?",
            std::nullopt,
            answer(1),
            answer(2),
            {40, kQuestionMs, answer(3)}
        ); // answers 1 accept, 2 refuse, 3 no answer; 40 characters per line
    }

    void WorldScene::onGuildPostAccepted(int post, std::int32_t id) {
        const auto record = m_otherRecords.find(id);
        if (record == m_otherRecords.end()) {
            return;
        }
        if (post == 1) { // post 1: Vice Guild Lord
            m_messages.show(record->second.name + " Agree to be the Vice Guild Lord.", 1500); // message display time in ms
        } else if (post == 2) { // post 2: Guild Lord
            m_messages.show(record->second.name + " Agree to be the Guild Lord.", 1500); // message display time in ms
        }
    }

    void WorldScene::onGuildWeaponCount(int kind, std::int32_t count) {
        if (kind >= 1 && kind < static_cast<int>(m_guildWeaponCounts.size())) {
            m_guildWeaponCounts[static_cast<std::size_t>(kind)] = count;
        }
    }

    void WorldScene::onGuildContribution(std::int32_t id, std::int32_t contribution) { m_guild.setContribution(id, contribution); }

    void WorldScene::onGuildMemberInfo(const protocol::GuildMemberRecord& record) { m_guild.updateMember(record); }

    void WorldScene::onGuildRuleHelp() {
        m_guildWindow.showRules(
            "Guild Lords, the usage of\rrules is easy as follows:\r1.Less than 100 letters\r2.Choose and modify the rules\r3.Enter the words you want to \r  say in the dialogue."
        );
    }

    void WorldScene::onGuildWork(const core::Bytes& payload) {
        switch (m_guildWork.received(payload, m_nowMs)) {
            case GuildWork::Pose::Stand: startAction(m_player, 8); break; // stand up: action slot 8
            case GuildWork::Pose::Recruit: startAction(m_player, 54); break; // recruiting: action slot 54
            case GuildWork::Pose::Create: startAction(m_player, 34); break; // crafting: action slot 34
            case GuildWork::Pose::Feather: startAction(m_player, 42); break; // feather work: action slot 42
            case GuildWork::Pose::None: break; // no pose change
        }
    }

    void WorldScene::paintGuildWork(gfx::Canvas& canvas, core::Point feet) {
        if (m_player.stall) {
            return;
        }
        const gfx::TextStyle style{core::Color::fromRgb565(65535), gfx::TextEffect::Shadow, std::nullopt}; // 65535: white in RGB565
        for (const GuildWork::Overhead& overhead : m_guildWork.overheads(m_nowMs)) {
            // texts sit 15 px left of the feet, upper line 15 px above, lower line at the feet; 8 px width per character, text height 12
            if (!overhead.upper.empty()) {
                canvas.drawText({feet.x - 15, feet.y - 15}, overhead.upper, style, static_cast<int>(overhead.upper.size()) * 8, 12);
            }
            canvas.drawText({feet.x - 15, feet.y}, overhead.lower, style, static_cast<int>(overhead.lower.size()) * 8, 12);
            if (const auto frame = m_services.pictures.find("icon_transition1")) {
                canvas.drawPicture(*frame, {feet.x - 15, feet.y + 15}); // the time bar frame sits 15 px left of and 15 px below the feet
            }
            if (const auto bar = m_services.pictures.find("icon_transition2"); bar && overhead.barLength > 0) {
                canvas.drawPicture(
                    *bar,
                    {feet.x - 15, feet.y + 15},
                    core::Rect{0, 0, overhead.barLength * 3, 11}
                ); // the bar is 3 px wide per length unit and 11 px high
            }
        }
    }

    void WorldScene::onGuildRight(std::int32_t id, int right, bool on) {
        m_guild.setRights(id, right, on);
        if (isOwn(id)) {
            layoutGuildInvite();
        }
    }

    void WorldScene::onGuildWelcome() { m_messages.show("You're welcome to join[ " + m_player.title + " ]", 1500); } // message display time in ms

    void WorldScene::onGuildStates(const std::vector<protocol::FriendState>& states) {
        for (const protocol::FriendState& state : states) {
            m_guild.setOnlineState(state.id, state.state);
        }
    }

    void WorldScene::startGuildInvite() { setMouseMode(kInviteMouseMode); }

    void WorldScene::inviteToGuild(std::int32_t playerId) {
        if (isGmId(playerId)) {
            m_messages.show("You are unable to invite Cupid.", 1500); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::guildInviteRequest(playerId));
    }

    void WorldScene::onAllianceMessage(const core::Bytes& payload) {
        constexpr int kMessageMs = 5000; // message display time, ms
        constexpr int kAnswerMs = 6000; // message display time of the other guild's answer, ms
        constexpr int kAskMs = 10000; // ally question auto-answer time, ms
        static constexpr int kAnswerSub = 2; // sub id 2 of message 43: the answer to an ally request
        core::ByteReader reader(payload, 1);
        const int sub = reader.u8();
        const auto leaderOnline = [this](std::uint32_t orgId) {
            const auto entry = m_allies.find(orgId);
            return entry && m_otherRecords.count(entry->get().leaderId) != 0;
        };
        const auto answer = [this, leaderOnline](int code) {
            const std::uint32_t orgId = m_allies.pending();
            if (!leaderOnline(orgId)) {
                m_messages.show("The opponent orgleader is offline.", PlayerMessages::kDefaultDurationMs);
            } else {
                core::ByteWriter writer;
                // message id 43/2: the answer to an ally request
                writer.u8(43).u8(kAnswerSub).u32(orgId).u8(static_cast<std::uint8_t>(code));
                m_services.connection.send(writer.take());
            }
            m_allies.setPending(0);
        };
        switch (sub) {
            case 1: { // 43/1: an ally request arrives (u32 orgId), asked with a timed question
                const std::uint32_t orgId = reader.u32();
                const auto entry = m_allies.find(orgId);
                if (reader.failed() || !entry) {
                    return;
                }
                m_allies.setPending(orgId);
                TalkDialog::Timing timing;
                timing.timeoutMs = kAskMs;
                timing.onTimeout = [answer] { answer(3); };
                // answers 1 accept, 2 refuse, 3 no answer
                m_talk.ask(
                    "Would you like to " + entry->get().orgName + "be allied.",
                    std::nullopt,
                    [answer] { answer(1); },
                    [answer] { answer(2); },
                    std::move(timing)
                );
                break;
            }
            case 2: { // 43/2: the answer of the other guild (u8 code, u32 allyId, u32 allianceId)
                const int code = reader.u8();
                const std::uint32_t allyId = reader.u32();
                const std::uint32_t allianceId = reader.u32();
                const auto entry = m_allies.find(allyId);
                if (reader.failed() || !entry) {
                    return;
                }
                const auto leader = m_otherRecords.find(entry->get().leaderId);
                std::string text = entry->get().orgName + " " + (leader != m_otherRecords.end() ? leader->second.name : std::string());
                switch (code) {
                    case 1: // code 1: accepted, the alliance is set
                        text += " Accept";
                        m_allies.setOwnAlliance(allianceId);
                        if (allyId != m_allies.own().orgId) {
                            entry->get().allianceId = allianceId;
                            m_allies.sort();
                        }
                        break;
                    case 2: text += " Refuse"; break; // code 2: refused
                    case 3: text += " did not respond."; break; // code 3: no response
                    case 4: text += " Fail to be allied."; break; // code 4: failed
                    case 5: text += " The opponent closes the Ally function."; break; // code 5: the other side has the ally function closed
                    default: break; // other codes add no text
                }
                m_messages.show(text, kAnswerMs);
                break;
            }
            case 3: { // 43/3: the alliance was disbanded or the request refused (u8 code)
                const int code = reader.u8();
                std::string text;
                switch (code) {
                    case 0: // code 0: the allies are disbanded
                        text = "Disband Allies.";
                        m_allies.disband();
                        break;
                    case 1: text = "No Ally."; break; // code 1: no ally
                    case 2: text = "Only the orgleader can use the function."; break; // code 2: only the orgleader may
                    case 255: text = "Error when you leave Allies."; break; // code 255: error on leaving
                    default: break; // other codes give no text
                }
                m_messages.show(text, kMessageMs);
                constexpr int kAllyChannel = 6; // chat channel 6: ally
                constexpr int kGuildChannel = 2; // chat channel 2: guild
                if (m_allies.own().allianceId == 0 && m_inputBar.channel() == kAllyChannel) {
                    m_inputBar.setChannel(kGuildChannel);
                }
                break;
            }
            case 4: { // 43/4: the list of allies
                m_allies.clear();
                while (reader.remaining() > 2) { // fewer than 3 bytes left cannot hold another entry
                    AllyList::Entry entry;
                    entry.allianceId = reader.u32();
                    entry.orgId = reader.u32();
                    entry.leaderId = reader.i32();
                    entry.leaderName = reader.shortText();
                    entry.orgName = reader.shortText();
                    entry.sortKey = reader.u8();
                    if (reader.failed()) {
                        break;
                    }
                    if (entry.leaderName.empty()) {
                        const auto leader = m_otherRecords.find(entry.leaderId);
                        entry.leaderName = leader != m_otherRecords.end() ? leader->second.name : std::string();
                    }
                    m_allies.add(entry, m_guild.leaderId());
                }
                m_allies.sort();
                break;
            }
            case 5: { // 43/5: an ally left (u32 orgId)
                const std::uint32_t orgId = reader.u32();
                if (!reader.failed()) {
                    m_allies.left(orgId);
                }
                break;
            }
            default: // other sub ids are ignored
                break;
        }
    }

}
