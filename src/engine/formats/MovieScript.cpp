#include "formats/MovieScript.h"

#include "core/ByteReader.h"

namespace formats {

    namespace {

        constexpr std::size_t kNameSize = 14; // bytes of a name field: 1 length byte + 13 characters
        constexpr int kMostPathCount = 255; // largest stored path point count (the original arrays hold 256 points)
        constexpr int kMostSteps = 805; // most steps in a movie script (the original's array size)

        /// @brief Reads a Pascal string[13] stored in 14 bytes.
        /// @param reader Source positioned at the string.
        /// @return The text.
        std::string readName(core::ByteReader& reader) {
            const std::size_t length = reader.u8();
            const std::string text = reader.text(kNameSize - 1);
            return text.substr(0, length < kNameSize ? length : kNameSize - 1);
        }

        /// @brief Reads one role path point.
        /// @details Original: Tft_RolePathElem, 45 bytes.
        /// @param reader Source positioned at the record.
        /// @return The step.
        MovieRoleStep readRoleStep(core::ByteReader& reader) {
            MovieRoleStep step;
            step.fixedDir = reader.i32();
            step.x = reader.i32();
            step.y = reader.i32();
            reader.skip(1);
            step.moveSpeed = reader.i32();
            step.action = reader.i32();
            reader.skip(4); // 4 unread bytes between action and frame
            step.frame = reader.i32();
            step.pathMode = reader.i32();
            step.soundNo = reader.i32();
            step.soundLoop = reader.u8() != 0;
            // 4 bytes sound handle + 3 bytes padding
            reader.skip(4 + 3);
            return step;
        }

        /// @brief Reads one picture path point.
        /// @details Original: Tft_PictureStep, 33 bytes.
        /// @param reader Source positioned at the record.
        /// @return The step.
        MoviePictureStep readPictureStep(core::ByteReader& reader) {
            MoviePictureStep step;
            step.targetX = reader.i32();
            step.targetY = reader.i32();
            reader.skip(1);
            step.moveMode = reader.i32();
            step.frameTick = reader.i32();
            step.soundNo = reader.i32();
            step.soundLoop = reader.u8() != 0;
            reader.skip(4 + 7); // 4 bytes sound handle + 7 bytes padding
            return step;
        }

        /// @brief Reads one ground path point.
        /// @details Original: Tft_GroundStep, 33 bytes.
        /// @param reader Source positioned at the record.
        /// @return The step.
        MovieGroundStep readGroundStep(core::ByteReader& reader) {
            MovieGroundStep step;
            step.targetX = reader.i32();
            step.targetY = reader.i32();
            reader.skip(1);
            step.scrollMode = reader.i32();
            reader.skip(20); // 20 unread trailing bytes of the 33-byte record
            return step;
        }

        /// @brief Reads a path's first step and point count.
        /// @param reader Source positioned at the counts.
        /// @param start Receives the first step.
        /// @param count Receives the point count minus one.
        /// @return False when the original's 256-point array could not hold the path.
        bool readPathCount(core::ByteReader& reader, int& start, int& count) {
            start = reader.i32();
            count = reader.i32();
            return !reader.failed() && count >= 0 && count <= kMostPathCount;
        }

        /// @brief Reads one role.
        /// @param reader Source positioned at the role.
        /// @param hasSortOffset Whether the role ends with a sort offset (the main role has none).
        /// @param role Receives the role.
        /// @return False when the role is invalid or cut short.
        bool readRole(core::ByteReader& reader, bool hasSortOffset, MovieRole& role) {
            role.surface = reader.i32();
            if (!readPathCount(reader, role.pathStart, role.pathCount)) {
                return false;
            }
            role.direction = reader.i32();
            for (int i = 0; i <= role.pathCount; ++i) {
                role.path.push_back(readRoleStep(reader));
            }
            if (hasSortOffset) {
                role.sortOffsetY = reader.i32();
            }
            return !reader.failed();
        }

