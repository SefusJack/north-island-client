#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_INSTANCE_GOALS_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_INSTANCE_GOALS_PAINTER_H

#include "client/services/GameServices.h"
#include "client/models/instances/Instances.h"
#include "core/Color.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "ui/Screen.h"

#include <string>

namespace client {

    /// @brief Draws an instance's time and goals.
    /// @details Original: DrawDupRemainTime, DrawDupGoals.
    class InstanceGoalsPainter {
        public:
            /// @brief Loads the instance overlay and hooks its painter.
            /// @param services Game services.
            /// @param screen Screen the InstanceOverlay.xml layout is loaded into.
            /// @param instances Instance state to draw.
            InstanceGoalsPainter(
                GameServices& services,
                ui::Screen& screen,
                const Instances& instances
            );

            /// @brief Draws the goals for the inner info tab: pictures before the lines, two columns.
            /// @details Original: DrawDupGoals.
            /// @param canvas Target canvas.
            /// @param at Position of the tab's goals area.
            void paintInner(gfx::Canvas& canvas, core::Point at);

        private:
            /// @brief Draws the time left and the conditions for winning, on the map (600, 50) or in a battle (9, 205).
            /// @details Original: DrawDupRemainTime.
            /// @param canvas Target canvas.
            void paintRemaining(gfx::Canvas& canvas);

            /// @brief How the goals are laid out: position, line height, text style and the two original flags.
            struct Layout {
                core::Point at;
                int lineHeight;
                gfx::TextEffect effect;
                core::Color color;
                bool header;
                bool shiftLeft;
            };

            /// @brief Draws a line for each NPC to beat and one for the boss.
            /// @details Original: DrawDupGoals.
            /// @param canvas Target canvas.
            /// @param layout The layout to use.
            void paintGoals(gfx::Canvas& canvas, const Layout& layout);

            /// @brief Draws a line of text 288 pixels wide.
            /// @details Original: DrawDupMisText.
            /// @param canvas Target canvas.
            /// @param line The text.
            /// @param at Position of the text.
            /// @param effect Text effect.
            /// @param color Text colour.
            void text(
                gfx::Canvas& canvas,
                const std::string& line,
                core::Point at,
                gfx::TextEffect effect,
                core::Color color
            );

            /// @brief Draws the picture before a goal line or the tick that follows when it is beaten.
            /// @param canvas Target canvas.
            /// @param name Picture name.
            /// @param at Position of the picture.
            void icon(gfx::Canvas& canvas, const char* name, core::Point at);

            /// @brief Looks up an NPC's name.
            /// @param npcId NPC id.
            /// @return The name, empty when the NPC is unknown.
            std::string npcName(int npcId) const;

            GameServices& m_services;
            const Instances& m_instances;
    };

}

#endif
