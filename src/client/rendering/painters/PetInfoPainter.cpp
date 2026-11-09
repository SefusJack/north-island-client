#include "client/rendering/painters/PetInfoPainter.h"

#include "client/rules/Experience.h"
#include "gfx/TextStyle.h"

#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr int kFigureSlot = 0; // character slot used to draw the pet figure
        constexpr core::Point kFigureFeet{6, -6}; // px: pet feet position relative to the preview origin
        constexpr core::Point kRebornMark{-23, 20}; // px: reborn mark (Okey3) position in the pet panel
        constexpr core::Point kBattleRebornMark{-23, 10}; // px: reborn mark (Okey3) position in the battle pet panel
        constexpr core::Point kLevelWord{0, 2}; // px: Lv word, relative to the parts origin
        constexpr core::Point kLevelDigits{17, 2}; // px: level digits, relative to the parts origin
        constexpr core::Point kHpWord{0, 10}; // px: HP word, relative to the parts origin
        constexpr core::Point kHpFrame{13, 11}; // px: HP bar frame, relative to the parts origin
        constexpr core::Point kHpBar{14, 12}; // px: HP bar fill, relative to the parts origin
        constexpr core::Point kSpWord{0, 18}; // px: SP word, relative to the parts origin
        constexpr core::Point kSpFrame{13, 19}; // px: SP bar frame, relative to the parts origin
        constexpr core::Point kSpBar{14, 20}; // px: SP bar fill, relative to the parts origin
        constexpr core::Point kExpPercent{-10, 29}; // px: experience percent number, relative to the parts origin
        constexpr core::Point kExpFrame{-31, 30}; // px: experience bar frame, relative to the parts origin
        constexpr core::Point kExpBar{-30, 31}; // px: experience bar fill, relative to the parts origin
        constexpr int kDigitStep = 5; // px: x advance per digit
        constexpr int kDigitPitch = 6; // px: height of one digit row in the digit picture

        /// @brief Fill ratio of a bar: Ceil(value / most * 100) %, at most 100.
        /// @param value Current value.
        /// @param most Maximum value.
        /// @return Ratio from 0 to 1.
        double barRatio(double value, double most) {
            if (most <= 0) {
                return 0;
            }
            return std::min(100.0, std::ceil(value / most * 100.0)) / 100.0; // ratio to percent and back
        }

        /// @brief A pet's experience within its level: gained so far and needed for the next level.
        struct PetExp {
            std::int64_t current = 0;
            std::int64_t next = 0;
        };

        /// @brief Works out a pet's experience within its level.
        /// @param formula Experience formulas.
        /// @param pet The pet.
        /// @return Experience gained in the level and needed for the next one.
        PetExp petExp(const formats::FormulaData& formula, const Pet& pet) {
            return {pet.exp - totalLevelExp(formula, pet.level, pet.reborn), levelUpExp(formula, pet.level + 1, pet.reborn)};
        }

    }

    core::Point PetInfoPainter::part(core::Point at, core::Point offset) const {
        return {at.x + m_style.parts.x + offset.x, at.y + m_style.parts.y + offset.y};
    }

    core::Rect PetInfoPainter::hpBar(core::Point at) const {
        const core::Point frame = part(at, kHpFrame);
        return {frame.x, frame.y, 27, 5}; // bar size in pixels (27 x 5)
    }

    core::Rect PetInfoPainter::spBar(core::Point at) const {
        const core::Point frame = part(at, kSpFrame);
        return {frame.x, frame.y, 27, 5}; // bar size in pixels (27 x 5)
    }

    void PetInfoPainter::drawNumber(gfx::Canvas& canvas, int value, core::Point at) const {
        const auto digits = m_services.pictures.find(m_style.blackDigits ? "Pet_Num_Lv_Black_1" : "Pet_Num_Lv_White_1");
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = m_services.pictures.picture(*digits);
        const std::string text = std::to_string(value);
        for (std::size_t i = 0; i < text.size(); ++i) {
            const int digit = text[i] - '0';
            canvas.drawPicture(
                *digits,
                {at.x + static_cast<int>(i) * kDigitStep, at.y},
                core::Rect{0, digit * kDigitPitch, picture.width(), picture.height() / 10}
            ); // the picture is a column of ten digits
        }
    }

    void PetInfoPainter::drawBar(gfx::Canvas& canvas, const char* picture, core::Point at, double ratio) const {
        const auto bar = m_services.pictures.find(picture);
        if (!bar) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*bar);
        const int width = static_cast<int>(data.width() * ratio);
        if (width > 0) {
            canvas.drawPicture(*bar, at, core::Rect{0, 0, width, data.height()});
        }
    }

    void PetInfoPainter::paintFigure(gfx::Canvas& canvas, core::Point at, const Pet& pet, std::uint64_t nowMs) {
        const int frame = m_animation.frame(nowMs, m_services.characters.petFrameCount(pet.info, kFigureSlot));
        m_services.characters.drawPet(canvas, pet.info, at + m_style.preview + kFigureFeet, kFigureSlot, frame);
    }

    void PetInfoPainter::paint(gfx::Canvas& canvas, core::Point at, const Pet& pet, std::uint64_t nowMs) {
        const auto picture = [this](const char* name) { return m_services.pictures.find(name); };
        const auto draw = [&](const char* name, core::Point where) {
            if (const auto id = picture(name)) {
                canvas.drawPicture(*id, where);
            }
        };
        if ((m_style.showExp || m_style.battle) && pet.reborn != 0) {
            draw("Okey3", at + (m_style.battle ? kBattleRebornMark : kRebornMark));
        }
        const int frame = m_animation.frame(nowMs, m_services.characters.petFrameCount(pet.info, kFigureSlot));
        m_services.characters.drawPet(canvas, pet.info, at + m_style.preview + kFigureFeet, kFigureSlot, frame);

        draw("Word_Lv_1", part(at, kLevelWord));
        drawNumber(canvas, pet.level, part(at, kLevelDigits));
        draw("Word_Hp_1", part(at, kHpWord));
        draw("Pet_Frame_Hp", part(at, kHpFrame));
        drawBar(canvas, "Pet_Bmp_Hp", part(at, kHpBar), barRatio(pet.hp, pet.maxHp));
        draw("Word_Sp_1", part(at, kSpWord));
        draw("Pet_Frame_Sp", part(at, kSpFrame));
        drawBar(canvas, "Pet_Bmp_Sp", part(at, kSpBar), barRatio(pet.sp, pet.maxSp));
        if (!m_style.showExp) {
            return;
        }
        const PetExp exp = petExp(m_services.formula, pet);
        draw("Pet_Frame_Exp", part(at, kExpFrame));
        drawBar(canvas, "Pet_Bmp_Exp", part(at, kExpBar), barRatio(static_cast<double>(exp.current), static_cast<double>(exp.next)));
        const int percent =
            exp.next > 0 ? static_cast<int>(static_cast<double>(exp.current) / static_cast<double>(exp.next) * 100.0) : 0; // fraction to percent
        if (percent < 0) {
            return;
        }
        const core::Point digits = part(at, kExpPercent);
        drawNumber(canvas, percent, digits);
        const int length = static_cast<int>(std::to_string(percent).size());
        draw("Pet_Percent_Exp", {digits.x + length * kDigitStep, digits.y + 1});
    }

    void PetInfoPainter::paintExpHint(gfx::Canvas& canvas, core::Point at, const Pet& pet, core::Point mouse) const {
        const core::Rect area{at.x + kExpPercent.x + 3, at.y + kExpPercent.y, 18, 4}; // hit area of the experience bar: 3 pixels right of the percent, 18 x 4
        if (!area.contains(mouse)) {
            return;
        }
        const PetExp exp = petExp(m_services.formula, pet);
        if (exp.next <= 0 ||
            static_cast<double>(exp.current) / static_cast<double>(exp.next) <= -0.01) { // the game's ratio threshold for a negative experience
            return;
        }
        const std::string text = "Exp: " + std::to_string(exp.current) + "/" + std::to_string(exp.next);
        const core::Rect box{
            mouse.x - 1,
            mouse.y - 23,
            static_cast<int>(text.size()) * 8 + 7,
            17
        }; // hint box: 1 left and 23 above the mouse, 8 pixels per glyph plus 7, 17 high
        canvas.fillRectAlpha(box, {61, 138, 249, 200}); // hint fill colour (RGBA)
        canvas.drawRectOutline(box, {0, 0, 128, 255}); // hint outline colour (RGBA)
        canvas.drawText(
            {box.x + 3, box.y},
            text,
            {{255, 255, 255, 255}, gfx::TextEffect::Shadow, std::nullopt},
            400,
            16
        ); // text 3 pixels in, white; box 400 x 16
    }

}
