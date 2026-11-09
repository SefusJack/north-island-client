#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_H

#include "client/subsystems/battle/BattleSounds.h"
#include "client/subsystems/battle/EffectLayer.h"
#include "client/subsystems/battle/FighterMotion.h"
#include "client/subsystems/battle/FighterMove.h"
#include "client/subsystems/battle/FighterStatuses.h"
#include "client/subsystems/battle/FloatingTexts.h"
#include "client/subsystems/battle/HitNumbers.h"
#include "client/subsystems/battle/LastCommand.h"
#include "client/subsystems/battle/SkillCutIn.h"
#include "client/subsystems/battle/ThrownItem.h"
#include "client/subsystems/battle/TargetEffect.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "core/Point.h"
#include "formats/NpcInfo.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief One figure on the battle field: a player drawn from their look, or an Npc.dat kind drawn from its map sprite.
    /// @details Original: TFightHum. Sits on a cell of the 4 x 4 grid. Rows 1 and 2 are side 2 on the left, rows 3 and 4
    /// side 1 on the right (GetSideOfRow).
    struct Fighter {
        static constexpr int kLayers = 10; // skill effect layers 1..10 per fighter

        int kind = 0;
        std::int32_t id = 0;
        std::int32_t ownerId = 0;
        int row = 0;
        int col = 0;
        std::int32_t hp = 0;
        std::int32_t maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        int level = 0;
        int element = 0;
        int reborn = 0;
        int job = 0;
        std::string name;
        int weaponType = 0;
        std::optional<CharacterLook> look;
        std::optional<formats::NpcInfo> npc;
        int weaponLook = 0;
        std::int32_t weaponColor = 0;
        bool waiting = false;
        std::optional<std::uint64_t> flyAwayMs;
        int flyRise = 0;
        bool acted = true;
        // default body height in pixels
        int bodyHeight = 80;
        // default sprite height in pixels
        int spriteHeight = 90;
        core::Point position;
        FigureAnimation animation;

        int entering = 0;
        bool dashing = false;
        int dashLength = 0;
        core::Point heading;
        std::uint64_t movedMs = 0;
        int action = 16; // 16: battle stance
        int holdFrame = 255; // 255: no frame held
        int drawnFrame = 0;
        bool jumping = false;
        int jumpHeight = 0;

        bool cameraFollow = false;
        bool casting = false;
        std::array<LastCommand, 3> lastCommands{}; // commander kinds 1 and 2, index 0 unused
        FighterStatuses statuses;
        std::array<FighterMotion, 5> statusMotions{}; // status sets 1..4, index 0 unused
        std::array<std::array<EffectLayer, kLayers + 1>, 5> statusLayers{}; // status sets 1..4, index 0 unused
        bool statusPose = false;
        int attackArea = 0;
        std::optional<TargetEffect::Value> cast;
        bool killed = false;
        bool dead = false;
        bool onDownAction = false;
        std::vector<TargetEffect> effects;
        int cmdSkillId = 0;
        int effectMotionId = 0;
        int hitKind = 0;
        bool pendingHit60011 = false;
        bool pendingHit65090 = false;
        bool specialHitStarted = false;
        std::int32_t pendingHpDamage = 0;
        bool isHit = false;
        bool hitEffectOn = false;
        bool hitAvoided = false;
        int hitMotionId = 0;
        int hitAttacker = 0;
        // 17: slots 1..16, index 0 unused
        std::array<bool, 17> missedBy{}; // by attacker slot 1..16
        std::vector<TargetEffect::Value> pending;
        SkillCutIn cutIn;
        std::vector<ThrownItem> thrown;
        FighterMotion skillMotion;
        FighterMotion hitMotion;
        FighterMove mover;
        std::array<EffectLayer, kLayers + 1> skillLayers{};
        std::array<EffectLayer, kLayers + 1> hitLayers{};
        BattleSounds skillSounds;
        BattleSounds hitSounds;
        HitNumbers numbers;
        FloatingTexts texts;

        /// @brief Returns the side of the field the fighter is on.
        /// @details Original: GetSideOfRow.
        /// @return 2 for rows 1 and 2 (left), 1 for rows 3 and 4 (right).
        int side() const { return row <= 2 ? 2 : 1; } // rows 1..2 are side 2, rows 3..4 side 1

        /// @brief Tells whether the fighter is a player.
        /// @return True when drawn from a character look.
        bool isPlayer() const { return look.has_value(); }

        /// @brief Tells whether the fighter keeps its fall frame through every later fall.
        /// @details Original: SetHumanAction 26 (FCurAction := 26, FAnimFrame := 0) and SetHumanAction 16 / 17 are ignored by a
        /// fighter in the dead or fly-away motion (FAfterImageMode 8, 7) already on action 26.
        /// @return True when dead or flown away and on the down action.
        bool holdsDownAction() const { return (dead || flyAwayMs.has_value()) && onDownAction; }

        /// @brief Starts the fall (action 26), unless the fighter already holds it.
        void fall() {
            if (!holdsDownAction()) {
                animation.restart();
                onDownAction = true;
            }
        }

        /// @brief Ends the fall (back to the stance), unless the fighter holds the down action.
        void stand() {
            if (!holdsDownAction()) {
                onDownAction = false;
            }
        }
    };

}

#endif
