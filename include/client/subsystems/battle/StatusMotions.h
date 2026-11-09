#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_STATUS_MOTIONS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_STATUS_MOTIONS_H

#include "client/subsystems/battle/ActionContext.h"
#include "client/subsystems/battle/Fighter.h"
#include "gfx/Canvas.h"

namespace client {

    /// @brief Steps the looping status motions of a fighter and marks whether one holds its pose.
    /// @details Original: UpdateHumStatusEffects (status kind 1); set up at the fighter's cell the first time, begun again when they end; also DrawFightHum's check of the first three statuses.
    /// @param fighter Fighter whose statuses are stepped.
    /// @param context Battle action context.
    void updateStatusMotions(Fighter& fighter, ActionContext& context);

    /// @brief Draws the layers of the looping statuses on a fighter.
    /// @details Original: DrawHumStatusEffects.
    /// @param canvas Target canvas.
    /// @param fighter Fighter wearing the statuses.
    /// @param skills Skill table.
    /// @param motions Motion table.
    /// @param shake Screen shake offset.
    void drawStatusMotions(
        gfx::Canvas& canvas,
        Fighter& fighter,
        const formats::SkillTable& skills,
        const formats::MotionTable& motions,
        core::Point shake
    );

}

#endif
