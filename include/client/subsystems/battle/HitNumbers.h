#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_HIT_NUMBERS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_HIT_NUMBERS_H

#include "core/Point.h"
#include "formats/MotionRecord.h"
#include "gfx/Canvas.h"

#include <array>
#include <cstdint>
#include <utility>

namespace client {

    /// @brief The HP and SP a fighter loses or gains, shown hit by hit.
    /// @details Original: TFightHum's FHpDamage .. FHpNumPhase. SplitDamageOverHits shares an action's amounts out over its
    /// motion's hits; each hit's numbers then fly up in an arc away from the fighter (yellow-red for HP, blue for SP), rise
    /// straight up, or bounce bigger, and are gone after a hundred steps (DrawHumDamageNumbers, AnimateHpNumber,
    /// AnimateSpNumber). One step a frame; with free frames one a beat, the frames between drawing the numbers between their
    /// last two places.
    class HitNumbers {
        public:
            static constexpr int kHits = 10; // most hits (damage numbers) in one motion record, 1-based

            /// @brief Shares an action's HP and SP amounts out over its motion's hits.
            /// @details Original: SplitDamageOverHits.
            /// @param motion The motion record with the hit percentages.
            /// @param hp HP amount.
            /// @param sp SP amount.
            void split(const formats::MotionRecord& motion, std::int32_t hp, std::int32_t sp);

            /// @brief Returns a hit's HP amount.
            /// @param hit Hit number (1-based).
            /// @return The HP amount.
            std::int32_t hpOf(int hit) const { return m_hits[index(hit)].hp; }

            /// @brief Returns a hit's SP amount.
            /// @param hit Hit number (1-based).
            /// @return The SP amount.
            std::int32_t spOf(int hit) const { return m_hits[index(hit)].sp; }

            /// @brief Tells whether a hit's HP number is being shown.
            /// @param hit Hit number (1-based).
            /// @return True while shown.
            bool hpShown(int hit) const { return m_hits[index(hit)].hpState == 1; }

            /// @brief Tells whether a hit's SP number has been started.
            /// @param hit Hit number (1-based).
            /// @return True once started.
            bool spShown(int hit) const { return m_hits[index(hit)].spState != 0; }

            /// @brief Starts a hit's HP number from the fighter's feet.
            /// @param hit Hit number (1-based).
            /// @param feet The fighter's feet on the screen.
            /// @param cameraX Camera x.
            /// @param style 0 arc, 1 rise, 2 bounce.
            void showHp(int hit, core::Point feet, int cameraX, int style);

            /// @brief Starts a hit's SP number from the fighter's feet.
            /// @param hit Hit number (1-based).
            /// @param feet The fighter's feet on the screen.
            /// @param cameraX Camera x.
            /// @param style 0 arc, 1 rise, 2 bounce.
            void showSp(int hit, core::Point feet, int cameraX, int style);

            /// @brief Sets a hit's HP amount outright.
            /// @details Original: RecvFighterDamage's hit, FHpDamage[1] set outright.
            /// @param hit Hit number (1-based).
            /// @param hp The HP amount.
            void setHp(int hit, std::int32_t hp) { m_hits[index(hit)].hp = hp; }

            /// @brief Marks a hit's HP number as shown.
            /// @param hit Hit number (1-based).
            void markShown(int hit) { m_hits[index(hit)].hpState = 1; }

            /// @brief Steps each shown number and draws it.
            /// @details Original: DrawHumDamageNumbers.
            /// @param canvas Target canvas.
            /// @param motion The motion record.
            /// @param side Side of the field the fighter is on.
            /// @param cameraX Camera x.
            void draw(gfx::Canvas& canvas, const formats::MotionRecord& motion, int side, int cameraX);

            /// @brief Does the same for a motion of hit mode 0: the first hit's number.
            /// @param canvas Target canvas.
            /// @param side Side of the field the fighter is on.
            /// @param cameraX Camera x.
            void drawSingle(gfx::Canvas& canvas, int side, int cameraX);

        private:
            /// @brief The numbers of one hit and their animation state.
            struct Hit {
                std::int32_t hp = 0;
                std::int32_t sp = 0;
                int hpState = 0;
                int spState = 0;
                core::Point hpAt;
                core::Point spAt;
                int hpCameraX = 0;
                int spCameraX = 0;
                int hpStep = 0;
                int hpRise = 0;
                int spStep = 0;
                int spRise = 0;
                int hpY = 0;
                int spY = 0;
                int hpStyle = 0;
                int spStyle = 0;
                int phase = 0;
            };

            /// @brief Maps a hit number to its array index.
            /// @param hit Hit number (1-based).
            /// @return The index, or 0 when out of range.
            static std::size_t index(int hit) { return static_cast<std::size_t>(hit >= 1 && hit <= kHits ? hit : 0); }

            /// @brief Works out the arc's height at a step and keeps it as the hit's HP rise.
            /// @details Original: CalcNumberArcY.
            /// @param hit Hit number (1-based).
            /// @param y Base y.
            /// @param step Step along the arc.
            /// @return The y lifted by the arc.
            int arcY(int hit, int y, int step);

            /// @brief Moves a hit's HP number one step.
            /// @details Original: AnimateHpNumber.
            /// @param hit Hit number (1-based).
            void animateHp(int hit);

            /// @brief Moves a hit's SP number one step.
            /// @details Original: AnimateSpNumber.
            /// @param hit Hit number (1-based).
            void animateSp(int hit);

            /// @brief Steps a hit's numbers on the frame that steps them, keeping them as they were before.
            /// @param canvas The canvas (tells whether this is an animation frame).
            /// @param hit Hit number (1-based).
            void step(const gfx::Canvas& canvas, int hit);

            /// @brief Works out where a hit's HP and SP numbers are drawn.
            /// @details Side 1 (the right) throws a lone hit's numbers to the right, side 2 to the left; several hits go out by
            /// turns to both sides, further each hit.
            /// @param h The hit.
            /// @param hit Hit number (1-based).
            /// @param side Side of the field the fighter is on.
            /// @param cameraX Camera x.
            /// @param single True for a motion of hit mode 0.
            /// @return The HP place and the SP place.
            static std::pair<core::Point, core::Point> places(
                const Hit& h,
                int hit,
                int side,
                int cameraX,
                bool single
            );

            /// @brief Draws a hit's HP and SP numbers.
            /// @param canvas Target canvas.
            /// @param hit Hit number (1-based).
            /// @param side Side of the field the fighter is on.
            /// @param cameraX Camera x.
            /// @param single True for a motion of hit mode 0.
            void drawHit(
                gfx::Canvas& canvas,
                int hit,
                int side,
                int cameraX,
                bool single
            );

            std::array<Hit, kHits + 1> m_hits{};
            std::array<Hit, kHits + 1> m_before{};
    };

}

#endif
