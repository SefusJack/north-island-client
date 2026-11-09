#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_COUPLE_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_COUPLE_PAINTER_H

#include "client/services/GameServices.h"
#include "client/models/social/TeamMate.h"
#include "core/Point.h"
#include "gfx/Canvas.h"

#include <optional>

namespace client {

    /// @brief Draws the two players' large faces side by side with their names under them.
    /// @details Original: DrawControl of the marriage and divorce forms.
    /// @param canvas Target canvas.
    /// @param services Game services (character renderer).
    /// @param at Top left corner of the form.
    /// @param applicant The applicant, if any.
    /// @param partner The partner, if any.
    void paintCouple(
        gfx::Canvas& canvas,
        GameServices& services,
        core::Point at,
        const std::optional<TeamMate>& applicant,
        const std::optional<TeamMate>& partner
    );

}

#endif
