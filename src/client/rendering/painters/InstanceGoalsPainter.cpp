#include "client/rendering/painters/InstanceGoalsPainter.h"

#include "ui/DrawingArea.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        constexpr int kTextWidth = 288; // px: text box width
        constexpr int kTextHeight = 18; // px: text box height
        constexpr int kRemainingLabelX = 335; // px: x of the "Remaining Time" label on the map
        constexpr int kRemainingLabelY = 10; // px: y of the "Remaining Time" label on the map
        constexpr int kRemainingValue = 120; // px: x gap from the label to the time value
        constexpr core::Point kMapGoals{600, 50}; // px: goal list position on the map
        constexpr core::Point kBattleGoals{9, 205}; // px: goal list position in battle
        constexpr int kOverlayLineHeight = 15; // px per line of the overlay list
        constexpr int kIconLineHeight = 23; // px per line of the goal list with icons
        constexpr int kIconTextIndent = 52; // px: text starts this far right of the goal icon
        constexpr int kColumnGap = 142; // px: x distance to the second goal column
        constexpr int kColumnShift = 12; // px: first column is shifted this far left
        constexpr int kLinesInAColumn = 5; // goal lines per column before moving to the second
        constexpr int kInnerGoalsLeft = -51; // px: inner goal list x offset from the window
        constexpr int kInnerGoalsDown = 50; // px: inner goal list y offset from the window
        const core::Color kWhite = core::Color::fromRgb565(65535); // RGB565 0xFFFF: white
        const core::Color kDark = core::Color::fromRgb565(2113); // RGB565 0x0841: near black

    }

    InstanceGoalsPainter::InstanceGoalsPainter(
        GameServices& services,
        ui::Screen& screen,
        const Instances& instances
    )
        : m_services(services),
          m_instances(instances) {
        screen.load(services.game.layout("InstanceOverlay.xml"), services.loadContext());
        screen.get<ui::DrawingArea>("InstanceOverlay").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t) { paintRemaining(canvas); });
    }

    std::string InstanceGoalsPainter::npcName(int npcId) const {
        const auto npc = m_services.npcs.find(npcId);
        return npc ? npc->name : std::string();
    }

    void InstanceGoalsPainter::text(
        gfx::Canvas& canvas,
        const std::string& line,
        core::Point at,
        gfx::TextEffect effect,
        core::Color color
    ) {
        canvas.drawText(at, line, {color, effect, std::nullopt}, kTextWidth, kTextHeight);
    }

    void InstanceGoalsPainter::icon(gfx::Canvas& canvas, const char* name, core::Point at) {
        if (const auto picture = m_services.pictures.find(name)) {
            canvas.drawPicture(*picture, at);
        }
    }

    void InstanceGoalsPainter::paintRemaining(gfx::Canvas& canvas) {
        if (m_instances.state() != 2 || !m_instances.here()) { // instance state 2 = running
            return;
        }
        const bool fighting = m_instances.fighting();
        const core::Point at = fighting ? kBattleGoals : kMapGoals;
        const core::Point label = fighting ? at : core::Point{kRemainingLabelX, kRemainingLabelY};
        text(canvas, "Remaining Time:", label, gfx::TextEffect::Shadow, kWhite);
        const InstanceInfo& own = m_instances.own();
        const std::string time = Instances::minutesText(own.remainingMinutes);
        text(
            canvas,
            own.remainPrefix.empty() ? time : own.remainPrefix + time + " Clear",
            {label.x + kRemainingValue, label.y},
            gfx::TextEffect::Shadow,
            kWhite
        );
        const int lineY = fighting ? at.y + kOverlayLineHeight : at.y;
        paintGoals(canvas, {{at.x, lineY}, kOverlayLineHeight, gfx::TextEffect::Shadow, kWhite, true, false});
    }

    void InstanceGoalsPainter::paintInner(gfx::Canvas& canvas, core::Point at) {
        paintGoals(canvas, {{at.x + kInnerGoalsLeft, at.y + kInnerGoalsDown}, kIconLineHeight, gfx::TextEffect::Plain, kDark, false, true});
    }

    void InstanceGoalsPainter::paintGoals(gfx::Canvas& canvas, const Layout& l) {
        int x = l.shiftLeft ? l.at.x - kColumnShift : l.at.x;
        int y = l.header ? l.at.y + l.lineHeight : l.at.y;
        const int secondColumn = l.at.x + kColumnGap;
        bool any = false;
        int lines = 0;
        const auto nextColumn = [&] {
            if (l.shiftLeft && lines >= kLinesInAColumn && x != secondColumn) {
                x = secondColumn;
                y = l.at.y;
            }
        };
        std::array<std::string, Instances::kGoals + 1> goalLines;
        for (int i = 1; i <= Instances::kGoals; ++i) {
            const InstanceGoal& goal = m_instances.goal(i);
            if (goal.npcId == 0 || (l.header && goal.have >= goal.need)) {
                continue;
            }
            any = true;
            const int have = goal.need != 0 ? std::min(goal.have, goal.need) : goal.have;
            std::string& line = goalLines[static_cast<std::size_t>(i)];
            line = npcName(goal.npcId);
            if (goal.need > 1) {
                line += " " + std::to_string(have) + "/" + std::to_string(goal.need);
            } else if (goal.need == 1 && !l.shiftLeft) {
                line = "Beat " + line;
            }
        }
        for (int i = 1; i <= Instances::kGoals; ++i) {
            const std::string& line = goalLines[static_cast<std::size_t>(i)];
            if (line.empty()) {
                continue;
            }
            if (!l.shiftLeft) {
                text(canvas, line, {x, y}, l.effect, l.color);
                y += l.lineHeight;
            } else {
                const InstanceGoal& goal = m_instances.goal(i);
                icon(canvas, goal.need > 1 ? "Icon_DupKillNum" : "Icon_Beat", {x, y});
                if (goal.have >= goal.need) {
                    icon(canvas, "Icon_Pass_1", {x - 15, y + 3}); // tick offset from the goal icon (x -15, y 3)
                }
                text(canvas, line, {x + kIconTextIndent, y + 2}, l.effect, l.color); // text 2 pixels down
                y += kIconLineHeight;
            }
            ++lines;
            nextColumn();
        }
        std::string bossLine;
        if (m_instances.bossNpc() != 0) {
            ++lines;
            nextColumn();
            bossLine = npcName(m_instances.bossNpc());
            if (m_instances.bossKills() == 0 || !l.header) {
                if (!l.shiftLeft) {
                    bossLine = "Beat " + bossLine;
                }
            } else {
                bossLine = "Mission Completed";
            }
        }
        if (!bossLine.empty()) {
            any = true;
            if (!l.shiftLeft) {
                text(canvas, bossLine, {x, y}, l.effect, l.color);
                y += l.lineHeight;
            } else {
                icon(canvas, "Icon_Beat", {x, y});
                if (m_instances.bossKills() >= 1) {
                    icon(canvas, "Icon_Pass_1", {x - 15, y + 3}); // tick offset from the goal icon (x -15, y 3)
                }
                text(canvas, bossLine, {x + kIconTextIndent, y + 2}, l.effect, l.color); // text 2 pixels down
            }
        }
        if (l.header) {
            text(canvas, "The conditions for win:", l.at, l.effect, l.color);
            if (!any) {
                text(canvas, "Mission Completed", {l.at.x, l.at.y + l.lineHeight}, l.effect, l.color);
            }
        } else if (!any && l.shiftLeft) {
            text(canvas, m_instances.state() == 1 ? "Hasn't started" : "Hasn't entered the instance", l.at, l.effect, l.color);
        }
    }

}
