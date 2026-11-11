#include "client/viewmodels/login/ServerTree.h"

#include <algorithm>

namespace client {

    std::string ServerTree::key(Section section) {
        switch (section) {
            case Section::Direct: return "direct"; // servers added by address
            case Section::List: break; // SERVER.INI's section: falls through to "list"
        }
        return "list";
    }

    std::string ServerTree::groupKey(int groupNumber) { return "group:" + std::to_string(groupNumber); }

    std::string ServerTree::rowKey(const Row& row) const {
        if (row.kind == Row::Kind::Group) {
            return groupKey(m_groups[static_cast<std::size_t>(row.group)].first);
        }
        return key(row.section);
    }

    void ServerTree::rebuild(int directCount, const std::vector<std::pair<int, int>>& groups) {
        m_directCount = directCount;
        m_groups = groups;
        m_rows.clear();
        const auto addSection = [this](Section section, int count) {
            const Row header{Row::Kind::Section, section, -1, -1};
            m_rows.push_back(header);
            if (isCollapsed(header)) {
                return;
            }
            for (int i = 0; i < count; ++i) {
                m_rows.push_back({Row::Kind::Server, section, -1, i});
            }
        };
        addSection(Section::Direct, directCount);
        addSection(Section::List, 0);
        if (!isCollapsed(m_rows.back())) {
            for (int group = 0; group < static_cast<int>(groups.size()); ++group) {
                const Row header{Row::Kind::Group, Section::List, group, -1};
                m_rows.push_back(header);
                if (!isCollapsed(header)) {
                    for (int i = 0; i < groups[static_cast<std::size_t>(group)].second; ++i) {
                        m_rows.push_back({Row::Kind::Server, Section::List, group, i});
                    }
                }
            }
        }
        if (m_selected && std::find(m_rows.begin(), m_rows.end(), *m_selected) == m_rows.end()) {
            m_selected.reset();
        }
    }

    bool ServerTree::isCollapsed(const Row& row) const { return row.kind != Row::Kind::Server && m_collapsed.count(rowKey(row)) > 0; }

    void ServerTree::toggle(const Row& row) {
        if (row.kind == Row::Kind::Server) {
            return;
        }
        const std::string folded = rowKey(row);
        if (!m_collapsed.erase(folded)) {
            m_collapsed.insert(folded);
        }
    }

    void ServerTree::select(std::optional<Row> row) { m_selected = row && row->kind == Row::Kind::Server ? row : std::nullopt; }

    int ServerTree::selectedIndex() const {
        if (!m_selected) {
            return -1;
        }
        const auto found = std::find(m_rows.begin(), m_rows.end(), *m_selected);
        return found == m_rows.end() ? -1 : static_cast<int>(found - m_rows.begin());
    }

    int ServerTree::count(const Row& row) const {
        if (row.kind == Row::Kind::Group) {
            return m_groups[static_cast<std::size_t>(row.group)].second;
        }
        switch (row.section) {
            case Section::Direct: return m_directCount; // count of added servers
            case Section::List: break; // SERVER.INI's section: sum of its groups below
        }
        int servers = 0;
        for (const auto& group : m_groups) {
            servers += group.second;
        }
        return servers;
    }

}
