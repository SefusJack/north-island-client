#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_PET_INFO_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_PET_INFO_PAINTER_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/pets/Pet.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"

#include <cstdint>

namespace client {

    /// @brief Draws a pet's small picture and bars: its figure, and at the parts' offset its level and HP and SP bars; on the status panel also its experience.
    /// @details Original: TRe_MiniPetInfo.DrawMiniPetInfo. Positions are the control's place on the screen.
    class PetInfoPainter {
        public:
            /// @brief Layout and variant flags of a pet info control (FPartsPos, FPreviewOffset, FInBattle).
            struct Style {
                core::Point parts;
                bool blackDigits = false;
                bool showExp = false;
                core::Point preview;
                bool battle = false;
            };

            /// @brief Creates a painter.
            /// @param services Game services (pictures, characters, formulas).
            /// @param style Layout and variant flags.
            PetInfoPainter(
                GameServices& services,
                Style style
            )
                : m_services(services),
                  m_style(style) {}

            /// @brief Draws the pet's figure, level, HP and SP bars (and experience when the style shows it).
            /// @details Original: DrawMiniPetInfo.
            /// @param canvas Target canvas.
            /// @param at The control's place on the screen.
            /// @param pet The pet.
            /// @param nowMs Current time in milliseconds, drives the figure animation.
            void paint(gfx::Canvas& canvas, core::Point at, const Pet& pet, std::uint64_t nowMs);

            /// @brief Draws only the pet's figure (the potential form's rows).
            /// @details Original: DrawMiniPetInfo with the parts at (0, 0).
            /// @param canvas Target canvas.
            /// @param at The control's place on the screen.
            /// @param pet The pet.
            /// @param nowMs Current time in milliseconds, drives the figure animation.
            void paintFigure(gfx::Canvas& canvas, core::Point at, const Pet& pet, std::uint64_t nowMs);

            /// @brief Draws "Exp: a/b" over the mouse while it is on the experience bar.
            /// @details Original: DrawPetExpHint.
            /// @param canvas Target canvas.
            /// @param at The control's place on the screen.
            /// @param pet The pet.
            /// @param mouse Mouse position on the screen.
            void paintExpHint(gfx::Canvas& canvas, core::Point at, const Pet& pet, core::Point mouse) const;

            /// @brief Where the HP bar is (27 x 5 at the frame).
            /// @details Original: UpdateTeamBarRects.
            /// @param at The control's place on the screen.
            /// @return Screen rectangle of the bar.
            core::Rect hpBar(core::Point at) const;

            /// @brief Where the SP bar is (27 x 5 at the frame).
            /// @details Original: UpdateTeamBarRects.
            /// @param at The control's place on the screen.
            /// @return Screen rectangle of the bar.
            core::Rect spBar(core::Point at) const;

        private:
            /// @brief Draws the level digits.
            /// @details Original: SetPetLevelDigits (a column of ten digits, 6 pixels apart, drawn 5 apart).
            /// @param canvas Target canvas.
            /// @param value The number.
            /// @param at Position of the first digit.
            void drawNumber(gfx::Canvas& canvas, int value, core::Point at) const;

            /// @brief Draws the left part of a bar picture.
            /// @param canvas Target canvas.
            /// @param picture Name of the bar picture.
            /// @param at Position of the bar.
            /// @param ratio Fill ratio from 0 to 1.
            void drawBar(gfx::Canvas& canvas, const char* picture, core::Point at, double ratio) const;

            /// @brief Position of a part from the control's place and the parts' offset (FPartsPos).
            /// @param at The control's place on the screen.
            /// @param offset Offset of the part from the parts' origin.
            /// @return Position on the screen.
            core::Point part(core::Point at, core::Point offset) const;

            GameServices& m_services;
            Style m_style;
            FigureAnimation m_animation;
    };

}

#endif
