#include "client/models/guild/AllyList.h"

#include <algorithm>

namespace client {

    void AllyList::clear() {
        m_entries.clear();
        m_own = Entry{};
    }

    void AllyList::add(const Entry& entry, std::int32_t ownLeaderId) {
        if (entry.leaderId == ownLeaderId) {
            m_own = entry;
        } else {
            m_entries.push_back(entry);
        }
    }

    void AllyList::sort() {
        const std::uint32_t own = m_own.allianceId;
        std::stable_sort(m_entries.begin(), m_entries.end(), [own](const Entry& a, const Entry& b) {
            if (own != 0) {
                if (a.allianceId == own && b.allianceId != own) {
                    return true;
                }
                if (a.allianceId != own && b.allianceId == own) {
                    return false;
                }
            }
            return a.sortKey > b.sortKey;
        });
    }

    std::optional<std::reference_wrapper<AllyList::Entry>> AllyList::find(std::uint32_t orgId) {
        const auto found = std::find_if(m_entries.begin(), m_entries.end(), [orgId](const Entry& entry) { return entry.orgId == orgId; });
        if (found == m_entries.end()) {
            return std::nullopt;
        }
        return std::ref(*found);
    }

    void AllyList::left(std::uint32_t orgId) {
        if (const auto entry = find(orgId)) {
            entry->get().allianceId = 0;
        }
        sort();
    }

    void AllyList::disband() {
        const std::uint32_t own = m_own.allianceId;
        int staying = 0;
        std::optional<std::reference_wrapper<Entry>> last;
        for (Entry& entry : m_entries) {
            if (entry.allianceId == own) {
                ++staying;
                last = std::ref(entry);
            }
        }
        m_own.allianceId = 0;
        if (staying == 1) {
            last->get().allianceId = 0;
            sort();
        }
    }

}
