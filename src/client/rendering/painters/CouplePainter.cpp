#include "client/rendering/painters/CouplePainter.h"

namespace client {

    namespace {

        constexpr core::Point kApplicantFace{82, 130}; // px: applicant portrait position in the form
        constexpr core::Point kPartnerFace{213, 130}; // px: partner portrait position in the form
        constexpr int kApplicantNameX = 80; // px: x of the applicant name centre
        constexpr int kPartnerNameX = 210; // px: x of the partner name centre
        constexpr int kNameY = 185; // px: y of both names
        constexpr int kFaceSlot = 2; // portrait slot passed to drawPortrait
        constexpr int kGlyphWidth = 8; // px: width of one text character
        const core::Color kTextColor = core::Color::fromRgb565(2113); // RGB565 0x0841: near black

        /// @brief Draws one person's large face and name.
        /// @param canvas Target canvas.
        /// @param services Game services (character renderer).
        /// @param at Top left corner of the form.
        /// @param face Offset of the face in the form.
        /// @param nameX X offset of the name's centre in the form.
        /// @param mate The person.
        void paintPerson(
            gfx::Canvas& canvas,
            GameServices& services,
            core::Point at,
            core::Point face,
            int nameX,
            const TeamMate& mate
        ) {
            services.characters.drawPortrait(canvas, mate.look, {at.x + face.x, at.y + face.y}, 0, kFaceSlot);
            const int length = static_cast<int>(mate.name.size());
            canvas.drawText(
                {at.x + nameX - (length - 1) * kGlyphWidth / 2, at.y + kNameY},
                mate.name,
                {kTextColor, gfx::TextEffect::Plain, std::nullopt},
                length * kGlyphWidth
            );
        }

    }

    void paintCouple(
        gfx::Canvas& canvas,
        GameServices& services,
        core::Point at,
        const std::optional<TeamMate>& applicant,
        const std::optional<TeamMate>& partner
    ) {
        if (applicant) {
            paintPerson(canvas, services, at, kApplicantFace, kApplicantNameX, *applicant);
        }
        if (partner) {
            paintPerson(canvas, services, at, kPartnerFace, kPartnerNameX, *partner);
        }
    }

}
