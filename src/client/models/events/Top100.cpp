#include "client/models/events/Top100.h"

#include "core/ByteReader.h"
#include "protocol/Top100Requests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kMaxServerLabel = 8; // server label width, characters (shorter names are padded)
        constexpr std::size_t kLongestLabel = 10; // longest server label, characters
        constexpr int kMostQuiz = 20; // most places kept in the Q&A list

        /// @brief Builds the label of a server group shown before a player's name in an event list.
        /// @param group Name of the server group.
        /// @return The label, at most 10 characters.
        std::string serverLabel(const std::string& group) {
            const std::size_t open = group.find('(');
            const std::size_t close = group.find(')');
            std::string label;
            if (open != std::string::npos && close != std::string::npos) {
                label = group.substr(0, open);
            } else if (group.size() > kMaxServerLabel) {
                label = group.substr(0, kMaxServerLabel);
            } else {
                label = group + std::string(kMaxServerLabel - group.size(), ' ');
            }
            return label.substr(0, kLongestLabel);
        }

    }

    Top100::Top100(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    core::Bytes Top100::request(int kind, int page) { return protocol::top100Request(kind, page); }

    const Top100::Place& Top100::event(int tab, int place) const { return m_events[tab == 1 ? 0 : 1][static_cast<std::size_t>(place)]; }

    int Top100::rowsPerPage() const { return m_kind == Kind::EventFirst || m_kind == Kind::EventSecond ? kEventRowsPerPage : kRowsPerPage; }

    int Top100::pageCount() const {
        int count = 0;
        switch (m_kind) {
            case Kind::Level: count = m_levelCount; break; // level list size
            case Kind::Money: count = m_moneyCount; break; // money list size
            case Kind::Quiz: count = m_quizCount; break; // Q&A list size
            case Kind::Pk: count = m_pkCount; break; // PK list size
            case Kind::EventFirst: count = m_eventCounts[0]; break; // first event list size
            case Kind::EventSecond: count = m_eventCounts[1]; break; // second event list size
            case Kind::None: break; // no list shown
        }
        return count % rowsPerPage() == 0 ? count / rowsPerPage() : count / rowsPerPage() + 1;
    }

    void Top100::open() {
        if (m_levelCount == 0 || m_moneyCount == 0) {
            m_kind = Kind::None;
            request();
        } else {
            showLevel();
        }
    }

    bool Top100::pageMissing() const {
        for (int place = (m_page - 1) * kRowsPerPage + 1; place <= m_page * kRowsPerPage; ++place) {
            if (placeEmpty(place)) {
                return true;
            }
        }
        return false;
    }

    void Top100::showLevel() {
        m_kind = Kind::Level;
        m_page = 1;
        if (pageMissing() || m_myLevelRank == 0) {
            request();
        }
    }

    void Top100::showMoney() {
        m_kind = Kind::Money;
        m_page = 1;
        if (pageMissing() || m_myMoneyRank == 0) {
            request();
        }
    }

    void Top100::showQuiz() {
        m_kind = Kind::Quiz;
        m_page = 1;
        if (pageMissing()) {
            request();
        }
    }

    void Top100::showPk() {
        m_kind = Kind::Pk;
        m_page = 1;
        for (int place = 1; place <= kRowsPerPage; ++place) {
            if (place > m_pkCount) {
                return;
            }
            if (m_pk[static_cast<std::size_t>(place)].name.empty()) {
                request();
                return;
            }
        }
    }

    void Top100::showEvents() { showEventTab(m_eventTab); }

    void Top100::showEventTab(int tab) {
        m_page = 1;
        const int chosen = tab == 2 ? 2 : 1; // event tab 1 or 2 (anything else is tab 1)
        m_kind = chosen == 1 ? Kind::EventFirst : Kind::EventSecond;
        m_eventTab = chosen;
        const std::vector<Place>& places = m_events[static_cast<std::size_t>(chosen - 1)];
        for (int place = 1; place <= kEventRowsPerPage; ++place) {
            if (place > m_eventCounts[static_cast<std::size_t>(chosen - 1)]) {
                return;
            }
            if (places[static_cast<std::size_t>(place)].name.empty() || m_refreshKind == static_cast<int>(m_kind)) {
                request();
                if (m_refreshKind == static_cast<int>(m_kind)) {
                    m_refreshKind = 0;
                }
                return;
            }
        }
    }

    void Top100::previousPage() {
        if (m_page > 1) {
            --m_page;
        }
    }

    int Top100::currentCount() const {
        switch (m_kind) {
            case Kind::Level: return m_levelCount; // level list size
            case Kind::Money: return m_moneyCount; // money list size
            case Kind::Quiz: return m_quizCount; // Q&A list size
            case Kind::Pk: return m_pkCount; // PK list size
            case Kind::EventFirst: return m_eventCounts[0]; // first event list size
            case Kind::EventSecond: return m_eventCounts[1]; // second event list size
            case Kind::None: break; // no list shown
        }
        return 0;
    }

    bool Top100::placeEmpty(int place) const {
        switch (m_kind) {
            case Kind::Level: return m_levels[static_cast<std::size_t>(place)].name.empty(); // level place not received
            case Kind::Money: return m_money[static_cast<std::size_t>(place)].empty(); // money place not received
            case Kind::Quiz: return place >= static_cast<int>(m_quiz.size()) || m_quiz[static_cast<std::size_t>(place)].empty(); // Q&A place not received
            case Kind::Pk: return m_pk[static_cast<std::size_t>(place)].name.empty(); // PK place not received
            case Kind::EventFirst: return m_events[0][static_cast<std::size_t>(place)].name.empty(); // first event place not received
            case Kind::EventSecond: return m_events[1][static_cast<std::size_t>(place)].name.empty(); // second event place not received
            case Kind::None: break; // no list shown
        }
        return true;
    }

    void Top100::nextPage() {
        const int perPage = rowsPerPage();
        const int count = currentCount();
        if (m_kind == Kind::None || m_page * perPage >= count || placeEmpty((m_page - 1) * perPage + 1)) {
            return;
        }
        ++m_page;
        if (m_kind != Kind::Level && m_kind != Kind::Money && m_kind != Kind::Quiz) {
            return;
        }
        for (int place = (m_page - 1) * perPage + 1; place <= m_page * perPage; ++place) {
            if (place > count) {
                return;
            }
            if (placeEmpty(place)) {
                request();
                return;
            }
        }
    }

    void Top100::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip the two leading bytes of the message
        const int kind = reader.u8();
        const int page = reader.u8();
        if (reader.failed()) {
            return;
        }
        switch (kind) {
            case 0: // server list message kind 0: list sizes (i32 levelCount, i32 moneyCount, u8 quiz, u8 pk, u8 event1, u8 event2)
                m_levelCount = reader.i32();
                m_moneyCount = reader.i32();
                m_quizCount = reader.u8();
                m_pkCount = reader.u8();
                m_eventCounts[0] = reader.u8();
                m_eventCounts[1] = reader.u8();
                m_kind = Kind::None;
                showLevel();
                break;
            case 1: // kind 1: a page of the level list (u8 job, u8 level, name per place; page 1 adds own rank and header)
                for (int i = 1; i <= kRowsPerPage; ++i) {
                    const int place = i + (page - 1) * kRowsPerPage;
                    if (place > m_levelCount || place > kMostRanks) {
                        break;
                    }
                    LevelRank rank;
                    rank.job = reader.u8();
                    rank.level = reader.u8();
                    rank.name = reader.shortText();
                    if (reader.failed()) {
                        return;
                    }
                    m_levels[static_cast<std::size_t>(place)] = rank;
                }
                if (page == 1) {
                    m_myLevelRank = reader.i32();
                    m_header = reader.rest();
                }
                break;
            case 2: // kind 2: a page of the money list (name per place; page 1 adds own rank)
                for (int i = 1; i <= kRowsPerPage; ++i) {
                    const int place = i + (page - 1) * kRowsPerPage;
                    if (place > m_moneyCount || place > kMostRanks) {
                        break;
                    }
                    const std::string name = reader.shortText();
                    if (reader.failed()) {
                        return;
                    }
                    m_money[static_cast<std::size_t>(place)] = name;
                }
                if (page == 1) {
                    m_myMoneyRank = reader.i32();
                }
                break;
            case 3: // kind 3: a page of the Q&A list (name per place)
                for (int i = 1; i <= kRowsPerPage; ++i) {
                    const int place = i + (page - 1) * kRowsPerPage;
                    if (place > m_quizCount || place > kMostQuiz) {
                        return;
                    }
                    const std::string name = reader.shortText();
                    if (reader.failed()) {
                        return;
                    }
                    m_quiz[static_cast<std::size_t>(place)] = name;
                }
                break;
            case 4: { // kind 4: the PK list (u8 count, then u8 rank and name per place)
                m_pkCount = reader.u8();
                if (m_pkCount == 0) {
                    return;
                }
                m_pk.assign(static_cast<std::size_t>(m_pkCount) + 1, {});
                for (int i = 1; i <= m_pkCount; ++i) {
                    Place place;
                    place.rank = reader.u8();
                    place.name = reader.shortText();
                    if (reader.failed()) {
                        return;
                    }
                    m_pk[static_cast<std::size_t>(i)] = place;
                }
                break;
            }
            case 5: // kind 5: first event list (u8 count, then u8 rank, u8 server code and name per place)
            case 6: { // kind 6: second event list (same layout as kind 5)
                const std::size_t tab = static_cast<std::size_t>(kind - 5); // kinds 5 and 6 are tabs 0 and 1
                m_eventCounts[tab] = reader.u8();
                if (m_eventCounts[tab] == 0) {
                    return;
                }
                m_events[tab].assign(static_cast<std::size_t>(m_eventCounts[tab]) + 1, {});
                for (int i = 1; i <= m_eventCounts[tab]; ++i) {
                    Place place;
                    place.rank = reader.u8();
                    const int code = reader.u8();
                    place.name = reader.shortText();
                    if (reader.failed()) {
                        return;
                    }
                    place.server = serverLabel(m_hooks.serverGroupName(code));
                    m_events[tab][static_cast<std::size_t>(i)] = place;
                }
                break;
            }
            default: // unknown kind: ignored
                break;
        }
    }

}