        /// @brief Reads one moving picture.
        /// @param reader Source positioned at the picture.
        /// @param picture Receives the picture.
        /// @return False when the picture is invalid or cut short.
        bool readPicture(core::ByteReader& reader, MoviePicture& picture) {
            picture.name = readName(reader);
            if (!readPathCount(reader, picture.pathStart, picture.pathCount)) {
                return false;
            }
            picture.transparentColor = reader.i32();
            picture.alpha = reader.i32();
            picture.frameRows = reader.i32();
            picture.frameCols = reader.i32();
            picture.interval = reader.i32();
            picture.loops = reader.i32();
            for (int i = 0; i <= picture.pathCount; ++i) {
                picture.path.push_back(readPictureStep(reader));
            }
            picture.sortOffsetY = reader.i32();
            return !reader.failed();
        }

        /// @brief Reads the movie's ground.
        /// @param reader Source positioned at the ground.
        /// @param ground Receives the ground.
        /// @return False when the ground is invalid or cut short.
        bool readGround(core::ByteReader& reader, MovieGround& ground) {
            ground.name = readName(reader);
            if (!readPathCount(reader, ground.pathStart, ground.pathCount)) {
                return false;
            }
            for (int i = 0; i <= ground.pathCount; ++i) {
                ground.path.push_back(readGroundStep(reader));
            }
            return !reader.failed();
        }

        /// @brief Reads one step's screen effect.
        /// @details Original: Tft_Efficacy, 11 bytes.
        /// @param reader Source positioned at the record.
        /// @return The effect.
        MovieStepEffect readEffect(core::ByteReader& reader) {
            MovieStepEffect effect;
            effect.fadeKind = reader.u8();
            effect.filter1 = reader.u8();
            effect.filter2 = reader.u8();
            effect.musicId = reader.i32();
            effect.fillAlpha = reader.u8();
            effect.fillRed = reader.u8();
            effect.fillGreen = reader.u8();
            effect.fillBlue = reader.u8();
            return effect;
        }

    }

    std::optional<MovieScript> MovieScript::parse(const core::Bytes& data) {
        core::ByteReader reader(data);
        reader.skip(1);
        const int mainRoles = reader.i32();
        const int roles = reader.i32();
        const int pictures = reader.i32();
        const int grounds = reader.i32();
        const int talks = reader.i32();
        MovieScript script;
        script.stepCount = reader.i32();
        // at most 1 main role; exactly 1 ground per script
        if (reader.failed() || mainRoles < 0 || mainRoles > 1 || roles < 0 || roles > kMostRoles || pictures < 0 || pictures > kMostPictures || grounds != 1 ||
            talks < 0 || talks > kMostTalks || script.stepCount < 0 || script.stepCount > kMostSteps) {
            return std::nullopt;
        }
        if (mainRoles > 0) {
            MovieRole role;
            if (!readRole(reader, false, role)) {
                return std::nullopt;
            }
            script.mainRole = std::move(role);
        }
        for (int i = 0; i < roles; ++i) {
            MovieRole role;
            if (!readRole(reader, true, role)) {
                return std::nullopt;
            }
            script.roles.push_back(std::move(role));
        }
        for (int i = 0; i < pictures; ++i) {
            MoviePicture picture;
            if (!readPicture(reader, picture)) {
                return std::nullopt;
            }
            script.pictures.push_back(std::move(picture));
        }
        if (!readGround(reader, script.ground)) {
            return std::nullopt;
        }
        if (script.stepCount > 0) {
            for (int i = 0; i <= script.stepCount; ++i) {
                script.stepMillis.push_back(reader.i32());
            }
            for (int i = 0; i <= script.stepCount; ++i) {
                script.effects.push_back(readEffect(reader));
            }
        }
        if (talks > 0) {
            for (int i = 0; i <= talks; ++i) {
                MovieTalk talk;
                talk.speaker = reader.i32();
                talk.textId = reader.i32();
                talk.step = reader.i32();
                talk.showTime = reader.i32();
                script.talks.push_back(talk);
            }
        }
        if (reader.failed()) {
            return std::nullopt;
        }
        return script;
    }

}
