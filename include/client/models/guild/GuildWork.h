#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_GUILD_WORK_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_GUILD_WORK_H

#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief What the own player does for the guild: labour (clear, cutting, farming, farm, build), crafts (badge, create) and making feathers.
    /// @details Original: TPlayer.FLabourKind, FCraftKind, FFeatherMaking and their skills, told by 39/50 to 39/59. While one goes on the
    /// player shows its job, its level and experience and a bar of the time over his head.
    class GuildWork {
        public:
            /// @brief What the player does next.
            /// @details Original: SetHumanAction 8 and PlayHumanAction 54, 34, 42.
            enum class Pose { None, Stand, Recruit, Create, Feather };

            /// @brief A job shown over the player's head: two lines of text and the length of the bar in 22 steps (-1: no bar).
            /// @details Original: DrawLabourProgress, DrawCraftProgress, DrawFeatherMakingProgress; the upper line is empty for feathers.
            struct Overhead {
                std::string upper;
                std::string lower;
                int barLength = -1;
            };

            /// @brief Handles a guild work message from the server (39/50..39/59).
            /// @details Original: RecvOrgRec1Value (50), RecvOrgRec1Update (51), 53, RecvOrgRec2Value (54), RecvRecruitState (55), 56,
            /// RecvPlayerDword3A75 (57), StartFeatherMaking (58), 59.
            /// @param payload Whole message; byte 1 is the sub id.
            /// @param nowMs Current time in milliseconds.
            /// @return What the player does next.
            Pose received(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief The player walks off a labour job.
            /// @details Original: StopLabour.
            /// @return Whether a job was going on (the server is told).
            bool stopLabour();

            /// @brief The player walks off a craft job.
            /// @details Original: StopCraft.
            /// @return Whether a job was going on (the server is told).
            bool stopCraft();

            /// @brief The jobs going on, with experience and level brought up to date as the game does at each frame.
            /// @param nowMs Current time in milliseconds.
            /// @return One overhead per job going on.
            std::vector<Overhead> overheads(std::uint64_t nowMs);

        private:
            /// @brief Experience and level of a labour or craft skill.
            struct Skill {
                std::uint32_t experience = 0;
                int level = 0;
            };

            /// @brief Builds the texts of a job and its skill, bringing the skill's level up to date.
            /// @details Original: DrawLabourProgress / DrawCraftProgress.
            /// @param name Name of the job.
            /// @param skill The skill (level updated from its experience).
            /// @return The overhead with its two text lines and no bar.
            static Overhead describe(const char* name, Skill& skill);

            int m_labourKind = 0;
            int m_craftKind = 0;
            bool m_featherMaking = false;
            std::array<Skill, 6> m_labour{}; // five labour kinds, index 0 unused
            std::array<Skill, 3> m_craft{}; // two craft kinds, index 0 unused
            std::uint64_t m_startMs = 0;
            std::int64_t m_durationMs = 0;
            int m_elapsed = 0;
    };

}

#endif
