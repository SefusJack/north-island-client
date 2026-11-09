#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_ACTION_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_SKILL_ACTION_H

#include "client/subsystems/battle/ActionContext.h"
#include "core/Point.h"
#include "formats/MotionRecord.h"
#include "formats/SkillRecord.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>

namespace client {

    struct Fighter;

    /// @brief One fighter's skill action played out: motion, camera, hit, hit motions, numbers, return to the cells.
    /// @details Original: TRE_Skill. Stages: 1 until the hit, 2 the hit motions, 3 back to the cells, 4 over.
    class SkillAction {
        public:
            /// @brief Starts the action: sets up the caster, marks who is hit and prepares their hit motions.
            /// @details Original: TRE_Skill.Create.
            /// @param context Battle action context.
            /// @param attackerSlot Fighter slot of the attacker.
            /// @param targetSlot Fighter slot of the target.
            /// @param skillId Skill (and motion) id played.
            SkillAction(
                ActionContext& context,
                int attackerSlot,
                int targetSlot,
                int skillId
            );

            /// @brief Puts both fighters back at their cells (down if their HP is gone).
            /// @details Original: TRE_Skill.Destroy / RestoreFighterPose.
            /// @param context Battle action context.
            void finish(ActionContext& context);

            /// @brief Whether the action is over.
            /// @return True once done.
            bool done() const { return m_done; }

            /// @brief Whether the action's effects are still running.
            /// @return True while effects are active.
            bool effectsActive() const { return m_effectsActive; }

            /// @brief The skill played.
            /// @return Skill id.
            int skillId() const { return m_skillId; }

            /// @brief The current stage (1 until the hit, 2 hit motions, 3 back to the cells, 4 over).
            /// @return Stage number.
            int stage() const { return m_stage; }

            /// @brief Draws the motion's own layers behind or in front of the attacker; a cut-in layer is left to the battle to draw.
            /// @details Original: DrawSkillEffectsBack / DrawSkillEffectsFront.
            /// @param canvas Target canvas.
            /// @param attacker The attacking fighter.
            /// @param front True for the layers in front, false for behind.
            /// @param shake Screen shake offset.
            /// @param cutIn Callback that draws a cut-in layer.
            void drawCasterLayers(
                gfx::Canvas& canvas,
                Fighter& attacker,
                bool front,
                core::Point shake,
                const std::function<void(Fighter&, const EffectLayer&)>& cutIn
            ) const;

            /// @brief Draws the hit motion's layers on one fighter that is hit.
            /// @details Original: DrawHitEffectsBack / DrawHitEffectsFront.
            /// @param canvas Target canvas.
            /// @param hit The fighter hit.
            /// @param front True for the layers in front, false for behind.
            /// @param shake Screen shake offset.
            void drawHitLayers(gfx::Canvas& canvas, Fighter& hit, bool front, core::Point shake) const;

            /// @brief Advances the action one tick: camera, caster motion, stages, screen effects, finish check.
            /// @details Original: UpdateSkill.
            /// @param context Battle action context.
            void update(ActionContext& context);

        private:
            /// @brief The attacking fighter.
            /// @param context Battle action context.
            /// @return Reference to the attacker.
            Fighter& attacker(ActionContext& context) const;

            /// @brief The target fighter.
            /// @param context Battle action context.
            /// @return Reference to the target.
            Fighter& target(ActionContext& context) const;

            /// @brief Works out how far the attacker goes and sets up its motion, layers, sounds and camera path.
            /// @details Original: InitSkillMotionOffsets / SetupSkillMotion / ResetSkillState.
            /// @param context Battle action context.
            void setUpCaster(ActionContext& context);

            /// @brief Loads the motion's layer points from the attacker (mirrored on side 1, L/R pictures swapped).
            /// @details Original: SetupCasterEffectPoints.
            /// @param context Battle action context.
            /// @param caster The attacker.
            /// @param move How far the attacker moves.
            void loadCasterLayers(ActionContext& context, Fighter& caster, core::Point move);

            /// @brief Loads the motion's sounds and the body sounds of the attacker.
            /// @details Original: LoadCasterSounds.
            /// @param context Battle action context.
            /// @param caster The attacker.
            void loadCasterSounds(ActionContext& context, Fighter& caster);

            /// @brief Marks the fighters the action reaches by the attacker's attack area.
            /// @details Original: MarkSkillHitTargets (FAttackArea).
            /// @param context Battle action context.
            void markTargets(ActionContext& context);

