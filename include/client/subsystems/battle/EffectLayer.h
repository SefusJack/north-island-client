#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_EFFECT_LAYER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_EFFECT_LAYER_H

#include "core/Point.h"
#include "core/Rect.h"
#include "formats/MotionLayer.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief An effect picture a motion plays: a strip of frames that waits for its start, then plays and moves.
    /// @details Original: CH_TLight. Pictures are named "S<id>", "S<id>L" and so on. The layer waits for its start
    /// (a motion step below 101, else milliseconds), plays its frames (once, round and round, held ...) while it stays on
    /// a point or travels from point to point, drawn centred there; pictures marked by their top-left pixel add their light.
    class EffectLayer {
        public:
            /// @brief Resets the layer's timers and point index.
            /// @details Original: ResetEffectLayer.
            /// @param nowMs Current time in milliseconds.
            void reset(std::uint64_t nowMs);

            /// @brief Loads the layer's points on the ground and how it plays.
            /// @details Original: SetupCasterEffectPoints / SetupSkillEffects.
            /// @param layer The motion layer data.
            /// @param points The layer's points (1-based).
            /// @param direction Picture suffix, 0..4.
            /// @param camera Camera position.
            void load(const formats::MotionLayer& layer, std::vector<core::Point> points, int direction, core::Point camera);

            /// @brief Makes a status's layer play over and over (frame mode 1, no repeats left).
            /// @details Original: SetupStatusLayers.
            void loopAsStatus() {
                m_playMode = 1;
                m_repeatCount = 0;
            }

            /// @brief Steps the frames and motion while it plays, or its start while it waits.
            /// @details Original: UpdateEffectLayer. No layer ever takes the camera along: GetCameraFocus, the only one to ask
            /// for it, never gets that far.
            /// @param pictures Picture library to load the strip from.
            /// @param motionPoint Current step of the motion.
            /// @param camera Camera position.
            /// @param nowMs Current time in milliseconds.
            void update(gfx::PictureLibrary& pictures, int motionPoint, core::Point camera, std::uint64_t nowMs);

            /// @brief Draws the layer's current frame.
            /// @details Original: DrawEffectLayer.
            /// @param canvas Target canvas.
            /// @param shake Screen-shake offset.
            void draw(gfx::Canvas& canvas, core::Point shake) const;

            /// @brief Tells whether the layer is playing.
            /// @return True while playing.
            bool playing() const { return m_state == State::Playing; }

            /// @brief Tells whether the layer is playing and put where it plays (the tick it starts on it is still where it was set up).
            /// @return True when placed.
            bool placed() const { return m_placed; }

            /// @brief Returns the top left of where the layer is drawn (BattleTween moves it between two ticks).
            /// @return The top-left point.
            core::Point position() const { return {m_bounds.x, m_bounds.y}; }

            /// @brief Sets the top left of where the layer is drawn.
            /// @param at The new top-left point.
            void setPosition(core::Point at) {
                m_bounds.x = at.x;
                m_bounds.y = at.y;
            }

            /// @brief Tells whether this is the cut-in layer S60000, which is no picture: the battle draws the caster's cut-in for it.
            /// @return True for S60000.
            bool isCutIn() const { return m_name == "S60000"; }

            /// @brief Returns the number of frames of the strip.
            /// @return The frame count.
            int frameCount() const { return m_frameCount; }

            /// @brief Returns the time of one frame.
            /// @return The frame delay in milliseconds.
            int frameDelay() const { return m_frameDelay; }

        private:
            /// @brief Where the layer is in its life.
            enum class State { Waiting, Playing, Done };

            /// @brief Loads the strip picture by the layer's name.
            /// @details Original: LoadEffectPic.
            /// @param pictures Picture library.
            void loadPicture(gfx::PictureLibrary& pictures);

            /// @brief Starts the layer playing.
            /// @details Original: TryStartLayerByFrame / TryStartLayerByDelay.
            /// @param pictures Picture library.
            /// @param nowMs Current time in milliseconds.
            void start(gfx::PictureLibrary& pictures, std::uint64_t nowMs);

            /// @brief Steps the frames by the play mode.
            /// @details Original: UpdateLayerFrames.
            /// @param nowMs Current time in milliseconds.
            void stepFrames(std::uint64_t nowMs);

            /// @brief Counts how many frame times passed since the last frame.
            /// @details Original: CountElapsedFrames.
            /// @param nowMs Current time in milliseconds.
            /// @param frames Receives the number of frames to go on by.
            /// @return True when at least one frame time is due.
            bool framesDue(std::uint64_t nowMs, int& frames);

            /// @brief Sets the source rectangle to the current frame.
            void showFrame();

            /// @brief Moves the layer by its move mode.
            /// @details Original: UpdateLayerMotion.
            /// @param pictures Picture library.
            /// @param nowMs Current time in milliseconds.
            void move(gfx::PictureLibrary& pictures, std::uint64_t nowMs);

            /// @brief Moves an arrow and turns its picture to the way it flies.
            /// @details Original: UpdateEffectPicDirection.
            /// @param pictures Picture library.
            /// @param nowMs Current time in milliseconds.
            void turnStep(gfx::PictureLibrary& pictures, std::uint64_t nowMs);

            /// @brief Starts a move from one point to another.
            /// @details Original: StartLayerMove.
            /// @param from Start point.
            /// @param to End point.
            void startMove(core::Point from, core::Point to);

            /// @brief Steps the move along a straight way.
            /// @details Original: StepLayerMove.
            /// @param nowMs Current time in milliseconds.
            void stepAlone(std::uint64_t nowMs);

            /// @brief Tells whether the layer has reached its move target.
            /// @details Original: IsLayerAtTarget.
            /// @return True at the target.
            bool atTarget() const;

            /// @brief Steps the move along an arc, up and down along the way.
            /// @details Original: MoveLayerWithArc.
            /// @param nowMs Current time in milliseconds.
            void arcStep(std::uint64_t nowMs);

            /// @brief Puts the layer at a world point on the screen.
            /// @details Original: SetLayerScreenPos.
            /// @param x World x.
            /// @param y World y.
            void placeScreen(int x, int y);

            /// @brief Recomputes the drawn bounds from the screen position.
            /// @details Original: UpdateLayerBounds.
            void updateBounds();

            std::string m_name;
            std::optional<gfx::PictureId> m_picture;
            int m_picWidth = 0;
            int m_picHeight = 0;
            bool m_translucent = false;
            bool m_moving = false;
            int m_frameDelay = 0;
            int m_frameCount = 1;
            int m_startFrame = 1;
            int m_endFrame = 1;
            int m_curFrame = 1;
            bool m_reverse = false;
            int m_screenX = 0;
            int m_screenY = 0;
            int m_cameraX = 0;
            int m_cameraY = 0;
            core::Point m_moveFrom;
            core::Point m_moveTo;
            int m_posX = 0;
            int m_posY = 0;
            int m_frameWidth = 0;
            int m_frameHeight = 0;
            int m_playMode = 0;
            int m_moveMode = 0;
            core::Rect m_source;
            core::Rect m_bounds;
            double m_stepPerMs = 0;
            std::uint64_t m_frameMs = 0;
            std::uint64_t m_moveMsX = 0;
            std::uint64_t m_moveMsY = 0;
            std::uint64_t m_delayMs = 0;
            State m_state = State::Waiting;
            bool m_placed = false;
            int m_startDelay = 0;
            int m_moveDistance = 0;
            int m_distX = 0;
            int m_distY = 0;
            int m_repeatCounter = 1;
            int m_repeatCount = 0;
            std::vector<core::Point> m_points;
            int m_pointIndex = 1;
            int m_arcRise = 0;
            float m_arcHeight = 0;
            int m_arcDirection = 0;
            int m_effectId = 0;
            int m_picDirection = 0;
    };

}

#endif
