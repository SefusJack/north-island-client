#include "client/subsystems/battle/StatusMotions.h"

#include <cstdlib>

namespace client {

    namespace {

        /// @brief A status's main place, the motion set it plays in and the status whose skill gives the motion.
        /// @details The second status takes the third's motion, as the game has it.
        struct StatusMotionPlace {
            int kind;
            int set;
            int motionFrom;
        };

        // rows are {status place, motion set, place whose skill gives the motion}
        constexpr std::array<StatusMotionPlace, 5> kPlaces{{{2, 1, 3}, {1, 2, 1}, {3, 3, 3}, {8, 3, 8}, {4, 4, 4}}};

        /// @brief Whether the skill's status loops as a motion on the fighter.
        /// @param skills Skill table.
        /// @param skillId Skill asked about.
        /// @return True when its status kind is 1.
        bool loops(const formats::SkillTable& skills, int skillId) {
            const auto skill = skills.find(skillId);
            return skill && skill->statusKind == 1; // status kind 1: loops on the fighter
        }

        /// @brief The motion record of a skill.
        /// @param skills Skill table.
        /// @param motions Motion table.
        /// @param skillId Skill whose motion is wanted.
        /// @return The motion record, or nothing if the skill or motion is unknown.
        std::optional<std::reference_wrapper<const formats::MotionRecord>> motionOf(
            const formats::SkillTable& skills,
            const formats::MotionTable& motions,
            int skillId
        ) {
            const auto skill = skills.find(skillId);
            return skill ? motions.find(skill->motionId) : std::nullopt;
        }

        /// @brief Loads the status motion's layers as looping layers, points from where the fighter stands (mirrored on side 1).
        /// @details Original: SetupStatusLayers.
        /// @param fighter Fighter wearing the status.
        /// @param layers Layer set loaded.
        /// @param record Motion record of the status.
        /// @param camera Camera position.
        void setUpLayers(Fighter& fighter, std::array<EffectLayer, Fighter::kLayers + 1>& layers, const formats::MotionRecord& record, core::Point camera) {
            for (std::size_t i = 0; i < record.layers.size() && i < Fighter::kLayers; ++i) {
                const formats::MotionLayer& layer = record.layers[i];
                std::vector<core::Point> points(1);
                for (const core::Point& point : layer.points) {
                    if (fighter.side() == 1) {
                        points.push_back({fighter.position.x - point.x + camera.x, point.y + fighter.position.y + camera.y});
                    } else {
                        points.push_back({point.x + fighter.position.x + camera.x, point.y + fighter.position.y + camera.y});
                    }
                }
                int direction = layer.direction;
                // directions 1 and 2 are the L / R pictures, swapped on side 1 (3 - direction)
                if (fighter.side() == 1 && (direction == 1 || direction == 2)) {
                    direction = 3 - direction;
                }
                EffectLayer& target = layers[i + 1];
                target.load(layer, std::move(points), direction, camera);
                target.loopAsStatus();
            }
        }

        /// @brief Steps one status motion: runs to the start, then through the record's poses, and begins again at the end.
        /// @details Original: StepStatusMotion.
        /// @param fighter Fighter wearing the status.
        /// @param motion The status motion stepped.
        /// @param context Battle action context.
        void step(Fighter& fighter, FighterMotion& motion, ActionContext& context) {
            motion.advancePath(context.camera, fighter.cameraFollow, fighter.jumpHeight, context.nowMs);
            motion.calcDrawPosition();
            if (motion.speedMode() == 0) { // speed mode 0: run to the start point
                motion.runToStart(fighter, true);
            } else if (motion.speedMode() == 1) { // speed mode 1: step through the record's poses
                fighter.jumping = false;
                motion.advancePoint(context.nowMs);
                if (std::abs(motion.screen().y - motion.home().y) > 0) {
                    fighter.jumping = true;
                    fighter.jumpHeight = std::abs(motion.screen().y - motion.home().y);
                    fighter.position = motion.home();
                } else {
                    fighter.position = motion.screen();
                    fighter.jumping = false;
                    fighter.jumpHeight = 0;
                }
                const bool over = motion.pathDone() && motion.pathEndReached() && motion.pointIndex() > motion.lastPointIndex();
                const int point = over ? motion.pointIndex() - 1 : motion.pointIndex();
                fighter.action = motion.frameSlot(point);
                fighter.holdFrame = motion.frameHold(point);
            }
            if (motion.pathEndReached() && motion.pathDone()) {
                motion.reset(context.nowMs);
            }
        }

    }

    void updateStatusMotions(Fighter& fighter, ActionContext& context) {
        if (fighter.hp <= 0) {
            // a fallen fighter lies still: a looping status left on it would move it and change its pose every beat
            // while the painter puts the death pose back every frame, so the body twitched
            if (fighter.statusPose && !fighter.casting && !fighter.hitEffectOn) {
                fighter.position = BattleField::cellPosition(fighter.row, fighter.col);
                fighter.jumping = false;
                fighter.jumpHeight = 0;
            }
            fighter.statusPose = false;
            return;
        }
        // status places 1, 2 and 3: the first three statuses hold the pose
        fighter.statusPose = loops(context.skills, fighter.statuses.main[1].skill) || loops(context.skills, fighter.statuses.main[2].skill) ||
                             loops(context.skills, fighter.statuses.main[3].skill);
        if (fighter.casting || fighter.hitEffectOn) {
            return;
        }
        for (const StatusMotionPlace& place : kPlaces) {
            StatusSlot& slot = fighter.statuses.main[static_cast<std::size_t>(place.kind)];
            if (slot.effect == 0 || !loops(context.skills, slot.skill)) {
                continue;
            }
            const auto record = motionOf(context.skills, context.motions, fighter.statuses.main[static_cast<std::size_t>(place.motionFrom)].skill);
            if (!record) {
                continue;
            }
            FighterMotion& motion = fighter.statusMotions[static_cast<std::size_t>(place.set)];
            auto& layers = fighter.statusLayers[static_cast<std::size_t>(place.set)];
            if (!slot.started) {
                motion.loadStatus(*record, fighter.side(), BattleField::cellPosition(fighter.row, fighter.col));
                setUpLayers(fighter, layers, *record, context.camera.position());
                slot.started = true;
                motion.reset(context.nowMs);
                for (EffectLayer& layer : layers) {
                    layer.reset(context.nowMs);
                }
                continue;
            }
            step(fighter, motion, context);
            for (std::size_t i = 1; i <= record->get().layers.size() && i <= Fighter::kLayers; ++i) {
                layers[i].update(context.pictures, motion.pointIndex(), context.camera.position(), context.nowMs);
            }
        }
    }

    void drawStatusMotions(
        gfx::Canvas& canvas,
        Fighter& fighter,
        const formats::SkillTable& skills,
        const formats::MotionTable& motions,
        core::Point shake
    ) {
        if (fighter.casting || fighter.hitEffectOn) {
            return;
        }
        for (const StatusMotionPlace& place : kPlaces) {
            const StatusSlot& slot = fighter.statuses.main[static_cast<std::size_t>(place.kind)];
            if (slot.effect == 0 || !loops(skills, slot.skill)) {
                continue;
            }
            const auto record = motionOf(skills, motions, slot.skill);
            if (!record) {
                continue;
            }
            auto& layers = fighter.statusLayers[static_cast<std::size_t>(place.set)];
            for (std::size_t i = 1; i <= record->get().layers.size() && i <= Fighter::kLayers; ++i) {
                layers[i].draw(canvas, shake);
            }
        }
    }

}
