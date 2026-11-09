#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_GUILD_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_GUILD_H

#include "client/models/guild/GuildMember.h"
#include "protocol/GuildRecord.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The own guild: name, members (lord first, then vice lords), rules and dates.
    /// @details Original: TCY_OrganManage and the OrgMembers list. The list holds only members that are there, as the guild
    /// pages (FOrgMemberIndices) show them; the guild exists once it has a name.
    class Guild {
        public:
            static constexpr int kMostVices = 4;

            /// @brief Whether the own guild exists (it has a name).
            /// @return True when it has a name.
            bool exists() const { return !m_name.empty(); }

            /// @brief The guild name.
            /// @return The name.
            const std::string& name() const { return m_name; }

            /// @brief Sets the name for the own player before the whole guild has come.
            /// @details Original: server packet 39/9 arrives before 39/2.
            /// @param name Guild name.
            void setName(const std::string& name) { m_name = name; }

            /// @brief Number of members in the list.
            /// @return The member count.
            int count() const { return static_cast<int>(m_members.size()); }

            /// @brief A member by list index.
            /// @param index Index in the list (0 is the lord).
            /// @return The member.
            const GuildMember& at(int index) const { return m_members[static_cast<std::size_t>(index)]; }

            /// @brief A member by list index (mutable).
            /// @param index Index in the list (0 is the lord).
            /// @return The member.
            GuildMember& at(int index) { return m_members[static_cast<std::size_t>(index)]; }

            /// @brief Index of a player in the list.
            /// @param id Player id.
            /// @return The index, -1 for a player not in the guild.
            int indexOf(std::int32_t id) const;

            /// @brief Number of vice lords.
            /// @return The vice lord count.
            int viceCount() const { return m_viceCount; }

            /// @brief Id of the lord.
            /// @details Original: Player.FGuildLeaderId.
            /// @return The lord's id, 0 without a guild.
            std::int32_t leaderId() const { return m_members.empty() ? 0 : m_members.front().record.id; }

            /// @brief Whether a player is the lord.
            /// @param id Player id.
            /// @return True for the lord.
            bool isLord(std::int32_t id) const { return id != 0 && id == leaderId(); }

            /// @brief The guild rules text.
            /// @return The rules.
            const std::string& rules() const { return m_rules; }

            /// @brief Sets the guild rules text.
            /// @param rules The rules.
            void setRules(const std::string& rules) { m_rules = rules; }

            /// @brief Whether a member has right 2 (invite).
            /// @param id Player id.
            /// @return True when allowed.
            bool mayInvite(std::int32_t id) const;

            /// @brief Whether a member has right 1 (set authority).
            /// @param id Player id.
            /// @return True when allowed.
            bool maySetAuthority(std::int32_t id) const;

            /// @brief Number of members whose tick box is set.
            /// @return The count.
            int checkedCount() const;

            /// @brief Clears all tick boxes.
            void uncheckAll();

            /// @brief Replaces the whole guild with the one the server sent.
            /// @details Original: RecvGuildInfo (39/2).
            /// @param info Guild data.
            void replace(const protocol::GuildInfo& info);

            /// @brief Forgets the guild.
            /// @details Original: ClearGuildData.
            void clear();

            /// @brief Adds a member.
            /// @details Original: AddOrgMemberFromWorld (39/8).
            /// @param record The member's record.
            void addMember(const protocol::GuildMemberRecord& record);

            /// @brief Takes a member off the list.
            /// @details Original: ShowLeftMembers (39/6).
            /// @param id Player id.
            /// @return The member's name, or nothing when not in the guild.
            std::optional<std::string> removeMember(std::int32_t id);

            /// @brief The member becomes the last vice lord.
            /// @details Original: RecvViceAppointed (39/12).
            /// @param id Player id.
            /// @return The member's name, or nothing when it cannot be done.
            std::optional<std::string> appointVice(std::int32_t id);

            /// @brief The vice lord goes to the end of the list.
            /// @details Original: RemoveViceLord (39/13).
            /// @param id Player id.
            void deposeVice(std::int32_t id);

            /// @brief The member and the lord change places in the list.
            /// @details Original: RecvLordChanged (39/63).
            /// @param id Player id of the new lord.
            /// @return The new lord's name, or nothing when it cannot be done.
            std::optional<std::string> swapLord(std::int32_t id);

            /// @brief A new lord takes over.
            /// @details Original: SetNewGuildLord (39/14): mode 0 the old lord is dropped, 2 he becomes a vice lord, 3 a member.
            /// @param id Player id of the new lord.
            /// @param mode What happens to the old lord.
            void newLord(std::int32_t id, int mode);

            /// @brief Sets one right of a member.
            /// @details Original: RecvMemberRights (39/24).
            /// @param id Player id.
            /// @param right Right number (1..5).
            /// @param on Whether the right is given.
            void setRights(std::int32_t id, int right, bool on);

            /// @brief Sets the online state of a member.
            /// @details Original: SetOrgMemberOnlineState (39/61).
            /// @param id Player id.
            /// @param state Online state (3 is offline).
            void setOnlineState(std::int32_t id, int state);

            /// @brief Sets the contribution of a member.
            /// @details Original: RecvOrgMemberValue (39/23).
            /// @param id Player id.
            /// @param contribution New contribution.
            void setContribution(std::int32_t id, std::int32_t contribution);

            /// @brief Updates a member's name, looks and nickname (not his contribution or rights).
            /// @details Original: RecvOrgMemberInfo (39/60).
            /// @param record The member's new record.
            void updateMember(const protocol::GuildMemberRecord& record);

        private:
            /// @brief Takes the member at index out of the list, keeping the vice lords' count right.
            /// @param index Index in the list.
            /// @return The member taken out.
            GuildMember takeOut(int index);

            std::string m_name;
            std::vector<GuildMember> m_members;
            int m_viceCount = 0;
            std::string m_rules;
    };

}

#endif
