#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_SERVER_TREE_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_SERVER_TREE_H

#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace client {

    /// @brief Rows of the qol server list: two foldable sections, SERVER.INI's foldable groups under its section, and the servers.
    class ServerTree {
        public:
            /// @brief The two top-level sections: servers added by address, and SERVER.INI's.
            enum class Section { Direct, List };

            /// @brief One row of the list.
            struct Row {
                /// @brief What a row is.
                enum class Kind { Section, Group, Server };
                Kind kind = Kind::Section;
                Section section = Section::Direct;
                int group = -1;
                int index = -1;

                /// @brief Compares two rows field by field.
                /// @param other Row to compare with.
                /// @return True when equal.
                bool operator==(const Row& other) const {
                    return kind == other.kind && section == other.section && group == other.group && index == other.index;
                }
            };

            /// @brief Gives the fold key of a section in saved-servers.json ("direct" or "list").
            /// @param section The section.
            /// @return The key.
            static std::string key(Section section);

            /// @brief Gives the fold key of a SERVER.INI group ("group:<number>").
            /// @param groupNumber The group's number.
            /// @return The key.
            static std::string groupKey(int groupNumber);

            /// @brief Rebuilds the rows; the selection is kept when its row is still shown.
            /// @param directCount Number of servers added by address.
            /// @param groups Each SERVER.INI group's number and server count.
            void rebuild(int directCount, const std::vector<std::pair<int, int>>& groups);

            /// @brief Gives the current rows.
            /// @return The rows, top to bottom.
            const std::vector<Row>& rows() const { return m_rows; }

            /// @brief Sets the fold keys that are folded.
            /// @param collapsed Keys of folded sections and groups.
            void setCollapsed(std::set<std::string> collapsed) { m_collapsed = std::move(collapsed); }

            /// @brief Gives the folded keys.
            /// @return The folded keys.
            const std::set<std::string>& collapsed() const { return m_collapsed; }

            /// @brief Tells whether a section or group row is folded.
            /// @param row The row.
            /// @return True when folded.
            bool isCollapsed(const Row& row) const;

            /// @brief Folds or unfolds a section or group row (rebuild afterwards).
            /// @param row The row; server rows are ignored.
            void toggle(const Row& row);

            /// @brief Gives the selected server's row; only a Server row is selected.
            /// @return The selected row, if any.
            const std::optional<Row>& selected() const { return m_selected; }

            /// @brief Selects a row if it is a Server row, else clears the selection.
            /// @param row The row to select.
            void select(std::optional<Row> row);

            /// @brief Gives the selected row's position in the rows.
            /// @return The index, or -1 when nothing is selected or shown.
            int selectedIndex() const;

            /// @brief Gives the count shown on a section or group row.
            /// @param row The row.
            /// @return Number of servers under it.
            int count(const Row& row) const;

        private:
            /// @brief Gives the fold key of a section or group row.
            /// @param row The row.
            /// @return The key.
            std::string rowKey(const Row& row) const;

            std::vector<Row> m_rows;
            std::set<std::string> m_collapsed;
            std::optional<Row> m_selected;
            int m_directCount = 0;
            std::vector<std::pair<int, int>> m_groups;
    };

}

#endif
