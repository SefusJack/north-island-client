#include "client/models/social/FriendList.h"

namespace client {

    namespace {

        constexpr std::int32_t kGmCenter = 100; // player id 100: GM center
        constexpr std::int32_t kGmCenter2 = 101; // player id 101: GM center (second id)

        /// @brief Tells whether an id is one of the two GM center ids.
        /// @param id Player id.
        /// @return True for the GM row ids.
        bool isGmRow(std::int32_t id) { return id == kGmCenter || id == kGmCenter2; }

    }

    FriendList::FriendList() {
        Friend gm;
        gm.record.id = kGmCenter;
        gm.record.name = "GM center";
        m_friends.push_back(gm);
    }

    int FriendList::indexOf(std::int32_t id) const {
        if (id <= 0) {
            return 0;
        }
        for (int i = 1; i <= count(); ++i) {
            if (at(i).record.id == id) {
                return i;
            }
        }
        return 0;
    }

    void FriendList::replace(const std::vector<protocol::FriendRecord>& records) {
        std::vector<int> states;
        for (const Friend& f : m_friends) {
            states.push_back(f.onlineState);
        }
        m_friends.resize(1);
        for (const protocol::FriendRecord& record : records) {
            if (isGmRow(record.id)) {
                m_friends[0].record = record;
            } else if (count() < kMostFriends) {
                Friend f;
                f.record = record;
                m_friends.push_back(f);
            }
        }
        for (std::size_t i = 0; i < m_friends.size() && i < states.size(); ++i) {
            m_friends[i].onlineState = states[i];
        }
    }

    void FriendList::put(const protocol::FriendRecord& record) {
        protocol::FriendRecord entry = record;
        entry.guild.clear();
        if (isGmRow(record.id)) {
            m_friends[0].record = entry;
            return;
        }
        if (const int index = indexOf(record.id)) {
            at(index).record = entry;
            return;
        }
        add(entry);
    }

    void FriendList::update(const protocol::FriendRecord& record) {
        if (const int index = indexOf(record.id)) {
            at(index).record = record;
        }
    }

    void FriendList::add(const protocol::FriendRecord& record) {
        if (count() >= kMostFriends) {
            return;
        }
        Friend f;
        f.record = record;
        m_friends.push_back(f);
    }

    std::optional<std::string> FriendList::remove(std::int32_t id) {
        const int index = indexOf(id);
        if (index == 0) {
            return std::nullopt;
        }
        const std::string name = at(index).record.name;
        m_friends.erase(m_friends.begin() + index);
        return name;
    }

    void FriendList::setOnlineState(std::int32_t id, int state) {
        if (const int index = indexOf(id)) {
            at(index).onlineState = state;
        }
    }

    void FriendList::setGuild(std::int32_t id, const std::string& guild) {
        if (const int index = indexOf(id)) {
            at(index).record.guild = guild;
        }
    }

}
