#ifndef NORTH_ISLAND_FORMATS_MOTION_RECORD_H
#define NORTH_ISLAND_FORMATS_MOTION_RECORD_H

#include "core/Point.h"
#include "formats/MotionCameraStep.h"
#include "formats/MotionFrame.h"
#include "formats/MotionLayer.h"
#include "formats/MotionScreenEffect.h"

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace formats {

    /// @brief How a skill (or a hit taken) plays on the battle field.
    /// @details Original: CH_TMotionInfo, one record of data\SkillData.MBTM; the fighter's steps along a path, the hits it deals,
    /// the effect pictures, sounds, the camera's path and the ground's.
    struct MotionRecord {
        int id = 0;
        int fileWord = 0;
        int fileByte = 0;
        int moveKind1 = 0;
        int moveKind2 = 0;
        std::int32_t fileInt = 0;
        std::int32_t jumpScale1 = 0;
        std::int32_t jumpScale2 = 0;
        int fileByte2 = 0;
        int hitMotionId = 0;
        int hitTime = 0;
        int hitMode = 0;
        std::vector<std::pair<int, int>> hits;
        std::vector<MotionFrame> frames;
        int pathPointCount = 0;
        std::vector<core::Point> pathPoints;
        std::vector<MotionLayer> layers;
        std::vector<int> drawOrder;
        std::vector<std::array<std::uint8_t, 7>> sounds;
        int cameraMode = 0;
        std::vector<MotionCameraStep> camera;
        int groundMode = 0;
        std::vector<core::Point> groundPoints;
        std::vector<std::pair<int, int>> groundSegments;
        std::vector<MotionScreenEffect> screenEffects;
        std::array<std::array<int, 6>, 6> weaponSounds{};
        std::array<int, 6> weaponSoundDelays{};
        std::array<int, 6> fileBytes{};
        std::array<int, 6> usableWeapons{};
        std::array<std::array<int, 6>, 5> bodySounds{};
        std::array<int, 6> bodySoundDelays{};
        std::array<int, 6> fileBytes2{};
    };

}

#endif
