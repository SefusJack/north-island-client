#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_PAINTER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_PAINTER_H

#include "client/subsystems/battle/BattleField.h"
#include "client/rendering/characters/CharacterRenderer.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <optional>

namespace client {

    /// @brief Draws a battle over its ground: fighters, HP bars, command countdown, names and hover info.
    /// @details Original: DrawFightField, DrawFightScene, DrawFighterInfo.
    class BattlePainter {
        public:
            /// @brief Creates a painter that draws with the given character renderer and picture library.
            /// @param characters Renderer for player and NPC figures.
            /// @param pictures Picture library for bars, numbers and marks.
            BattlePainter(
                CharacterRenderer& characters,
                gfx::PictureLibrary& pictures
            )
                : m_characters(characters),
                  m_pictures(pictures) {}

            /// @brief The command countdown state shown over the battle.
            struct Clock {
                int countdown = -1;
                int battleType = 0;
                bool runningOut = false;
                int timeLimit = -1;
            };

            /// @brief Draws all fighters from the back of the field to the front.
            /// @details Original: DrawFightHumsSorted.
            /// @param canvas Target canvas.
            /// @param field The battle field holding the fighters.
            /// @param hoveredSlot Slot of the fighter under the mouse, drawn lit (0: none).
            /// @param nowMs Current time in milliseconds.
            void paintFighters(gfx::Canvas& canvas, BattleField& field, int hoveredSlot, std::uint64_t nowMs);

            /// @brief Draws the skill cut-in: a streaked band across the screen with the caster's face flying along it.
            /// @details Original: DrawS60000Layer. Nothing is drawn for a fighter that is not a player.
            /// @param canvas Target canvas.
            /// @param fighter The casting fighter.
            /// @param layer The effect layer driving the cut-in.
            /// @param nowMs Current time in milliseconds.
            void drawCutIn(gfx::Canvas& canvas, Fighter& fighter, const EffectLayer& layer, std::uint64_t nowMs);

            /// @brief Draws the command countdown, the marks of fighters still choosing and the HP bars.
            /// @details Original: DrawCommandCountdown, DrawPendingFighterMarks, DrawFighterHpBars.
            /// @param canvas Target canvas.
            /// @param field The battle field.
            /// @param clock Countdown state.
            /// @param nowMs Current time in milliseconds.
            void paintClock(gfx::Canvas& canvas, const BattleField& field, const Clock& clock, std::uint64_t nowMs);

            /// @brief Draws the fighters' names.
            /// @details Original: DrawFightScene; while the skill zoom shows a part below the screen's top, THum.DrawHumInfo moves each
            /// name by the zoom's scale.
            /// @param canvas Target canvas.
            /// @param field The battle field.
            /// @param zoomScale SkillZoomScale while the skill zoom's part starts below the screen's top, else nothing.
            void paintNames(gfx::Canvas& canvas, const BattleField& field, std::optional<float> zoomScale = std::nullopt);

            /// @brief Draws what the fighter under the mouse is.
            /// @details Original: DrawFighterInfo.
            /// @param canvas Target canvas.
            /// @param fighter The hovered fighter.
            void paintInfo(gfx::Canvas& canvas, const Fighter& fighter);

            /// @brief Settles the pose: a fighter not acting stands ready (16 / 17) or lies (26 / 27); one acting shows its action's pose.
            /// @details Original: DrawFightHum. The side picks which way it faces.
            /// @param fighter The fighter to settle.
            static void settlePose(Fighter& fighter);

            /// @brief Returns the frame of the fighter's action to draw now and moves its animation on.
            /// @param fighter The fighter.
            /// @param count Number of frames of the action.
            /// @param nowMs Current time in milliseconds.
            /// @return The frame index to draw.
            static int nextFrame(Fighter& fighter, int count, std::uint64_t nowMs);

            /// @brief Returns the frame the fighter currently has, moving nothing (for looking at a fighter under the mouse).
            /// @param fighter The fighter.
            /// @param count Number of frames of the action.
            /// @return The frame index shown.
            static int shownFrame(const Fighter& fighter, int count);

            /// @brief Finds the slot of the fighter drawn under a screen point, the front one first.
            /// @param field The battle field.
            /// @param point Screen point.
            /// @return The slot, or 0 for none.
            int slotAt(BattleField& field, core::Point point);

        private:
            /// @brief Returns the sprite slot used to draw the fighter.
            /// @param fighter The fighter.
            /// @return The sprite slot.
            int spriteSlot(const Fighter& fighter) const;

            /// @brief Returns the number of frames of the fighter's current action.
            /// @param fighter The fighter.
            /// @return The frame count.
            int frameCount(const Fighter& fighter) const;

            /// @brief Draws one fighter in its pose.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            /// @param hovered True when the mouse is over it (drawn lit).
            /// @param shake Screen-shake offset.
            /// @param nowMs Current time in milliseconds.
            void drawFighter(
                gfx::Canvas& canvas,
                Fighter& fighter,
                bool hovered,
                core::Point shake,
                std::uint64_t nowMs
            );

            /// @brief Draws the ghosts a casting fighter leaves on its run, each fainter than the one before.
            /// @details Original: DrawHumAfterImages.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            void drawAfterImages(gfx::Canvas& canvas, Fighter& fighter);

            /// @brief Draws an NPC kind's figure, holding the weapon it wears.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            /// @param feet Screen point of the figure's feet.
            /// @param slot Sprite slot.
            /// @param frame Frame index.
            /// @param highlight Highlight / scale to draw with.
            void drawNpcFigure(
                gfx::Canvas& canvas,
                const Fighter& fighter,
                core::Point feet,
                int slot,
                int frame,
                SpriteHighlight highlight
            );

            /// @brief Draws a fighter flying away with its after-image.
            /// @details Original: DrawAfterImageMode7.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            /// @param slot Sprite slot.
            /// @param frame Frame index.
            void drawFlyAway(gfx::Canvas& canvas, const Fighter& fighter, int slot, int frame);

            /// @brief Draws a fighter's HP bar.
            /// @details Original: DrawHpBar.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            void drawHpBar(gfx::Canvas& canvas, const Fighter& fighter);

            /// @brief Sets the highlight's scale from the battle's DrawScale / ScaleDiv for a fighter.
            /// @param fighter The fighter.
            /// @param highlight Highlight to fill in.
            static void scaleFor(const Fighter& fighter, SpriteHighlight& highlight);

            /// @brief Marks a fighter that is still choosing its command.
            /// @details Original: DrawPendingFighterMarks.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            /// @param nowMs Current time in milliseconds.
            void drawPendingMark(gfx::Canvas& canvas, const Fighter& fighter, std::uint64_t nowMs);

            /// @brief Draws a number of 20 x 27 digits centred on x.
            /// @details Original: DrawNumberByName.
            /// @param canvas Target canvas.
            /// @param font Name of the digit font.
            /// @param at Point to centre on.
            /// @param value The number.
            void drawBigNumber(gfx::Canvas& canvas, const char* font, core::Point at, int value);

            /// @brief Draws the seconds left of the battle's time limit.
            /// @details Original: DrawBattleTimeLimit.
            /// @param canvas Target canvas.
            /// @param secondsLeft Seconds left.
            void drawTimeLimit(gfx::Canvas& canvas, int secondsLeft);

            /// @brief Lists the slots from the back of the field to the front.
            /// @details Original: CompareFighterDrawOrder.
            /// @param field The battle field.
            /// @return Slots in drawing order.
            static std::vector<int> drawingOrder(const BattleField& field);

            CharacterRenderer& m_characters;
            gfx::PictureLibrary& m_pictures;
    };

}

#endif
