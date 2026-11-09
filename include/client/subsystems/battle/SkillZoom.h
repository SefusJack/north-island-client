#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_ZOOM_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_ZOOM_H

#include "core/Rect.h"

#include <optional>

namespace client {

    /// @brief The battle's skill zoom: the part of the game screen stretched over it while a close-range skill's attacker nears its target.
    /// @details Original: TForm1.FZoomRect / FZoomLevel as CalcSkillZoom and ApplySkillZoom (RE_Skill.pas) set them, SkillZoomScale, and the
    /// clamp of TForm1.DXTimer1Timer before StretchBufToScreen. It runs only with the system window's camera switch on (BattleSwitch 0).
    /// The scale is how close the attacker has come: 0 where it set off (the span between the two fighters when the skill began), 1 on
    /// the target. At 1 the part is the screen with 100 x 75 pixels cut off every edge, shifted up or down by where the attacker stands
    /// against the near ground layer's bottom.
    class SkillZoom {
        public:
            static constexpr int kScreenWidth = 800; // the game screen's width, px
            static constexpr int kScreenHeight = 600; // the game screen's height, px
            static constexpr int kInsetX = 100; // px cut off the left and right edges at scale 1
            static constexpr int kInsetY = 75; // px cut off the top and bottom edges at scale 1
            static constexpr int kViewHalfHeight = kScreenHeight / 2; // BattleViewHalfHeight: half the battle view's height, px

            /// @brief Decides from the attacker's place whether to zoom; when not, the zoom goes back to the whole screen.
            /// @details Original: CalcSkillZoom. Not zoomed while position - targetPosition is at least the span (signed, so an attacker
            /// on the target's right that has not come closer) or the span is not above 0; else the scale is (span - |position -
            /// targetPosition|) / span.
            /// @param span The attack span: |attacker x - target x| on the screen when the skill began (FAttackSpanX).
            /// @param position The attacker's motion x on the ground.
            /// @param targetPosition The target's x on the ground (its screen x plus the camera's x).
            /// @return True when the zoom is to be applied.
            bool decide(int span, int position, int targetPosition);

            /// @brief Sets the zoomed part by the scale.
            /// @details Original: ApplySkillZoom. The screen with 100 / 75 x the scale (rounded) cut off each edge; when the attacker's
            /// motion y plus half the view lies below the near layer's bottom, a top above 0 and a bottom below 600 each move by
            /// (that bottom - the motion y); the part is then pushed back inside the screen.
            /// @param casterY The attacker's motion y on the ground.
            /// @param nearBottom The near ground layer's height plus its y (FLayer1Height + FLayer1OffsetY).
            void apply(int casterY, int nearBottom);

            /// @brief Puts the zoom back to the whole screen.
            /// @details Original: ResetFightersForRound and CloseFightField (FZoomRect 0, 0, 800, 600; FZoomLevel 0).
            void reset();

            /// @brief The scale the last decision gave.
            /// @return SkillZoomScale: 0 when not zoomed, up to 1 on the target (below 0 once the attacker is past it).
            float scale() const { return m_scale; }

            /// @brief The part of the game screen that is stretched over it now.
            /// @details Original: the clamp of DXTimer1Timer (left and top kept at or above 0, right at or below 800, bottom at or below 600).
            /// @return The part in game screen coordinates, or nothing when the zoom is off or shows the whole screen.
            std::optional<core::Rect> part() const;

            /// @brief The part of a window to stretch over it so that a zoomed part of the game screen fills the game screen.
            /// @details The original's window is the 800 x 600 screen, so this is the part itself; a larger window (the game screen in its
            /// middle) is stretched by the same factor about the same point.
            /// @param part The zoomed part, in game screen coordinates.
            /// @param window The window, in game screen coordinates.
            /// @return The part of the window, in game screen coordinates.
            static core::Rect sourceIn(const core::Rect& part, const core::Rect& window);

        private:
            /// @brief Pushes the rectangle back inside the screen, left before right and top before bottom.
            /// @details Original: the four OffsetRect lines after each change of FZoomRect.
            void keepOnScreen();

            int m_left = 0;
            int m_top = 0;
            int m_right = kScreenWidth;
            int m_bottom = kScreenHeight;
            bool m_on = false;
            float m_scale = 0.0f;
    };

}

#endif
