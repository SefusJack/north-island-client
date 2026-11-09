#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_ACTION_CONTEXT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_ACTION_CONTEXT_H

#include "client/subsystems/battle/BattleCamera.h"
#include "client/subsystems/battle/BattleField.h"
#include "formats/MotionTable.h"
#include "formats/SkillTable.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief What a playing action works with: the field and its camera, the motion and skill tables, the pictures, where the battle sounds are and the time.
    struct ActionContext {
        BattleField& field;
        BattleCamera& camera;
        const formats::MotionTable& motions;
        const formats::SkillTable& skills;
        gfx::PictureLibrary& pictures;
        std::function<std::string(int)> soundFile;
        std::function<void(const std::string&)> notice;
        std::function<void(const std::string&)> chatLine;
        std::int32_t ownId = 0;
        std::uint64_t nowMs = 0;
        std::function<std::uint64_t(int)> sleeper;
        std::function<std::optional<int>(int)> actionStage;
        bool skillZoomOn = false; // the system window's camera switch is on (BattleSwitch 0)
        std::function<void(int span, int position, int targetPosition, int casterY)> skillZoom; // CalcSkillZoom, then ApplySkillZoom when it says so

        /// @brief Sleeps the game's one thread for a time while the clock goes on.
        /// @details Original: Sleep(ms); the time of the rest of the tick is the time after the sleep (GetTickCount is read afresh). Set by the battle via sleeper; none (a test): the tick's time moves on by the sleep.
        /// @param ms The sleep in milliseconds; nothing happens when not above 0.
        void sleep(int ms) {
            if (ms > 0) {
                nowMs = sleeper ? sleeper(ms) : nowMs + static_cast<std::uint64_t>(ms);
            }
        }
    };

}

#endif
