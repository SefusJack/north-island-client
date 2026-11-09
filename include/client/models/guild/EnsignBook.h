#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_ENSIGN_BOOK_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_ENSIGN_BOOK_H

#include "formats/EmblemBlock.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace client {

    /// @brief The guilds' badges the client knows, each with the version the server gave.
    /// @details Original: TCY_OrganManage.FEnsigns (CY_Organ.pas RecvEnsignList, RecvEnsign, RecvOwnEnsignUpdate, LoadAllEnsigns,
    /// SaveAllEnsigns, FlushOrgSendQueue). The server lists the guilds that have a badge (39/31); copies in
    /// user\Server<n>\AllOrgEnsign.dat with the same version are used, the others are asked for (39/19, ten a second) and arrive in 39/32.
    class EnsignBook {
        public:
            /// @brief The badge of one guild.
            struct Ensign {
                std::uint32_t id = 0;
                int revision = 0;
                formats::EmblemBlock data;
            };

            /// @brief Replaces the list with the guilds that have a badge; each starts empty, the file's copies with the same version fill it and the rest are queued to be asked for.
            /// @details Original: RecvEnsignList (39/31).
            /// @param listed Guild id and badge version pairs.
            /// @param file Local copy of the badges (AllOrgEnsign.dat).
            void setList(const std::vector<std::pair<std::uint32_t, int>>& listed, const std::filesystem::path& file);

            /// @brief A badge asked for, or pushed because it changed; the list is saved.
            /// @details Original: RecvEnsign (39/32).
            /// @param id Guild id.
            /// @param revision Badge version.
            /// @param data The badge.
            /// @param file Local copy to save to.
            void received(std::uint32_t id, int revision, const formats::EmblemBlock& data, const std::filesystem::path& file);

            /// @brief The own guild's badge changed.
            /// @details Original: RecvOwnEnsignUpdate.
            /// @param guildId Own guild id.
            /// @param revision Badge version.
            /// @param data The badge.
            void setOwn(std::uint32_t guildId, int revision, const formats::EmblemBlock& data);

            /// @brief Replaces the badge of a known guild with a sample's.
            /// @details Original: RecvEnsignReset.
            /// @param id Guild id.
            /// @param data The new badge.
            void replaceData(std::uint32_t id, const formats::EmblemBlock& data);

            /// @brief Finds the badge of a guild.
            /// @details Original: FindEnsign.
            /// @param id Guild id.
            /// @return The badge, or nothing.
            std::optional<std::reference_wrapper<const Ensign>> find(std::uint32_t id) const;

            /// @brief Takes the guild ids to ask for now (at most ten, once a second; none while the queue is empty).
            /// @details Original: FlushOrgSendQueue (asks with 39/19).
            /// @param nowMs Current time in milliseconds.
            /// @return Ids to ask for.
            std::vector<std::uint32_t> takeRequest(std::uint64_t nowMs);

        private:
            /// @brief Loads the local copy: queues every badge, then takes copies with the same version.
            /// @details Original: LoadAllEnsigns.
            /// @param file Local copy of the badges.
            void loadFile(const std::filesystem::path& file);

            /// @brief Saves all badges to the local copy (nothing is written for an empty list).
            /// @details Original: SaveAllEnsigns.
            /// @param file Local copy to write.
            void save(const std::filesystem::path& file) const;

            /// @brief Index of a guild's badge in the list.
            /// @details Original: FindEnsignIndex.
            /// @param id Guild id.
            /// @return The index, -1 for none.
            int indexOf(std::uint32_t id) const;

            std::vector<Ensign> m_ensigns;
            std::vector<std::uint32_t> m_queue;
            std::uint64_t m_tickMs = 0;
    };

}

#endif
