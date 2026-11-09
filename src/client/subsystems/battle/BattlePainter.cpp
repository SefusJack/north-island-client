#include "client/subsystems/battle/BattlePainter.h"

#include "client/subsystems/battle/DashArc.h"
#include "client/rendering/characters/FigureShadow.h"
#include "core/Between.h"
#include "gfx/TextStyle.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr int kCharWidth = 8; // width of one name character, px
        constexpr int kNameGap = 10; // gap between the head and the name, px
        constexpr int kShortSprite = 50; // sprites shorter than this (px) get the larger name gap
        constexpr int kShortNameGap = 25; // name gap above a short sprite, px
        constexpr int kBarWidth = 40; // full HP bar width, px
        constexpr int kBarHeight = 4; // HP bar height, px
        constexpr int kBarLeft = 18; // px the HP bar starts left of the feet x
        constexpr int kTopmostBar = 80; // highest y an HP bar may be drawn at, px
        constexpr core::Point kCountdown{400, 300}; // centre of the countdown number on the screen, px
        constexpr int kLowCountdown = 50; // px the countdown is lowered for battle types 2 and 7
        constexpr int kDigitWidth = 20; // width of one big number digit, px
        constexpr int kDigitHeight = 27; // height of one big number digit, px
        constexpr int kDigitRows = 10; // rows (digits 0-9) in the big number picture
        const core::Color kPlayerName = core::Color::fromRgb565(0xFFE0); // yellow
        const core::Color kFighterName = core::Color::fromRgb565(0x7FF); // cyan
        const core::Color kInfoText = core::Color::fromRgb565(65535); // white

        /// @brief Builds a text style with a drop shadow.
        /// @param color Text colour.
        /// @return The shadowed style.
        gfx::TextStyle shadowed(core::Color color) { return {color, gfx::TextEffect::Shadow, std::nullopt}; }

        /// @brief Returns the HP shown for a fighter, as a boss with several bars shows it.
        /// @param fighter The fighter.
        /// @return The HP of the bar being taken and the size of that bar.
        std::pair<std::int32_t, std::int32_t> shownHp(const Fighter& fighter) {
            // kind 4: pet, never shown with several bars
            if (fighter.npc && fighter.kind != 4 && fighter.npc->multiHpBars != 0 && fighter.npc->barHp > 0) {
                const std::int32_t bar = fighter.npc->barHp;
                const std::int32_t full = fighter.hp / bar;
                if (fighter.hp % bar == 0 && full > 0) {
                    return {bar, bar};
                }
                return {fighter.hp - bar * full, bar};
            }
            return {fighter.hp, fighter.maxHp};
        }

        /// @brief Returns how high a fighter on its run-in dash is drawn above the ground.
        /// @details Original: ApplyJumpArc, which lifts only a fighter on its run-in dash (FDashing), never a move of an attack.
        /// @param fighter The fighter.
        /// @return The lift in pixels (0 when not dashing).
        int dashLift(const Fighter& fighter) {
            return fighter.dashing && fighter.entering == 2 ? DashArc::lift(fighter.position, fighter.heading, fighter.dashLength) : 0;
        } // entering 2: running in on the dash

        /// @brief Delphi's Round: to the nearest whole number, halves to the even one.
        /// @param value The value.
        /// @return The rounded value.
        int delphiRound(double value) { return static_cast<int>(std::nearbyint(value)); }

        /// @brief Moves a fighter's name for the skill zoom, which stretches the fighter but not its name.
        /// @details Original: THum.DrawHumInfo of a fighter while FZoomRect.Top > 0. By the band of the screen the fighter stands in
        /// (feet y up to 369, 412, 455, below), the name goes up by a share of its y (a third, a quarter, a seventh, a tenth of it times
        /// SkillZoomScale) and by a set height times the scale (players: kinds 1, 2 and 9; the others by their Npc.dat height mode),
        /// then 30 px up for a jumping fighter (10 for the others); on side 1 it goes right by 30 x the scale.
        /// @param fighter The fighter.
        /// @param name Where the name would be without the zoom (its y the original's NameY).
        /// @param scale SkillZoomScale.
        /// @return Where the name goes.
        core::Point zoomedName(const Fighter& fighter, core::Point name, float scale) {
            constexpr int kBand1 = 369; // feet y of the far row's band end, px
            constexpr int kBand2 = 412; // feet y of the second band's end, px
            constexpr int kBand3 = 455; // feet y of the third band's end, px
            const double share = scale;
            const bool human = fighter.kind == 1 || fighter.kind == 2 || fighter.kind == 9; // fighter kinds 1, 2 and 9: players
            const int heightMode = fighter.npc ? fighter.npc->heightMode : 0; // Npc.dat height mode, 0 for a kind not found
            const int feetY = fighter.position.y;
            int y = name.y;
            if (feetY <= kBand1) {
                y -= delphiRound(y * share) / 3; // a third of the name's y
                if (human || heightMode == 0) {
                    y -= delphiRound(10 * share); // 10 px at scale 1
                } else if (heightMode == 1) {
                    y -= delphiRound(40 * share); // 40 px at scale 1 for a tall kind
                }
            } else if (feetY <= kBand2) {
                y -= delphiRound(y * share) / 4; // a quarter of the name's y
                if (!human && heightMode == 1) {
                    y -= delphiRound(50 * share); // 50 px at scale 1 for a tall kind
                }
            } else if (feetY <= kBand3) {
                y -= delphiRound(y * share) / 7; // a seventh of the name's y
                if (human || heightMode == 0) {
                    y -= delphiRound(20 * share); // 20 px at scale 1
                } else if (heightMode == 1) {
                    y -= delphiRound(30 * share); // 30 px at scale 1 for a tall kind
                }
            } else {
                y -= delphiRound(y * share) / 10; // a tenth of the name's y
                if (human) {
                    y -= 10; // 10 px more for a player in the near band
                }
                if (human || heightMode == 0) {
                    y -= delphiRound(20 * share); // 20 px at scale 1
                } else if (heightMode == 1) {
                    y -= delphiRound(30 * share); // 30 px at scale 1 for a tall kind
                }
            }
            y -= fighter.jumping ? 30 : 10; // 30 px up while jumping, else 10
            const int x = fighter.side() == 1 ? name.x + delphiRound(30 * share) : name.x; // 30 px right at scale 1 on side 1
            return {x, y};
        }

    }

    void BattlePainter::settlePose(Fighter& fighter) {
        if (fighter.hp <= 0) {
            if (!fighter.hitEffectOn && !fighter.casting) {
                fighter.action = fighter.side() == 1 ? 26 : 27; // death action 26 (side 1) / 27
                fighter.holdFrame = 255; // 255: no frame held
            }
        } else if (!fighter.casting && !fighter.isHit && !fighter.statusPose) {
            fighter.action = fighter.side() == 1 ? 16 : 17; // battle stance 16 (side 1) / 17
            fighter.holdFrame = 255; // 255: no frame held
        }
    }

    int BattlePainter::spriteSlot(const Fighter& fighter) const {
        if (fighter.npc) {
            return m_characters.npcSlot(fighter.npc->spriteId, fighter.npc->directionKind, fighter.action);
        }
        return fighter.action;
    }

    int BattlePainter::frameCount(const Fighter& fighter) const {
        const int slot = spriteSlot(fighter);
        return fighter.npc ? m_characters.npcFrameCount(fighter.npc->spriteId, slot) : m_characters.figureFrameCount(*fighter.look, slot);
    }

    int BattlePainter::shownFrame(const Fighter& fighter, int count) {
        if (count <= 0) {
            return 0;
        }
        // 255: no frame held
        return std::min(fighter.holdFrame != 255 ? fighter.holdFrame : fighter.animation.current(), count - 1);
    }

    int BattlePainter::nextFrame(Fighter& fighter, int count, std::uint64_t nowMs) {
        const bool dead = fighter.hp <= 0 && (fighter.action == 26 || fighter.action == 27); // death actions 26 / 27
        const int turning =
            dead ? fighter.animation.frame(nowMs, count, 230, true) : fighter.animation.frame(nowMs, count); // 230: frame time of the death action in ms
        if (fighter.holdFrame != 255 && count > 0) { // 255: no frame held
            return std::min(fighter.holdFrame, count - 1);
        }
        return turning;
    }

    std::vector<int> BattlePainter::drawingOrder(const BattleField& field) {
        std::vector<int> slots;
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (field.fighter(slot)) {
                slots.push_back(slot);
            }
        }
        std::sort(slots.begin(), slots.end(), [&](int a, int b) {
            const Fighter& first = *field.fighter(a);
            const Fighter& second = *field.fighter(b);
            if (first.position.y != second.position.y) {
                return first.position.y < second.position.y;
            }
            if (first.position.x != second.position.x) {
                return first.position.x < second.position.x;
            }
            return a < b;
        });
        return slots;
    }

    void BattlePainter::paintFighters(gfx::Canvas& canvas, BattleField& field, int hoveredSlot, std::uint64_t nowMs) {
        for (const int slot : drawingOrder(field)) {
            drawAfterImages(canvas, *field.fighter(slot));
            drawFighter(canvas, *field.fighter(slot), slot == hoveredSlot, field.effects.shake, nowMs);
        }
    }

    void BattlePainter::drawCutIn(gfx::Canvas& canvas, Fighter& fighter, const EffectLayer& layer, std::uint64_t nowMs) {
        constexpr int kBandY = 100; // screen y of the cut-in band, px
        constexpr int kFaceY = 196; // screen y of the cut-in face, px
        constexpr int kFaceStep = 90; // px the face moves per frame
        constexpr int kBandStep = 40; // px the band scrolls per frame
        constexpr int kBandRepeat = 40; // frames after which the band scroll wraps
        SkillCutIn& cutIn = fighter.cutIn;
        if (!cutIn.on) {
            return;
        }
        if (layer.frameCount() < cutIn.frame) {
            cutIn = {};
            return;
        }
        const int delay = std::max(layer.frameDelay(), 1);
        if (!cutIn.startMs) {
            cutIn.startMs = nowMs;
        } else if (nowMs < *cutIn.startMs + static_cast<std::uint64_t>(delay) * static_cast<std::uint64_t>(cutIn.lastFrame)) {
            cutIn.startMs = nowMs;
        }
        if (!fighter.look) {
            return;
        }
        const bool fromRight = fighter.side() == 1;
        const int scroll = fromRight ? 1 : -1;
        // screen x of the band copies (back, front) and of the face's start, per side
        const int back = fromRight ? -848 : -16; // px
        const int front = fromRight ? -16 : 816; // px
        const int baseX = fromRight ? 850 : -50; // px, just off the screen edge (screen is 800 wide)
        cutIn.frame = static_cast<int>((nowMs - *cutIn.startMs) / static_cast<std::uint64_t>(delay));
        cutIn.lastFrame = cutIn.frame;
        const int step = std::min(cutIn.frame, 3); // the face stops after 3 steps
        if (const auto band = m_pictures.find("SpecEffectBg")) {
            const int shift = (cutIn.frame % kBandRepeat) * scroll * kBandStep;
            canvas.drawPicture(*band, {shift + front, kBandY});
            canvas.drawPicture(*band, {shift + back, kBandY});
        }
        m_characters.drawCutInFace(
            canvas,
            *fighter.look,
            {baseX - scroll * step * kFaceStep, kFaceY},
            fromRight ? 0 : 1,
            200,
            100
        ); // 200 / 100: face scale and divisor (200 %)
    }

    void BattlePainter::paintClock(gfx::Canvas& canvas, const BattleField& field, const Clock& clock, std::uint64_t nowMs) {
        if (clock.countdown >= 0) {
            const bool low = clock.battleType == 2 || clock.battleType == 7; // battle types 2 and 7 show the countdown lower
            drawBigNumber(canvas, "Num_Blue10_2", {kCountdown.x, kCountdown.y + (low ? kLowCountdown : 0)}, clock.countdown);
        }
        if (clock.runningOut) {
            for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
                if (const auto fighter = field.fighter(slot)) {
                    drawPendingMark(canvas, *fighter, nowMs);
                }
            }
        }
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            if (const auto fighter = field.fighter(slot); fighter && !fighter->get().waiting && fighter->get().entering == 0) {
                drawHpBar(canvas, *fighter);
            }
        }
        if (clock.timeLimit >= 0) {
            drawTimeLimit(canvas, clock.timeLimit);
        }
    }

    void BattlePainter::drawTimeLimit(gfx::Canvas& canvas, int secondsLeft) {
        constexpr int kLeft = 290; // screen x the time limit display starts at, px
        constexpr int kTop = 150; // screen y of the time limit display, px
        constexpr int kColonSize = 32; // width and height of the colon picture, px
        drawBigNumber(canvas, "Num_Blue10_2", {kLeft + 80, kTop}, secondsLeft / 60); // 80: x offset of the minutes; 60 seconds per minute
        if (const auto colon = m_pictures.find("SC_F_1")) {
            canvas.drawPicture(*colon, {kLeft + 90, kTop - 4}, core::Rect{0, 0, kColonSize, kColonSize}); // 90 / 4: x / y offset of the colon
        }
        drawBigNumber(canvas, "Num_Blue10_2", {kLeft + 130, kTop}, secondsLeft % 60 / 10); // 130: x offset of the seconds' tens digit
        drawBigNumber(canvas, "Num_Blue10_2", {kLeft + 150, kTop}, secondsLeft % 10); // 150: x offset of the seconds' units digit
    }

    void BattlePainter::drawFighter(
        gfx::Canvas& canvas,
        Fighter& fighter,
        bool hovered,
        core::Point shake,
        std::uint64_t nowMs
    ) {
        constexpr std::uint8_t kHoverLight = 140; // brightness of a hovered fighter (0-255)
        if (!fighter.flyAwayMs) {
            settlePose(fighter);
        }
        const int slot = spriteSlot(fighter);
        const int at = nextFrame(fighter, frameCount(fighter), nowMs);
        fighter.drawnFrame = at;
        if (fighter.flyAwayMs) {
            drawFlyAway(canvas, fighter, slot, at);
            return;
        }
        if (fighter.statuses.sub53 && (fighter.action == 16 || fighter.action == 17)) { // standing actions 16 / 17
            constexpr int kTransformed = 9016; // sign sprite drawn instead of the fighter while status 53 is on
            constexpr int kTransformedOf483 = 9038; // sign sprite used instead when the fighter is a sprite-483 NPC
            constexpr int kSprite483 = 483; // NPC sprite id that gets its own transformed sprite
            const core::Point feet{fighter.position.x + shake.x, fighter.position.y - (fighter.jumping ? fighter.jumpHeight : 0) - dashLift(fighter) + shake.y};
            drawFigureShadow(canvas, fighter.position, fighter.npc ? fighter.npc->shadow : 0);
            m_characters
                .drawSignSprite(canvas, fighter.npc && fighter.npc->spriteId == kSprite483 ? kTransformedOf483 : kTransformed, feet, fighter.action, at);
            return;
        }
        SpriteHighlight light = hovered ? SpriteHighlight{kHoverLight, true} : SpriteHighlight{};
        light.statusTint = fighter.statuses.tintKind;
        scaleFor(fighter, light);
        const core::Point body{fighter.position.x + shake.x, fighter.position.y - (fighter.jumping ? fighter.jumpHeight : 0) - dashLift(fighter) + shake.y};
        if (fighter.npc) {
            drawFigureShadow(canvas, fighter.position, fighter.npc->shadow);
            drawNpcFigure(canvas, fighter, body, slot, at, light);
        } else {
            drawFigureShadow(canvas, fighter.position, 0);
            m_characters.drawFigure(canvas, *fighter.look, body, slot, at, light);
        }
    }

    void BattlePainter::drawNpcFigure(
        gfx::Canvas& canvas,
        const Fighter& fighter,
        core::Point feet,
        int slot,
        int frame,
        SpriteHighlight highlight
    ) {
        if (fighter.weaponLook > 0) {
            m_characters.drawArmedNpc(canvas, *fighter.npc, fighter.weaponLook, fighter.weaponColor, feet, slot, frame, highlight);
        } else {
            m_characters.drawNpc(canvas, *fighter.npc, feet, slot, frame, highlight);
        }
    }

    void BattlePainter::drawAfterImages(gfx::Canvas& canvas, Fighter& fighter) {
        if (!fighter.casting) {
            return;
        }
        FighterMotion& motion = fighter.skillMotion;
        const core::Point camera = motion.cameraPosition();
        for (AfterImage& image : motion.afterImages()) {
            if (!image.active) {
                continue;
            }
            if (canvas.isAnimationFrame()) {
                image.drawnAlpha = image.alpha;
            }
            const auto amount = canvas.betweenBeats();
            const int alpha = amount ? core::between(image.drawnAlpha, std::max(image.drawnAlpha - image.fade, 0), *amount) : image.drawnAlpha;
            SpriteHighlight ghost;
            ghost.opacity = static_cast<std::uint8_t>(std::clamp(alpha, 0, 255)); // 255: full opacity
            scaleFor(fighter, ghost);
            const core::Point at{image.position.x - camera.x, image.position.y - camera.y};
            if (fighter.npc) {
                const int slot = m_characters.npcSlot(fighter.npc->spriteId, fighter.npc->directionKind, image.slot);
                const int count = m_characters.npcFrameCount(fighter.npc->spriteId, slot);
                drawNpcFigure(canvas, fighter, at, slot, std::min(image.frame, std::max(count - 1, 0)), ghost);
            } else {
                const int count = m_characters.figureFrameCount(*fighter.look, image.slot);
                m_characters.drawFigure(canvas, *fighter.look, at, image.slot, std::min(image.frame, std::max(count - 1, 0)), ghost);
            }
            if (canvas.isAnimationFrame()) {
                image.alpha = std::max(image.alpha - image.fade, 0);
                image.active = image.alpha != 0;
            }
        }
    }

    void BattlePainter::drawFlyAway(gfx::Canvas& canvas, const Fighter& fighter, int slot, int frame) {
        constexpr std::uint8_t kPlayerGhost = 35; // opacity of a flying-away player (0-255)
        constexpr std::uint8_t kGhost = 128; // opacity of any other flying-away fighter (0-255)
        SpriteHighlight ghost{fighter.isPlayer() ? kPlayerGhost : kGhost, false, true, fighter.statuses.tintKind};
        scaleFor(fighter, ghost);
        for (const core::Point at : {fighter.position, core::Point{fighter.position.x, fighter.position.y - fighter.flyRise}}) {
            if (fighter.npc) {
                drawNpcFigure(canvas, fighter, at, slot, frame, ghost);
            } else {
                m_characters.drawFigure(canvas, *fighter.look, at, slot, frame, ghost);
            }
        }
    }

    void BattlePainter::drawHpBar(gfx::Canvas& canvas, const Fighter& fighter) {
        const auto rail = m_pictures.find("icon_rail2");
        const auto fill = m_pictures.find("icon_HP2");
        if (!rail || !fill) {
            return;
        }
        const int lift = fighter.jumping ? fighter.jumpHeight : 0;
        const int y = std::max(kTopmostBar, fighter.position.y - lift - fighter.bodyHeight);
        const int x = fighter.position.x - kBarLeft;
        int bars = 1;
        std::int32_t hp = fighter.hp;
        std::int32_t barHp = fighter.maxHp;
        if (fighter.npc && fighter.kind != 4 && fighter.npc->multiHpBars != 0 && fighter.npc->barHp > 0) { // kind 4: pet, never shown with several bars
            barHp = fighter.npc->barHp;
            bars = fighter.hp / barHp + 1;
            hp = fighter.hp - barHp * (bars - 1);
            if (fighter.hp % barHp == 0 && bars > 1) {
                --bars;
                hp = barHp;
            }
        }
        const int width = static_cast<int>(std::lround(std::min(hp, barHp) * kBarWidth / (barHp + 0.01))); // 0.01: keeps the divisor above zero
        const int shift = (bars - 1) * 2; // 2: px the stack shifts down per extra bar
        for (int bar = 1; bar <= bars; ++bar) {
            const int barY = y + shift - (bar - 1) * kBarHeight;
            canvas.drawPicture(*rail, {x, barY});
            if (bar == 1) {
                canvas.drawPicture(*fill, {x, y + shift}, core::Rect{0, 0, width, kBarHeight});
            } else {
                canvas.drawPicture(*fill, {x, barY});
            }
        }
    }

    void BattlePainter::drawPendingMark(gfx::Canvas& canvas, const Fighter& fighter, std::uint64_t nowMs) {
        constexpr int kFrames = 4; // frames stacked in the mark picture
        constexpr std::uint64_t kFrameMs = 250; // time per mark frame, ms
        // fighter kinds that choose a command: 2 player, 4 pet, 9, 13, 15
        const bool chooses = fighter.kind == 2 || fighter.kind == 4 || fighter.kind == 9 || fighter.kind == 13 || fighter.kind == 15;
        if (!chooses || fighter.waiting || fighter.hp <= 0 || fighter.acted) {
            return;
        }
        const auto mark = m_pictures.find("E119");
        if (!mark) {
            return;
        }
        const gfx::Picture& picture = m_pictures.picture(*mark);
        const int frameHeight = picture.height() / kFrames;
        const int frame = static_cast<int>(nowMs / kFrameMs % kFrames);
        int y = fighter.position.y - fighter.spriteHeight;
        if (y <= kTopmostBar) {
            y = 90; // lowest y of the mark near the top of the screen
        }
        // 20: x offset of the mark from the figure's centre
        canvas.drawPicture(*mark, {fighter.position.x - picture.width() / 2 + 20, y}, core::Rect{0, frame * frameHeight, picture.width(), frameHeight});
    }

    void BattlePainter::drawBigNumber(gfx::Canvas& canvas, const char* font, core::Point at, int value) {
        const auto digits = m_pictures.find(font);
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = m_pictures.picture(*digits);
        const int glyphHeight = picture.height() / kDigitRows;
        const std::string text = std::to_string(std::abs(value));
        const int left = at.x - static_cast<int>(text.size()) * (kDigitWidth / 2);
        for (std::size_t i = 0; i < text.size(); ++i) {
            const int digit = text[i] - '0';
            canvas.drawPictureScaled(
                *digits,
                {left + static_cast<int>(i) * kDigitWidth, at.y, kDigitWidth, kDigitHeight},
                core::Rect{0, digit * glyphHeight, picture.width(), glyphHeight}
            );
        }
    }

    void BattlePainter::paintNames(gfx::Canvas& canvas, const BattleField& field, std::optional<float> zoomScale) {
        for (int slot = 1; slot <= BattleField::kSlots; ++slot) {
            const auto found = field.fighter(slot);
            if (!found || found->get().waiting) {
                continue;
            }
            const Fighter& fighter = *found;
            const int lift = (fighter.jumping ? fighter.jumpHeight : 0) + dashLift(fighter);
            const int width = static_cast<int>(fighter.name.size()) * kCharWidth;
            core::Point name{fighter.position.x - width / 2, fighter.position.y - lift - fighter.bodyHeight - kNameGap};
            if (zoomScale) {
                name = zoomedName(fighter, name, *zoomScale);
            }
            const int x = name.x;
            const int nameY = name.y;
            if (fighter.isPlayer()) {
                canvas.drawText({x, nameY - kNameGap}, fighter.name, shadowed(kPlayerName));
            } else {
                const int gap = fighter.spriteHeight < kShortSprite ? kShortNameGap : kNameGap;
                canvas.drawText({x, nameY - gap}, fighter.name, shadowed(kFighterName));
            }
        }
    }

    void BattlePainter::paintInfo(gfx::Canvas& canvas, const Fighter& fighter) {
        static const std::array<const char*, 6> kElements{"None", "Earth", "Water", "Fire", "Wind", "Heart"};
        static const std::array<const char*, 7>
            kJobIcons{"", "Icon_Asn_2", "Icon_Samurai_2", "Icon_Knight_2", "Icon_Sage_2", "Icon_Clergyman_2", "Icon_Prophet_2"};
        constexpr int kIconWidth = 24; // width of the job / reborn sign, px
        const std::string element =
            fighter.element >= 0 && fighter.element < static_cast<int>(kElements.size()) ? kElements[static_cast<std::size_t>(fighter.element)] : "";
        std::optional<gfx::PictureId> icon;
        if (fighter.job > 0 && fighter.job < static_cast<int>(kJobIcons.size())) {
            icon = m_pictures.find(kJobIcons[static_cast<std::size_t>(fighter.job)]);
        }
        if (!fighter.isPlayer() && fighter.reborn != 0) {
            icon = m_pictures.find("Okey3");
        }
        const auto [hp, maxHp] = shownHp(fighter);
        const std::string level = "LV:" + std::to_string(fighter.level);
        const std::string hpText = "HP:" + std::to_string(hp) + "/" + std::to_string(maxHp);
        const bool leftOf = fighter.row == 1 || fighter.row == 3; // rows 1 and 3 are the near rows, info goes left
        const core::Point feet = fighter.position;
        const auto lineX = [&](const std::string& text) {
            // 25 / 30: gap from the feet to the text on the left / right side
            return leftOf ? feet.x - 25 - static_cast<int>(text.size()) * kCharWidth : feet.x + 30;
        };
        const gfx::TextStyle style = shadowed(kInfoText);
        // y offsets above the feet of the name, element, level and HP lines
        canvas.drawText({lineX(fighter.name), feet.y - 50}, fighter.name, style);
        canvas.drawText({lineX(element), feet.y - 35}, element, style);
        if (!icon) {
            canvas.drawText({lineX(level), feet.y - 20}, level, style);
            canvas.drawText({lineX(hpText), feet.y - 10}, hpText, style);
            return;
        }
        if (leftOf) {
            canvas.drawPicture(*icon, {lineX(level) - kIconWidth - 1, feet.y - 20}); // 20: y offset of the sign above the feet
            canvas.drawText({lineX(level), feet.y - 18}, level, style); // 18: y offset of the level line when a sign is shown
        } else {
            canvas.drawPicture(*icon, {feet.x + 29, feet.y - 20}); // 29 / 20: x / y offset of the sign
            canvas.drawText({feet.x + 30 + kIconWidth, feet.y - 18}, level, style); // 30 / 18: x / y offset of the level text
        }
        canvas.drawText({lineX(hpText), feet.y - 4}, hpText, style); // 4: y offset of the HP line when a sign is shown
    }

    void BattlePainter::scaleFor(const Fighter& fighter, SpriteHighlight& highlight) {
        const int size = fighter.statuses.sizeMode;
        if (fighter.npc && fighter.npc->heightMode == 1) {
            highlight.scale = 20; // 20 / 10: a tall kind is drawn twice the size
            highlight.scaleDivisor = 10;
            return;
        }
        switch (size) {
            case 1: // size status 1: half again (150 %), lower layers drop 32
                highlight.scale = 15; // 15 / 10: scale 1.5
                highlight.scaleDivisor = 10;
                highlight.lowDrop = 32; // lower layers drop 32 pixels
                break;
            case 2: // size status 2: half (50 %), lower layers rise 32
                highlight.scale = 50; // 50 / 100: scale 0.5
                highlight.scaleDivisor = 100;
                highlight.lowDrop = -32; // lower layers rise 32 pixels
                break;
            case 3: // size status 3: double (200 %)
                highlight.scale = 200; // 200 / 100: scale 2
                highlight.scaleDivisor = 100;
                break;
            default: // no size change
                break;
        }
    }

    int BattlePainter::slotAt(BattleField& field, core::Point point) {
        const std::vector<int> order = drawingOrder(field);
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            Fighter& fighter = *field.fighter(*it);
            if (fighter.waiting) {
                continue;
            }
            if (!fighter.flyAwayMs) {
                settlePose(fighter);
            }
            const int slot = spriteSlot(fighter);
            SpriteHighlight scaled;
            scaleFor(fighter, scaled);
            const bool under = fighter.npc
                                   ? m_characters.npcContains(*fighter.npc, fighter.position, slot, shownFrame(fighter, frameCount(fighter)), point, scaled)
                                   : m_characters.figureFrameArea(*fighter.look, fighter.position, slot).contains(point);
            if (under) {
                return *it;
            }
        }
        return 0;
    }

}