            /// @brief Gives each fighter reached its share of the action and the motion it plays.
            /// @details Original: SetupHitMotions.
            /// @param context Battle action context.
            void setUpHitMotions(ActionContext& context);

            /// @brief Loads a hit motion onto a fighter that is hit.
            /// @details Original: PrepareSkillMotion (SetupBattleMotion, SetupSkillEffects, LoadTargetSounds).
            /// @param context Battle action context.
            /// @param slot Fighter slot hit.
            /// @param motionId Motion played on it.
            void prepareHitMotion(ActionContext& context, int slot, int motionId);

            /// @brief Loads a motion onto a fighter: its motion, layers and sounds (into the hit side or the skill side).
            /// @details Original: SetupBattleMotion / SetupSkillEffects / LoadTargetSounds, or SetupTargetMotion / SetupTargetEffectPoints / LoadHitSoundNames for the skill side.
            /// @param context Battle action context.
            /// @param fighter Fighter that receives the motion.
            /// @param motion Motion record loaded.
            /// @param skillSide True to load into the skill motion, false into the hit motion.
            void loadMotion(ActionContext& context, Fighter& fighter, const formats::MotionRecord& motion, bool skillSide);

            /// @brief Replaces the attacker's own action by motion 60011 or 65090 and takes the told damage off its HP with a number.
            /// @details Original: ProcessSkillHit block for FPendingHit60011 / FPendingHit65090 (packet 50/4).
            /// @param context Battle action context.
            /// @param motionId 60011 or 65090.
            void startSpecialHit(ActionContext& context, int motionId);

            /// @brief Takes the queued effect for the hit fighter's cell and applies it (guard pose, miss, cure, values, death).
            /// @details Original: ApplyQueuedEffectAtCell / ApplyEffectToFighter.
            /// @param context Battle action context.
            /// @param hit Fighter hit.
            void applyEffect(ActionContext& context, Fighter& hit);

            /// @brief The motion whose hits give the damage shares: the camera's action's, whoever's action the damage is of.
            /// @details Original: SplitDamageOverHits.
            /// @param context Battle action context.
            /// @return Motion record to split the damage by.
            const formats::MotionRecord& splitMotion(ActionContext& context) const;

            /// @brief The slot of the fighter the camera follows (the first record of a 50/1).
            /// @param context Battle action context.
            /// @return Fighter slot, 0 if none.
            static int leadSlot(ActionContext& context);

            /// @brief Holds those hit in the hit pose until the camera's action has reached its own hit.
            /// @details Original: UpdateNonCameraSkillStages.
            /// @param context Battle action context.
            void holdHitFighters(ActionContext& context);

            /// @brief Sets whether the fighter hit shows its numbers as the action's hits land.
            /// @details Original: SetFighterReactFlag.
            /// @param context Battle action context.
            /// @param slot Fighter slot hit.
            void setReactFlag(ActionContext& context, int slot);

            /// @brief Moves the camera as the record's camera mode says (follow, jumping path, path).
            /// @details Original: UpdateSkillCamera.
            /// @param context Battle action context.
            void updateCamera(ActionContext& context);

            /// @brief Stands the fighters not acting at their cells as the camera moves.
            /// @details Original: ResetFighterPoses.
            /// @param context Battle action context.
            void placeIdleFighters(ActionContext& context);

            /// @brief Steps the attacker's motion through its run, path and return home, and updates its layers and sounds.
            /// @details Original: UpdateSkillMotion. First, with the camera switch on, the camera's attacker of a skill of range kind 1
            /// decides and applies the skill zoom (CalcSkillZoom, ApplySkillZoom).
            /// @param context Battle action context.
            void updateCasterMotion(ActionContext& context);

            /// @brief At the end of the camera caster's path, applies the queued attribute changes (50/3) and raises a number off each fighter.
            /// @param context Battle action context.
            void applyCasts(ActionContext& context);

            /// @brief Steps the attacker on from point to point in the record's poses.
            /// @details Original: UpdateCasterJump.
            /// @param context Battle action context.
            /// @param caster The attacker.
            void stepCasterPoints(ActionContext& context, Fighter& caster);

            /// @brief Runs the stage logic: the hit, the hit motions, the return and the hits' effects.
            /// @details Original: UpdateCameraSkillStages.
            /// @param context Battle action context.
            void updateStages(ActionContext& context);

            /// @brief Lands the hit at the record's step; a miss says so.
            /// @details Original: ProcessSkillHit.
            /// @param context Battle action context.
            void processHit(ActionContext& context);

