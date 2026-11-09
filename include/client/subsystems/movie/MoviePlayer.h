#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_PLAYER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_PLAYER_H

#include "client/subsystems/movie/MovieFrame.h"
#include "client/subsystems/movie/MovieGroundObject.h"
#include "client/subsystems/movie/MovieHost.h"
#include "core/Point.h"
#include "core/Size.h"
#include "formats/MovieScript.h"
#include "formats/SoundTable.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace client {

    /// @brief One movie script running: a list of steps with a ground, roles, pictures, talks, fades and screen filters.
    /// @details Original: FT_Movie_Object.pas TMovie (UpdateMovie and DrawMovie). A step lasts until its ground, roles and pictures have walked their
    /// path point, its minimum time (the crunode) has passed and its talk box is closed. Times are the ms of the clock given to update and frame.
    class MoviePlayer {
        public:
            /// @brief Creates the player: the ground's view is centred on its first point and every timer starts.
            /// @details Original: the end of LoadMovieScript.
            /// @param script The parsed movie script.
            /// @param sounds Sound table for sound ids.
            /// @param host The game around the movie.
            /// @param groundSize Size of the ground picture (the map's, for a map ground).
            /// @param nowMs Current clock in ms.
            /// @param mapObjects Ground objects of a map ground.
            MoviePlayer(
                formats::MovieScript script,
                const formats::SoundTable& sounds,
                MovieHost& host,
                core::Size groundSize,
                std::uint64_t nowMs,
                std::vector<MovieGroundObject> mapObjects = {}
            );

            /// @brief Tells whether the movie still plays.
            /// @details Original: FPlaying.
            /// @return False once the last step is past.
            bool playing() const { return m_playing; }

            /// @brief Gives the index of the current step.
            /// @return Step index.
            int stepIndex() const { return m_stepIndex; }

            /// @brief Advances the movie to the clock.
            /// @details Original: UpdateMovie.
            /// @param nowMs Current clock in ms.
            void update(std::uint64_t nowMs);

            /// @brief Builds what to paint now (the pictures' frames advance here).
            /// @details Original: DrawMovie. With free frames the ground, roles, pictures and fade are drawn betweenBeats of the way from where the update of
            /// the beat at beatMs found them to where it left them.
            /// @param nowMs Current clock in ms.
            /// @param betweenBeats How far the frame is from the beat towards the next, none for the beat's own pace.
            /// @param beatMs Time of the beat.
            /// @return What to draw, in order.
            MovieFrame frame(std::uint64_t nowMs, std::optional<double> betweenBeats = std::nullopt, std::uint64_t beatMs = 0);

        private:
            /// @brief The ground's view: position, target, path point and scroll mode.
            struct Ground {
                int x = 0;
                int y = 0;
                int targetX = 0;
                int targetY = 0;
                int pathIndex = 0;
                int scrollMode = 4; // scroll mode 4: the usual speed
                std::uint64_t scrollMs = 0;
                core::Point before;
            };

            /// @brief A picture of the movie as it runs: position, target, move mode, strip frame and loops.
            struct Picture {
                int x = 0;
                int y = 0;
                int targetX = 0;
                int targetY = 0;
                int pathIndex = 0;
                int moveMode = 4; // move mode 4: the usual speed
                int frameTick = 1;
                int loops = 0;
                std::uint64_t moveMs = 0;
                std::uint64_t intervalMs = 0;
                core::Point before;
                std::vector<int> soundHandles;
            };

            /// @brief A role as the movie runs it: where it is and goes, what it does, its afterimages.
            /// @details Original: Tft_Role; definition is the script's role it follows (-1: the main role); the path is a copy with room past its end, as the
            /// original's array of 256 points has.
            struct Role {
                int definition = 0;
                int surface = 0;
                int pathIndex = 0;
                int x = 0;
                int y = 0;
                int targetX = 0;
                int targetY = 0;
                int moveSpeed = 4; // move speed 4: the usual speed
                int action = 0;
                int direction = 0;
                int animFrame = 0;
                bool pathDone = false;
                std::uint64_t moveMs = 0;
                std::uint64_t intervalMs = 0;
                std::uint64_t trailMs = 0;
                core::Point before;
                std::array<core::Point, 5> trail{}; // five afterimage places
                std::vector<formats::MovieRoleStep> path;
                std::vector<int> soundHandles;
                int faceFrame = 0;
                bool faceFlip = false;
            };

            /// @brief The state of one talk box of the script: finished or not, and when it was shown.
            struct Talk {
                bool done = false;
                std::uint64_t shownMs = 0;
            };

            /// @brief Tells whether a step lies within a path's steps (start to start + count).
            /// @param step Step index.
            /// @param start First step of the path.
            /// @param count Number of path steps.
            /// @return True when in range.
            static bool inRange(int step, int start, int count) { return step >= start && step <= start + count; }

            /// @brief Gives the signed time between two clock values.
            /// @param nowMs Current clock in ms.
            /// @param sinceMs Earlier clock in ms.
            /// @return Difference in ms.
            static std::int64_t elapsed(std::uint64_t nowMs, std::uint64_t sinceMs) {
                return static_cast<std::int64_t>(nowMs) - static_cast<std::int64_t>(sinceMs);
            }

            /// @brief Gives a ground path point, the zeros the original's 256-point array holds past the end.
            /// @param index Path point index.
            /// @return The point.
            formats::MovieGroundStep groundStep(int index) const;

            /// @brief Gives a picture path point, zeros past the end.
            /// @param picture The picture definition.
            /// @param index Path point index.
            /// @return The point.
            formats::MoviePictureStep pictureStep(const formats::MoviePicture& picture, int index) const;

            /// @brief Gives the effects (fade, filters, music) of a step.
            /// @param step Step index.
            /// @return The effect, empty out of range.
            const formats::MovieStepEffect& effectOf(int step) const;

            /// @brief Gives the minimum time of a step.
            /// @param step Step index.
            /// @return Time in ms, 0 out of range.
            int millisOf(int step) const;

            /// @brief Gives the script role a running role follows.
            /// @param role Running role.
            /// @return The definition (the main role for -1).
            const formats::MovieRole& definitionOf(const Role& role) const;

            /// @brief Gives a role's path point, empty out of range.
            /// @param role Running role.
            /// @param index Path point index.
            /// @return The point.
            const formats::MovieRoleStep& stepOf(const Role& role, int index) const;

            /// @brief Moves a role toward its target and records its afterimage places.
            /// @details Original: Tft_Role.MoveRole with RecordRoleTrail.
            /// @param role Running role.
            /// @param nowMs Current clock in ms.
            void moveRole(Role& role, std::uint64_t nowMs);

            /// @brief Takes a role to its next path point, with its action and sound, at the end of a step.
            /// @details Original: UpdateMovie's end of a step for a role in its steps.
            /// @param role Running role.
            void advanceRole(Role& role);

            /// @brief Tells whether a role's frame delay has passed, and restarts it if so.
            /// @details Original: RoleIntervalElapsed.
            /// @param role Running role.
            /// @param delayMs Delay in ms.
            /// @param nowMs Current clock in ms.
            /// @return True when elapsed.
            static bool roleIntervalElapsed(Role& role, int delayMs, std::uint64_t nowMs);

            /// @brief Works out the direction a role faces.
            /// @details Original: CalcRoleDirection with AdjustRoleDirection.
            /// @param role Running role.
            /// @param fromX Current x.
            /// @param fromY Current y.
            /// @param toX Target x.
            /// @param toY Target y.
            /// @param fixedDir A fixed direction, or 255 to face the way it walks.
            /// @return The direction for the role's action.
            static int calcRoleDirection(
                const Role& role,
                int fromX,
                int fromY,
                int toX,
                int toY,
                int fixedDir
            );

            /// @brief Fills the drawing of a role at a place, as an afterimage or itself.
            /// @details Original: DrawRole.
            /// @param role Running role.
            /// @param at Where its feet are.
            /// @param trail True for an afterimage.
            /// @param trailIndex Afterimage number 1 to 5.
            /// @param pathMode Path mode of the role's point.
            /// @param fadingScreen True while the screen fades.
            /// @param nowMs Current clock in ms.
            /// @param drawn Filled with the drawing.
            /// @return False when nothing is drawn.
            bool drawRole(
                Role& role,
                core::Point at,
                bool trail,
                int trailIndex,
                int pathMode,
                bool fadingScreen,
                std::uint64_t nowMs,
                MovieFrame::Role& drawn
            );

            /// @brief Adds a role's afterimages and then the role itself to the frame.
            /// @details Original: DrawMovie's role entries.
            /// @param role Running role.
            /// @param nowMs Current clock in ms.
            /// @param between How far between the beats the frame is drawn, none for the place itself.
            /// @param frame The frame being built.
            void drawRoleEntry(Role& role, std::uint64_t nowMs, std::optional<double> between, MovieFrame& frame);

            /// @brief Gives the sort key of a draw entry (feet plus sort offset).
            /// @details Original: GetDrawSortKey.
            /// @param entry The draw entry.
            /// @return Sort key.
            int sortKeyOf(const MovieFrame::Drawn& entry) const;

            /// @brief Scrolls the ground toward its target and keeps the view on the ground's picture.
            /// @details Original: Tft_Ground.ScrollGround.
            /// @param nowMs Current clock in ms.
            void scrollGround(std::uint64_t nowMs);

            /// @brief Moves a picture toward its target.
            /// @details Original: Tft_Picture.MovePicture.
            /// @param picture The picture.
            /// @param nowMs Current clock in ms.
            void movePicture(Picture& picture, std::uint64_t nowMs);

            /// @brief Tells whether a picture's frame interval has passed, and restarts it if so.
            /// @details Original: PictureIntervalElapsed.
            /// @param definition The picture definition.
            /// @param picture The running picture.
            /// @param nowMs Current clock in ms.
            /// @return True when elapsed.
            bool intervalElapsed(const formats::MoviePicture& definition, Picture& picture, std::uint64_t nowMs) const;

            /// @brief Applies a screen filter of a kind for the step.
            /// @details Original: ApplyMovieScreenFx1 (timer 2) and ApplyMovieScreenFx2 (timer 3).
            /// @param kind Filter kind 0 to 6.
            /// @param timerMs The filter's timer.
            /// @param second True for the second filter.
            /// @param nowMs Current clock in ms.
            void applyFilter(int kind, std::uint64_t& timerMs, bool second, std::uint64_t nowMs);

            /// @brief Switches on the filters a step begins with.
            /// @details Original: UpdateMovie's end of a step.
            /// @param effect The step's effect.
            void beginFilters(const formats::MovieStepEffect& effect);

            /// @brief Moves the fade one frame and holds the move timers meanwhile.
            /// @details Original: UpdateMovieFade.
            /// @param nowMs Current clock in ms.
            void updateFade(std::uint64_t nowMs);

            /// @brief Waits for the step's talk boxes.
            /// @param nowMs Current clock in ms.
            /// @return True when the step may go on.
            bool talkFinished(std::uint64_t nowMs);

            /// @brief Ends a step: the paths take their next point and the next step begins.
            /// @param nowMs Current clock in ms.
            void advance(std::uint64_t nowMs);

            /// @brief Begins the current step's fade, efficacy mode and music.
            void beginStep();

            /// @brief Plays a sound of the sound table.
            /// @param id Sound id.
            void playSoundId(int id);

            /// @brief Plays the sound of a reached path point, looped when the point says so, and stops the previous point's loop.
            /// @param beforeNo Sound id of the previous point.
            /// @param beforeLoop Whether the previous point looped.
            /// @param beforeHandle Loop handle the previous point started.
            /// @param nextNo Sound id of the reached point.
            /// @param nextLoop Whether the reached point loops.
            /// @return Handle of the loop it started, 0 for none.
            int pointSound(
                int beforeNo,
                bool beforeLoop,
                int beforeHandle,
                int nextNo,
                bool nextLoop
            );

            /// @brief Stops the loops the last path points started, at the end of the movie.
            void stopLoops();

            /// @brief Gives each role of the team's looks a random seat and decides whether the roles are drawn as faces.
            /// @details Original: LoadMovieScript; roles 99901 to 99904 take a seat, one that is not the player himself makes it a movie with another
            /// player in it (MovieHasOtherPlayer).
            void seatTeamRoles();

            formats::MovieScript m_script;
            const formats::SoundTable& m_sounds;
            MovieHost& m_host;
            core::Size m_groundSize;
            bool m_filter1On = false;
            bool m_filter2On = false;
            int m_shakeStage = 0;
            int m_shakeLevel = 0;
            core::Point m_viewOffset;
            std::uint64_t m_filter1Ms = 0;
            std::uint64_t m_filter2Ms = 0;
            bool m_groundShown = true;
            bool m_playing = true;
            int m_stepIndex = 0;
            bool m_talkShowing = false;
            bool m_fading = false;
            int m_fadeLevel = 255; // fully visible
            int m_fadeBefore = 255; // fully visible
            std::uint64_t m_updateMs = 0;
            int m_efficacyMode = 0;
            bool m_faces = false;
            std::uint64_t m_crunodeMs = 0;
            Ground m_ground;
            std::vector<Picture> m_pictures;
            std::vector<MovieGroundObject> m_mapObjects;
            int m_mapViewY = 0;
            std::vector<Role> m_roles;
            std::vector<Talk> m_talks;
    };

}

#endif
