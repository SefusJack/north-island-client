#include "client/subsystems/movie/MoviePlayer.h"

#include "core/Between.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game screen width, px
        constexpr int kScreenHeight = 600; // game screen height, px
        constexpr int kFadeStep = 5; // fade level change per step (0 to 255)
        constexpr int kOpaque = 255; // fully faded level (no transparency)
        constexpr int kFadeOutKind = 2; // fade kind 2: fade out (kind 1 is fade in)
        constexpr int kKeepAction = 255; // action value meaning "keep the role's current action"
        constexpr int kAutoDirection = 255; // direction value meaning "face the way of travel"
        constexpr int kStanding = 8; // role action 8: standing
        constexpr int kObjectLift = 999; // sort key shift that puts map objects over or under the roles
        constexpr int kMostFilterKind = 6; // filter kind 6: zoom to the most
        constexpr int kShakeStartLevel = 180; // start interval of the quickening shake, ms
        constexpr int kShakeQuicken = 8; // interval cut per jolt of the quickening shake, ms
        constexpr int kZoomGapMs = 30; // time between zoom steps, ms
        // view offsets in px {x, y}, one per shake stage (steady shake, then the quickening shake)
        constexpr core::Point kShakeOffsets[] = {{5, -5}, {-5, 5}, {5, 2}, {-5, -5}, {0, 0}, {5, 5}, {0, -2}};
        constexpr core::Point kFadingShakeOffsets[] = {{2, -2}, {-2, 2}, {3, 0}, {-2, -2}, {0, 0}, {5, 5}, {0, 0}};
        constexpr int kShakeStages = 7; // stages in a shake cycle (entries of the offset tables)
        constexpr std::size_t kPathRoom = 3; // spare path entries kept per role beyond its path count
        constexpr int kSnapPixels = 160; // per-axis distance in px beyond which a move is drawn as a jump (no interpolation)
        constexpr int kTrailCount = 5; // afterimages kept behind a moving role
        constexpr int kTrailGapMs = 100; // time between afterimage snapshots, ms
        // alpha (0 to 255) of an afterimage, indexed by its trail index
        constexpr int kTrailAlphas[] = {255, 200, 160, 120, 80, 40};

        /// @brief Gives the speed of a scroll or move mode.
        /// @param mode Scroll or move mode 1 to 7.
        /// @return Pixels per millisecond; 0.12 for any other mode.
        double speedOf(int mode) {
            switch (mode) {
                case 1: return 0.0008; // mode 1: very slow
                case 2: return 0.008; // mode 2
                case 3: return 0.08; // mode 3
                case 4: return 0.12; // mode 4: the usual speed
                case 5: return 0.4; // mode 5
                case 6: return 4.0; // mode 6
                case 7: return 40.0; // mode 7: 1200 pixels a beat, a cut
                default: return 0.12; // any other mode: the usual speed
            }
        }

        /// @brief Gives the time a frame of a role's walk lasts.
        /// @details Original: GetSpeedDelay; a speed it does not name has none, the usual one stands in.
        /// @param speed Role move speed.
        /// @return Delay in ms.
        int speedDelayOf(int speed) {
            switch (speed) {
                case 1: return 500; // speed 1: 500 ms a frame
                case 2: return 400; // speed 2: 400 ms a frame
                case 3: return 300; // speed 3: 300 ms a frame
                case 5: return 100; // speed 5: 100 ms a frame
                case 6: return 50; // speed 6: 50 ms a frame
                case 7: return 1; // speed 7: 1 ms a frame
                default: return 230; // any other speed (4 too): the usual 230 ms a frame
            }
        }

        /// @brief Gives a place drawn part of the way from where the beat's update found it to where it left it.
        /// @param before Place before the beat's update.
        /// @param now Place after it.
        /// @param between How far between the beats, none without free frames.
        /// @return The place to draw at; now itself without free frames.
        core::Point shownAt(core::Point before, core::Point now, std::optional<double> between) {
            return between ? core::between(before, now, *between, kSnapPixels) : now;
        }

        /// @brief Rounds like Delphi's Round: to the nearest, halves to even.
        /// @param value Value to round.
        /// @return Rounded integer.
        int roundOf(double value) { return static_cast<int>(std::nearbyint(value)); }

        /// @brief One move of the ground or a picture: the pixel step on each axis, or arrival.
        struct Stride {
            int dx = 0;
            int dy = 0;
            bool arrived = false;
        };

        /// @brief Works out one move: the distance of the elapsed time split between the axes by their ratio.
        /// @details Original: the body ScrollGround and MovePicture share.
        /// @param elapsedMs Time since the last move in ms.
        /// @param mode Scroll or move mode.
        /// @param fromX Current x.
        /// @param fromY Current y.
        /// @param toX Target x.
        /// @param toY Target y.
        /// @param equalMoves True for the ground, which moves on an equal place too (<= and >=); pictures do not.
        /// @return The step, or arrived.
        Stride strideOf(
            std::int64_t elapsedMs,
            int mode,
            int fromX,
            int fromY,
            int toX,
            int toY,
            bool equalMoves
        ) {
            Stride stride;
            const int step = roundOf(static_cast<double>(elapsedMs) * speedOf(mode));
            const double distX = std::abs(toX - fromX);
            const double distY = std::abs(toY - fromY);
            if (step > distX && step > distY) {
                stride.arrived = true;
                return stride;
            }
            double ratioYX = 0;
            double ratioXY = 0;
            if (distY > distX) {
                ratioYX = distY / (0.001 + distX); // 0.001 avoids dividing by zero (as the original)
            } else {
                ratioXY = distX / (0.001 + distY); // 0.001 avoids dividing by zero (as the original)
            }
            // 0.001 avoids dividing by zero (as the original)
            double stepX = step / (ratioYX + 0.001);
            double stepY = step / (ratioXY + 0.001);
            if (step < stepX) {
                stepX = step;
            }
            if (step < stepY) {
                stepY = step;
            }
            if (equalMoves ? fromX <= toX : fromX < toX) {
                stride.dx = roundOf(stepX);
            } else if (equalMoves ? fromX >= toX : fromX > toX) {
                stride.dx = -roundOf(stepX);
            }
            if (equalMoves ? fromY <= toY : fromY < toY) {
                stride.dy = roundOf(stepY);
            } else if (equalMoves ? fromY >= toY : fromY > toY) {
                stride.dy = -roundOf(stepY);
            }
            return stride;
        }

        /// @brief Gives the efficacy mode a fade kind sets.
        /// @param fadeKind Fade kind of a step.
        /// @return Efficacy mode: 1 leaves, 2 snow, 3 rain for kinds 3 to 5, the kind itself for 7 to 10, else 0.
        int efficacyModeOf(int fadeKind) {
            switch (fadeKind) {
                case 3: // fade kind 3: leaves (mode 1)
                case 4: // fade kind 4: snow (mode 2)
                case 5: // fade kind 5: rain (mode 3)
                    return fadeKind - 2;
                case 7: // fade kind 7: a colour (own mode)
                case 8: // fade kind 8: a colour (own mode)
                case 9: // fade kind 9: a colour (own mode)
                case 10: // fade kind 10: steam (own mode)
                    return fadeKind;
                default: // other kinds leave the mode
                    return 0;
            }
        }

    }

    MoviePlayer::MoviePlayer(
        formats::MovieScript script,
        const formats::SoundTable& sounds,
        MovieHost& host,
        core::Size groundSize,
        std::uint64_t nowMs,
        std::vector<MovieGroundObject> mapObjects
    )
        : m_script(std::move(script)),
          m_sounds(sounds),
          m_host(host),
          m_groundSize(groundSize),
          m_mapObjects(std::move(mapObjects)) {
        const formats::MovieGroundStep first = groundStep(0);
        m_ground.x = first.targetX - kScreenWidth / 2;
        m_ground.y = first.targetY - kScreenHeight / 2;
        m_ground.targetX = m_ground.x;
        m_ground.targetY = m_ground.y;
        m_ground.scrollMs = nowMs;
        m_ground.before = {m_ground.x, m_ground.y};
        m_crunodeMs = nowMs;
        m_updateMs = nowMs;
        for (const formats::MoviePicture& definition : m_script.pictures) {
            Picture picture;
            const formats::MoviePictureStep start = pictureStep(definition, 0);
            picture.x = start.targetX;
            picture.y = start.targetY;
            picture.targetX = start.targetX;
            picture.targetY = start.targetY;
            picture.before = {picture.x, picture.y};
            picture.loops = definition.loops;
            picture.moveMs = nowMs;
            picture.intervalMs = nowMs;
            picture.soundHandles.assign(static_cast<std::size_t>(definition.pathCount) + kPathRoom + 1, 0);
            m_pictures.push_back(picture);
        }
        const auto startRole = [&](int definition, const formats::MovieRole& role) {
            Role state;
            state.definition = definition;
            state.surface = role.surface;
            state.direction = role.direction;
            state.path = role.path;
            state.path.resize(static_cast<std::size_t>(role.pathCount) + kPathRoom);
            state.soundHandles.assign(state.path.size(), 0);
            // role surfaces 99901 to 99904 and 99909 to 99912 are the player's own looks
            const bool playerLook = (role.surface >= 99901 && role.surface <= 99904) || (role.surface >= 99909 && role.surface <= 99912);
            if (!playerLook) {
                state.path[1].action = state.path[0].action; // the second point takes the first one's action
            }
            state.x = state.targetX = state.path[0].x;
            state.y = state.targetY = state.path[0].y;
            state.before = {state.x, state.y};
            state.moveMs = state.intervalMs = state.trailMs = nowMs;
            m_roles.push_back(std::move(state));
        };
        if (m_script.mainRole) {
            startRole(-1, *m_script.mainRole);
        }
        for (std::size_t i = 0; i < m_script.roles.size(); ++i) {
            startRole(static_cast<int>(i), m_script.roles[i]);
        }
        seatTeamRoles();
        m_talks.assign(m_script.talks.size(), Talk{false, nowMs});
        switch (effectOf(0).fadeKind) {
            case 1: // fade kind 1: fade in, starts black
                m_fadeLevel = 0;
                break;
            default: // other kinds: only set the efficacy mode
                m_efficacyMode = efficacyModeOf(effectOf(0).fadeKind);
                break;
        }
        m_fadeBefore = m_fadeLevel;
    }

    formats::MovieGroundStep MoviePlayer::groundStep(int index) const {
        const auto& path = m_script.ground.path;
        if (index >= 0 && static_cast<std::size_t>(index) < path.size()) {
            return path[static_cast<std::size_t>(index)];
        }
        if (index == 1) { // the second path point
            return formats::MovieGroundStep{kScreenWidth / 2, kScreenHeight / 2, 0};
        }
        return {};
    }

    formats::MoviePictureStep MoviePlayer::pictureStep(const formats::MoviePicture& picture, int index) const {
        if (index >= 0 && static_cast<std::size_t>(index) < picture.path.size()) {
            return picture.path[static_cast<std::size_t>(index)];
        }
        return {};
    }

    const formats::MovieStepEffect& MoviePlayer::effectOf(int step) const {
        static const formats::MovieStepEffect none;
        return step >= 0 && static_cast<std::size_t>(step) < m_script.effects.size() ? m_script.effects[static_cast<std::size_t>(step)] : none;
    }

    int MoviePlayer::millisOf(int step) const {
        return step >= 0 && static_cast<std::size_t>(step) < m_script.stepMillis.size() ? m_script.stepMillis[static_cast<std::size_t>(step)] : 0;
    }

    void MoviePlayer::scrollGround(std::uint64_t nowMs) {
        const std::int64_t gone = elapsed(nowMs, m_ground.scrollMs);
        m_ground.scrollMs = nowMs;
        const Stride stride = strideOf(gone, m_ground.scrollMode, m_ground.x, m_ground.y, m_ground.targetX, m_ground.targetY, true);
        if (stride.arrived) {
            m_ground.x = m_ground.targetX;
            m_ground.y = m_ground.targetY;
            return;
        }
        m_ground.x += stride.dx;
        m_ground.y += stride.dy;
        if (m_ground.x <= 0) {
            m_ground.targetX = 0;
            m_ground.x = 0;
        }
        if (m_groundSize.width - kScreenWidth <= m_ground.x) {
            m_ground.targetX = m_groundSize.width - kScreenWidth;
            m_ground.x = m_groundSize.width - kScreenWidth;
        }
        if (m_ground.y <= 0) {
            m_ground.targetY = 0;
            m_ground.y = 0;
        }
        if (m_groundSize.height - kScreenHeight <= m_ground.y) {
            m_ground.targetY = m_groundSize.height - kScreenHeight;
            m_ground.y = m_groundSize.height - kScreenHeight;
        }
    }

    const formats::MovieRole& MoviePlayer::definitionOf(const Role& role) const {
        return role.definition < 0 ? *m_script.mainRole : m_script.roles[static_cast<std::size_t>(role.definition)];
    }

    const formats::MovieRoleStep& MoviePlayer::stepOf(const Role& role, int index) const {
        static const formats::MovieRoleStep none;
        return index >= 0 && static_cast<std::size_t>(index) < role.path.size() ? role.path[static_cast<std::size_t>(index)] : none;
    }

    void MoviePlayer::moveRole(Role& role, std::uint64_t nowMs) {
        const std::int64_t gone = elapsed(nowMs, role.moveMs);
        if (elapsed(nowMs, role.trailMs) > kTrailGapMs) {
            for (int i = kTrailCount - 1; i > 0; --i) {
                role.trail[static_cast<std::size_t>(i)] = role.trail[static_cast<std::size_t>(i) - 1];
            }
            role.trail[0] = {role.x, role.y};
            role.trailMs = nowMs;
        }
        role.moveMs = nowMs;
        const Stride stride = strideOf(gone, role.moveSpeed, role.x, role.y, role.targetX, role.targetY, false);
        if (stride.arrived) {
            role.x = role.targetX;
            role.y = role.targetY;
            return;
        }
        role.x += stride.dx;
        role.y += stride.dy;
    }

    void MoviePlayer::advanceRole(Role& role) {
        const formats::MovieRole& definition = definitionOf(role);
        ++role.pathIndex;
        const std::size_t index = static_cast<std::size_t>(role.pathIndex);
        if (role.pathIndex >= definition.pathCount) {
            role.pathDone = true;
            role.targetX = role.x;
            role.targetY = role.y;
            if (index < role.path.size()) {
                role.path[index].frame = stepOf(role, role.pathIndex - 1).frame;
            }
            return;
        }
        const formats::MovieRoleStep next = stepOf(role, role.pathIndex);
        const formats::MovieRoleStep before = stepOf(role, role.pathIndex - 1);
        role.targetX = next.x;
        role.targetY = next.y;
        role.moveSpeed = next.moveSpeed;
        role.pathDone = false;
        role.action = next.action != kKeepAction ? next.action : before.action;
        role.path[index].action = role.action;
        const int beforeHandle = role.soundHandles[index - 1];
        role.soundHandles[index] = pointSound(before.soundNo, before.soundLoop, beforeHandle, next.soundNo, next.soundLoop);
    }

    bool MoviePlayer::roleIntervalElapsed(Role& role, int delayMs, std::uint64_t nowMs) {
        if (delayMs < elapsed(nowMs, role.intervalMs)) {
            role.intervalMs = nowMs;
            return true;
        }
        return false;
    }

    int MoviePlayer::calcRoleDirection(
        const Role& role,
        int fromX,
        int fromY,
        int toX,
        int toY,
        int fixedDir
    ) {
        int direction = fixedDir;
        if (fixedDir == kAutoDirection) {
            const double slope = std::abs(toY - fromY) / (std::abs(toX - fromX) + 0.001);
            // a slope between 0.25 and 2 counts as diagonal; 0.001 avoids dividing by zero
            const bool diagonal = slope > 0.25 && slope < 2;
            // directions: 0 north, 1 north-west, 2 west, 3 south-west, 4 south, 5 south-east, 6 east, 7 north-east
            if (toX < fromX && toY > fromY && diagonal) {
                direction = 3;
            } else if (toX < fromX && toY < fromY && diagonal) {
                direction = 1;
            } else if (toX > fromX && toY > fromY && diagonal) {
                direction = 5;
            } else if (toX > fromX && toY < fromY && diagonal) {
                direction = 7;
            } else if (toX > fromX && slope <= 0.25) {
                direction = 6;
            } else if (toX < fromX && slope <= 0.25) {
                direction = 2;
            } else if (toY > fromY) {
                direction = 4;
            } else if (toY < fromY) {
                direction = 0;
            } else {
                direction = role.direction;
            }
        }
        // the actions below face two ways only: direction 0 for a northern direction (0 to 3), 1 for the rest
        switch (role.action) {
            case 16: // action 16 (standing): two-way
            case 26: // action 26: two-way
            case 28: // action 28: two-way
            case 30: // action 30: two-way
            case 32: // action 32: two-way
            case 34: // action 34: two-way
            case 36: // action 36: two-way
            case 38: // action 38: two-way
            case 40: // action 40: two-way
            case 42: // action 42: two-way
            case 44: // action 44: two-way
            case 62: // action 62: two-way
            case 64: // action 64: two-way
                return direction >= 0 && direction <= 3 ? 0 : 1;
            default: // any other action: eight directions, low byte of the direction
                return direction & 0xFF; // low byte mask
        }
    }

    void MoviePlayer::movePicture(Picture& picture, std::uint64_t nowMs) {
        const std::int64_t gone = elapsed(nowMs, picture.moveMs);
        picture.moveMs = nowMs;
        const Stride stride = strideOf(gone, picture.moveMode, picture.x, picture.y, picture.targetX, picture.targetY, false);
        if (stride.arrived) {
            picture.x = picture.targetX;
            picture.y = picture.targetY;
            return;
        }
        picture.x += stride.dx;
        picture.y += stride.dy;
    }

    bool MoviePlayer::intervalElapsed(const formats::MoviePicture& definition, Picture& picture, std::uint64_t nowMs) const {
        if (definition.interval < elapsed(nowMs, picture.intervalMs)) {
            picture.intervalMs = nowMs;
            return true;
        }
        return false;
    }

    void MoviePlayer::applyFilter(int kind, std::uint64_t& timerMs, bool second, std::uint64_t nowMs) {
        const auto elapsedBy = [&](int interval) {
            if (elapsed(nowMs, timerMs) > interval) {
                timerMs = nowMs;
                return true;
            }
            return false;
        };
        switch (kind) {
            case 0: // filter 0: none, the view at rest
                m_viewOffset = {0, 0};
                break;
            case 1: // filter 1: shake, a jolt a frame
                m_viewOffset = kShakeOffsets[m_shakeStage];
                m_shakeStage = (m_shakeStage + 1) % kShakeStages;
                break;
            case 2: // filter 2: shake that quickens
                if (elapsedBy(m_shakeLevel)) {
                    m_shakeLevel = std::max(m_shakeLevel - kShakeQuicken, 0);
                    m_viewOffset = kFadingShakeOffsets[m_shakeStage];
                    m_shakeStage = (m_shakeStage + 1) % kShakeStages;
                }
                break;
            case 3: // filter 3: leave the ground out
                m_groundShown = false;
                break;
            case 4: // filter 4: zoom in every 30 ms
                if (elapsedBy(kZoomGapMs)) {
                    m_host.zoomIn();
                }
                break;
            case 5: // filter 5: zoom out every 30 ms
                if (elapsedBy(kZoomGapMs)) {
                    m_host.zoomOut();
                }
                break;
            case 6: // filter 6: zoom to the most
                if (second) {
                    m_filter1On = true;
                } else {
                    m_host.zoomMost();
                }
                break;
            default: // other kinds: nothing
                break;
        }
    }

    void MoviePlayer::beginFilters(const formats::MovieStepEffect& effect) {
        const auto begin = [&](int kind, bool& on) {
            switch (kind) {
                case 0: // filter 0: switch the filter off
                    m_viewOffset = {0, 0};
                    on = false;
                    break;
                case 1: // filter 1: shake on
                    m_viewOffset = {0, 0};
                    on = true;
                    break;
                case 2: // filter 2: quickening shake on, from the start interval
                    m_viewOffset = {0, 0};
                    on = true;
                    m_shakeLevel = kShakeStartLevel;
                    break;
                case 3: // filter 3: ground left out
                case 4: // filter 4: zoom in
                case 5: // filter 5: zoom out
                    on = true;
                    break;
                default: // other kinds: nothing
                    break;
            }
        };
        begin(effect.filter1, m_filter1On);
        begin(effect.filter2, m_filter2On);
        if (effect.filter1 == kMostFilterKind || effect.filter2 == kMostFilterKind) {
            m_filter1On = true;
        }
    }

    void MoviePlayer::updateFade(std::uint64_t nowMs) {
        const int kind = effectOf(m_stepIndex).fadeKind;
        if (kind != 1 && kind != kFadeOutKind) { // fade kind 1: fade in
            return;
        }
        if (kind == 1) { // fade kind 1: fade in
            if (m_fadeLevel == kOpaque) {
                m_fading = false;
            }
            m_fadeLevel = std::min(m_fadeLevel + kFadeStep, kOpaque);
        } else {
            if (m_fadeLevel == 0) {
                m_fading = false;
            }
            m_fadeLevel = std::max(m_fadeLevel - kFadeStep, 0);
        }
        m_crunodeMs = nowMs;
        m_ground.scrollMs = nowMs;
        for (Picture& picture : m_pictures) {
            picture.moveMs = nowMs;
        }
    }

    bool MoviePlayer::talkFinished(std::uint64_t nowMs) {
        for (std::size_t i = 1; i < m_script.talks.size(); ++i) {
            const formats::MovieTalk& talk = m_script.talks[i];
            if (talk.step != m_stepIndex) {
                continue;
            }
            if (elapsed(nowMs, m_talks[i].shownMs) <= talk.showTime) {
                for (std::size_t k = 0; k < m_pictures.size(); ++k) {
                    const formats::MoviePicture& definition = m_script.pictures[k];
                    if (intervalElapsed(definition, m_pictures[k], nowMs)) {
                        ++m_pictures[k].frameTick;
                    }
                    if (definition.frameRows * definition.frameCols < m_pictures[k].frameTick) {
                        m_pictures[k].frameTick = 1;
                    }
                }
                return false;
            }
            m_talks[i].shownMs = nowMs;
            if (m_host.talkOpen()) {
                return false;
            }
            m_talkShowing = false;
            m_talks[i].done = true;
        }
        return true;
    }

    void MoviePlayer::playSoundId(int id) {
        if (const auto file = m_sounds.file(id)) {
            m_host.playSound("sound/" + *file);
        }
    }

    void MoviePlayer::seatTeamRoles() {
        constexpr int kFirstTeamLook = 99901; // role surface 99901: first team member's look
        constexpr int kLastTeamLook = 99904; // role surface 99904: fourth team member's look
        constexpr int kSeats = 4;
        // start values beyond any screen x, so the first role sets the minimum and maximum
        int minX = 9999;
        int maxX = -9999;
        std::array<bool, kSeats + 1> taken{};
        std::vector<std::reference_wrapper<Role>> team;
        for (Role& role : m_roles) {
            if (role.surface < kFirstTeamLook || role.surface > kLastTeamLook) {
                continue;
            }
            const int x = role.path[0].x;
            maxX = std::max(maxX, x);
            minX = std::min(minX, x);
            role.faceFrame = m_host.random(3); // face frame 0 to 2
            int seat = 0;
            do {
                seat = m_host.random(kSeats) + 1;
            } while (taken[static_cast<std::size_t>(seat)]);
            taken[static_cast<std::size_t>(seat)] = true;
            role.surface = kFirstTeamLook - 1 + seat;
            m_faces = m_faces || !m_host.ownSeat(role.surface);
            team.push_back(std::ref(role));
        }
        for (Role& role : team) {
            role.faceFlip = !(role.path[0].x > maxX / 2 + minX);
        }
    }

    int MoviePlayer::pointSound(
        int beforeNo,
        bool beforeLoop,
        int beforeHandle,
        int nextNo,
        bool nextLoop
    ) {
        int handle = 0;
        if (nextNo > 0 && (beforeNo != nextNo || !beforeLoop)) {
            if (nextLoop) {
                if (const auto file = m_sounds.file(nextNo)) {
                    handle = m_host.playLoopedSound("sound/" + *file, beforeHandle);
                }
            } else {
                playSoundId(nextNo);
            }
        }
        if (beforeLoop && beforeNo != nextNo && beforeHandle != 0) {
            m_host.stopLoopedSound(beforeHandle);
        }
        return handle;
    }

    void MoviePlayer::stopLoops() {
        const auto stop = [this](int handle) {
            if (handle != 0) {
                m_host.stopLoopedSound(handle);
            }
        };
        for (const Role& role : m_roles) {
            stop(role.soundHandles[static_cast<std::size_t>(definitionOf(role).pathCount)]);
        }
        for (std::size_t i = 0; i < m_pictures.size(); ++i) {
            stop(m_pictures[i].soundHandles[static_cast<std::size_t>(m_script.pictures[i].pathCount)]);
        }
    }

    void MoviePlayer::update(std::uint64_t nowMs) {
        if (!m_playing) {
            return;
        }
        m_updateMs = nowMs;
        m_ground.before = {m_ground.x, m_ground.y};
        for (Role& role : m_roles) {
            role.before = {role.x, role.y};
        }
        for (Picture& picture : m_pictures) {
            picture.before = {picture.x, picture.y};
        }
        m_fadeBefore = m_fadeLevel;
        m_groundShown = true;
        if (m_stepIndex > m_script.stepCount) {
            m_playing = false;
            stopLoops();
            return;
        }
        if (m_fading) {
            updateFade(nowMs);
            if (m_fading) {
                return;
            }
        }
        if (m_filter1On) {
            applyFilter(effectOf(m_stepIndex).filter1, m_filter1Ms, false, nowMs);
        }
        if (m_filter2On) {
            applyFilter(effectOf(m_stepIndex).filter2, m_filter2Ms, true, nowMs);
        }
        if (m_talkShowing && !talkFinished(nowMs)) {
            return;
        }
        const formats::MovieGround& ground = m_script.ground;
        int active = 1;
        int done = 0;
        const bool groundActive = inRange(m_stepIndex, ground.pathStart, ground.pathCount);
        if (groundActive) {
            ++active;
            if (ground.pathStart == m_stepIndex) {
                m_ground.targetX = groundStep(1).targetX - kScreenWidth / 2;
                m_ground.targetY = groundStep(1).targetY - kScreenHeight / 2;
            }
        }
        for (const Role& role : m_roles) {
            if (inRange(m_stepIndex, definitionOf(role).pathStart, definitionOf(role).pathCount)) {
                ++active;
            }
        }
        for (const formats::MoviePicture& picture : m_script.pictures) {
            if (inRange(m_stepIndex, picture.pathStart, picture.pathCount)) {
                ++active;
            }
        }
        if (groundActive) {
            scrollGround(nowMs);
            if (m_ground.x == m_ground.targetX && m_ground.y == m_ground.targetY && m_ground.pathIndex <= ground.pathCount) {
                ++done;
            }
        }
        for (Role& role : m_roles) {
            const formats::MovieRole& definition = definitionOf(role);
            if (!inRange(m_stepIndex, definition.pathStart, definition.pathCount)) {
                continue;
            }
            moveRole(role, nowMs);
            if (role.x == role.targetX && role.y == role.targetY && role.pathIndex <= definition.pathCount) {
                role.pathDone = true;
                if (role.action == 0) {
                    role.action = kStanding;
                }
                ++done;
            }
        }
        for (std::size_t i = 0; i < m_pictures.size(); ++i) {
            const formats::MoviePicture& definition = m_script.pictures[i];
            Picture& picture = m_pictures[i];
            if (!inRange(m_stepIndex, definition.pathStart, definition.pathCount)) {
                continue;
            }
            movePicture(picture, nowMs);
            if (picture.x == picture.targetX && picture.y == picture.targetY && picture.pathIndex <= definition.pathCount) {
                ++done;
            }
        }
        if (elapsed(nowMs, m_crunodeMs) > millisOf(m_stepIndex)) {
            ++done;
        }
        if (active != done) {
            return;
        }
        for (std::size_t i = 1; i < m_script.talks.size(); ++i) {
            if (m_script.talks[i].step != m_stepIndex || m_talkShowing || m_talks[i].done) {
                continue;
            }
            m_host.say(m_script.talks[i]);
            m_talks[i].shownMs = nowMs;
            m_talkShowing = true;
            return;
        }
        advance(nowMs);
    }

    void MoviePlayer::advance(std::uint64_t nowMs) {
        const formats::MovieGround& ground = m_script.ground;
        if (inRange(m_stepIndex, ground.pathStart, ground.pathCount)) {
            ++m_ground.pathIndex;
            const formats::MovieGroundStep next = groundStep(m_ground.pathIndex);
            m_ground.targetX = next.targetX - kScreenWidth / 2;
            m_ground.targetY = next.targetY - kScreenHeight / 2;
            m_ground.scrollMode = next.scrollMode;
        }
        for (Role& role : m_roles) {
            if (inRange(m_stepIndex, definitionOf(role).pathStart, definitionOf(role).pathCount)) {
                advanceRole(role);
            }
        }
        for (std::size_t i = 0; i < m_pictures.size(); ++i) {
            const formats::MoviePicture& definition = m_script.pictures[i];
            Picture& picture = m_pictures[i];
            if (!inRange(m_stepIndex, definition.pathStart, definition.pathCount)) {
                continue;
            }
            ++picture.pathIndex;
            const formats::MoviePictureStep next = pictureStep(definition, picture.pathIndex);
            const formats::MoviePictureStep before = pictureStep(definition, picture.pathIndex - 1);
            picture.targetX = next.targetX;
            picture.targetY = next.targetY;
            picture.moveMode = next.moveMode;
            const std::size_t index = static_cast<std::size_t>(picture.pathIndex);
            picture.soundHandles[index] = pointSound(before.soundNo, before.soundLoop, picture.soundHandles[index - 1], next.soundNo, next.soundLoop);
        }
        ++m_stepIndex;
        if (m_stepIndex <= m_script.stepCount) {
            beginStep();
        }
        m_crunodeMs = nowMs;
        m_filter1Ms = nowMs;
        m_filter2Ms = nowMs;
    }

    void MoviePlayer::beginStep() {
        const formats::MovieStepEffect& effect = effectOf(m_stepIndex);
        beginFilters(effect);
        switch (effect.fadeKind) {
            case 0: // fade kind 0: none, fully visible
                m_efficacyMode = 0;
                m_fadeLevel = kOpaque;
                m_fading = false;
                break;
            case 1: // fade kind 1: fade in, starts black
                m_fadeLevel = 0;
                break;
            case kFadeOutKind: // fade kind 2: fade out, starts opaque
                m_fadeLevel = kOpaque;
                m_fading = true;
                break;
            case 6: // fade kind 6: stop fading
                m_fading = false;
                break;
            case 3: // fade kind 3: leaves
            case 4: // fade kind 4: snow
            case 5: // fade kind 5: rain
            case 7: // fade kind 7: colour effect
            case 8: // fade kind 8: colour effect
            case 9: // fade kind 9: colour effect
            case 10: // fade kind 10: steam
                m_efficacyMode = efficacyModeOf(effect.fadeKind);
                m_fading = false;
                break;
            default: // other kinds: nothing
                break;
        }
        m_fadeBefore = m_fadeLevel;
        if (m_stepIndex == 1 && effectOf(0).musicId == 0 && effectOf(1).musicId == 0) { // steps 0 and 1 name no music
            m_host.playSceneMusic();
        }
        if (effect.musicId > 0) {
            if (const auto file = m_sounds.file(effect.musicId)) {
                m_host.playMusic("sound/" + *file);
            }
        }
    }

    int MoviePlayer::sortKeyOf(const MovieFrame::Drawn& entry) const {
        if (entry.object) {
            const MovieGroundObject& object = m_mapObjects[entry.index];
            // object kinds 4 and 5 are put over the roles, 1 and 2 under them
            const int lift = object.kind == 4 || object.kind == 5 ? kObjectLift : object.kind == 1 || object.kind == 2 ? -kObjectLift : 0;
            return m_mapViewY + object.depth + lift;
        }
        if (entry.role) {
            const Role& role = m_roles[entry.index];
            return role.y + (role.definition < 0 ? 0 : definitionOf(role).sortOffsetY);
        }
        return m_pictures[entry.index].y + m_script.pictures[entry.index].sortOffsetY;
    }

    bool MoviePlayer::drawRole(
        Role& role,
        core::Point at,
        bool trail,
        int trailIndex,
        int pathMode,
        bool fadingScreen,
        std::uint64_t nowMs,
        MovieFrame::Role& drawn
    ) {
        if (role.surface <= 0) {
            return false;
        }
        drawn = MovieFrame::Role{};
        drawn.surface = role.surface;
        drawn.foot = at;
        drawn.trail = trail;
        if (trail) {
            drawn.alpha = kTrailAlphas[trailIndex];
            // afterimage draw mode by path mode: 1 blended (2), 2 ghost (3), 3 next ghost (4), else plain (1)
            drawn.drawMode = pathMode == 1 ? 2 : pathMode == 2 ? 3 : pathMode == 3 ? 4 : 1;
        }
        drawn.flipped = pathMode == 4; // path mode 4: upside down
        // scale over 10: path mode 5 is 1.5 times, path mode 6 half, else normal size
        drawn.scale = pathMode == 5 ? 15 : pathMode == 6 ? 5 : 10;
        drawn.shadow = !trail && !fadingScreen && !m_faces;
        drawn.face = m_faces;
        drawn.faceFrame = role.faceFrame;
        drawn.faceFlip = role.faceFlip;
        drawn.action = role.action;
        if (!role.pathDone) {
            role.direction = calcRoleDirection(role, at.x, at.y, role.targetX, role.targetY, stepOf(role, role.pathIndex).fixedDir);
        }
        drawn.slot = role.direction + role.action;
        const int count = m_host.roleFrames(role.surface, drawn.slot);
        if (count <= 0) {
            drawn.slot = -1;
            return drawn.shadow;
        }
        const int held = stepOf(role, role.pathIndex).frame;
        if (held == 0) {
            if (roleIntervalElapsed(role, speedDelayOf(role.moveSpeed), nowMs)) {
                ++role.animFrame;
            }
            if (role.animFrame >= count) {
                role.animFrame = 0;
            }
        } else if (held > count) {
            role.animFrame = count - 1;
        } else {
            role.animFrame = held - 1;
        }
        drawn.frame = role.animFrame;
        return true;
    }

    void MoviePlayer::drawRoleEntry(Role& role, std::uint64_t nowMs, std::optional<double> between, MovieFrame& frame) {
        const formats::MovieRole& definition = definitionOf(role);
        if (definition.pathStart + 1 > m_stepIndex || definition.pathStart + definition.pathCount + 1 <= m_stepIndex) {
            return;
        }
        const int pathMode = stepOf(role, role.pathIndex).pathMode;
        MovieFrame::Role drawn;
        if (pathMode >= 1 && pathMode <= 3) { // path modes 1 to 3: afterimages
            for (int i = kTrailCount; i >= 1; --i) {
                const core::Point& place = role.trail[static_cast<std::size_t>(i) - 1];
                if (place.x != 0 && place.y != 0 && drawRole(role, place, true, i, pathMode, m_fading, nowMs, drawn)) {
                    frame.roles.push_back(drawn);
                    frame.order.push_back({true, false, frame.roles.size() - 1});
                }
            }
        }
        // path modes 4 to 6 (flip, bigger, smaller) apply to the role itself
        if (drawRole(role, {role.x, role.y}, false, 0, pathMode >= 4 && pathMode <= 6 ? pathMode : 0, m_fading, nowMs, drawn)) {
            drawn.foot = shownAt(role.before, {role.x, role.y}, between);
            frame.roles.push_back(drawn);
            frame.order.push_back({true, false, frame.roles.size() - 1});
        }
    }

    MovieFrame MoviePlayer::frame(std::uint64_t nowMs, std::optional<double> betweenBeats, std::uint64_t beatMs) {
        const std::optional<double> between = betweenBeats && m_updateMs >= beatMs ? betweenBeats : std::nullopt;
        MovieFrame frame;
        frame.mapGround = m_script.ground.kind() == formats::MovieGround::Kind::Map;
        frame.groundName = m_script.ground.pictureName();
        const core::Point ground = shownAt(m_ground.before, {m_ground.x, m_ground.y}, between);
        frame.groundOrigin = {ground.x + m_viewOffset.x, ground.y + m_viewOffset.y};
        frame.groundShown = m_groundShown;
        if (m_fading) {
            frame.groundDarkening = kOpaque - (between ? core::between(m_fadeBefore, m_fadeLevel, *between) : m_fadeLevel);
        }
        std::vector<MovieFrame::Drawn> order;
        for (std::size_t i = 0; i < m_roles.size(); ++i) {
            order.push_back({true, false, i});
        }
        for (std::size_t i = 0; i < m_pictures.size(); ++i) {
            order.push_back({false, false, i});
        }
        for (std::size_t i = 0; m_groundShown && i < m_mapObjects.size(); ++i) {
            order.push_back({false, true, i});
        }
        for (std::size_t i = 0; i + 1 < order.size(); ++i) {
            for (std::size_t j = i + 1; j < order.size(); ++j) {
                if (sortKeyOf(order[i]) > sortKeyOf(order[j])) {
                    std::swap(order[i], order[j]);
                }
            }
        }
        for (const MovieFrame::Drawn& entry : order) {
            if (entry.object) {
                frame.order.push_back(entry);
                continue;
            }
            if (entry.role) {
                drawRoleEntry(m_roles[entry.index], nowMs, between, frame);
                continue;
            }
            const std::size_t index = entry.index;
            const formats::MoviePicture& definition = m_script.pictures[index];
            Picture& picture = m_pictures[index];
            if (definition.pathStart + 1 > m_stepIndex || definition.pathStart + definition.pathCount + 1 <= m_stepIndex || picture.loops == 0) {
                continue;
            }
            const int held = pictureStep(definition, picture.pathIndex).frameTick;
            if (held == 0) {
                if (picture.loops > 0) {
                    if (intervalElapsed(definition, picture, nowMs)) {
                        ++picture.frameTick;
                    }
                    if (definition.frameRows * definition.frameCols < picture.frameTick) {
                        picture.frameTick = 1;
                        --picture.loops;
                    }
                }
            } else {
                picture.frameTick = held;
            }
            frame.pictures.push_back(
                {definition.name,
                 shownAt(picture.before, {picture.x, picture.y}, between),
                 picture.frameTick,
                 definition.frameRows,
                 definition.frameCols,
                 definition.alpha,
                 m_fading || definition.alpha != kOpaque}
            );
            frame.order.push_back({false, false, frame.pictures.size() - 1});
        }
        m_mapViewY = m_ground.y;
        frame.efficacyMode = m_efficacyMode;
        switch (m_efficacyMode) {
            case 7: // efficacy 7: a fixed colour over everything
                // colour components of the fixed fill
                frame.fill = core::Color{250, 125, 125, 125};
                break;
            case 8: // efficacy 8: another fixed colour over everything
                // colour components of the fixed fill
                frame.fill = core::Color{125, 125, 125, 175};
                break;
            case 9: // efficacy 9: the step's own fill colour
                if (m_stepIndex <= m_script.stepCount) {
                    const formats::MovieStepEffect& effect = effectOf(m_stepIndex);
                    frame.fill = core::Color{
                        static_cast<std::uint8_t>(effect.fillRed),
                        static_cast<std::uint8_t>(effect.fillGreen),
                        static_cast<std::uint8_t>(effect.fillBlue),
                        static_cast<std::uint8_t>(effect.fillAlpha)
                    };
                }
                break;
            default: // other efficacy modes: no fill
                break;
        }
        return frame;
    }

}