            /// @brief Plays the hit motion's poses on a fighter hit.
            /// @details Original: UpdateFighterHitMotion.
            /// @param context Battle action context.
            /// @param slot Fighter slot hit.
            void updateHitMotion(ActionContext& context, int slot);

            /// @brief Brings a fighter hit back to its cell, standing.
            /// @details Original: EndFighterHitMotion.
            /// @param context Battle action context.
            /// @param slot Fighter slot hit.
            void endHitMotion(ActionContext& context, int slot);

            /// @brief Shows the hits' numbers and applies their values as the record's hits land.
            /// @details Original: ApplySkillHitEffects.
            /// @param context Battle action context.
            /// @param slot Fighter slot hit.
            void applyHitEffects(ActionContext& context, int slot);

            /// @brief Runs the motion's screen effects: shake (1), fade (2) and ghosts (3), for the attacker's step.
            /// @details Original: UpdateSkillScreenEffects.
            /// @param context Battle action context.
            void updateScreenEffects(ActionContext& context);

            /// @brief Whether a screen effect is on at that step or time.
            /// @details Original: IsInEffectWindow (indexes from 1 as the original's).
            /// @param context Battle action context.
            /// @param index Screen effect index from 1.
            /// @param frame Current step of the attacker's motion.
            /// @return True if the effect's window is open.
            bool inEffectWindow(ActionContext& context, int index, int frame);

            /// @brief Steps the ground fade of a fade effect: darkens to a floor level while the window lasts, then lightens.
            /// @details Original: UpdateSkillFade.
            /// @param context Battle action context.
            /// @param index Screen effect index from 1.
            void updateFade(ActionContext& context, int index);

            /// @brief Gives the screen a new random shake offset.
            /// @details Original: StartScreenShake.
            /// @param context Battle action context.
            /// @param index Screen effect index from 1.
            void startShake(ActionContext& context, int index);

            /// @brief Finishes the action once the attacker is back and nobody hit still moves.
            /// @details Original: CheckSkillFinished.
            /// @param context Battle action context.
            void checkFinished(ActionContext& context);

            /// @brief Ends the action: the attacker stops casting and the action is done.
            /// @details Original: FinishSkill.
            /// @param context Battle action context.
            void finishSkill(ActionContext& context);

            /// @brief Whether a hit or effect is due at a motion step or after a time.
            /// @details Original: IsSkillHitDue / IsSkillEffectDue; a delay of 1..100 counts motion steps, else milliseconds.
            /// @param point Current motion step.
            /// @param delay Step number (1..100) or time in milliseconds.
            /// @param sinceMs Time the wait began.
            /// @param nowMs Current time.
            /// @return True when due.
            bool due(int point, int delay, std::uint64_t sinceMs, std::uint64_t nowMs) const;

            /// @brief Where the fighter is in the record's draw order.
            /// @details Original: FindSkillMotionDrawLayer / FindHitMotionDrawLayer.
            /// @param record Motion record.
            /// @return Draw position from 1.
            static int drawPosition(const formats::MotionRecord& record);

            int m_attacker = 0;
            int m_target = 0;
            int m_skillId = 0;
            std::optional<std::reference_wrapper<const formats::MotionRecord>> m_motion;
            std::optional<std::reference_wrapper<const formats::MotionRecord>> m_hitMotion;
            std::optional<formats::SkillRecord> m_skill;
            int m_attackerSide = 0;
            int m_targetSide = 0;
            bool m_hitApplied = false;
            std::optional<std::reference_wrapper<const formats::MotionRecord>> m_knockedMotion;
            bool m_done = false;
            bool m_effectsActive = false;
            int m_stage = 0;
            int m_returnStage = 0;
            std::array<bool, 17> m_reacts{}; // per battle slot 1..16 (BattleField::kSlots + 1): whether the fighter reacts to the hit
            std::uint64_t m_hitMs = 0;
            std::uint64_t m_effectMs = 0;
            static constexpr int kScreenEffects = 10; // most screen effects (shake, fade, ghosts...) a motion has, 1-based
            std::array<int, kScreenEffects + 1> m_effectStates{};
            std::array<std::uint64_t, kScreenEffects + 1> m_effectTicks{};
            std::array<bool, kScreenEffects + 1> m_effectFlags{};
            bool m_afterImages = false;
            bool m_fadeDown = false;
            bool m_fadeUp = false;
            int m_casterDrawPosition = 1;
            int m_hitDrawPosition = 1;
            int m_attackSpan = 0; // FAttackSpanX: px between the camera's attacker and its target on the screen when the skill began
            std::mt19937 m_random{std::random_device{}()};
    };

}

#endif
