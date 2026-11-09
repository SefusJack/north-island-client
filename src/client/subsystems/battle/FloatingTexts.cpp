#include "client/subsystems/battle/FloatingTexts.h"

#include "client/rendering/hud/GameNumbers.h"
#include "core/Between.h"

#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kNextDelay = 200; // ms a text waits more than the previous one that starts at the same y
        constexpr int kArcKind = 4; // text kind 4: flies in an arc on its way
        constexpr int kNumberKind = 3; // text kind 3: a number drawn with a fixed font size
        constexpr int kSnapPixels = 160; // moves further than this (px) between beats are not smoothed

        /// @brief Rounds a double to the nearest int.
        /// @param value The value.
        /// @return The rounded value.
        int round(double value) { return static_cast<int>(std::lrint(value)); }

        /// @brief Picks the number font of a colour index.
        /// @details Original: InitNumberFonts' order, by which DrawNumber picks a font.
        /// @param colour Colour index 1..13.
        /// @return The font picture name, or "" for an unknown colour.
        const char* fontOf(int colour) {
            static const char* const kFonts[] = {
                "",
                "Num_White3_1",
                "Num_Black1_1",
                "Num_Blue1_1",
                "Num_Red1_1",
                "Num_White1_1",
                "Num_White3_2",
                "Num_Yellow3_1",
                "Num_YellowRed10_1",
                "Num_Blue10_1",
                "Num_YellowRed10_2",
                "Num_Blue10_2",
                "Num_YellowRed10_3",
                "Num_White5_1"
            };
            return colour >= 1 && colour <= 13 ? kFonts[colour] : ""; // 13: the last font index
        }

        /// @brief Tells whether a text is a whole number and parses it.
        /// @param text The text.
        /// @param value Receives the number when the text is one.
        /// @return True when the text is a number.
        bool isNumber(const std::string& text, int& value) {
            if (text.empty()) {
                return false;
            }
            char* end = nullptr;
            const long parsed = std::strtol(text.c_str(), &end, 10); // 10: decimal
            if (*end != '\0') {
                return false;
            }
            value = static_cast<int>(parsed);
            return true;
        }

    }

    void FloatingTexts::add(
        const std::string& text,
        int kind,
        core::Point from,
        core::Point to,
        double speed,
        int colour,
        std::uint64_t nowMs
    ) {
        Text added;
        added.text = text;
        added.kind = kind;
        added.x = from.x;
        added.y = from.y;
        added.baseY = from.y;
        added.targetX = to.x;
        added.targetY = to.y;
        added.speed = speed;
        added.colour = colour;
        added.distance = round(std::sqrt(static_cast<double>((to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y))));
        added.moveMs = nowMs;
        added.drawnAt = from;
        added.drawnFrom = from;
        if (!m_texts.empty() && m_texts.back().y == from.y) {
            added.startDelay = m_texts.back().startDelay + kNextDelay;
        }
        m_texts.push_back(added);
    }

    void FloatingTexts::move(Text& text, std::uint64_t nowMs) {
        const int step = round(static_cast<double>(nowMs - text.moveMs) * text.speed);
        text.moveMs = nowMs;
        const double dx = text.targetX - text.x;
        const double dy = text.targetY - text.y;
        if (std::abs(dx) <= step && std::abs(dy) <= step) {
            text.x = text.targetX;
            text.y = text.targetY;
            text.arrived = true;
        } else {
            text.arrived = false;
            const double slope = std::abs(dy / (0.001 + dx)); // 0.001 keeps the divisor above zero
            const double stepX = 1 / std::sqrt(slope * slope + 1) * step;
            text.x += dx >= 0 ? round(stepX) : -round(stepX);
            text.y += dy >= 0 ? round(slope * stepX) : -round(slope * stepX);
        }
        if (text.kind == kArcKind) {
            text.baseY = text.y;
            const int remain =
                round(std::sqrt(static_cast<double>((text.targetX - text.x) * (text.targetX - text.x) + (text.targetY - text.y) * (text.targetY - text.y))));
            // 0.001 keeps the divisor above zero; 360 is the peak height scale of the arc; 0.003 of the squared height is taken off
            int arc = round((0.5 - std::abs(remain / (text.distance + 0.001) - 0.5)) * 360);
            arc = round(arc - static_cast<double>(arc) * arc * 0.003);
            text.y -= arc;
        }
        text.drawnFrom = text.drawnAt;
        text.drawnAt = {text.x, text.y};
    }

    void FloatingTexts::update(std::uint64_t nowMs) {
        for (auto it = m_texts.end(); it != m_texts.begin();) {
            --it;
            if (it->startDelay > 0) {
                if (nowMs - it->moveMs >= static_cast<std::uint64_t>(it->startDelay)) {
                    it->startDelay = 0;
                    it->moveMs = nowMs;
                }
                continue;
            }
            move(*it, nowMs);
            if (it->arrived) {
                it = m_texts.erase(it);
            }
        }
    }

    void FloatingTexts::draw(gfx::Canvas& canvas) {
        constexpr float kLetterScale = 0.4F; // scale of the text letters (40 %)
        constexpr int kLetterStep = 20; // advance per letter, px
        constexpr int kSize = 4; // number font size of kind 3 numbers
        for (Text& text : m_texts) {
            if (text.startDelay > 0) {
                continue;
            }
            core::Point at{text.x, text.y};
            if (!canvas.isAnimationFrame()) {
                at = text.drawnAt;
            }
            if (const auto amount = canvas.betweenBeats(); amount && text.moveMs >= canvas.beatMs()) {
                at = core::between(text.drawnFrom, text.drawnAt, *amount, kSnapPixels);
            }
            int value = 0;
            if (!isNumber(text.text, value)) {
                drawGameLetters(canvas, at, text.text, 0, kLetterScale, kLetterStep, 0, true);
            } else if (text.kind == kNumberKind) {
                // colour 8: HP's yellow-red font, colour 9: SP's blue font
                const char* font = text.colour == 8 ? "Num_YellowRed10_2" : (text.colour == 9 ? "Num_Blue10_1" : fontOf(text.colour));
                drawGameNumber(canvas, font, at, value, kSize, true);
            } else {
                const int remain =
                    round(std::sqrt(static_cast<double>((at.x - text.targetX) * (at.x - text.targetX) + (at.y - text.targetY) * (at.y - text.targetY))));
                const int size = static_cast<int>(
                    std::ceil(remain / (text.distance + 0.001) * 5)
                ); // 5: the number shrinks from size 5 to 0 along its way; 0.001 keeps the divisor above zero
                drawGameNumber(canvas, fontOf(text.colour), at, value, size, true);
            }
            if (text.kind == kArcKind && canvas.isAnimationFrame()) {
                text.y = text.baseY;
            }
        }
    }

}
