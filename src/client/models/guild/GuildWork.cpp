#include "client/models/guild/GuildWork.h"

#include "client/rules/CubicLevels.h"
#include "core/ByteReader.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kBarSteps = 22; // steps of the progress bar over the head
        // craft duration by craft kind (index 0 unused): kind 1 Badge 1 hour, kind 2 Create 3 hours
        constexpr std::array<std::int64_t, 3> kCraftDurationsMs = {0, 3600000, 10800000};
        constexpr std::int64_t kFeatherDurationMs = 3600000; // feather making takes 1 hour, ms
        constexpr double kElapsedStepMs = 120000.0; // one "step done" from the server is 2 minutes, ms
        // overhead names by labour kind 1..5 (index 0 unused)
        const char* const kLabourNames[] = {"", "Clear", "Cutting", "Farming", "Farm", "Build"};
        // overhead names by craft kind 1..2 (index 0 unused)
        const char* const kCraftNames[] = {"", "Badge", "Create"};
        const char* const kFeatherText = "Making Feathers";

    }

    GuildWork::Pose GuildWork::received(const core::Bytes& payload, std::uint64_t nowMs) {
        if (payload.size() < 2) { // main id and sub id bytes
            return Pose::None;
        }
        core::ByteReader reader(payload, 2); // skip main id and sub id
        switch (payload[1]) {
            case 50: // 39/50: labour kind and experience (u8 kind, u32 experience)
            case 51: { // 39/51: labour kind, experience and level; the job starts (u8 kind, u32 experience, u16 level)
                const int kind = reader.u8();
                const std::uint32_t experience = reader.u32();
                const int level = payload[1] == 51 ? reader.u16() : 0; // sub id 51 carries the level
                if (reader.failed() || kind < 1 || kind >= static_cast<int>(m_labour.size())) {
                    return Pose::None;
                }
                m_labourKind = kind;
                m_labour[static_cast<std::size_t>(kind)].experience = experience;
                if (payload[1] == 51) { // sub id 51: the job starts
                    m_labour[static_cast<std::size_t>(kind)].level = level;
                    m_startMs = nowMs;
                }
                return Pose::None;
            }
            case 53: // 39/53: labour stopped, the player stands
                m_labourKind = 0;
                return Pose::Stand;
            case 54: { // 39/54: craft kind and experience (u8 kind, u32 experience)
                const int kind = reader.u8();
                const std::uint32_t experience = reader.u32();
                if (reader.failed() || kind < 1 || kind >= static_cast<int>(m_craft.size())) {
                    return Pose::None;
                }
                m_craftKind = kind;
                m_craft[static_cast<std::size_t>(kind)].experience = experience;
                return Pose::None;
            }
            case 55: { // 39/55: craft started (u8 kind, u32 experience, u8 level, u16 steps done)
                const int kind = reader.u8();
                const std::uint32_t experience = reader.u32();
                const int level = reader.u8();
                const int elapsed = reader.u16();
                if (reader.failed() || kind < 1 || kind >= static_cast<int>(m_craft.size())) {
                    return Pose::None;
                }
                m_craftKind = kind;
                m_craft[static_cast<std::size_t>(kind)] = Skill{experience, level};
                m_startMs = nowMs;
                m_durationMs = kCraftDurationsMs[static_cast<std::size_t>(kind)];
                m_elapsed = elapsed;
                return kind == 1 ? Pose::Recruit : Pose::Create;
            }
            case 56: // 39/56: craft stopped, the player stands
                m_craftKind = 0;
                return Pose::Stand;
            case 57: // 39/57: feather make value, not read
                return Pose::None;
            case 58: { // 39/58: feather making started (i32, u8, u16 steps done)
                reader.u32();
                reader.u8();
                const int elapsed = reader.u16();
                if (reader.failed()) {
                    return Pose::None;
                }
                m_startMs = nowMs;
                m_durationMs = kFeatherDurationMs;
                m_elapsed = elapsed;
                m_featherMaking = true;
                return Pose::Feather;
            }
            case 59: // 39/59: feather making stopped, the player stands
                m_featherMaking = false;
                return Pose::Stand;
            default: // other sub ids: ignored
                return Pose::None;
        }
    }

    bool GuildWork::stopLabour() {
        const bool was = m_labourKind != 0;
        m_labourKind = 0;
        return was;
    }

    bool GuildWork::stopCraft() {
        const bool was = m_craftKind != 0;
        m_craftKind = 0;
        return was;
    }

    GuildWork::Overhead GuildWork::describe(const char* name, Skill& skill) {
        skill.level = cubicLevelFromExp(skill.experience, skill.level);
        const std::uint32_t current = totalCubicLevelExp(skill.level);
        const std::uint32_t next = totalCubicLevelExp(skill.level + 1);
        Overhead overhead;
        overhead.upper = name + std::to_string(skill.level);
        overhead.lower = std::to_string(skill.experience - current) + "/" + std::to_string(next - current);
        return overhead;
    }

    std::vector<GuildWork::Overhead> GuildWork::overheads(std::uint64_t nowMs) {
        std::vector<Overhead> shown;
        const double passed = static_cast<double>(nowMs - m_startMs);
        const auto craftBar = [&] {
            const double length = std::llround(passed * kBarSteps / static_cast<double>(m_durationMs)) +
                                  std::llround(m_elapsed * kElapsedStepMs * kBarSteps / static_cast<double>(m_durationMs));
            return static_cast<int>(std::min<double>(length, kBarSteps));
        };
        if (m_labourKind != 0) {
            Skill& skill = m_labour[static_cast<std::size_t>(m_labourKind)];
            Overhead overhead = describe(kLabourNames[m_labourKind], skill);
            if (m_durationMs > 0) {
                const long long steps = std::llround(passed * kBarSteps / static_cast<double>(m_durationMs));
                overhead.barLength =
                    steps >= 0 && steps <= 18 ? static_cast<int>(steps) : kBarSteps; // bar steps drawn by the time worked; more than 18 steps shows a full bar
                if (overhead.barLength == kBarSteps && skill.level < 1) {
                    m_startMs = nowMs;
                }
            }
            shown.push_back(std::move(overhead));
        }
        if (m_craftKind != 0 && m_durationMs > 0) {
            Overhead overhead = describe(kCraftNames[m_craftKind], m_craft[static_cast<std::size_t>(m_craftKind)]);
            overhead.barLength = craftBar();
            shown.push_back(std::move(overhead));
        }
        if (m_featherMaking && m_durationMs > 0) {
            Overhead overhead;
            overhead.lower = kFeatherText;
            overhead.barLength = craftBar();
            shown.push_back(std::move(overhead));
        }
        return shown;
    }

}
