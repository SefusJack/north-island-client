#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_ALLY_LIST_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_ALLY_LIST_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The guilds the own guild may ally with, and its alliance.
    /// @details Original: TCY_OrganManage.FAllyList and FOwnOrgData (CY_Organ.pas); each entry has a guild, its lord, its alliance (0: none) and a sort key.
    class AllyList {
        public:
            /// @brief One guild of the list.
            struct Entry {
                std::uint32_t allianceId = 0;
                std::uint32_t orgId = 0;
                std::int32_t leaderId = 0;
                std::string leaderName;
                std::string orgName;
                int sortKey = 0;
            };

            /// @brief Empties the list and the own entry.
            /// @details Original: ClearAllyList.
            void clear();

            /// @brief Adds an entry; the entry of the own guild's lord is kept apart as the own one.
            /// @details Original: AddAllyEntry.
            /// @param entry The guild entry.
            /// @param ownLeaderId Id of the own guild's lord.
            void add(const Entry& entry, std::int32_t ownLeaderId);

            /// @brief Sorts the list: guilds of the own alliance first, then the higher keys.
            /// @details Original: CompareAllyEntries.
            void sort();

            /// @brief Finds the entry of a guild.
            /// @details Original: FindAllyEntry.
            /// @param orgId Guild id.
            /// @return The entry, or nothing.
            std::optional<std::reference_wrapper<Entry>> find(std::uint32_t orgId);

            /// @brief A guild is in no alliance any more.
            /// @details Original: RecvAllyLeft.
            /// @param orgId Guild id.
            void left(std::uint32_t orgId);

            /// @brief The own alliance is gone; when one guild stayed in it, it goes too.
            /// @details Original: RecvAllyDisband (0).
            void disband();

            /// @brief The guilds of the list.
            /// @return The entries.
            const std::vector<Entry>& entries() const { return m_entries; }

            /// @brief The own guild's entry.
            /// @return The entry.
            const Entry& own() const { return m_own; }

            /// @brief Sets the alliance of the own guild.
            /// @param allianceId Alliance id (0: none).
            void setOwnAlliance(std::uint32_t allianceId) { m_own.allianceId = allianceId; }

            /// @brief The guild a request or an invitation is about.
            /// @details Original: FPendingAllyId.
            /// @return Guild id, 0 for none.
            std::uint32_t pending() const { return m_pending; }

            /// @brief Sets the guild a request or an invitation is about.
            /// @param orgId Guild id, 0 for none.
            void setPending(std::uint32_t orgId) { m_pending = orgId; }

        private:
            std::vector<Entry> m_entries;
            Entry m_own;
            std::uint32_t m_pending = 0;
    };

}

#endif
