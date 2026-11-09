#include "client/models/guild/Guild.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kRightSetAuthority = 1;
        constexpr int kRightInvite = 2;

    }

    int Guild::indexOf(std::int32_t id) const {
        for (int i = 0; i < count(); ++i) {
            if (at(i).record.id == id) {
                return i;
            }
        }
        return -1;
    }

    bool Guild::mayInvite(std::int32_t id) const {
        const int index = indexOf(id);
        return index >= 0 && at(index).record.rights[kRightInvite - 1];
    }

    bool Guild::maySetAuthority(std::int32_t id) const {
        const int index = indexOf(id);
        return index >= 0 && at(index).record.rights[kRightSetAuthority - 1];
    }

    int Guild::checkedCount() const {
        return static_cast<int>(std::count_if(m_members.begin(), m_members.end(), [](const GuildMember& m) { return m.checked; }));
    }

    void Guild::uncheckAll() {
        for (GuildMember& member : m_members) {
            member.checked = false;
        }
    }

    void Guild::replace(const protocol::GuildInfo& info) {
        m_name = info.name;
        m_viceCount = info.viceCount;
        m_members.clear();
        for (const protocol::GuildMemberRecord& record : info.members) {
            m_members.push_back({record, false});
        }
        m_rules = info.rules;
    }

    void Guild::clear() {
        m_name.clear();
        m_members.clear();
        m_viceCount = 0;
        m_rules.clear();
    }

    void Guild::addMember(const protocol::GuildMemberRecord& record) { m_members.push_back({record, false}); }

    GuildMember Guild::takeOut(int index) {
        GuildMember member = std::move(m_members[static_cast<std::size_t>(index)]);
        m_members.erase(m_members.begin() + index);
        if (index >= 1 && index <= m_viceCount) {
            --m_viceCount;
        }
        return member;
    }

    std::optional<std::string> Guild::removeMember(std::int32_t id) {
        const int index = indexOf(id);
        if (index < 0) {
            return std::nullopt;
        }
        return takeOut(index).record.name;
    }

    std::optional<std::string> Guild::appointVice(std::int32_t id) {
        const int index = indexOf(id);
        if (index <= m_viceCount || m_viceCount >= kMostVices) {
            return std::nullopt;
        }
        GuildMember member = takeOut(index);
        const std::string name = member.record.name;
        m_members.insert(m_members.begin() + 1 + m_viceCount, std::move(member));
        ++m_viceCount;
        return name;
    }

    void Guild::deposeVice(std::int32_t id) {
        const int index = indexOf(id);
        if (index < 1 || index > m_viceCount) {
            return;
        }
        m_members.push_back(takeOut(index));
    }

    std::optional<std::string> Guild::swapLord(std::int32_t id) {
        const int index = indexOf(id);
        if (index < 1) {
            return std::nullopt;
        }
        std::swap(m_members.front(), m_members[static_cast<std::size_t>(index)]);
        return m_members.front().record.name;
    }

    void Guild::newLord(std::int32_t id, int mode) {
        const int index = indexOf(id);
        if (index < 1) {
            return;
        }
        const bool wasMember = index > m_viceCount;
        GuildMember lord = takeOut(index);
        GuildMember old = std::move(m_members.front());
        m_members.erase(m_members.begin());
        m_members.insert(m_members.begin(), std::move(lord));
        switch (mode) {
            case 2: // 39/14 mode 2: the old lord becomes a vice lord
                m_members.insert(m_members.begin() + 1 + m_viceCount, std::move(old));
                ++m_viceCount;
                break;
            case 3: // 39/14 mode 3: the old lord becomes a member
                if (wasMember) {
                    m_members.insert(m_members.begin() + std::min<int>(index, count()), std::move(old));
                } else {
                    m_members.push_back(std::move(old));
                }
                break;
            default: // 39/14 mode 0: the old lord is dropped
                break;
        }
    }

    void Guild::setRights(std::int32_t id, int right, bool on) {
        const int index = indexOf(id);
        if (index >= 0 && right >= 1 && right <= 5) { // five member rights
            at(index).record.rights[static_cast<std::size_t>(right - 1)] = on;
        }
    }

    void Guild::setOnlineState(std::int32_t id, int state) {
        const int index = indexOf(id);
        if (index >= 0) {
            at(index).onlineState = state;
        }
    }

    void Guild::updateMember(const protocol::GuildMemberRecord& record) {
        const int index = indexOf(record.id);
        if (index < 0) {
            return;
        }
        protocol::GuildMemberRecord& member = at(index).record;
        member.name = record.name;
        member.level = record.level;
        member.job = record.job;
        member.reborn = record.reborn;
        member.element = record.element;
        member.bodyType = record.bodyType;
        member.head = record.head;
        member.colorCode1 = record.colorCode1;
        member.colorCode5 = record.colorCode5;
        member.nickname = record.nickname;
    }

    void Guild::setContribution(std::int32_t id, std::int32_t contribution) {
        const int index = indexOf(id);
        if (index >= 0) {
            at(index).record.contribution = contribution;
        }
    }

}
