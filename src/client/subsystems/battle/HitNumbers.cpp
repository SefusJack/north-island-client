#include "client/subsystems/battle/HitNumbers.h"

#include "client/rendering/hud/GameNumbers.h"
#include "core/Between.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kSteps = 100; // length of an arc flight, in steps
        constexpr int kStep = 3; // steps / px a number advances per animation frame
        constexpr int kMostRise = 300; // most a rising number climbs, px
        constexpr int kSideShift = 30; // sideways offset of a rising number, px
        constexpr int kNumberSize = 4; // number font size
        constexpr std::array<int, 4> kPhaseSizes{4, 6, 6, 4}; // number font size per bounce phase 0..3
        const char* const kHpFont = "Num_YellowRed10_2"; // font of HP numbers
        const char* const kSpFont = "Num_Blue10_1"; // font of SP numbers

        /// @brief Rounds a double to the nearest int.
        /// @param value The value.
        /// @return The rounded value.
        int round(double value) { return static_cast<int>(std::lrint(value)); }

        /// @brief Shares an amount out over a motion's hits by their percentages, leaving what the rounding left on the last.
        /// @details Original: SplitDamageOverHits, using FHitPercents.
        /// @param parts Receives the amount of each hit (1-based).
        /// @param motion The motion record with the hit percentages.
        /// @param total The amount to share.
        void share(std::array<std::int32_t, HitNumbers::kHits + 1>& parts, const formats::MotionRecord& motion, std::int32_t total) {
            const int count = std::min<int>(static_cast<int>(motion.hits.size()), HitNumbers::kHits);
            if (count <= 0) {
                return;
            }
            std::int32_t sum = 0;
            for (int k = 1; k <= count; ++k) {
                const int part =
                    round(motion.hits[static_cast<std::size_t>(k - 1)].second * static_cast<double>(total) / 100); // 100: the hit's share is a percentage
                sum += part;
                parts[static_cast<std::size_t>(k)] += part;
            }
            auto& last = parts[static_cast<std::size_t>(count)];
            if (sum < total) {
                last += total - sum;
            } else if (sum > total) {
                std::int32_t over = sum - total;
                if (last - over < 0) {
                    for (int j = count; j >= 1 && over != 0; --j) {
                        auto& part = parts[static_cast<std::size_t>(j)];
                        if (part - over < 0) {
                            over -= part;
                            part = 0;
                        } else {
                            part -= over;
                            over = 0;
                        }
                    }
                } else {
                    last -= over;
                }
            }
        }

    }

    void HitNumbers::split(const formats::MotionRecord& motion, std::int32_t hp, std::int32_t sp) {
        if (motion.hitMode == 0) {
            m_hits[1].hp += hp;
            m_hits[1].sp += sp;
            return;
        }
        std::array<std::int32_t, kHits + 1> hpParts{};
        std::array<std::int32_t, kHits + 1> spParts{};
        share(hpParts, motion, hp);
        share(spParts, motion, sp);
        for (int k = 1; k <= kHits; ++k) {
            m_hits[static_cast<std::size_t>(k)].hp += hpParts[static_cast<std::size_t>(k)];
            m_hits[static_cast<std::size_t>(k)].sp += spParts[static_cast<std::size_t>(k)];
        }
    }

    void HitNumbers::showHp(int hit, core::Point feet, int cameraX, int style) {
        Hit& h = m_hits[index(hit)];
        h.hpState = 1;
        h.hpAt = feet;
        h.hpCameraX = cameraX;
        h.spState = 1;
        h.hpStyle = style;
    }

    void HitNumbers::showSp(int hit, core::Point feet, int cameraX, int style) {
        Hit& h = m_hits[index(hit)];
        h.hpState = 1;
        h.spAt = feet;
        h.spCameraX = cameraX;
        h.spState = 1;
        h.spStyle = style;
    }

    int HitNumbers::arcY(int hit, int y, int step) {
        const int remain = std::abs(kSteps - step);
        // 0.001 keeps the divisor above zero; 360 is the peak height scale of the arc; 0.003 of the squared height is taken off
        int arc = round((0.5 - std::abs(remain / (kSteps + 0.001) - 0.5)) * 360);
        arc = round(arc - static_cast<double>(arc) * arc * 0.003);
        m_hits[index(hit)].hpRise = arc;
        return y - arc;
    }

    void HitNumbers::animateHp(int hit) {
        Hit& h = m_hits[index(hit)];
        switch (h.hpStyle) {
            case 0: // HP style 0: flies up in an arc
                if (h.hpStep <= kSteps) {
                    h.hpY = arcY(hit, h.hpAt.y - h.hpRise, h.hpStep);
                    h.hpStep += kStep;
                } else {
                    h.hpState = 2; // state 2: done
                }
                break;
            case 1: // HP style 1: rises straight up
                if (h.hpRise <= kMostRise) {
                    h.hpRise += kStep;
                    h.hpAt.y -= kStep;
                } else {
                    h.hpState = 2; // state 2: done
                }
                break;
            case 2: // HP style 2: bounces bigger
                if (h.hpStep <= kSteps) {
                    if (h.phase <= 0) {
                        ++h.phase;
                    }
                    h.hpY = arcY(hit, h.hpAt.y - h.hpRise, h.hpStep);
                    h.hpStep += kStep;
                    if (h.hpY <= 250) { // 250: screen y at which the bounce jumps to its last size phase (3)
                        h.phase = 3;
                    }
                } else {
                    h.hpState = 2; // state 2: done
                }
                break;
            default: // other HP styles: nothing moves
                break;
        }
    }

    void HitNumbers::animateSp(int hit) {
        Hit& h = m_hits[index(hit)];
        switch (h.spStyle) {
            case 0: // SP style 0: flies up in an arc
                if (h.spStep <= kSteps) {
                    h.spY = arcY(hit, h.spAt.y - h.spRise, h.spStep);
                    h.spStep += kStep;
                } else {
                    h.hpState = 2; // state 2: done
                }
                break;
            case 1: // SP style 1: rises straight up
                if (h.spRise <= kMostRise) {
                    h.spRise += kStep;
                    h.spAt.y -= kStep;
                } else {
                    h.hpState = 2; // state 2: done
                }
                break;
            default: // other SP styles: nothing moves
                break;
        }
    }

    void HitNumbers::step(const gfx::Canvas& canvas, int hit) {
        if (!canvas.isAnimationFrame()) {
            return;
        }
        m_before[index(hit)] = m_hits[index(hit)];
        animateHp(hit);
        animateSp(hit);
    }

    std::pair<core::Point, core::Point> HitNumbers::places(
        const Hit& h,
        int hit,
        int side,
        int cameraX,
        bool single
    ) {
        const int hpBase = h.hpCameraX - cameraX;
        const int spBase = h.spCameraX - cameraX;
        int direction = side == 1 ? 1 : -1;
        int hpReach = h.hpStep;
        int spReach = h.spStep;
        if (!single) {
            direction = (hit % 2 == 1) == (side == 1) ? 1 : -1;
            hpReach = h.hpStep + h.hpStep * hit / 2;
            spReach = h.spStep + h.spStep * hit / 2;
        }
        const bool hpRises = (single || h.hpStyle == 2) && h.hpStyle == 1;
        const bool spRises = single && h.spStyle == 1;
        const core::Point hpAt =
            hpRises ? core::Point{hpBase + h.hpAt.x - direction * kSideShift, h.hpAt.y} : core::Point{h.hpAt.x + direction * hpReach + hpBase, h.hpY};
        const core::Point spAt =
            spRises ? core::Point{spBase + h.spAt.x + direction * kSideShift, h.spAt.y} : core::Point{h.spAt.x + direction * spReach + spBase, h.spY};
        return {hpAt, spAt};
    }

    void HitNumbers::drawHit(
        gfx::Canvas& canvas,
        int hit,
        int side,
        int cameraX,
        bool single
    ) {
        constexpr int kSnapPixels = 160; // moves further than this (px) between beats are not smoothed
        Hit& h = m_hits[index(hit)];
        auto [hpAt, spAt] = places(h, hit, side, cameraX, single);
        if (const auto amount = canvas.betweenBeats()) {
            const auto [hpFrom, spFrom] = places(m_before[index(hit)], hit, side, cameraX, single);
            hpAt = core::between(hpFrom, hpAt, *amount, kSnapPixels);
            spAt = core::between(spFrom, spAt, *amount, kSnapPixels);
        }
        if (h.hp != 0) {
            if (!single && h.hpStyle != 2) {
                drawGameNumber(canvas, kHpFont, hpAt, h.hp, kNumberSize, true);
            } else if (h.hpStyle == 0 || h.hpStyle == 1) {
                drawGameNumber(canvas, kHpFont, hpAt, h.hp, kNumberSize, true);
            } else {
                drawGameNumber(
                    canvas,
                    kHpFont,
                    hpAt,
                    h.hp,
                    kPhaseSizes[static_cast<std::size_t>(std::clamp(h.phase, 0, 3))],
                    true
                ); // 3: last size phase of the bounce
            }
        }
        if (h.sp != 0 && (!single || h.spStyle == 0 || h.spStyle == 1)) {
            drawGameNumber(canvas, kSpFont, spAt, h.sp, kNumberSize, true);
        }
        if (h.hpState == 2) { // state 2: done
            h = Hit{};
        }
    }

    void HitNumbers::drawSingle(gfx::Canvas& canvas, int side, int cameraX) {
        if (m_hits[1].hpState != 1) {
            return;
        }
        step(canvas, 1);
        drawHit(canvas, 1, side, cameraX, true);
    }

    void HitNumbers::draw(gfx::Canvas& canvas, const formats::MotionRecord& motion, int side, int cameraX) {
        if (motion.hitMode == 0) {
            drawSingle(canvas, side, cameraX);
            return;
        }
        const int count = std::min<int>(static_cast<int>(motion.hits.size()), kHits);
        for (int hit = 1; hit <= count; ++hit) {
            if (m_hits[index(hit)].hpState != 1) {
                continue;
            }
            step(canvas, hit);
            drawHit(canvas, hit, side, cameraX, false);
        }
    }

}
